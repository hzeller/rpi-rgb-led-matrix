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
#include <string.h>
#include <assert.h>

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
    fprintf(stderr, "err code: %d %s\n", result, strerror(result));
  }
  started_ = false;
}

void Thread::Start(int priority) {
  assert(!started_);
  pthread_create(&thread_, NULL, &PthreadCallRun, this);

  if (priority > 0) {
    struct sched_param p;
    p.sched_priority = priority;
    pthread_setschedparam(thread_, SCHED_FIFO, &p);
  }

  started_ = true;
}

}  // namespace rgb_matrix
