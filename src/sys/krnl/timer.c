//
// timer.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Timer functions
//

#include <os/krnl.h>

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

struct timer_vec 
{
  int index;
  struct timer_link vec[TVN_SIZE];
};

struct timer_vec_root 
{
  int index;
  struct timer_link vec[TVR_SIZE];
};

static unsigned int timer_ticks;

static struct timer_vec tv5;
static struct timer_vec tv4;
static struct timer_vec tv3;
static struct timer_vec tv2;
static struct timer_vec_root tv1;

static struct timer_vec * const tvecs[] = 
{
  (struct timer_vec *)&tv1, &tv2, &tv3, &tv4, &tv5
};

#define NOOF_TVECS (sizeof(tvecs) / sizeof(tvecs[0]))

//
// attach_timer
//

static void attach_timer(struct timer *timer)
{
  unsigned int expires = timer->expires;
  unsigned int idx = expires - timer_ticks;
  struct timer_link *vec;

  if (idx < TVR_SIZE) 
  {
    int i = expires & TVR_MASK;
    vec = tv1.vec + i;
  } 
  else if (idx < 1 << (TVR_BITS + TVN_BITS)) 
  {
    int i = (expires >> TVR_BITS) & TVN_MASK;
    vec = tv2.vec + i;
  } 
  else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) 
  {
    int i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
    vec =  tv3.vec + i;
  } 
  else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) 
  {
    int i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
    vec = tv4.vec + i;
  } 
  else if ((signed long) idx < 0) 
  {
    // Can happen if you add a timer with expires == timer_ticks, 
    // or you set a timer to go off in the past
    vec = tv1.vec + tv1.index;
  } 
  else
  {
    int i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
    vec = tv5.vec + i;
  } 

  timer->link.next = vec;
  timer->link.prev = vec->prev;
  vec->prev->next = &timer->link;
  vec->prev = &timer->link;

  timer->active = 1;
}

//
// detach_timer
//

static int detach_timer(struct timer *timer)
{
  if (!timer->active) return 0;

  timer->link.next->prev = timer->link.prev;
  timer->link.prev->next = timer->link.next;
  timer->active = 0;

  return 1;
}

//
// init_timers
//

void init_timers()
{
  int i;

  for (i = 0; i < TVN_SIZE; i++) 
  {
    tv5.vec[i].next = tv5.vec[i].prev = tv5.vec + i;
    tv4.vec[i].next = tv4.vec[i].prev = tv4.vec + i;
    tv3.vec[i].next = tv3.vec[i].prev = tv3.vec + i;
    tv2.vec[i].next = tv2.vec[i].prev = tv2.vec + i;
  }
  for (i = 0; i < TVR_SIZE; i++)
  {
    tv1.vec[i].next = tv1.vec[i].prev = tv1.vec + i;
  }
}

//
// init_timer
//

void init_timer(struct timer *timer, void (*handler)(void *arg), void *arg)
{
  timer->link.next = NULL;
  timer->link.next = NULL;
  timer->expires = 0;
  timer->active = 0;
  timer->handler = handler;
  timer->arg = arg;
}

//
// add_timer
//

void add_timer(struct timer *timer)
{
  if (timer->active) 
  {
    kprintf("timer: timer is already active\n");
    return;
  }

  attach_timer(timer);
}

//
// del_timer
//

int del_timer(struct timer *timer)
{
  int rc;

  rc = detach_timer(timer);
  timer->link.next = NULL;
  timer->link.prev = NULL;

  return rc;
}

//
// mod_timer
//

int mod_timer(struct timer *timer, unsigned int expires)
{
  int rc;

  timer->expires = expires;
  rc = detach_timer(timer);
  attach_timer(timer);

  return rc;
}

//
// cascade_timers
//
// Cascade all the timers from tv up one level. We are removing 
// all timers from the list, so we don't  have to detach them 
// individually, just clear the list afterwards.
//

static void cascade_timers(struct timer_vec *tv)
{
  struct timer_link *head, *curr, *next;

  head = tv->vec + tv->index;
  curr = head->next;

  while (curr != head) 
  {
    struct timer *timer;

    timer = (struct timer *) curr;

    next = curr->next;
    attach_timer(timer);
    curr = next;
  }

  head->next = head->prev = head;
  tv->index = (tv->index + 1) & TVN_MASK;
}

//
// run_timer_list
//

void run_timer_list()
{
  while ((long) (ticks - timer_ticks) >= 0) 
  {
    struct timer_link *head, *curr;

    if (!tv1.index) 
    {
      int n = 1;
      do { cascade_timers(tvecs[n]); } while (tvecs[n]->index == 1 && ++n < NOOF_TVECS);
    }

    while (1)
    {
      struct timer *timer;
      void (*handler)(void *arg);
      void *arg;

      head = tv1.vec + tv1.index;
      curr = head->next;
      if (curr == head) break;

      timer = (struct timer *) curr;
      handler = timer->handler;
      arg = timer->arg;

      detach_timer(timer);
      timer->link.next = timer->link.prev = NULL;

      handler(arg);
    }

    timer_ticks++;
    tv1.index = (tv1.index + 1) & TVR_MASK;
  }
}
