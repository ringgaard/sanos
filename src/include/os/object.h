//
// object.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Object manager
//

#ifndef THREAD_H
#define THREAD_H

#define MAX_WAIT_OBJECTS  16

#define WAIT_ALL 0
#define WAIT_ANY 1

#define OBJECT_ANY       -1

#define OBJECT_THREAD     0
#define OBJECT_EVENT      1
#define OBJECT_TIMER      2
#define OBJECT_MUTEX      3
#define OBJECT_SEMAPHORE  4
#define OBJECT_FILE       5
#define OBJECT_SOCKET     6

#define THREAD_STATE_INITIALIZED 0
#define THREAD_STATE_READY       1
#define THREAD_STATE_RUNNING     2
#define THREAD_STATE_WAITING     3
#define THREAD_STATE_TERMINATED  4

#define THREAD_WAIT_OBJECT       0
#define THREAD_WAIT_BUFFER       1
#define THREAD_WAIT_TASK         2
#define THREAD_WAIT_SOCKET       3
#define THREAD_WAIT_SLEEP        4

#define THREAD_FPU_USED          1
#define THREAD_FPU_ENABLED       2

struct thread;
struct waitblock;

typedef void *object_t;

struct object
{
  unsigned short type;
  short signaled;

  int handle_count;

  struct waitblock *waitlist_head;
  struct waitblock *waitlist_tail;
};

struct waitblock
{
  struct thread *thread;
  struct object *object;

  unsigned short waittype;
  short waitkey;

  struct waitblock *next;

  struct waitblock *next_wait;
  struct waitblock *prev_wait;
};

struct event
{
  struct object object;
  int manual_reset;
};

struct sem
{
  struct object object;
  unsigned int count;
};

struct mutex
{
  struct object object;
  struct thread *owner;
  int recursion;
};

struct waitable_timer
{
  struct object object;
  struct timer timer;
};

struct thread
{
  struct object object;

  int state;
  int wait_reason;
  int flags;
  int priority;
  tid_t id;
  handle_t hndl;
  struct tib *tib;
  int suspend_count;
  void *entrypoint;
  int exitcode;
  char *name;
  unsigned long utime;
  unsigned long stime;
  unsigned long context_switches;

  struct thread *next;
  struct thread *prev;

  struct thread *next_ready;
  struct waitblock *waitlist;
  int waitkey;

  struct thread *next_waiter;

  struct fpu *fpustate;
  struct context *ctxt;
  void *uctxt;
};

extern struct object **htab;
extern int htabsize;

void init_objects();

void init_object(struct object *o, int type);
int close_object(struct object *o);

void init_thread(struct thread *t, int priority);

krnlapi void init_event(struct event *e, int manual_reset, int initial_state);
krnlapi void pulse_event(struct event *e);
krnlapi void set_event(struct event *e);
krnlapi void reset_event(struct event *e);

krnlapi void init_sem(struct sem *s, unsigned int initial_count);
krnlapi unsigned int release_sem(struct sem *s, unsigned int count);
krnlapi unsigned int set_sem(struct sem *s, unsigned int count);

krnlapi void init_mutex(struct mutex *m, int owned);
krnlapi void release_mutex(struct mutex *m);

krnlapi void init_waitable_timer(struct waitable_timer *t, unsigned int expires);
krnlapi void modify_waitable_timer(struct waitable_timer *t, unsigned int expires);
krnlapi void cancel_waitable_timer(struct waitable_timer *t);

krnlapi int wait_for_object(object_t hobj, unsigned int timeout);
krnlapi int wait_for_all_objects(struct object **objs, int count, unsigned int timeout);
krnlapi int wait_for_any_object(struct object **objs, int count, unsigned int timeout);

krnlapi handle_t halloc(struct object *o);
krnlapi int hfree(handle_t h);

krnlapi struct object *olock(handle_t h, int type);
krnlapi int orel(object_t hobj);

#endif
