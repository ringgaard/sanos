//
// pthread.h
//
// POSIX threads library
//
// Copyright (C) 2004 Michael Ringgaard
// Copyright (C) 1998 John E. Bossom
// Copyright (C) 1999,2003 Pthreads-win32 contributors
//
// The current list of contributors can be seen at the
// following World Wide Web location:
// http://sources.redhat.com/pthreads-win32/contributors.html
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library in the file COPYING.LIB;
// if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef PTHREAD_H
#define PTHREAD_H

//
// Basic types
//

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#ifndef _TID_T_DEFINED
#define _TID_T_DEFINED
typedef unsigned long tid_t;
#endif

#ifndef _TLS_T_DEFINED
#define _TLS_T_DEFINED
typedef unsigned long tls_t;
#endif

#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED
struct timespec 
{
  long tv_sec;
  long tv_nsec;
};
#endif

//
// POSIX Options
//

#ifndef _POSIX_THREADS
#define _POSIX_THREADS
#endif

#ifndef _POSIX_READER_WRITER_LOCKS
#define _POSIX_READER_WRITER_LOCKS
#endif

#ifndef _POSIX_SPIN_LOCKS
#define _POSIX_SPIN_LOCKS
#endif

#ifndef _POSIX_BARRIERS
#define _POSIX_BARRIERS
#endif

#define _POSIX_THREAD_SAFE_FUNCTIONS
#define _POSIX_THREAD_ATTR_STACKSIZE
#define _POSIX_THREAD_PRIORITY_SCHEDULING

//
// POSIX limits
//

#define PTHREAD_DESTRUCTOR_ITERATIONS         4
#define PTHREAD_KEYS_MAX                      64
#define PTHREAD_STACK_MIN                     0
#define PTHREAD_THREADS_MAX                   2019

//
// POSIX pthread types
//

typedef tid_t pthread_t;
typedef tls_t pthread_key_t;
typedef struct pthread_once pthread_once_t;

typedef struct pthread_attr *pthread_attr_t;
typedef struct pthread_mutex *pthread_mutex_t;
typedef struct pthread_mutexattr *pthread_mutexattr_t;
typedef struct pthread_cond *pthread_cond_t;
typedef struct pthread_condattr *pthread_condattr_t;
typedef struct pthread_rwlock *pthread_rwlock_t;
typedef struct pthread_rwlockattr *pthread_rwlockattr_t;
typedef struct pthread_spinlock *pthread_spinlock_t;
typedef struct pthread_barrier *pthread_barrier_t;
typedef struct pthread_barrierattr *pthread_barrierattr_t;

//
// POSIX Threads
//

enum 
{
  // pthread_attr_{get,set}detachstate
  PTHREAD_CREATE_JOINABLE       = 0,  // Default
  PTHREAD_CREATE_DETACHED       = 1,

  // pthread_attr_{get,set}inheritsched
  PTHREAD_INHERIT_SCHED         = 0,
  PTHREAD_EXPLICIT_SCHED        = 1,  // Default

  // pthread_{get,set}scope
  PTHREAD_SCOPE_PROCESS         = 0,
  PTHREAD_SCOPE_SYSTEM          = 1,  // Default

  // pthread_setcancelstate paramters
  PTHREAD_CANCEL_ENABLE         = 0,  // Default
  PTHREAD_CANCEL_DISABLE        = 1,

  // pthread_setcanceltype parameters
  PTHREAD_CANCEL_ASYNCHRONOUS   = 0,
  PTHREAD_CANCEL_DEFERRED       = 1,  // Default

  // pthread_mutexattr_{get,set}pshared
  // pthread_condattr_{get,set}pshared
  PTHREAD_PROCESS_PRIVATE       = 0,
  PTHREAD_PROCESS_SHARED        = 1,

  // pthread_barrier_wait
  PTHREAD_BARRIER_SERIAL_THREAD = -1
};

//
// Cancelation
//

#define PTHREAD_CANCELED       ((void *) -1)

//
// Once key
//

#define PTHREAD_ONCE_INIT       {0, -1}

struct pthread_once
{
  volatile int done;        // Indicates if user function executed
  long started;             // First thread to increment this value 
                            // to zero executes the user function
};

//
// Object initialisers
//

#define PTHREAD_MUTEX_INITIALIZER ((pthread_mutex_t) -1)
#define PTHREAD_COND_INITIALIZER ((pthread_cond_t) -1)
#define PTHREAD_RWLOCK_INITIALIZER ((pthread_rwlock_t) -1)
#define PTHREAD_SPINLOCK_INITIALIZER ((pthread_spinlock_t) -1)

//
// Mutex types
//

enum
{
  PTHREAD_MUTEX_NORMAL     = 0,
  PTHREAD_MUTEX_RECURSIVE  = 1,
  PTHREAD_MUTEX_ERRORCHECK = 2,
  PTHREAD_MUTEX_DEFAULT    = 3
};

//
// POSIX thread routines
//

#ifdef  __cplusplus
extern "C" {
#endif

//
// Attribute functions
//

int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr);
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param);
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
int pthread_attr_getschedpolicy(pthread_attr_t *attr, int *policy);
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
int pthread_attr_getinheritsched(pthread_attr_t *attr, int *inheritsched);
int pthread_attr_setscope(pthread_attr_t *attr, int contentionscope);
int pthread_attr_getscope(const pthread_attr_t *attr, int *contentionscope);

//
// Thread functions
//

int pthread_create(pthread_t *tid, const pthread_attr_t *attr, void *(*start)(void *), void *arg);
int pthread_detach(pthread_t tid);
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
// Scheduling
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
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int kind);
int pthread_mutexattr_gettype(pthread_mutexattr_t *attr, int *kind);

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

#ifdef  __cplusplus
}
#endif

#endif
