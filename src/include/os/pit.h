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

krnlapi extern struct timeval systemclock;

void init_pit();

krnlapi unsigned int get_tick_count();
krnlapi time_t get_time();

time_t time(time_t *time);

#endif
