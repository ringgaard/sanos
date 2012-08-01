//
// pthread.c
//
// POSIX threads library
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
#include <sched.h> 
#include <atomic.h>
#include <string.h>

//
// Helper functions
//

long __abstime2timeout(const struct timespec *abstime) {
  struct timeval curtime;
  long timeout;

  if (abstime == NULL) return INFINITE;
  if (gettimeofday(&curtime, NULL) < 0) return -1;
  timeout = ((long) (abstime->tv_sec - curtime.tv_sec) * 1000L +
             (long)((abstime->tv_nsec / 1000) - curtime.tv_usec) / 1000L);
  if (timeout < 0) timeout = 0L;
  return timeout;
}

//
// Threads
//

int pthread_attr_init(pthread_attr_t *attr) {
  if (!attr) return EINVAL;
  attr->stackaddr = NULL;
  attr->stacksize = 0;
  attr->detachstate = PTHREAD_CREATE_JOINABLE;
  attr->param.sched_priority = PRIORITY_NORMAL;
  attr->inheritsched = PTHREAD_EXPLICIT_SCHED;
  attr->contentionscope = PTHREAD_SCOPE_SYSTEM;
  return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr) {
  return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate) {
  if (!attr || !detachstate) return EINVAL;
  *detachstate = attr->detachstate;
  return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate) {
  if (!attr) return EINVAL;
  if (detachstate != PTHREAD_CREATE_JOINABLE && detachstate != PTHREAD_CREATE_DETACHED) return EINVAL;
  attr->detachstate = detachstate;
  return 0;
}

int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr) {
  if (!attr || !stackaddr) return EINVAL;
  *stackaddr = attr->stackaddr;
  return 0;
}

int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr) {
  if (!attr) return EINVAL;
  attr->stackaddr = stackaddr;
  return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize) {
  if (!attr || !stacksize) return EINVAL;
  *stacksize = attr->stacksize;
  return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize) {
  if (!attr) return EINVAL;
  attr->stacksize = stacksize;
  return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param) {
  if (!attr || !param) return EINVAL;
  memcpy(param, &attr->param, sizeof(struct sched_param));
  return 0;
}

int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param) {
  if (!attr || !param) return EINVAL;
  if (param->sched_priority < PRIORITY_IDLE || param->sched_priority > PRIORITY_TIME_CRITICAL) return EINVAL; 
  memcpy(&attr->param, param, sizeof(struct sched_param));
  return 0;
}

int pthread_attr_getschedpolicy(pthread_attr_t *attr, int *policy) {
  if (!attr || !policy) return EINVAL;
  *policy = SCHED_OTHER;
  return 0;
}

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy) {
  if (!attr) return EINVAL;
  if (policy != SCHED_OTHER) return ENOSYS;
  return 0;
}

int pthread_attr_getinheritsched(pthread_attr_t *attr, int *inheritsched) {
  if (!attr || !inheritsched) return EINVAL;
  *inheritsched = attr->inheritsched;
  return 0;
}

int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched) {
  if (!attr) return EINVAL;
  if (inheritsched != PTHREAD_INHERIT_SCHED && inheritsched != PTHREAD_EXPLICIT_SCHED) return EINVAL;
  attr->inheritsched = inheritsched;
  return 0;
}

int pthread_attr_getscope(const pthread_attr_t *attr, int *contentionscope) {
  if (!attr || !contentionscope) return EINVAL;
  *contentionscope = attr->contentionscope;
  return 0;
}

int pthread_attr_setscope(pthread_attr_t *attr, int contentionscope) {
  if (!attr) return EINVAL;
  if (contentionscope != PTHREAD_INHERIT_SCHED && contentionscope != PTHREAD_EXPLICIT_SCHED) return EINVAL;
  attr->contentionscope = contentionscope;
  return 0;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg) {
  unsigned int stacksize = 0;
  int priority = PRIORITY_NORMAL;
  int detach = 0;
  struct tib *tib;
  handle_t hthread;

  if (!thread) return EINVAL;

  if (attr) {
    stacksize = attr->stacksize;
    if (attr->detachstate == PTHREAD_CREATE_DETACHED) detach = 1;
    priority = attr->param.sched_priority;
  }

  hthread = beginthread((void (__stdcall *)(void *)) start, stacksize, arg, CREATE_POSIX | CREATE_SUSPENDED, NULL, &tib);
  if (hthread < 0) return errno;

  if (priority != PRIORITY_NORMAL) setprio(hthread, priority);
  if (detach) {
    *thread = tib->hndl;
    resume(hthread);
    close(hthread);
  } else {
    *thread = hthread;
    resume(hthread);
  }

  return 0;
}

int pthread_detach(pthread_t thread) {
  if (close(thread) < 0) return errno;
  return 0;
}

int pthread_equal(pthread_t t1, pthread_t t2) {
  if (t1 == t2) return 1;
  if (getthreadblock(t1) == getthreadblock(t2)) return 1;
  return 0;
}

void pthread_exit(void *value_ptr) {
  endthread((int) value_ptr);
}

int pthread_join(pthread_t thread, void **value_ptr) {
  int rc;

  if (pthread_equal(thread, pthread_self())) return EDEADLK;
  rc = waitone(thread, INFINITE);
  if (rc < 0) return errno;
  if (*value_ptr) *value_ptr = (void *) rc;
  return 0;
}

pthread_t pthread_self(void) {
  return self();
}

int pthread_cancel(pthread_t thread) {
  return 0;
}

int pthread_setcancelstate(int state, int *oldstate) {
  return ENOSYS;
}

int pthread_setcanceltype(int type, int *oldtype) {
  return ENOSYS;
}

void pthread_testcancel(void) {
}

//
// Scheduling functions
//

int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param) {
  if (policy < SCHED_MIN || policy > SCHED_MAX) return EINVAL;
  if (policy != SCHED_OTHER) return ENOSYS;
  if (!param) return EINVAL;
  if (param->sched_priority < PRIORITY_IDLE || param->sched_priority > PRIORITY_TIME_CRITICAL) return EINVAL; 
  if (setprio(thread, param->sched_priority) < 0) return errno;
  return 0;
}

int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param) {
  int prio;
  if (!policy || !param) return EINVAL;
  prio = getprio(thread);
  if (prio < 0) return errno;
  *policy = SCHED_OTHER;
  param->sched_priority = prio;
  return 0;
}

int pthread_setconcurrency(int level) {
  return 0;
}

int pthread_getconcurrency(void) {
  return 0;
}

//
// Once
//

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void)) {
  if (!once_control || !init_routine) return EINVAL;

  if (!once_control->done) {
    if (atomic_increment(&once_control->started) == 0) {
      // First thread to increment the started variable
      (*init_routine)();
      once_control->done = 1;
    } else {
      // Block until other thread finishes executing the once routine
      while (!once_control->done) msleep(0);
    }
  }

  return 0;
}

//
// Thread specific data
//

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *)) {
  // TODO: implement support for tls destructors
  *key = tlsalloc();
  if (*key == INVALID_TLS_INDEX) return EAGAIN;
  return 0;
}

int pthread_key_delete(pthread_key_t key) {
  tlsfree(key);
  return 0;
}

int pthread_setspecific(pthread_key_t key, const void *value) {
  tlsset(key, (void *) value);
  return 0;
}

void *pthread_getspecific(pthread_key_t key) {
  return tlsget(key);
}
