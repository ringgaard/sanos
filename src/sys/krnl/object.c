//
// object.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Object manager
//

#include <os/krnl.h>

struct waitable_timer *timer_list = NULL;
int nexttid = 1;

#define HANDLES_PER_PAGE (PAGESIZE / sizeof(struct object *))

struct object **htab = (struct object **) HTABBASE;
handle_t hfreelist = NOHANDLE;
int htabsize = 0;

//
// Insert thread wait block in wait list for object
//

static void insert_in_waitlist(struct object *obj, struct waitblock *wb)
{
  wb->next_wait = NULL;
  wb->prev_wait = obj->waitlist_tail;
  if (obj->waitlist_tail) obj->waitlist_tail->next_wait = wb;
  obj->waitlist_tail = wb;
  if (!obj->waitlist_head) obj->waitlist_head = wb;
}

//
// Remove thread wait block from wait list for object
//

static void remove_from_waitlist(struct waitblock *wb)
{
  if (wb->next_wait) wb->next_wait->prev_wait = wb->prev_wait;
  if (wb->prev_wait) wb->prev_wait->next_wait = wb->next_wait;
  if (wb->object)
  {
    if (wb == wb->object->waitlist_head) wb->object->waitlist_head = wb->next_wait;
    if (wb == wb->object->waitlist_tail) wb->object->waitlist_tail = wb->prev_wait;
  }
}

//
// Test if thread is ready to run, i.e. all waitall objects on the
// waiting list are signaled or one waitany object is signaled.
//

static int thread_ready_to_run(struct thread *t)
{
  int any = 0;
  int all = 1;

  struct waitblock *wb = t->waitlist;
  while (wb)
  {
//kprintf("check wb %p type %d key %d signaled %d\n", wb, wb->waittype, wb->waitkey, wb->object->signaled);
    if (wb->waittype == WAIT_ANY)
    {
      if (!wb->object || wb->object->signaled) 
      {
	any = 1;
	t->waitkey = wb->waitkey;
      }
    }
    else
    {
      if (wb->object && !wb->object->signaled) 
	all = 0;
      else
	t->waitkey = wb->waitkey;
    }

    wb = wb->next;
  }

  return any || all;
}

//
// Release thread and mark is as ready to run
//

static void release_thread(struct thread *t)
{
  // Remove thread from wait lists
  struct waitblock *wb = t->waitlist;
  while (wb)
  {
    remove_from_waitlist(wb);
    wb = wb->next;
  }

  // Mark thread as ready
  mark_thread_ready(t);
}

//
// Wait for object to become signaled.
//

int wait_for_object(object_t hobj, unsigned int timeout)
{
  struct object *obj = (struct object *) hobj;

  // If object is signaled we does not have to wait
  if (obj->signaled)
  {
    switch (obj->type)
    {
      case OBJECT_THREAD:
	// Do nothing
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
	((struct mutex *) obj)->owner = current_thread();
	((struct mutex *) obj)->recursion = 1;
	break;

      case OBJECT_SEMAPHORE:
	// Decrease count and set to nonsignaled if count reaches zero
	if (--(((struct sem *) obj)->count) == 0) obj->signaled = 0;
	break;

      case OBJECT_FILE:
	// Do nothing
	break;
    }
  }
  else if (obj->type == OBJECT_MUTEX && ((struct mutex *) obj)->owner == current_thread())
  {
    // Mutex is already owned by current thread, increase recursion count
    ((struct mutex *) obj)->recursion++;
  }
  else if (timeout == 0)
    return -ETIMEOUT;
  else
  {
    struct thread *t = current_thread();

    // Mark thread as waiting
    t->state = THREAD_STATE_WAITING;

    // Insert thread in waitlist for object
    t->waitlist = &t->auxwb0;
    t->auxwb0.thread = t;
    t->auxwb0.object = obj;
    t->auxwb0.waittype = WAIT_ALL;
    t->auxwb0.waitkey = 0;
    t->auxwb0.next = NULL;

    insert_in_waitlist(obj, &t->auxwb0);

    if (timeout == INFINITE)
    {
      // Wait for object to become signaled
      dispatch();

      // Clear waitlist and return waitkey
      t->waitlist = NULL;
      return t->auxwb0.waitkey;
    }
    else
    {
      // Initialize timer
      init_waitable_timer(&t->auxtimer, ticks + timeout / MSECS_PER_TICK);
      t->auxwb0.waittype = WAIT_ANY;
      t->auxwb0.next = &t->auxwb1;
      t->auxwb1.thread = t;
      t->auxwb1.object = &t->auxtimer.object;
      t->auxwb1.waittype = WAIT_ANY;
      t->auxwb1.waitkey = -ETIMEOUT;
      t->auxwb1.next = NULL;

      insert_in_waitlist(&t->auxtimer.object, &t->auxwb1);

      // Wait for object to become signaled or time out
      dispatch();

      // Stop timer
      cancel_waitable_timer(&t->auxtimer);

      // Clear wait list
      t->waitlist = NULL;

      // Return wait key
      return t->waitkey;
    }
  }

  return 0;
}

//
// Initialize object
//

void init_object(struct object *o, int type)
{
  o->type = type;
  o->signaled = 0;
  o->handle_count = 0;
  o->waitlist_head = NULL;
  o->waitlist_tail = NULL;
}

//
// Close object
//

int close_object(struct object *o)
{
  // TODO: release all waitblocks waiting on object

  switch (o->type)
  {
    case OBJECT_THREAD:
      return destroy_thread((struct thread *) o);

    case OBJECT_EVENT:
      kfree(o);
      return 0;

    case OBJECT_TIMER:
      cancel_waitable_timer((struct waitable_timer *) o);
      kfree(o);
      break;

    case OBJECT_MUTEX:
      kfree(o);
      return 0;

    case OBJECT_SEMAPHORE:
      kfree(o);
      return 0;
    
    case OBJECT_FILE:
      return close((struct file *) o);

    case OBJECT_SOCKET:
      return closesocket((struct socket *) o);
  }

  return -EBADF;
}

//
// Initialize thread object
//

void init_thread(struct thread *t, int priority)
{
  memset(t, 0, sizeof(struct thread));
  init_object(&t->object, OBJECT_THREAD);

  t->priority = priority;
  t->state = THREAD_STATE_INITIALIZED;
  t->id = nexttid++;
}

//
// Initialize event object
//

void init_event(struct event *e, int manual_reset, int initial_state)
{
  init_object(&e->object, OBJECT_EVENT);
  e->manual_reset = manual_reset;
  e->object.signaled = initial_state;
}

//
// Pulse event
//

void pulse_event(struct event *e)
{
  struct waitblock *wb;
  struct waitblock *wb_next;

  // Set the event object to the signaled state
  e->object.signaled = 1;

  // Release waiting threads
  wb = e->object.waitlist_head;
  while (wb)
  {
    wb_next = wb->next_wait;
    if (thread_ready_to_run(wb->thread))
    {
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
// Set event
//

void set_event(struct event *e)
{
  struct waitblock *wb;
  struct waitblock *wb_next;

  // Set the event object to the signaled state
  e->object.signaled = 1;

  // Release waiting threads
  wb = e->object.waitlist_head;
  while (wb)
  {
    wb_next = wb->next_wait;
    if (thread_ready_to_run(wb->thread))
    {
      // Release waiting thread
      release_thread(wb->thread);
      
      // Auto reset only releases one thread
      if (!e->manual_reset)
      {
        e->object.signaled = 0;
        break;
      }
    }

    wb = wb_next;
  }
}

//
// Reset event
//

void reset_event(struct event *e)
{
  // Set the event object to the nonsignaled state
  e->object.signaled = 0;
}

//
// Initialize semaphore object
//

void init_sem(struct sem *s, unsigned int initial_count)
{
  init_object(&s->object, OBJECT_SEMAPHORE);
  s->count = initial_count;
  s->object.signaled = (s->count > 0);
}

//
// Release 'count' resouces for the semaphore
//

unsigned int release_sem(struct sem *s, unsigned int count)
{
  unsigned int prevcount;
  struct waitblock *wb;
  struct waitblock *wb_next;

  // Increase semaphore count
  prevcount = s->count;
  s->count += count;

  if (s->count > 0)
  {
    // Set semaphore to signaled state
    s->object.signaled = 1;

    // Release waiting threads
    wb = s->object.waitlist_head;
    while (wb)
    {
      wb_next = wb->next_wait;
      if (thread_ready_to_run(wb->thread))
      {
	// Decrease semaphore count
	s->count--;

	// Release waiting thread
	release_thread(wb->thread);

	// If count reaches zero then set semaphore to the nonsignaled state
	if (s->count == 0)
	{
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
// Set semaphore resource count
//

unsigned int set_sem(struct sem *s, unsigned int count)
{
  unsigned int prevcount;
  struct waitblock *wb;
  struct waitblock *wb_next;

  // Increase semaphore count
  prevcount = s->count;
  s->count = count;

  if (s->count > 0)
  {
    // Set semaphore to signaled state
    s->object.signaled = 1;

    // Release waiting threads
    wb = s->object.waitlist_head;
    while (wb)
    {
      wb_next = wb->next_wait;
      if (thread_ready_to_run(wb->thread))
      {
	// Decrease semaphore count
	s->count--;

	// Release waiting thread
	release_thread(wb->thread);

	// If count reaches zero then set semaphore to the nonsignaled state
	if (s->count == 0)
	{
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
// Initialize mutex object
//

void init_mutex(struct mutex *m, int owned)
{
  init_object(&m->object, OBJECT_MUTEX);
  if (owned)
  {
    m->owner = current_thread();
    m->object.signaled = 0;
    m->recursion = 1;
  }
  else
  {
    m->owner = NULL;
    m->object.signaled = 1;
    m->recursion = 0;
  }
}

//
// Release mutex
//

void release_mutex(struct mutex *m)
{
  struct waitblock *wb;

  // Check that caller is the owner
  if (m->owner != current_thread()) return;

  // Check for recursion
  if (--m->recursion > 0) return;

  // Set the mutex to the signaled state
  m->object.signaled = 1;
  m->owner = NULL;

  // Release first waiting thread
  wb = m->object.waitlist_head;
  while (wb)
  {
    if (thread_ready_to_run(wb->thread)) break;
    wb = wb->next_wait;
  }

  if (wb != NULL)
  {
    // Set mutex to nonsignal state
    m->owner = wb->thread;
    m->recursion = 1;
    m->object.signaled = 0;

    // Release waiting thread
    release_thread(wb->thread);
  }
}

//
// Expire waitable timer
//

static void expire_waitable_timer(void *arg)
{
  struct waitable_timer *t = arg;
  struct waitblock *wb;
  struct waitblock *wb_next;

  // Set signaled state
  t->object.signaled = 1;

  // Release all waiting threads
  wb = t->object.waitlist_head;
  while (wb)
  {
    wb_next = wb->next_wait;
    if (thread_ready_to_run(wb->thread)) release_thread(wb->thread);
    wb = wb_next;
  }
}

//
// Initialize timer
//

void init_waitable_timer(struct waitable_timer *t, unsigned int expires)
{
  init_object(&t->object, OBJECT_TIMER);
  init_timer(&t->timer, expire_waitable_timer, t);
  t->timer.expires = expires;
  
  if (time_before_eq(expires, ticks))
  {
    // Set timer to signaled state immediately
    t->object.signaled = 1;
  }
  else
  {
    // Start timer
    add_timer(&t->timer);
  }
}

//
// Modify timer
//

void modify_waitable_timer(struct waitable_timer *t, unsigned int expires)
{
  mod_timer(&t->timer, expires);
}

//
// Cancel timer
//

void cancel_waitable_timer(struct waitable_timer *t)
{
  if (t->timer.active) del_timer(&t->timer);
}

//
// Sleep for a number of milliseconds
//

void sleep(unsigned int millisecs)
{
  struct waitable_timer timer;

  init_waitable_timer(&timer, ticks + millisecs / MSECS_PER_TICK);
  wait_for_object(&timer, INFINITE);
}

//
// Allocate handle
//

handle_t halloc(struct object *o)
{
  handle_t h;

  // Expand handle table if full
  if (hfreelist == NOHANDLE)
  {
    unsigned long pfn;

    if (htabsize == HTABSIZE / sizeof(struct object *)) return -ENFILE;
    pfn = alloc_pageframe(PFT_SYS);
    map_page(htab + htabsize, pfn, PT_WRITABLE | PT_PRESENT);

    for (h = htabsize + HANDLES_PER_PAGE - 1; h >= htabsize; h--)
    {
      htab[h] = (struct object *) hfreelist;
      hfreelist = h;
    }

    htabsize += HANDLES_PER_PAGE;
  }

  h = hfreelist;
  hfreelist = (handle_t) htab[h];
  htab[h] = o;
  o->handle_count++;

  return h;
}

//
// Free handle
//

int hfree(handle_t h)
{
  struct object *o;

  if (h < 0 || h >= htabsize) return -EBADF;
  o = htab[h];

  if (o == (struct object *) NOHANDLE) return -EBADF;
  if (o < (struct object *) OSBASE) return -EBADF;

  return hrel(h);
}

//
// Release handle
//

int hrel(handle_t h)
{
  struct object *o;
  int rc;

  o = htab[h];
  if (--o->handle_count == 0) 
  {
    rc = close_object(o);
    htab[h] = (struct object *) hfreelist;
    hfreelist = h;
  }
  else
    rc = 0;

  return rc;
}

//
// Lock handle
//

struct object *hlock(handle_t h, int type)
{
  struct object *o;

  if (h < 0 || h >= htabsize) return NULL;
  o = htab[h];

  if (o == (struct object *) NOHANDLE) return NULL;
  if (o < (struct object *) OSBASE) return NULL;
  if (o->type != type && type != OBJECT_ANY) return NULL;
  o->handle_count++;
  return o;
}
