//
// pthread.h
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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef PTHREAD_H
#define PTHREAD_H

#include <sys/types.h>

//
// Basic types
//

#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED
struct timespec {
  long tv_sec;
  long tv_nsec;
};
#endif

#ifndef _SCHED_PARAM_DEFINED
#define _SCHED_PARAM_DEFINED
struct sched_param {
  int sched_priority;
};
#endif

//
// POSIX options
//

#define _POSIX_THREADS
#define _POSIX_READER_WRITER_LOCKS
#define _POSIX_SPIN_LOCKS
#define _POSIX_BARRIERS
#define _POSIX_THREAD_SAFE_FUNCTIONS
#define _POSIX_THREAD_ATTR_STACKSIZE
#define _POSIX_THREAD_PRIORITY_SCHEDULING

//
// POSIX limits
//

#define PTHREAD_KEYS_MAX                      64
#define PTHREAD_STACK_MIN                     0
#define PTHREAD_THREADS_MAX                   2019

//
// POSIX pthread types
//

typedef handle_t pthread_t;
typedef tls_t pthread_key_t;

//
// POSIX thread attribute values
//

#define PTHREAD_CREATE_JOINABLE       0
#define PTHREAD_CREATE_DETACHED       1

#define PTHREAD_INHERIT_SCHED         0
#define PTHREAD_EXPLICIT_SCHED        1

#define PTHREAD_SCOPE_PROCESS         0
#define PTHREAD_SCOPE_SYSTEM          1

#define PTHREAD_CANCEL_ENABLE         0
#define PTHREAD_CANCEL_DISABLE        1

#define PTHREAD_CANCEL_ASYNCHRONOUS   0
#define PTHREAD_CANCEL_DEFERRED       1

#define PTHREAD_PROCESS_PRIVATE       0
#define PTHREAD_PROCESS_SHARED        1

//
// Threads
//

struct pthread_attr {
  void *stackaddr;
  size_t stacksize;
  int detachstate;
  struct sched_param param;
  int inheritsched;
  int contentionscope;
};

typedef struct pthread_attr pthread_attr_t;

//
// Once key
//

struct pthread_once {
  volatile int done;        // Indicates if user function executed
  int started;              // First thread to increment this value 
                            // to zero executes the user function
};

typedef struct pthread_once pthread_once_t;

#define PTHREAD_ONCE_INIT       {0, -1}

//
// Mutex
//

#define PTHREAD_MUTEX_NORMAL     0
#define PTHREAD_MUTEX_RECURSIVE  1
#define PTHREAD_MUTEX_ERRORCHECK 2
#define PTHREAD_MUTEX_DEFAULT    PTHREAD_MUTEX_NORMAL

struct pthread_mutexattr {
  int pshared;
  int kind;
};

typedef struct pthread_mutexattr pthread_mutexattr_t;

struct pthread_mutex {
  int lock;                 // Exclusive access to mutex state:
                            //  0: unlocked/free
                            //  1: locked - no other waiters
                            // -1: locked - with possible other waiters

  int recursion;            // Number of unlocks a thread needs to perform
                            // before the lock is released (recursive mutexes only)
  int kind;                 // Mutex type
  pthread_t owner;          // Thread owning the mutex
  handle_t event;           // Mutex release notification to waiting threads
};

typedef struct pthread_mutex pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER {0, 0, -1, -1, -1}
#define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP {0, 0, -1, -1, -1}

//
// Condition variables
//

struct pthread_condattr {
  int pshared;
};

typedef struct pthread_condattr pthread_condattr_t;

struct pthread_cond {
  int waiting;
  handle_t semaphore;
};

typedef struct pthread_cond pthread_cond_t;

#define PTHREAD_COND_INITIALIZER ((pthread_cond_t) -1)

//
// Barriers
//

#define PTHREAD_BARRIER_SERIAL_THREAD 1

struct pthread_barrierattr {
  int pshared;
};

typedef struct pthread_barrierattr pthread_barrierattr_t;

struct pthread_barrier
{
  unsigned int curr_height;
  unsigned int init_height;
  int step;
  handle_t breeched[2];
};

typedef struct pthread_barrier pthread_barrier_t;

//
// Read-write locks
//

struct pthread_rwlockattr {
  int pshared;
};

typedef struct pthread_rwlockattr pthread_rwlockattr_t;

struct pthread_rwlock {
  pthread_mutex_t mutex;
  handle_t shared_waiters;
  handle_t exclusive_waiters;
  int num_shared_waiters;
  int num_exclusive_waiters;
  int num_active;
  pthread_t owner;
};

typedef struct pthread_rwlock pthread_rwlock_t;

#define PTHREAD_RWLOCK_INITIALIZER {PTHREAD_MUTEX_INITIALIZER, 0, 0, 0, 0, 0, 0}

//
// Spinlocks
//

#define SPINLOCK_UNLOCKED    1
#define SPINLOCK_LOCKED      2
#define SPINLOCK_USEMUTEX    3

struct pthread_spinlock {
  int interlock;
  pthread_mutex_t mutex;
};

typedef struct pthread_spinlock pthread_spinlock_t;

#define PTHREAD_SPINLOCK_INITIALIZER {0, 0};

//
// POSIX thread routines
//

#ifdef  __cplusplus
extern "C" {
#endif

//
// Thread attribute functions
//

int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr);
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param);
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
int pthread_attr_getschedpolicy(pthread_attr_t *attr, int *policy);
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
int pthread_attr_getinheritsched(pthread_attr_t *attr, int *inheritsched);
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
int pthread_attr_getscope(const pthread_attr_t *attr, int *contentionscope);
int pthread_attr_setscope(pthread_attr_t *attr, int contentionscope);

//
// Thread functions
//

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg);
int pthread_detach(pthread_t thread);
int pthread_equal(pthread_t t1, pthread_t t2);
void pthread_exit(void *value_ptr);
int pthread_join(pthread_t thread, void **value_ptr);
pthread_t pthread_self(void);
int pthread_cancel(pthread_t thread);
int pthread_setcancelstate(int state, int *oldstate);
int pthread_setcanceltype(int type, int *oldtype);
void pthread_testcancel(void);
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));

//
// Scheduling functions
//

int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param);
int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param);
int pthread_setconcurrency(int level);
int pthread_getconcurrency(void);

//
// Thread specific data functions
//

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_key_delete(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);
void *pthread_getspecific(pthread_key_t key);

//
// Mutex attribute functions
//

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared);
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);
int pthread_mutexattr_gettype(pthread_mutexattr_t *attr, int *kind);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int kind);

//
// Mutex functions
//

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

//
// Condition variable attribute functions
//

int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);
int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared);
int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared);

//
// Condition variable functions
//

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);

//
// Barrier attribute functions
//

int pthread_barrierattr_init(pthread_barrierattr_t *attr);
int pthread_barrierattr_destroy(pthread_barrierattr_t *attr);
int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr, int *pshared);
int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared);

//
// Barrier functions
//

int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count);
int pthread_barrier_destroy(pthread_barrier_t *barrier);
int pthread_barrier_wait(pthread_barrier_t *barrier);

//
// Read-write lock attribute functions
//

int pthread_rwlockattr_init(pthread_rwlockattr_t *attr);
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr);
int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared);
int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared);

//
// Read-write lock functions
//

int pthread_rwlock_init(pthread_rwlock_t *lock, const pthread_rwlockattr_t *attr);
int pthread_rwlock_destroy(pthread_rwlock_t *lock);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *lock);
int pthread_rwlock_trywrlock(pthread_rwlock_t *lock);
int pthread_rwlock_rdlock(pthread_rwlock_t *lock);
int pthread_rwlock_timedrdlock(pthread_rwlock_t *lock, const struct timespec *abstime);
int pthread_rwlock_wrlock(pthread_rwlock_t *lock);
int pthread_rwlock_timedwrlock(pthread_rwlock_t *lock, const struct timespec *abstime);
int pthread_rwlock_unlock(pthread_rwlock_t *lock);

//
// Spinlock functions
//

int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
int pthread_spin_destroy(pthread_spinlock_t *lock);
int pthread_spin_lock(pthread_spinlock_t *lock);
int pthread_spin_trylock(pthread_spinlock_t *lock);
int pthread_spin_unlock(pthread_spinlock_t *lock);

//
// Helper functions
//

long __abstime2timeout(const struct timespec *abstime);

#ifdef  __cplusplus
}
#endif

#endif
