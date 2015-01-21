#ifndef CMDLINE_H_
#define CMDLINE_H_

#include <string>

namespace sm {
#ifndef VERSION
#define VERSION_ID "unknown"
#else
#define VERSION_ID VERSION
#endif

#ifndef VERSION_SCMPF
#define VERSION_DESC "debug version ;-)"
#else
#define VERSION_DESC VERSION_SCMPF
#endif
  class Cmdline {
  public:
    Cmdline (int argc, char **argv, const std::string &procname);
    ~Cmdline();
    const std::string& getVersion() {return _version;}
    const std::string& getDesc() {return _desc;}
  private:
    std::string _progname, _desc, _version;
  };
}
#endif
