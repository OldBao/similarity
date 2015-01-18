#include <inttypes.h>
#include "json/json.h"
#include "sim_server.h"
#include "log.h"

using namespace sm;
using namespace std;

/*
const static int ERROR_CODE_##message = -code;          \
const static char *ERROR_MSG_##message = #message;   \
*/
#define DEF_ERROR(code, message)                         \
  const static char *ERROR_JSON_##message = "{ \"error_code\" : -" #code ", \"error_message\" : \"" #message " \"}";

DEF_ERROR(1, FORMAT)
DEF_ERROR(2, DOCID_NOT_SET)
DEF_ERROR(3, DOCID)
DEF_ERROR(4, RESULT)
DEF_ERROR(5, THRESHOLD)
DEF_ERROR(10, INTERNAL)

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
SimServer::getSimilarities (sim_t *sims, uint64_t docid, float threshold, int max_result) {
  int ret;
  _dataLock.AcquireRead();
  ret = _server_data->getSimilarity (sims, docid, threshold, max_result);
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
  sim_t sims;
  SimServer *server = (SimServer *) _fserver;
  uint64_t docid;
  float threshold = server->getDefaultThreshold();
  int max_result = server->getDefaultMaxResult();
  string ret;
  char resheader[1024];
  
  string request((char *)req_buf+headlen, bodylen);
  if (0 == _get_request(request, &docid, &threshold, &max_result, &ret)) {
    if (0 != server->getSimilarities (&sims, docid, threshold, max_result)){
      SM_LOG_NOTICE("get similarity fail [%" PRIu64 "]", docid );
      ret = ERROR_JSON_INTERNAL;
    } else {
      _sims_to_json(sims, &ret);
    }
  }

  int res_header_len;

  res_header_len = snprintf (resheader, 1024, 
                             "HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n"
                             "Content-Length: %zu\r\n\r\n", ret.size());

  void *res_buf = this->get_write_buffer(res_header_len+ret.size());

  memcpy (res_buf, resheader, res_header_len);
  memcpy ((char *)res_buf+res_header_len, ret.data(), ret.size());
  awrite(res_header_len + ret.size());
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
  for (map<uint64_t, SimServerData *>::iterator iter = _datas.begin();
       iter != _datas.end();
       iter++)
    {
      delete iter->second;
    }
    
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
  if (_dict) delete _dict;
  if (_corpus) delete _corpus;
  if (_model) delete _model;
}


int
SimServerData::getSimilarity(sim_t *sims, uint64_t docid, float threshold, int max){
  SM_ASSERT (sims && sims->size()==0, "ret can't be empty");
  bow_t bow;
  int id = _corpus->getIdFromDocid (docid);
  if ( 0 == _sim->getSimilarities(&bow, id, threshold, max) ){
    for (size_t i = 0; i < bow.size(); i++) {
      sim_unit_t u;
      u.docid = _corpus->getDocid(bow[i].id);
      u.sim = bow[i].weight;
      sims->push_back(u);
    }
    return 0;
  }

  return -1;
}


uint64_t
SimServerData::getVersion(){
  return _version;
}


int
SimServerData::load(const string &path) {
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
  for (size_t i = 0; i < _corpus->size(); i++) {
    bow_t retbow;
    cout << "Sim of [ " << _corpus->getDocid(i)  << ":";
    _sim->getSimilarities(&retbow, i, 0.0, 5);
    cout << retbow.size() << "]\t";

    for (size_t j = 0; j < retbow.size(); j++) {
      cout << "[" << _corpus->getDocid(retbow[j].id) << ":" << retbow[j].weight << "]";
    }
    cout << endl;
  }

  return 0;
}


int 
SimServerEvent::_get_request(const string &request, 
                             uint64_t* docid, float* threshold, int* max_result, string *err) {
  Json::Reader reader;
  Json::Value value;
  if (!reader.parse(request, value, false)) {
    (*err) = ERROR_JSON_FORMAT;
    return -1;
  }

  if (!value.isMember ("docid") ) {
    (*err) = ERROR_JSON_DOCID_NOT_SET;
    return -1;
  }
  
  if (!value["docid"].isNumeric()) {
    (*err) = ERROR_JSON_DOCID;
    return -1;
  }

  *docid = value["docid"].asUInt64();

  if (value.isMember ("filter")) {
    if (value["filter"].isMember("threshold")) {
      if (!value["filter"]["threshold"].isNumeric()) {
        (*err) = ERROR_JSON_THRESHOLD;
        return -1;
      } else {
        *threshold = value["filter"]["threshold"].asFloat();
      }
    }

    if (value["filter"].isMember("max_result")) {
      if (!value["filter"]["max_result"].isInt()) {
        (*err) = ERROR_JSON_RESULT;
        return -1;
      } else {
        *threshold = value["filter"]["max_result"].asInt();
      }
    }
  }

  SM_LOG_NOTICE ("[docid:%" PRIu64 " threshold: %lf max_result:%d",
                 *docid, *threshold, *max_result);
  return 0;
}


void 
SimServerEvent::_sims_to_json(const sim_t &sims, string *strret) {
  Json::Value ret(Json::objectValue), simarr(Json::arrayValue);
  ret["error_code"] = 0;
  ret["error_message"] = "ok";

   for (size_t i = 0; i < sims.size(); i++) {
     Json::Value sim(Json::objectValue);

     Json::Value simv(sims[i].sim);
     std::stringstream ss;
     ss << sims[i].docid;
     Json::Value docidv(ss.str());

     sim["docid"] = docidv;
     sim["similarity"] = simv;
     simarr.append(sim);
  }
   ret["similarities"] = simarr;

   Json::FastWriter writer;
   *strret = writer.write(ret);
}
#undef DEF_ERROR