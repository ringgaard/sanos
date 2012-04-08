//
// pit.h
//
// Programmable Interval Timer functions (PIT i8253)
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

#ifndef PIT_H
#define PIT_H

#define TIMER_FREQ      100

#define TICKS_PER_SEC   TIMER_FREQ
#define CLOCKS_PER_SEC  1000

#define HZ TICKS_PER_SEC

#define CLOCKS_PER_TICK (CLOCKS_PER_SEC / TIMER_FREQ)

#define USECS_PER_TICK  (1000000 / TIMER_FREQ)
#define MSECS_PER_TICK  (1000 / TIMER_FREQ)

extern struct timeval systemclock;
extern volatile unsigned int ticks;
extern volatile unsigned int clocks;

krnlapi unsigned int get_ticks();

krnlapi void udelay(unsigned long us);

krnlapi unsigned char read_cmos_reg(int reg);
krnlapi void write_cmos_reg(int reg, unsigned char val);

void init_pit();
void calibrate_delay();

krnlapi time_t get_time();

time_t time(time_t *time);
void set_time(struct timeval *tv);
int load_sysinfo(struct loadinfo *info);

#endif
