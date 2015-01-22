#ifndef SM_REDIS_WRAPPER
#define SM_REDIS_WRAPPER

#include <string.h>

#include "mco/RedisEngine.h"
#include "singleton.h"

namespace sm {
class RedisEngineManager {
  SM_DECLARE_SINGLETON(RedisEngineManager)
      
  int init(const std::string &path, const std::string &file);
  baidu::mco::RedisEngine& getEngine();
  
private:
  baidu::mco::RedisEngine *_engine;
};

};

#endif
