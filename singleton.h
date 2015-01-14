#ifndef SINGLETON_H_
#define SINGLETON_H_

#include "concurrent.h"

#define SM_DECLARE_SINGLETON(clz, para)           \
  private:                                        \
 virtual ~clz();                                  \
 clz();                                           \
 static clz *__i_nstance;                         \
 static Lock __l_ock;                             \
public:                                         \
static clz *getInstance();                     

#define SM_IMP_SINGLETON(clz) \
  clz* clz::__i_nstance = 0;   \
  Lock clz::__l_ock;           \
  clz::clz(){};                \
  clz *clz::getInstance(){     \
  if (!__i_nstance) {          \
  __l_ock.Acquire();           \
  if (!__i_nstance) {          \
    __i_nstance = new clz;     \
  }                            \
  }                            \
  return __i_nstance;          \
  }

#endif
