#include <inttypes.h>
#include "json/json.h"
#include "sim_server.h"
#include "log.h"

using namespace sm;
using namespace std;

static void bow_to_json(const bow_t &bow, string *ret);

#define DEF_ERROR(code, message)                         \
  const static int ERROR_CODE_##message = -code;          \
  const static char *ERROR_MSG_##message = #message;   \
  const static char *ERROR_JSON_##message = "{ \"error_code\" : -" #code ", \"error_message\" : \"" #message " \"}";

DEF_ERROR(1, FORMAT)
DEF_ERROR(2, DOCID)
DEF_ERROR(3, FILTER)
DEF_ERROR(4, RESULT)
DEF_ERROR(5, THRESHOLD)


typedef ub::SmartEvent<SimServerEvent> SimServerEventPtr;

SimServer::SimServer(ub::NetReactor *reactor):
  ub::UbAServer (reactor), _server_data(NULL)
{
  
}

SimServer::~SimServer(){
  
}

int
SimServer::updateServerData(SimServerData *data) {
  _dataLock.AcquireWrite();
  if (_server_data == NULL) this->getready();
  _server_data = data;
  _dataLock.Release();

  SM_LOG_NOTICE ("data has upgraded to version : %" PRIu64, data->getVersion());
  return 0;
}

int 
SimServer::getSimilarities (bow_t *bow, uint64_t docid, float threshold, int max_result) {
  int ret;
  _dataLock.AcquireRead();
  ret = _server_data->getSimilarity (bow, docid, threshold, max_result);
  _dataLock.Release();
  return ret;
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
    SM_LOG_WARNING ("didn't get 'docid' key in [%s]", (char *)req_buf+headlen);
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
  int res_header_len, res_body_len = strlen(retstr);

  res_header_len = snprintf (resheader, 1024, 
                             "HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n"
                             "Content-Length: %zu\r\n\r\n", strlen (retstr));

  void *res_buf = this->get_write_buffer(res_header_len+res_body_len);

  SM_LOG_DEBUG ("max rd bufsize %d max write buf size %d", 
                    server->get_max_readbufsize(), server->get_max_writebufsize());

  memcpy (res_buf, resheader, res_header_len);
  memcpy (res_buf+res_header_len, retstr, res_body_len);
  SM_LOG_DEBUG ("retstr :%d-%d: %s", res_header_len, res_body_len, (char *)res_buf);
  //TODO free memory ;-)
  
  awrite(res_header_len + res_body_len);
}

void
SimServerEvent::event_error_callback (){
  UBEVENT_TRACE(this,"SimServerEvent(read_done_callback) event error(0x%X)[%s]", this->get_sock_status(),
                status_to_string(get_sock_status()).c_str());
  _fserver->session_done(this);
  return;
}


SM_IMP_SINGLETON (SimServerDataManager)

SimServerDataManager::~SimServerDataManager(){
}

int
SimServerDataManager::init (const string& basepath) {
  _basepath = basepath;
  _local_version = 0; //TODO check dir

  return 0;
}


int
SimServerDataManager::checkVersion(){
  uint64_t remote_version = 0; // TODO get version from remote

  SM_LOG_NOTICE ("check trainer version %" PRIu64 " : localversion is %" PRIu64
                 , remote_version, _local_version);

  if (remote_version >= _local_version) { //TODO change this to >
    //TODO sync remote version to local
    if (_datas.find(_local_version) != _datas.end())
      return 0;


    SimServerData *data = new SimServerData(remote_version);
    if (0 != data->load (_basepath)){
      delete data;
      return -1;
    }
    
    _datas[remote_version] = data;
    for (std::vector <SimServer *>::iterator iter = _servers.begin();
         iter != _servers.end();
         iter++)
      {
        (*iter)->updateServerData(data);
      }
    return remote_version+1;
  }

  return 0;
}


int
SimServerDataManager::registerSimServer(SimServer *server) {
  _servers.push_back (server);
  return 0;
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


uint64_t
SimServerData::getVersion(){
  return _version;
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
