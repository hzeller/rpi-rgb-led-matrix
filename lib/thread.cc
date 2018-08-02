// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2013 Henner Zeller <h.zeller@acm.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#include "thread.h"

#include <sched.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

namespace rgb_matrix {
void *Thread::PthreadCallRun(void *tobject) {
  reinterpret_cast<Thread*>(tobject)->Run();
  return NULL;
}

Thread::Thread() : started_(false) {}
Thread::~Thread() {
  WaitStopped();
}

void Thread::WaitStopped() {
  if (!started_) return;
  int result = pthread_join(thread_, NULL);
  if (result != 0) {
    perror("Issue joining thread");
  }
  started_ = false;
}

void Thread::Start(int priority, uint32_t affinity_mask) {
  assert(!started_);  // Did you call WaitStopped() ?
  pthread_create(&thread_, NULL, &PthreadCallRun, this);
  int err;

  if (priority > 0) {
    struct sched_param p;
    p.sched_priority = priority;
    if ((err = pthread_setschedparam(thread_, SCHED_FIFO, &p))) {
      fprintf(stderr, "FYI: Can't set realtime thread priority=%d %s\n",
              priority, strerror(err));
    }
  }

  if (affinity_mask != 0) {
    cpu_set_t cpu_mask;
    CPU_ZERO(&cpu_mask);
    for (int i = 0; i < 32; ++i) {
      if ((affinity_mask & (1<<i)) != 0) {
        CPU_SET(i, &cpu_mask);
      }
    }
    if ((err=pthread_setaffinity_np(thread_, sizeof(cpu_mask), &cpu_mask))) {
      //fprintf(stderr, "FYI: Couldn't set affinity 0x%x: %s\n",
      //        affinity_mask, strerror(err));
    }
  }

  started_ = true;
}

bool Mutex::WaitOn(pthread_cond_t *cond, long timeout_ms) {
  if (timeout_ms < 0) {
    pthread_cond_wait(cond, &mutex_);
    return true;
  }
  else {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    t.tv_sec += timeout_ms / 1000;
    t.tv_nsec += (timeout_ms % 1000) * 1000000;
    t.tv_sec += t.tv_nsec / 1000000000;
    t.tv_nsec %= 1000000000;
    // TODO(hzeller): It doesn't seem we return with EINTR on signal. We should.
    return pthread_cond_timedwait(cond, &mutex_, &t) == 0;
  }
}
}  // namespace rgb_matrix
