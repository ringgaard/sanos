//
// pit.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Programmable Interval Timer functions (PIT i8253)
//

#ifndef PIT_H
#define PIT_H

#define TIMER_FREQ      100

#define TICKS_PER_SEC   TIMER_FREQ
#define CLOCKS_PER_SEC  1000

#define CLOCKS_PER_TICK (CLOCKS_PER_SEC / TIMER_FREQ)

#define USECS_PER_TICK  (1000000 / TIMER_FREQ)
#define MSECS_PER_TICK  (1000 / TIMER_FREQ)

krnlapi extern struct timeval systemclock;
krnlapi extern volatile unsigned int ticks;
krnlapi extern volatile unsigned int clocks;

__inline unsigned int get_tick_count()
{
  return ticks;
}

krnlapi void usleep(unsigned long us);

krnlapi unsigned char read_cmos_reg(int reg);
krnlapi void write_cmos_reg(int reg, unsigned char val);

void init_pit();
void calibrate_delay();

krnlapi time_t get_time();

time_t time(time_t *time);
void set_time(struct timeval *tv);

#endif
