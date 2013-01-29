//
// object.c
//
// Object manager
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

#include <os/krnl.h>

struct waitable_timer *timer_list = NULL;
int nexttid = 1;

//
// insert_in_waitlist
//
// Insert thread wait block in wait list for object
//

static void insert_in_waitlist(struct object *obj, struct waitblock *wb) {
  wb->next_wait = NULL;
  wb->prev_wait = obj->waitlist_tail;
  if (obj->waitlist_tail) obj->waitlist_tail->next_wait = wb;
  obj->waitlist_tail = wb;
  if (!obj->waitlist_head) obj->waitlist_head = wb;
}

//
// remove_from_waitlist
//
// Remove thread wait block from wait list for object
//

static void remove_from_waitlist(struct waitblock *wb) {
  if (wb->next_wait) wb->next_wait->prev_wait = wb->prev_wait;
  if (wb->prev_wait) wb->prev_wait->next_wait = wb->next_wait;
  if (wb == wb->object->waitlist_head) wb->object->waitlist_head = wb->next_wait;
  if (wb == wb->object->waitlist_tail) wb->object->waitlist_tail = wb->prev_wait;
  wb->object = NULL;
  wb->next_wait = wb->prev_wait = NULL;
}

//
// thread_ready_to_run
//
// Test if thread is ready to run, i.e. all waitall objects on the
// waiting list are signaled or one waitany object is signaled.
// This routine also sets the waitkey for the thread from the waitblock.
//

int thread_ready_to_run(struct thread *t) {
  int any = 0;
  int all = 1;

  struct waitblock *wb = t->waitlist;
  while (wb) {
    //kprintf("check wb %p type %d key %d signaled %d\n", wb, wb->waittype, wb->waitkey, wb->object->signaled);
    
    if (wb->waittype == WAIT_ANY) {
      if (wb->object->signaled) {
        any = 1;
        t->waitkey = wb->waitkey;
      }
    } else {
      if (!wb->object->signaled) {
        all = 0;
      } else {
        t->waitkey = wb->waitkey;
      }
    }

    wb = wb->next;
  }

  return any || all;
}

//
// cancel_wait
//
// Remove thread from all wait blocks
//

void cancel_wait(struct thread *t) {
  // Remove thread from wait lists
  struct waitblock *wb = t->waitlist;
  while (wb) {
    remove_from_waitlist(wb);
    wb = wb->next;
  }

  // Clear wait list for thread
  t->waitlist = NULL;
}

//
// release_thread
//
// Release thread and mark it as ready to run
//

void release_thread(struct thread *t) {
  // Remove thread from wait lists
  cancel_wait(t);

  // Mark thread as ready
  mark_thread_ready(t, 1, 1);
}

//
// release_waiters
//
// Release all threads waiting for object
//

void release_waiters(struct object *o, int waitkey) {
  struct waitblock *wb;
  struct waitblock *next;

  wb = o->waitlist_head;
  while (wb) {
    next = wb->next_wait;
    if (thread_ready_to_run(wb->thread))  {
      wb->thread->waitkey = waitkey;
      release_thread(wb->thread);
    }
    wb = next;
  }
}

//
// enter_object
//
// Called when an object waits on a signaled object.
//

int enter_object(struct object *obj) {
  int rc = 0;

  switch (obj->type) {
    case OBJECT_THREAD:
      // Set thread exit code as wait key
      rc = ((struct thread *) obj)->exitcode;
      break;

    case OBJECT_EVENT:
      // Reset auto-reset event
      if (!((struct event *) obj)->manual_reset) obj->signaled = 0;
      break;

    case OBJECT_TIMER:
      // Do nothing
      break;

    case OBJECT_MUTEX:
      // Set state to nonsignaled and set current thread as owner
      obj->signaled = 0;
      ((struct mutex *) obj)->owner = self();
      ((struct mutex *) obj)->recursion = 1;
      break;

    case OBJECT_SEMAPHORE:
      // Decrease count and set to nonsignaled if count reaches zero
      if (--(((struct sem *) obj)->count) == 0) obj->signaled = 0;
      break;

    case OBJECT_FILE:
      // Do nothing
      break;

    case OBJECT_SOCKET:
      // Do nothing
      break;

    case OBJECT_IOMUX:
      rc = dequeue_event_from_iomux((struct iomux *) obj);
      break;

    case OBJECT_FILEMAP:
      obj->signaled = 0;
      break;
  }

  return rc;
}

//
// wait_for_object
//
// Wait for object to become signaled.
//

int wait_for_object(object_t hobj, unsigned int timeout) {
  return wait_for_one_object(hobj, timeout, 0);
}

//
// wait_for_one_object
//
// Wait for object to become signaled.
//

int wait_for_one_object(object_t hobj, unsigned int timeout, int alertable) {
  struct object *obj = (struct object *) hobj;
  struct thread *t = self();
  struct waitblock wb;

  // If object is signaled we do not have to wait
  if (obj->signaled) return enter_object(obj);

  if (obj->type == OBJECT_MUTEX && ((struct mutex *) obj)->owner == self()) {
    // Mutex is already owned by current thread, increase recursion count
    ((struct mutex *) obj)->recursion++;
    return 0;
  }

  if (timeout == 0) return -ETIMEOUT;

  // Insert thread in waitlist for object
  t->waitlist = &wb;
  wb.thread = t;
  wb.object = obj;
  wb.waittype = WAIT_ANY;
  wb.waitkey = 0;
  wb.next = NULL;

  insert_in_waitlist(obj, &wb);

  if (timeout == INFINITE) {
    // Wait for object to become signaled
    if (alertable) {
      int rc = enter_alertable_wait(THREAD_WAIT_OBJECT);
      if (rc < 0) {
        cancel_wait(t);
        t->waitkey = rc;
      }
    } else {
      enter_wait(THREAD_WAIT_OBJECT);
    }

    // Return waitkey
    return t->waitkey;
  } else {
    struct waitable_timer timer;
    struct waitblock wbtmo;

    // Initialize timer
    if (timeout < MSECS_PER_TICK) timeout = MSECS_PER_TICK;
    init_waitable_timer(&timer, ticks + timeout / MSECS_PER_TICK);
    wb.next = &wbtmo;
    wbtmo.thread = t;
    wbtmo.object = &timer.object;
    wbtmo.waittype = WAIT_ANY;
    wbtmo.waitkey = -ETIMEOUT;
    wbtmo.next = NULL;

    insert_in_waitlist(&timer.object, &wbtmo);

    // Wait for object to become signaled or time out
    if (alertable) {
      int rc = enter_alertable_wait(THREAD_WAIT_OBJECT);
      if (rc < 0) {
        cancel_wait(t);
        t->waitkey = rc;
      }
    } else {
      enter_wait(THREAD_WAIT_OBJECT);
    }

    // Stop timer
    cancel_waitable_timer(&timer);

    // Return wait key
    return t->waitkey;
  }
}

//
// wait_for_all_objects
//
// Wait for all objects to become signaled
//

int wait_for_all_objects(struct object **objs, int count, unsigned int timeout, int alertable) {
  int n;
  struct thread *t = self();
  struct waitblock wb[MAX_WAIT_OBJECTS];
  struct waitblock wbtmo;
  struct waitable_timer timer;
  int all;

  if (count == 0) {
    if (timeout == INFINITE) return -EINVAL;
    if (msleep(timeout) > 0) return -EINTR;
    return 0;
  }

  if (count > MAX_WAIT_OBJECTS) return -EINVAL;

  // Check for all objects signaled
  all = 1;
  for (n = 0; n < count; n++) {
    if (objs[n]->type == OBJECT_MUTEX) {
      if (!objs[n]->signaled || ((struct mutex *) objs[n])->owner != self()) {
        all = 0;
        break;
      }
    } else {
      if (!objs[n]->signaled) {
        all = 0;
        break;
      }
    }
  }

  // If all objects are signaled enter all objects and return
  if (all) {
    int rc;

    rc = 0;
    for (n = 0; n < count; n++) {
      if (objs[n]->signaled) {
        rc |= enter_object(objs[n]);
      } else if (objs[n]->type == OBJECT_MUTEX && ((struct mutex *) objs[n])->owner == self()) {
        // Mutex is already owned by current thread, increase recursion count
        ((struct mutex *) objs[n])->recursion++;
      }
    }

    return rc;
  }

  // Return immediately if timeout is zero
  if (timeout == 0) return -ETIMEOUT;

  // Setup waitblocks
  for (n = 0; n < count; n++) {
    // Insert thread in waitlist for object
    if (n == 0) {
      t->waitlist = &wb[n];
    } else {
      wb[n - 1].next = &wb[n];
    }

    wb[n].thread = t;
    wb[n].object = objs[n];
    wb[n].waittype = WAIT_ALL;
    wb[n].waitkey = n;
    wb[n].next = NULL;

    insert_in_waitlist(objs[n], &wb[n]);
  }

  // Add waitable timer for timeout
  if (timeout != INFINITE) {
    if (timeout < MSECS_PER_TICK) timeout = MSECS_PER_TICK;
    init_waitable_timer(&timer, ticks + timeout / MSECS_PER_TICK);
    wb[count - 1].next = &wbtmo;
    wbtmo.thread = t;
    wbtmo.object = &timer.object;
    wbtmo.waittype = WAIT_ANY;
    wbtmo.waitkey = -ETIMEOUT;
    wbtmo.next = NULL;

    insert_in_waitlist(&timer.object, &wbtmo);
  }

  // Wait for all objects to become signaled or time out
  if (alertable) {
    int rc = enter_alertable_wait(THREAD_WAIT_OBJECT);
    if (rc < 0) {
      cancel_wait(t);
      t->waitkey = rc;
    }
  } else {
    enter_wait(THREAD_WAIT_OBJECT);
  }

  // Stop timer
  if (timeout != INFINITE) cancel_waitable_timer(&timer);

  // Return wait key
  return t->waitkey;
}

//
// wait_for_any_object
//
// Wait for some object to become signaled
//

int wait_for_any_object(struct object **objs, int count, unsigned int timeout, int alertable) {
  int n;
  struct thread *t = self();
  struct waitblock wb[MAX_WAIT_OBJECTS];
  struct waitblock wbtmo;
  struct waitable_timer timer;

  if (count == 0) {
    if (timeout == INFINITE) return -EINVAL;
    if (msleep(timeout) > 0) return -EINTR;
    return 0;
  }

  if (count > MAX_WAIT_OBJECTS) return -EINVAL;

  // Check for any object signaled
  for (n = 0; n < count; n++) {
    if (objs[n]->signaled) {
      int rc = enter_object(objs[n]);
      return rc ? rc : n;
    } else if (objs[n]->type == OBJECT_MUTEX && ((struct mutex *) objs[n])->owner == self()) {
      // Mutex is already owned by current thread, increase recursion count
      ((struct mutex *) objs[n])->recursion++;
      return n;
    }
  }

  // Return immediately if timeout is zero
  if (timeout == 0) return -ETIMEOUT;

  // Setup waitblocks
  for (n = 0; n < count; n++) {
    // Insert thread in waitlist for object
    if (n == 0) {
      t->waitlist = &wb[n];
    } else {
      wb[n - 1].next = &wb[n];
    }

    wb[n].thread = t;
    wb[n].object = objs[n];
    wb[n].waittype = WAIT_ANY;
    wb[n].waitkey = n;
    wb[n].next = NULL;

    insert_in_waitlist(objs[n], &wb[n]);
  }

  // Add waitable timer for timeout
  if (timeout != INFINITE) {
    if (timeout < MSECS_PER_TICK) timeout = MSECS_PER_TICK;
    init_waitable_timer(&timer, ticks + timeout / MSECS_PER_TICK);
    wb[count - 1].next = &wbtmo;
    wbtmo.thread = t;
    wbtmo.object = &timer.object;
    wbtmo.waittype = WAIT_ANY;
    wbtmo.waitkey = -ETIMEOUT;
    wbtmo.next = NULL;

    insert_in_waitlist(&timer.object, &wbtmo);
  }

  // Wait for any object to become signaled or time out
  if (alertable) {
    int rc = enter_alertable_wait(THREAD_WAIT_OBJECT);
    if (rc < 0) {
      cancel_wait(t);
      t->waitkey = rc;
    }
  } else {
    enter_wait(THREAD_WAIT_OBJECT);
  }

  // Stop timer
  if (timeout != INFINITE) cancel_waitable_timer(&timer);

  // Return wait key
  return t->waitkey;
}

//
// init_object
//
// Initialize object
//

void init_object(struct object *o, int type) {
  o->type = type;
  o->signaled = 0;
  o->handle_count = 0;
  o->lock_count = 0;
  o->waitlist_head = NULL;
  o->waitlist_tail = NULL;
}

//
// close_object
//
// Close object after all handles have been released
//

int close_object(struct object *o) {
  // Release all waitblocks waiting on object
  while (o->waitlist_head) {
    o->waitlist_head->thread->waitkey = -EINTR;
    release_thread(o->waitlist_head->thread);
  }

  switch (o->type) {
    case OBJECT_THREAD:
    case OBJECT_EVENT:
    case OBJECT_MUTEX:
    case OBJECT_SEMAPHORE:
    case OBJECT_FILEMAP:
      return 0;

    case OBJECT_TIMER:
      cancel_waitable_timer((struct waitable_timer *) o);
      return 0;
    
    case OBJECT_FILE:
      return close((struct file *) o);

    case OBJECT_SOCKET:
      return closesocket((struct socket *) o);

    case OBJECT_IOMUX:
      return close_iomux((struct iomux *) o);
  }

  return -EBADF;
}

//
// destroy_object
//
// Destroy object after all handles have been closed and
// all locks have been released
//

int destroy_object(struct object *o) {
  switch (o->type) {
    case OBJECT_THREAD:
      return destroy_thread((struct thread *) o);

    case OBJECT_EVENT:
    case OBJECT_TIMER:
    case OBJECT_MUTEX:
    case OBJECT_SEMAPHORE:
    case OBJECT_IOMUX:
    case OBJECT_SOCKET:
    case OBJECT_FILEMAP:
      kfree(o);
      return 0;

    case OBJECT_FILE:
      return destroy((struct file *) o);
  }

  return -EBADF;
}

//
// init_thread
//
// Initialize thread object
//

void init_thread(struct thread *t, int priority) {
  memset(t, 0, sizeof(struct thread));
  init_object(&t->object, OBJECT_THREAD);

  t->priority = t->base_priority = priority;
  t->state = THREAD_STATE_INITIALIZED;
  t->id = nexttid++;
}

//
// exit_thread
//
// Called when a thread exits
//

void exit_thread(struct thread *t) {
  // Set signaled state
  t->object.signaled = 1;

  // Release all waiting threads
  release_waiters(&t->object, t->exitcode);
}

//
// init_event
//
// Initialize event object
//

void init_event(struct event *e, int manual_reset, int initial_state) {
  init_object(&e->object, OBJECT_EVENT);
  e->manual_reset = manual_reset;
  e->object.signaled = initial_state;
}

//
// pulse_event
//
// Pulse event
//

void pulse_event(struct event *e) {
  struct waitblock *wb;
  struct waitblock *wb_next;

  // Set the event object to the signaled state
  e->object.signaled = 1;

  // Release waiting threads
  wb = e->object.waitlist_head;
  while (wb) {
    wb_next = wb->next_wait;
    if (thread_ready_to_run(wb->thread)) {
      // Release waiting thread
      release_thread(wb->thread);

      // Auto reset only releases one thread
      if (!e->manual_reset) break;

      wb = wb_next;
    }
  }

  // Set the event object to the nonsignaled state
  e->object.signaled = 0;
}

//
// set_event
//
// Set event
//

void set_event(struct event *e) {
  struct waitblock *wb;
  struct waitblock *wb_next;

  // Set the event object to the signaled state
  e->object.signaled = 1;

  // Release waiting threads
  wb = e->object.waitlist_head;
  while (wb) {
    wb_next = wb->next_wait;
    if (thread_ready_to_run(wb->thread)) {
      // Release waiting thread
      release_thread(wb->thread);
      
      // Auto reset only releases one thread
      if (!e->manual_reset) {
        e->object.signaled = 0;
        break;
      }
    }

    wb = wb_next;
  }
}

//
// reset_event
//
// Reset event
//

void reset_event(struct event *e) {
  // Set the event object to the nonsignaled state
  e->object.signaled = 0;
}

//
// init_sem
//
// Initialize semaphore object
//

void init_sem(struct sem *s, unsigned int initial_count) {
  init_object(&s->object, OBJECT_SEMAPHORE);
  s->count = initial_count;
  s->object.signaled = (s->count > 0);
}

//
// release_sem
//
// Release 'count' resouces for the semaphore
//

unsigned int release_sem(struct sem *s, unsigned int count) {
  unsigned int prevcount;
  struct waitblock *wb;
  struct waitblock *wb_next;

  // Increase semaphore count
  prevcount = s->count;
  s->count += count;

  if (s->count > 0) {
    // Set semaphore to signaled state
    s->object.signaled = 1;

    // Release waiting threads
    wb = s->object.waitlist_head;
    while (wb) {
      wb_next = wb->next_wait;
      if (thread_ready_to_run(wb->thread)) {
        // Decrease semaphore count
        s->count--;

        // Release waiting thread
        release_thread(wb->thread);

        // If count reaches zero then set semaphore to the nonsignaled state
        if (s->count == 0) {
          s->object.signaled = 0;
          break;
        }
      }

      wb = wb_next;
    }
  }

  return prevcount;
}

//
// set_sem
//
// Set semaphore resource count
//

unsigned int set_sem(struct sem *s, unsigned int count) {
  unsigned int prevcount;
  struct waitblock *wb;
  struct waitblock *wb_next;

  // Increase semaphore count
  prevcount = s->count;
  s->count = count;

  if (s->count > 0) {
    // Set semaphore to signaled state
    s->object.signaled = 1;

    // Release waiting threads
    wb = s->object.waitlist_head;
    while (wb) {
      wb_next = wb->next_wait;
      if (thread_ready_to_run(wb->thread)) {
        // Decrease semaphore count
        s->count--;

        // Release waiting thread
        release_thread(wb->thread);

        // If count reaches zero then set semaphore to the nonsignaled state
        if (s->count == 0) {
          s->object.signaled = 0;
          break;
        }
      }

      wb = wb_next;
    }
  }

  return prevcount;
}

//
// init_mutex
//
// Initialize mutex object
//

void init_mutex(struct mutex *m, int owned) {
  init_object(&m->object, OBJECT_MUTEX);
  if (owned) {
    m->owner = self();
    m->object.signaled = 0;
    m->recursion = 1;
  } else {
    m->owner = NULL;
    m->object.signaled = 1;
    m->recursion = 0;
  }
}

//
// release_mutex
//
// Release mutex
//

int release_mutex(struct mutex *m) {
  struct waitblock *wb;

  // Check that caller is the owner
  if (m->owner != self()) return -EPERM;

  // Check for recursion
  if (--m->recursion > 0) return 0;

  // Set the mutex to the signaled state
  m->object.signaled = 1;
  m->owner = NULL;

  // Release first waiting thread
  wb = m->object.waitlist_head;
  while (wb) {
    if (thread_ready_to_run(wb->thread)) break;
    wb = wb->next_wait;
  }

  if (wb != NULL) {
    // Set mutex to nonsignal state
    m->owner = wb->thread;
    m->recursion = 1;
    m->object.signaled = 0;

    // Release waiting thread
    release_thread(wb->thread);
  }

  return 0;
}

//
// unlock_filemap
//
// Unlock filemap
//

int unlock_filemap(struct filemap *m)  {
  struct waitblock *wb;

  // Set the filemap to the signaled state
  m->object.signaled = 1;

  // Release first waiting thread
  wb = m->object.waitlist_head;
  while (wb) {
    if (thread_ready_to_run(wb->thread)) break;
    wb = wb->next_wait;
  }

  if (wb != NULL) {
    // Set filemap to nonsignal state
    m->object.signaled = 0;

    // Release waiting thread
    release_thread(wb->thread);
  }

  return 0;
}

//
// expire_waitable_timer
//
// Expire waitable timer
//

static void expire_waitable_timer(void *arg) {
  struct waitable_timer *t = arg;

  // Set signaled state
  t->object.signaled = 1;

  // Release all waiting threads
  release_waiters(&t->object, 0);
}

//
// init_waitable_timer
//
// Initialize timer
//

void init_waitable_timer(struct waitable_timer *t, unsigned int expires) {
  init_object(&t->object, OBJECT_TIMER);
  init_timer(&t->timer, expire_waitable_timer, t);
  t->timer.expires = expires;
  
  if (time_before_eq(expires, ticks)) {
    // Set timer to signaled state immediately
    t->object.signaled = 1;
  } else {
    // Start timer
    add_timer(&t->timer);
  }
}

//
// modify_waitable_timer
//
// Modify timer
//

void modify_waitable_timer(struct waitable_timer *t, unsigned int expires) {
  mod_timer(&t->timer, expires);
}

//
// cancel_waitable_timer
//
// Cancel timer
//

void cancel_waitable_timer(struct waitable_timer *t) {
  if (t->timer.active) del_timer(&t->timer);
}
