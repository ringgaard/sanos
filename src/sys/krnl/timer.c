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
  struct timer *vec[TVN_SIZE];
};

struct timer_vec_root 
{
  int index;
  struct timer *vec[TVR_SIZE];
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

static void attach_timer(struct timer *timer)
{
  unsigned int expires = timer->expires;
  unsigned int idx = expires - timer_ticks;
  struct timer **vec;

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

  if (*vec)
  {
    timer->prev = (*vec)->prev;
    timer->next = (*vec);
    (*vec)->prev->next = timer;
    (*vec)->prev = timer;
  }
  else
  {
    *vec = timer;
    timer->next = timer->prev = timer;
  }

  timer->active = 1;
}
