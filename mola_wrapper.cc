#include <inttypes.h>
#include <string>
#include <json/json.h>
#include "mola_wrapper.h"


using namespace std;
using namespace baidu::mco;
using namespace sm;

SM_IMP_SINGLETON(MolaEngineManager);

MolaEngineManager::~MolaEngineManager(){

}

MolaEngine &
MolaEngineManager::getEngine(){
  return _engine;
}

int 
MolaEngineManager::init(const string &path, const std::string &conf) {
  int ret = _engine.init(path.c_str(), conf.c_str(), NULL) >= 0;
  if (ret < 0) return -1;
  return 0;
}


int sm::getUrlFromDocid (uint64_t docid, string* url){
  SM_ASSERT (url && url->empty(), "url should be empty");

  MolaEngine &engine = MolaEngineManager::getInstance()->getEngine();

  Slice s((char *)&docid, sizeof (docid));
  Slice raw;
  RetStatus status;

  if (engine.get(docid, raw, RssRawData, status, "") <= 0) {
    return -1;
  }

  switch (status.m_retcode_) {
  case retFail:
    SM_LOG_WARNING ("docid [% " PRIu64 "] fails", docid);
    return -1;
  case retNoHit:
    SM_LOG_WARNING ("docid [% " PRIu64 "]not hit", docid);
    return -1;
  case retThroughputExceed:
    SM_LOG_WARNING ("Get url from docid %" PRIu64 " fails", docid);
    return -1;
  }

  
  SM_CHECK_RET_ERR (status.m_retcode_ == retSuccess, "unknown status code");
  Json::Reader reader;
  Json::Value value;
  if (!reader.parse(raw.data(), value)) {
    SM_LOG_WARNING ("parse json of docid %" PRIu64 "fail", docid);
    return -1;
  }
  
  if (!value.isMember("link") || value["link"].isString()) {
    *url = value["link"].asString();
  } else {
    SM_LOG_WARNING ("'link' key has error");
    return -1;
  }
  
  return 0;

}
