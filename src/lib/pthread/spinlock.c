//
// spinlock.c
//
// POSIX spin locks
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

int pthread_spin_init(pthread_spinlock_t *lock, int pshared) {
  if (!lock) return EINVAL;
  
  if (0 /* number of cpus > 1 */) {
    lock->interlock = SPINLOCK_UNLOCKED;
  } else {
    lock->interlock = SPINLOCK_USEMUTEX;
    pthread_mutex_init(&lock->mutex, NULL);
  }

  return 0;
}

int pthread_spin_destroy(pthread_spinlock_t *lock) {
  if (!lock) return EINVAL;
  if (lock->interlock == SPINLOCK_USEMUTEX) pthread_mutex_destroy(&lock->mutex);
  return 0;
}

int pthread_spin_lock(pthread_spinlock_t *lock) {
  while (atomic_compare_and_exchange(&lock->interlock, SPINLOCK_LOCKED, SPINLOCK_UNLOCKED) == SPINLOCK_LOCKED);

  if (lock->interlock == SPINLOCK_LOCKED) {
    return 0;
  } else if (lock->interlock == SPINLOCK_USEMUTEX) {
    return pthread_mutex_lock(&lock->mutex);
  }

  return EINVAL;
}

int pthread_spin_trylock(pthread_spinlock_t *lock) {
  switch (atomic_compare_and_exchange(&lock->interlock, SPINLOCK_LOCKED, SPINLOCK_UNLOCKED)) {
    case SPINLOCK_UNLOCKED:
      return 0;

    case SPINLOCK_LOCKED:
      return EBUSY;

    case SPINLOCK_USEMUTEX:
      return pthread_mutex_trylock(&lock->mutex);
  }

  return EINVAL;
}

int pthread_spin_unlock(pthread_spinlock_t *lock) {
  switch (atomic_compare_and_exchange(&lock->interlock, SPINLOCK_UNLOCKED, SPINLOCK_LOCKED)) {
    case SPINLOCK_LOCKED:
      return 0;

    case SPINLOCK_UNLOCKED:
      return EPERM;

    case SPINLOCK_USEMUTEX:
      return pthread_mutex_unlock(&lock->mutex);
  }

  return EINVAL;
}
