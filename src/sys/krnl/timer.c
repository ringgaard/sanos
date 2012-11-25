//
// timer.c
//
// Timer functions 
// Derived from Finn Arne Gangstad's kernel timer implementation
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

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

struct timer_vec {
  int index;
  struct timer_link vec[TVN_SIZE];
};

struct timer_vec_root {
  int index;
  struct timer_link vec[TVR_SIZE];
};

static unsigned int timer_ticks = 0;

static struct timer_vec tv5;
static struct timer_vec tv4;
static struct timer_vec tv3;
static struct timer_vec tv2;
static struct timer_vec_root tv1;

static struct timer_vec * const tvecs[] = {
  (struct timer_vec *)&tv1, &tv2, &tv3, &tv4, &tv5
};

#define NOOF_TVECS (sizeof(tvecs) / sizeof(tvecs[0]))

//
// attach_timer
//

static void attach_timer(struct timer *timer) {
  unsigned int expires = timer->expires;
  unsigned int idx = expires - timer_ticks;
  struct timer_link *vec;

  if (idx < TVR_SIZE) {
    int i = expires & TVR_MASK;
    vec = tv1.vec + i;
  } else if (idx < 1 << (TVR_BITS + TVN_BITS)) {
    int i = (expires >> TVR_BITS) & TVN_MASK;
    vec = tv2.vec + i;
  } else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) {
    int i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
    vec =  tv3.vec + i;
  } else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) {
    int i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
    vec = tv4.vec + i;
  } else if ((signed long) idx < 0) {
    // Can happen if you add a timer with expires == timer_ticks, 
    // or you set a timer to go off in the past
    vec = tv1.vec + tv1.index;
  } else {
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

static int detach_timer(struct timer *timer) {
  if (!timer->active) return 0;

  timer->link.next->prev = timer->link.prev;
  timer->link.prev->next = timer->link.next;
  timer->active = 0;

  return 1;
}

//
// init_timers
//

void init_timers() {
  int i;

  for (i = 0; i < TVN_SIZE; i++)  {
    tv5.vec[i].next = tv5.vec[i].prev = tv5.vec + i;
    tv4.vec[i].next = tv4.vec[i].prev = tv4.vec + i;
    tv3.vec[i].next = tv3.vec[i].prev = tv3.vec + i;
    tv2.vec[i].next = tv2.vec[i].prev = tv2.vec + i;
  }
  for (i = 0; i < TVR_SIZE; i++) {
    tv1.vec[i].next = tv1.vec[i].prev = tv1.vec + i;
  }
}

//
// init_timer
//

void init_timer(struct timer *timer, timerproc_t handler, void *arg) {
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

void add_timer(struct timer *timer) {
  if (timer->active) {
    kprintf("timer: timer is already active\n");
    return;
  }

  attach_timer(timer);
}

//
// del_timer
//

int del_timer(struct timer *timer) {
  int rc;

  rc = detach_timer(timer);
  timer->link.next = NULL;
  timer->link.prev = NULL;

  return rc;
}

//
// mod_timer
//

int mod_timer(struct timer *timer, unsigned int expires) {
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

static void cascade_timers(struct timer_vec *tv) {
  struct timer_link *head, *curr, *next;

  head = tv->vec + tv->index;
  curr = head->next;

  while (curr != head) {
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

void run_timer_list() {
  while ((long) (ticks - timer_ticks) >= 0) {
    struct timer_link *head, *curr;

    if (!tv1.index) {
      int n = 1;
      do { cascade_timers(tvecs[n]); } while (tvecs[n]->index == 1 && ++n < NOOF_TVECS);
    }

    while (1) {
      struct timer *timer;
      timerproc_t handler;
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

//
// tmr_sleep
//
// End sleep state for thread
//

static void tmr_sleep(void *arg) {
  struct thread *t = arg;
  mark_thread_ready(t, 1, 0);
}

//
// msleep
//
// Sleep for a number of milliseconds
//

int msleep(unsigned int millisecs) {
  struct timer timer;
  int rc;

  if (millisecs == 0) {
    yield();
    rc = 0;
  } else {
    init_timer(&timer, tmr_sleep, self());
    timer.expires = ticks + millisecs / MSECS_PER_TICK;
    add_timer(&timer);
    rc = enter_alertable_wait(THREAD_WAIT_SLEEP);
    if (rc == -EINTR) {
      rc = (timer.expires - ticks) * MSECS_PER_TICK;
      if (rc < 0) rc = 0;
    }
    del_timer(&timer);
  }

  return rc;
}
