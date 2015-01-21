#include "configurable.h"

using namespace sm;
using namespace std;

SM_IMP_SINGLETON(Configurable)


Configurable::~Configurable(){}

int
Configurable::load (const string &path, const string &conf, const string &range) {
  return _conf.load(path.c_str(), conf.c_str(), range.c_str());
}
