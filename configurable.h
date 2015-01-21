#ifndef SM_CONFIGURABLE_H_
#define SM_CONFIGURABLE_H_

#include <string>
#include <sstream>
#include <Configure.h>
#include "singleton.h"

namespace sm {
  
#define SM_CONFIG_BEGIN(section) {                                      \
    try {                                                               \
      const comcfg::ConfigUnit &u = Configurable::getInstance()->getConf()[#section]; \

  
#define SM_CONFIG_PROP(p, t, d)             {                     \
    u[#p].get_##t(&this->_##p, d);                                \
    std::stringstream ss;                                        \
    ss << "loaded key [" << #p << "] with value " << _##p;       \
    SM_LOG_NOTICE ("%s", ss.str().c_str());                    \
  }                                                              \

#define SM_CONFIG_PROP_STR(p, d) {              \
    bsl::string fuckbsl, bslshit(d);            \
    u[#p].get_bsl_string(&fuckbsl, bslshit);    \
    this->_##p = fuckbsl.c_str();                                     \
    SM_LOG_NOTICE("loaded key [" #p "] with value %s", _##p.c_str()); \
}

#define SM_CONFIG_END                                              \
  } catch (comcfg::ConfigException ex) {                           \
    SM_LOG_FATAL("load config error : %s", ex.what());             \
    abort();                                                       \
  }                                                                \
}


  class Configurable {
    SM_DECLARE_SINGLETON(Configurable);
    int load(const std::string& path, const std::string& conf, const std::string& range);

    const comcfg::Configure& getConf () { return _conf; }
  protected:
    comcfg::Configure _conf;
  };
};
#endif
