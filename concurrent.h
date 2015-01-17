#ifndef CONCURRENT_H_
#define CONCURRENT_H_

#include <queue>
#include <pthread.h>
#include <stdint.h>

namespace sm {
  
  enum work_priority_t {
    WORK_EMERGENCY =  5,
    WORK_HIGH = 4,
    WORK_NORMAL = 3,
    WORK_LOW = 2,
    WORK_DUMMPY = 1
  };

  class Lock {
    friend class Cond;
  public:
    Lock();
    ~Lock();
    void Acquire();
    void Release();

  private:
    pthread_mutex_t _lock;
  };

  class RWLock {
  public:
    RWLock();
    ~RWLock();
    
    void AcquireRead();
    void AcquireWrite();
    void Release();
  private:
    pthread_rwlock_t _lock;
  };


  class Cond {
  public:
    Cond();
    ~Cond();
    
    void Wait(Lock &lock, int64_t ms=-1);
    void Signal();
  private:
    pthread_cond_t _cond;
  };

  template <typename T>
  class _internal_Job {
  public:
    work_priority_t priority;
    T job;
    inline bool operator <(const _internal_Job &rhs) const {
      return priority < rhs.priority;
    }
  };


  template <typename T>
    class Thread {
  public:
    Thread ();
    ~Thread ();

    void start ();
    void stop ();
    size_t size();
    int addJob (T job, work_priority_t prior = WORK_NORMAL);

    virtual int doJob(const T& ) = 0;

    virtual void waitAllJobDone();
    void thread_member();
  private:
    bool _stopping, _stopped, _done, _wait_done;
    pthread_t _td;
    std::priority_queue< _internal_Job <T> > _jobQueue;
    Lock _cmdLock, _jobLock, _emptyLock;
    Cond _jobCond, _emptyCond;

  };

};

#include "concurrent.hpp"

#endif
