#ifndef SM_MOLA_WRAPPER
#define SM_MOLA_WRAPPER

#include <string.h>

#include "mco/MolaEngine.h"
#include "singleton.h"

namespace sm {
class MolaEngineManager {
  SM_DECLARE_SINGLETON(MolaEngineManager)
      
  int init(const std::string &path, const std::string &file);
  baidu::mco::MolaEngine& getEngine();
  
private:
  baidu::mco::MolaEngine _engine;
};

extern int getUrlFromDocid (uint64_t docid, std::string* url);
};

#endif
