#ifndef LOCK_H_
#define LOCK_H_

#include <pthread.h>

namespace sm {
  class Lock {
  public:
    Lock();
    ~Lock();
    void Acquire();
    void Release();

  private:
    pthread_mutex_t _lock;
  };
};
#endif
