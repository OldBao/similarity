#include "lock.h"

using namespace sm;

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
