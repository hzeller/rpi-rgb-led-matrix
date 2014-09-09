// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
#ifndef RPI_THREAD_H
#define RPI_THREAD_H

#include <pthread.h>

namespace rgb_matrix {
// Simple thread abstraction.
class Thread {
public:
  Thread();

  // The destructor waits for Run() to return so make sure it does.
  virtual ~Thread();
  
  // Wait for the Run() method to return.
  void WaitStopped();

  // Start thread. If realtime_priority is > 0, then this will be a
  // thread with SCHED_FIFO and the given priority.
  void Start(int realtime_priority = 0);

  // Override this.
  virtual void Run() = 0;

private:
  static void *PthreadCallRun(void *tobject);
  bool started_;
  pthread_t thread_;
};

// Non-recursive Mutex.
class Mutex {
public:
  Mutex() { pthread_mutex_init(&mutex_, NULL); }
  ~Mutex() { pthread_mutex_destroy(&mutex_); }
  void Lock() { pthread_mutex_lock(&mutex_); }
  void Unlock() { pthread_mutex_unlock(&mutex_); }
  void WaitOn(pthread_cond_t *cond) { pthread_cond_wait(cond, &mutex_); }

private:
  pthread_mutex_t mutex_;
};

// Useful RAII wrapper around mutex.
class MutexLock {
public:
  MutexLock(Mutex *m) : mutex_(m) { mutex_->Lock(); }
  ~MutexLock() { mutex_->Unlock(); }
private:
  Mutex *const mutex_;
};

}  // end namespace rgb_matrix

#endif  // RPI_THREAD_H
