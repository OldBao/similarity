#include "concurrent.h"
using namespace sm;

RWLock::RWLock(){
  pthread_rwlock_init (&_lock, NULL);
}

RWLock::~RWLock(){
  pthread_rwlock_destroy (&_lock);
}

void
RWLock::AcquireRead (){
  pthread_rwlock_rdlock (&_lock);
}

void
RWLock::AcquireWrite (){
  pthread_rwlock_wrlock (&_lock);
}

void
RWLock::Release(){
  pthread_rwlock_unlock(&_lock);
}


Lock::Lock (){
  pthread_mutex_init (&_lock, NULL);
}

Lock::~Lock (){
  pthread_mutex_destroy (&_lock);
}

void
Lock::Acquire(){
  pthread_mutex_lock (&_lock);
}

void
Lock::Release(){
  pthread_mutex_unlock (&_lock);
}


Cond::Cond(){
  pthread_cond_init(&_cond, NULL);
}


Cond::~Cond(){
  pthread_cond_destroy(&_cond);
}

void
Cond::Wait(Lock &lock, int64_t ms) {
  if (ms != -1)  {
    struct timespec timeout;
    timeout.tv_sec  = (ms/1000);
    timeout.tv_nsec = 0;
    
    pthread_cond_timedwait (&_cond, &lock._lock, &timeout);
  }
  else
    pthread_cond_wait (&_cond, &lock._lock);
}

void
Cond::Signal(){
  pthread_cond_signal(&_cond);
}


