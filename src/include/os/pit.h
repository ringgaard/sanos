//
// pit.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// i8259 Programmable Interrupt Controller functions
//

#ifndef PIT_H
#define PIT_H

#define TICKS_PER_SEC 1000

#define time_after(a, b)     ((long) (b) - (long) (a) < 0)
#define time_before(a, b)    time_after(b, a)

#define time_after_eq(a, b)  ((long) (a) - (long) (b) >= 0)
#define time_before_eq(a ,b) time_after_eq(b, a)

krnlapi extern struct timeval systemclock;

krnlapi void usleep(unsigned long us);

krnlapi unsigned char read_cmos_reg(int reg);
krnlapi void write_cmos_reg(int reg, unsigned char val);

void init_pit();
void calibrate_delay();

krnlapi unsigned int get_tick_count();
krnlapi time_t get_time();

time_t time(time_t *time);


#endif
