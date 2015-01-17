#include <inttypes.h>
#include "json/json.h"
#include "sim_server.h"
#include "log.h"

using namespace sm;
using namespace std;
typedef ub::SmartEvent<SimServerEvent> SimServerEventPtr;

SimServer::SimServer(ub::NetReactor *reactor):
  ub::UbAServer (reactor), _server_data(NULL)
{
  
}

SimServer::~SimServer(){
  
}

int
SimServer::updateServerData(SimServerData *data) {
  _server_data = data;
}

int 
SimServer::getSimilarities (bow_t *bow, uint64_t docid, float threshold, int max_result) {
  return _server_data->getSimilarity (bow, docid, threshold, max_result);
}


void
SimServer::on_accept (ub::UbEvent *event) {
  SimServerEventPtr mev;
  session_begin(&mev);
}


void
SimServerEvent::http_read_header_done(){

}


void
SimServerEvent::http_read_section_done(){

}


void
SimServerEvent::session_begin (){
  http_package_read ();
}


void
SimServerEvent::read_done_callback (){
  if (isError()) {
    SM_LOG_WARNING ("http package error");
    return;
  }

  const void *req_buf = this->get_read_buffer();
  void *res_buf = this->get_write_buffer();
  int headlen = this->get_http_headlen();
  int bodylen = this->get_http_bodylen();
  int ret;

  json_tokener *tokener = json_tokener_new();
  json_object *obj = json_tokener_parse_ex (tokener, (char *)req_buf+headlen, bodylen);
  const char *docidstr = NULL;
  uint64_t docid = -1;
  if (!obj) {
    SM_LOG_WARNING("error json format");
    return;
  }

  json_object_object_foreach (obj, key, value) {
    if (!strcmp ("docid", key)) {
      docidstr = json_object_get_string(value);
    }
  }


  if (!docidstr) {
    SM_LOG_WARNING ("didn't get 'docid' key in [%s]", req_buf+headlen);
    return;
  }

  ret = sscanf (docidstr, "%" PRIu64, &docid);
  if (1 != ret) {
    SM_LOG_WARNING ("docid not uint64");
    return;
  }
  
  bow_t bow;
  SimServer *server = (SimServer *) _fserver;
  server->getSimilarities (&bow, docid, 0.0, 5);
  SM_LOG_NOTICE ("getted %zu similar docs", bow.size());


  json_object *retobj = json_object_new_object();

  json_object *simarr = json_object_new_array ();

  for (size_t i = 0; i < bow.size(); i++) {
    json_object *simobj = json_object_new_object();
    json_object *idobj = json_object_new_int(bow[i].id);
    json_object *weightobj = json_object_new_double(bow[i].weight);
    json_object_object_add (simobj, "docid", idobj);
    json_object_object_add (simobj, "similarity", weightobj);

    json_object_array_put_idx (simarr, i, simobj);
  }
  json_object_object_add (retobj, "similarities", simarr);

  const char *retstr = json_object_get_string (retobj);
  char resheader[1024];

  snprintf (resheader, 1024, 
            "HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n"
            "Content-Length: %zu\r\n\r\n", strlen (retstr));

  memcpy (res_buf, resheader, strlen(resheader));
  memcpy (res_buf, retstr, strlen(retstr));
  int writelen = strlen(resheader) + strlen(retstr);
  //TODO free memory ;-)

  awrite(writelen);
}

void
SimServerEvent::event_error_callback (){
  SM_LOG_WARNING ("read error [%d]", this->get_sock_status());

  _fserver->session_done(this);
  return;
}


SM_IMP_SINGLETON (SimServerDataManager)

SimServerDataManager::~SimServerDataManager(){

}

int
SimServerDataManager::init (const string& basepath) {

  //TODO get newest version
  SimServerData *data = new SimServerData(0);
  
  if (0 != data->load (basepath)){
    delete data;
    return -1;
  }

  _datas[0] = data;
  return 0;
}

int
SimServerDataManager::registerSimServer(SimServer *server) {
  return server->updateServerData (_datas[0]);
}


SimServerData::SimServerData(uint64_t version):
  _version (version) 
{

}

SimServerData::~SimServerData(){

}


int
SimServerData::getSimilarity(bow_t *bow, uint64_t docid, float threshold, int max){
  int id = _corpus->getIdFromDocid (docid);
  return _sim->getSimilarities(bow, id, threshold, max);
}

int
SimServerData::load(const string &path) {
  int ret;
  _dict = new Dictionary(_version);
  if ( 0 != _dict->load(path, "similarity")){
    SM_LOG_DEBUG ("load dict error");
    return -1;
  }
  
  _corpus = new Corpus();
  if ( 0 != _corpus->load(path, "similarity")){
    SM_LOG_DEBUG ( "load corpus error");
    return -1;
  }

  _model = new LDAModel(_corpus, _dict);
  if ( 0 != _model->load (path, "similarity")){
    SM_LOG_DEBUG( "load lda error");
    return -1;
  }

  _sim = new TopicSimilarity (_model, _corpus, _dict, 30);

  SM_LOG_NOTICE ("new version %" PRIu64 " Begin training", _version);
  for (int i = 0; i <= _model->getNTopics(); i++) {
    _sim->calculate (i);
  }

  _sim->waitAllJobDone();

  SM_LOG_NOTICE ("new version %" PRIu64 "  training down", _version);


#include <iostream>
  for (int i = 0; i < _corpus->size(); i++) {
    bow_t retbow;
    cout << "Sim of [ " << _corpus->getDocid(i)  << ":";
    _sim->getSimilarities(&retbow, i, 0.0, 5);
    cout << retbow.size() << "]\t";

    for (int j = 0; j < retbow.size(); j++) {
      cout << "[" << _corpus->getDocid(retbow[j].id) << ":" << retbow[j].weight << "]";
    }
    cout << endl;
  }

  return 0;

}
