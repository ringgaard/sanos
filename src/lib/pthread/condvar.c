//
// condvar.c
//
// POSIX condition variables
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

int pthread_condattr_init(pthread_condattr_t *attr) {
  if (!attr) return EINVAL;
  attr->pshared = PTHREAD_PROCESS_PRIVATE;
  return 0;
}

int pthread_condattr_destroy(pthread_condattr_t *attr) {
  return 0;
}

int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared) {
  if (!attr || !pshared) return EINVAL;
  *pshared = attr->pshared;
  return 0;
}

int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared) {
  if (!attr) return EINVAL;
  if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED) return EINVAL;
  attr->pshared = pshared;
  return 0;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
  if (!cond) return EINVAL;
  if (attr && attr->pshared == PTHREAD_PROCESS_SHARED) return ENOSYS;

  cond->waiting = 0;
  cond->semaphore = mksem(0);
  if (cond->semaphore < 0) return ENOSPC;
  return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond) {
  if (!cond) return EINVAL;
  if (close(cond->semaphore) < 0) return errno;
  return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
  int rc = 0;

  atomic_increment(&cond->waiting);
  pthread_mutex_unlock(mutex);
  if (waitone(cond->semaphore, INFINITE) < 0) rc = errno;
  atomic_decrement(&cond->waiting);
  pthread_mutex_lock(mutex);
  return errno;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime) {
  int rc = 0;

  atomic_increment(&cond->waiting);
  pthread_mutex_unlock(mutex);
  if (waitone(cond->semaphore, __abstime2timeout(abstime)) < 0) rc = errno;
  atomic_decrement(&cond->waiting);
  pthread_mutex_lock(mutex);
  return rc;
}

int pthread_cond_signal(pthread_cond_t *cond) {
  if (cond->waiting) semrel(cond->semaphore, 1);
  return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond) {
  if (cond->waiting) semrel(cond->semaphore, cond->waiting);
  return 0;
}
