//
// pit.c
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

#include <os/krnl.h>

#define TMR_CTRL        0x43    // I/O for control
#define TMR_CNT0        0x40    // I/O for counter 0
#define TMR_CNT1        0x41    // I/O for counter 1
#define TMR_CNT2        0x42    // I/O for counter 2

#define TMR_SC0         0       // Select channel 0
#define TMR_SC1         0x40    // Select channel 1
#define TMR_SC2         0x80    // Select channel 2

#define TMR_LOW         0x10    // RW low byte only
#define TMR_HIGH        0x20    // RW high byte only
#define TMR_BOTH        0x30    // RW both bytes

#define TMR_MD0         0       // Mode 0
#define TMR_MD1         0x2     // Mode 1
#define TMR_MD2         0x4     // Mode 2
#define TMR_MD3         0x6     // Mode 3
#define TMR_MD4         0x8     // Mode 4
#define TMR_MD5         0xA     // Mode 5

#define TMR_BCD         1       // BCD mode

#define TMR_LATCH       0       // Latch command

#define TMR_READ        0xF0    // Read command
#define TMR_CNT         0x20    // CNT bit  (Active low, subtract it)
#define TMR_STAT        0x10    // Status bit  (Active low, subtract it)

#define TMR_CH0         0x00    // Channel 0 bit
#define TMR_CH1         0x40    // Channel 1 bit
#define TMR_CH2         0x80    // Channel 2 bit

#define PIT_CLOCK       1193180
#define TIMER_FREQ      100

#define USECS_PER_TICK  (1000000 / TIMER_FREQ)
#define MSECS_PER_TICK  (1000 / TIMER_FREQ)

#define LOADTAB_SIZE        TIMER_FREQ

#define LOADTYPE_IDLE       0
#define LOADTYPE_USER       1
#define LOADTYPE_KERNEL     2
#define LOADTYPE_DPC        3

volatile unsigned int ticks = 0;
volatile unsigned int clocks = 0;
struct timeval systemclock = { 0, 0 };
time_t upsince;
unsigned long cycles_per_tick;
unsigned long loops_per_tick;

unsigned char loadtab[LOADTAB_SIZE];
unsigned char *loadptr;
unsigned char *loadend;

struct interrupt timerintr;
struct dpc timerdpc;

void timer_dpc(void *arg) {
  run_timer_list();
}

int timer_handler(struct context *ctxt, void *arg) {
  struct thread *t;

  // Update timer clock
  clocks += CLOCKS_PER_TICK;

  // Update tick counter
  ticks++;

  // Update system clock
  systemclock.tv_usec += USECS_PER_TICK;
  while (systemclock.tv_usec >= 1000000)  {
    systemclock.tv_sec++;
    systemclock.tv_usec -= 1000000;
  }

  // Update thread times and load average
  t = self();
  if (in_dpc) {
    dpc_time++;
    *loadptr = LOADTYPE_DPC;
  } else {
    if (USERSPACE(ctxt->eip)) {
      t->utime++;
      *loadptr = LOADTYPE_USER;
    } else {
      t->stime++;
      if (t->base_priority == PRIORITY_SYSIDLE) {
        *loadptr = LOADTYPE_IDLE;
      } else {
        *loadptr = LOADTYPE_KERNEL;
      }
    }
  }

  if (++loadptr == loadend) loadptr = loadtab;

  // Adjust thread quantum
  t->quantum -= QUANTUM_UNITS_PER_TICK;
  if (t->quantum <= 0) preempt = 1;

  // Queue timer DPC
  queue_irq_dpc(&timerdpc, timer_dpc, NULL);

  eoi(IRQ_TMR);
  return 0;
}

unsigned char read_cmos_reg(int reg) {
  unsigned char val;

  outp(0x70, reg);
  val = inp(0x71) & 0xFF;

  return val;
}

void write_cmos_reg(int reg, unsigned char val) {
  outp(0x70, reg);
  outp(0x71, val);
}

static int read_bcd_cmos_reg(int reg) {
  int val;

  val = read_cmos_reg(reg);
  return (val >> 4) * 10 + (val & 0x0F);
}

static void write_bcd_cmos_reg(int reg, unsigned char val) {
  write_cmos_reg(reg, (unsigned char) (((val / 10) << 4) + (val % 10)));
}

static void get_cmos_time(struct tm *tm) {
  tm->tm_year = read_bcd_cmos_reg(0x09) + (read_bcd_cmos_reg(0x32) * 100) - 1900;
  tm->tm_mon = read_bcd_cmos_reg(0x08) - 1;
  tm->tm_mday = read_bcd_cmos_reg(0x07);
  tm->tm_hour = read_bcd_cmos_reg(0x04);
  tm->tm_min = read_bcd_cmos_reg(0x02);
  tm->tm_sec = read_bcd_cmos_reg(0x00);

  tm->tm_wday = 0;
  tm->tm_yday = 0;
  tm->tm_isdst = 0;
}

static void set_cmos_time(struct tm *tm) {
  write_bcd_cmos_reg(0x09, (unsigned char) (tm->tm_year % 100));
  write_bcd_cmos_reg(0x32, (unsigned char) ((tm->tm_year + 1900) / 100));
  write_bcd_cmos_reg(0x08, (unsigned char) (tm->tm_mon + 1));
  write_bcd_cmos_reg(0x07, (unsigned char) (tm->tm_mday));
  write_bcd_cmos_reg(0x04, (unsigned char) (tm->tm_hour));
  write_bcd_cmos_reg(0x02, (unsigned char) (tm->tm_min));
  write_bcd_cmos_reg(0x00, (unsigned char) (tm->tm_sec));
}

static void tsc_delay(unsigned long cycles) {
  long end, now;

  end = (unsigned long) rdtsc() + cycles;
  do  {
    __asm { nop };
    now = (unsigned long) rdtsc();
  } while (end - now > 0);
}

static void timed_delay(unsigned long loops) {
  __asm {
    delay_loop:
      dec loops
      jns delay_loop; 
  }
}

void calibrate_delay() {
  static int cpu_speeds[] =  {
    16, 20, 25, 33, 40, 50, 60, 66, 75, 80, 90, 
    100, 110, 120, 133, 150, 166, 180, 188,
    200, 233, 250, 266,
    300, 333, 350, 366, 
    400, 433, 450, 466, 
    500, 533, 550, 566, 
    600, 633, 650, 667, 
    700, 733, 750, 766, 
    800, 833, 850, 866,
    900, 933, 950, 966,
    1000, 1130, 1200, 1260
  };

  unsigned long mhz;
  unsigned long t, bit;
  int precision;

  if (cpu.features & CPU_FEATURE_TSC) {
    unsigned long start;
    unsigned long end;

    t = ticks;
    while (t == ticks);
    start = (unsigned long) rdtsc();

    t = ticks;
    while (t == ticks);
    end = (unsigned long) rdtsc();

    cycles_per_tick = end - start;
  } else {
    // Determine magnitude of loops_per_tick
    loops_per_tick = 1 << 12;
    while (loops_per_tick <<= 1) {
      t = ticks;
      while (t == ticks);
    
      t = ticks;
      timed_delay(loops_per_tick);
      if (t != ticks) break;
    }

    // Do a binary approximation of cycles_per_tick
    precision = 8;
    loops_per_tick >>= 1;
    bit = loops_per_tick;
    while (precision-- && (bit >>= 1)) {
      loops_per_tick |= bit;
      t = ticks;
      while (t == ticks);
      t = ticks;
      timed_delay(loops_per_tick);
      if (ticks != t) loops_per_tick &= ~bit; // Longer than 1 tick
    }

    // Each loop takes 10 cycles
    cycles_per_tick = 10 * loops_per_tick;
  }

  mhz = cycles_per_tick * TIMER_FREQ / 1000000;

  if (mhz > 1275) {
    mhz = ((mhz + 25) / 50) * 50;
  } else if (mhz > 14) {
    int *speed = cpu_speeds;
    int i;
    unsigned long bestmhz = 0;

    for (i = 0; i < sizeof(cpu_speeds) / sizeof(int); i++) {
      int curdiff = mhz - bestmhz;
      int newdiff = mhz - *speed;

      if (curdiff < 0) curdiff = -curdiff;
      if (newdiff < 0) newdiff = -newdiff;

      if (newdiff < curdiff) bestmhz = *speed;
      speed++;
    }

    mhz = bestmhz;
  }

  kprintf(KERN_INFO "speed: %d cycles/tick, %d MHz processor\n", cycles_per_tick, mhz);
  cpu.mhz = mhz;
}

static int uptime_proc(struct proc_file *pf, void *arg) {
  int days, hours, mins, secs;
  int uptime = get_time() - upsince;

  days = uptime / (24 * 60 * 60);
  uptime -= days * (24 * 60 * 60);
  hours = uptime / (60 * 60);
  uptime -= hours * (60 * 60);
  mins = uptime / 60;
  secs = uptime % 60;

  pprintf(pf, "%d day%s %d hour%s %d minute%s %d second%s\n",
    days, days == 1 ? "" : "s",
    hours, hours == 1 ? "" : "s",
    mins, mins == 1 ? "" : "s",
    secs, secs == 1 ? "" : "s");

  return 0;
}

static int loadavg_proc(struct proc_file *pf, void *arg) {
  int loadavg[4];
  unsigned char *p;

  memset(loadavg, 0, sizeof loadavg);
  p = loadtab;
  while (p < loadend) loadavg[*p++]++;

  pprintf(pf, "Load kernel: %d%% user: %d%% dpc: %d%% idle: %d%%\n",
    loadavg[LOADTYPE_KERNEL],
    loadavg[LOADTYPE_USER],
    loadavg[LOADTYPE_DPC],
    loadavg[LOADTYPE_IDLE]);

  return 0;
}

void init_pit() {
  struct tm tm;

  unsigned int cnt = PIT_CLOCK / TIMER_FREQ;
  outp(TMR_CTRL, TMR_CH0 + TMR_BOTH + TMR_MD3);
  outp(TMR_CNT0, (unsigned char) (cnt & 0xFF));
  outp(TMR_CNT0, (unsigned char) (cnt >> 8));

  loadptr = loadtab;
  loadend = loadtab + LOADTAB_SIZE;

  get_cmos_time(&tm);
  systemclock.tv_sec = mktime(&tm);
  upsince = systemclock.tv_sec;

  init_dpc(&timerdpc);
  timerdpc.flags |= DPC_NORAND; // Timer tick is a bad source for randomness
  register_interrupt(&timerintr, INTR_TMR, timer_handler, NULL);
  enable_irq(IRQ_TMR);

  register_proc_inode("uptime", uptime_proc, NULL);
  register_proc_inode("loadavg", loadavg_proc, NULL);
}

void udelay(unsigned long us) {
  if (cpu.features & CPU_FEATURE_TSC) {
    tsc_delay(us * (cycles_per_tick / (1000000 / TIMER_FREQ)));
  } else {
    timed_delay(us * (loops_per_tick / (1000000 / TIMER_FREQ)));
  }
}

unsigned int get_ticks() {
  return ticks;
}

time_t get_time() {
  return systemclock.tv_sec;
}

time_t time(time_t *time) {
  if (time) {
    *time = get_time();
    return *time;
  } else {
    return get_time();
  }
}

void set_time(struct timeval *tv) {
  struct tm tm;

  upsince += (tv->tv_sec - systemclock.tv_sec);
  systemclock.tv_usec = tv->tv_usec;
  systemclock.tv_sec = tv->tv_sec;
  gmtime_r(&tv->tv_sec, &tm);
  set_cmos_time(&tm);
}

int load_sysinfo(struct loadinfo *info) {
  int loadavg[4];
  unsigned char *p;

  memset(loadavg, 0, sizeof loadavg);
  p = loadtab;
  while (p < loadend) loadavg[*p++]++;

  info->uptime = get_time() - upsince;
  info->load_user = loadavg[LOADTYPE_USER];
  info->load_system = loadavg[LOADTYPE_KERNEL];
  info->load_intr = loadavg[LOADTYPE_DPC];
  info->load_idle = loadavg[LOADTYPE_IDLE];

  return 0;
}
