#include <inttypes.h>
#include <string>
#include <json/json.h>
#include "redis_wrapper.h"


using namespace std;
using namespace baidu::mco;
using namespace sm;

SM_IMP_SINGLETON(RedisEngineManager);

RedisEngineManager::~RedisEngineManager(){
  delete _engine;
}

RedisEngine &
RedisEngineManager::getEngine(){
  return *_engine;
}

int 
RedisEngineManager::init(const string &path, const std::string &conf) {
  _engine = newKSarchRedisEngine();
  if (_engine->init(path.c_str(), conf.c_str()) == 0){
    return -1;
  }
  return 0;
}

