#include "concurrent.h"
#include "log.h"

namespace sm {
  template <typename T>
  static void *
  thread_func_s (void *arg) {
    Thread<T> *_t = (Thread<T> *) arg;
    _t->thread_member();
    return NULL;
  }


  template <typename T>
  Thread<T>::Thread(): _stopping(false), _stopped(true), _done(false){

  }

  template <typename T>
  Thread<T>::~Thread(){
    stop();
  }

  template <typename T> int
  Thread<T>::addJob(T job, work_priority_t prior, bool autoDelete) {
    _internal_Job<T> _job ;
    _job.job = job;
    _job.priority = prior;
    _job.autoDelete = autoDelete;

    _jobLock.Acquire();
    _jobQueue.push (_job);
    _jobCond.Signal();
    _jobLock.Release();
  }


  template <typename T>
  void
  Thread<T>::start(){
    int ret;
    ret = pthread_create (&_td, NULL, thread_func_s<T>, this);
    if (ret != 0) {
      SM_LOG_FATAL ("pthread create error");
      return;
    }

    _stopped = false;
  }

  template <typename T>
  size_t
  Thread<T>::size(){
    size_t ret;
    _jobLock.Acquire();
    ret = _jobQueue.size();
    _jobLock.Release();
    return ret;
  }


  template <typename T>
  void
  Thread<T>::stop(){
    if (!_stopping) {
      _cmdLock.Acquire();
      _stopping = true;
      _cmdLock.Release();
  
      _jobLock.Acquire();
      _jobCond.Signal();
      _jobLock.Release();

      pthread_join(_td, NULL);
    }
  }

  template <typename T>
  void
  Thread<T>::waitAllJobDone(){
    _emptyLock.Acquire();
    while (!_done) _emptyCond.Wait(_emptyLock, 3000);
    _emptyLock.Release();
    stop();
  }



  template <typename T>
  void
  Thread<T>::thread_member(){
    int    total_time;

    SM_LOG_DEBUG ("thread started");
    while (1) {
      //check father tell me to stopped
      _cmdLock.Acquire();
      if (_stopping) {
        _stopping = false;
        _stopped = true;
        _cmdLock.Release();
        break;
      }
  
      _jobLock.Acquire();
      _cmdLock.Release();

      _internal_Job<T> j;
      bool has_newjob = false;
      if (!_jobQueue.empty()) {
        j = _jobQueue.top();
        _jobQueue.pop();
        has_newjob = true;
        _jobLock.Release();
      } else {
        _emptyLock.Acquire();
        _emptyCond.Signal();
        _done = true;
        _emptyLock.Release();

        _jobCond.Wait(_jobLock, 3000);
        _jobLock.Release();
      }

      if (has_newjob) {
        //SM_LOG_TRACE ("thread %d doing job %lld", _id, j.jobid);
        doJob (j.job);
        if (j.autoDelete) {
          delete j.job;
        }
      }
    }

    SM_LOG_NOTICE ("thread statistics [JobDone : %d]");
  }

}
