//
// barrier.c
//
// POSIX barriers
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#include <os.h>
#include <pthread.h>
#include <atomic.h>

int pthread_barrierattr_init(pthread_barrierattr_t *attr) {
  if (!attr) return EINVAL;
  attr->pshared = PTHREAD_PROCESS_PRIVATE;
  return 0;
}

int pthread_barrierattr_destroy(pthread_barrierattr_t *attr) {
  return 0;
}

int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr, int *pshared) {
  if (!attr || !pshared) return EINVAL;
  *pshared = attr->pshared;
  return 0;
}

int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared) {
  if (!attr) return EINVAL;
  if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED) return EINVAL;
  attr->pshared = pshared;
  return 0;
}

int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count) {
  if (!barrier || count == 0) return EINVAL;
  if (attr && attr->pshared == PTHREAD_PROCESS_SHARED) return ENOSYS;

  barrier->curr_height = barrier->init_height = count;
  barrier->step = 0;

  barrier->breeched[0] = mksem(0);
  if (barrier->breeched[0] < 0) return errno;

  barrier->breeched[1] = mksem(0);
  if (barrier->breeched[1] < 0) {
    int rc = errno;
    close(barrier->breeched[0]);
    return rc;
  }

  return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier) {
  if (!barrier) return EINVAL;
  if (close(barrier->breeched[0]) < 0) return errno;
  if (close(barrier->breeched[1]) < 0) return errno;
  return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier) {
  int step = barrier->step;
  int rc;

  if (atomic_decrement(&barrier->curr_height) == 0) {
    barrier->curr_height = barrier->init_height;
    if (barrier->init_height > 1) {
      rc = semrel(barrier->breeched[step], barrier->init_height - 1);
    } else {
      rc = 0;
    }
  } else {
    rc = waitone(barrier->breeched[step], INFINITE);
  }

  if (rc == 0) {
    if (atomic_compare_and_exchange(&barrier->step, 1 - step, step) == step) {
      rc = PTHREAD_BARRIER_SERIAL_THREAD;
    } else {
      rc = 0;
    }
  }

  return rc;
}
