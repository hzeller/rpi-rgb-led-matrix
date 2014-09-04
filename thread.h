// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
#ifndef RPI_THREAD_H
#define RPI_THREAD_H
#include <pthread.h>

// Simple thread abstraction.
class Thread {
public:
  Thread();

  // The destructor waits for Run() to return so make sure it does.
  virtual ~Thread();
  
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

#endif  // RPI_THREAD_H
