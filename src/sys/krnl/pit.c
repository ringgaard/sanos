//
// pit.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// i8253 Programmable Interval Timer functions
//

#include <os/krnl.h>

#define TMR_CTRL	0x43	// I/O for control
#define	TMR_CNT0	0x40	// I/O for counter 0
#define	TMR_CNT1	0x41	// I/O for counter 1
#define	TMR_CNT2	0x42	// I/O for counter 2

#define	TMR_SC0		0	// Select channel 0
#define	TMR_SC1		0x40	// Select channel 1
#define	TMR_SC2		0x80	// Select channel 2

#define	TMR_LOW		0x10	// RW low byte only
#define	TMR_HIGH	0x20	// RW high byte only
#define	TMR_BOTH	0x30	// RW both bytes

#define	TMR_MD0		0	// Mode 0
#define	TMR_MD1		0x2	// Mode 1
#define	TMR_MD2		0x4	// Mode 2
#define	TMR_MD3		0x6	// Mode 3
#define	TMR_MD4		0x8	// Mode 4
#define	TMR_MD5		0xA	// Mode 5

#define	TMR_BCD		1	// BCD mode

#define	TMR_LATCH	0	// Latch command

#define	TMR_READ	0xF0	// Read command
#define	TMR_CNT		0x20	// CNT bit  (Active low, subtract it)
#define	TMR_STAT	0x10	// Status bit  (Active low, subtract it)

#define	TMR_CH0		0x00	// Channel 0 bit
#define	TMR_CH1		0x40	// Channel 1 bit
#define	TMR_CH2		0x80	// Channel 2 bit

#define PIT_CLOCK       1193180
#define TIMER_FREQ      100

#define USECS_PER_TICK  (1000000 / TIMER_FREQ)
#define MSECS_PER_TICK  (1000 / TIMER_FREQ)

volatile unsigned int ticks = 0;
struct timeval systemclock = { 0, 0 };
time_t upsince;
unsigned long cycles_per_tick;
unsigned long loops_per_tick;

struct dpc timerdpc;

void timer_dpc(void *arg)
{
  handle_timer_expiry(ticks);
}

void timer_handler(struct context *ctxt, void *arg)
{
  // Update timer tick count
  ticks += MSECS_PER_TICK;

  // Update system clock
  systemclock.tv_usec += USECS_PER_TICK;
  while (systemclock.tv_usec >= 1000000) 
  {
    systemclock.tv_sec++;
    systemclock.tv_usec -= 1000000;
  }

  // Queue timer DPC
  queue_irq_dpc(&timerdpc, timer_dpc, NULL);

  eoi(IRQ_TMR);
}

unsigned char read_cmos_reg(int reg)
{
  unsigned char val;

  _outp(0x70, reg);
  val = _inp(0x71) & 0xFF;

  return val;
}

void write_cmos_reg(int reg, unsigned char val)
{
  _outp(0x70, reg);
  _outp(0x71, val);
}

static int read_bcd_cmos_reg(int reg)
{
  int val;

  val = read_cmos_reg(reg);
  return (val >> 4) * 10 + (val & 0x0F);
}

static void get_cmos_time(struct tm *tm)
{
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

__inline long __declspec(naked) rdtscl()
{
  __asm { rdtsc }
  __asm { ret }
}

static void tsc_delay(unsigned long cycles)
{
  long start, now;
    
  start = rdtscl();
  do 
  {
    __asm { nop };
    now = rdtscl();
  } while ((start + cycles) - now < 0);
}

static void timed_delay(unsigned long loops)
{
  __asm
  {
    delay_loop:
      dec loops
      jns delay_loop; 
  }
}

void calibrate_delay()
{
  static int cpu_speeds[] = 
  {
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

  if (cpu.features & CPU_FEATURE_TSC)
  {
    unsigned long start;
    unsigned long end;

    t = ticks;
    while (t == ticks);
    start = rdtscl();

    t = ticks;
    while (t == ticks);
    end = rdtscl();

    cycles_per_tick = end - start;
  }
  else
  {
    // Determine magnitude of loops_per_tick
    loops_per_tick = 1 << 12;
    while (loops_per_tick <<= 1)
    {
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
    while (precision-- && (bit >>= 1))
    {
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

  if (mhz > 1275)
    mhz = ((mhz + 25) / 50) * 50;
  else if (mhz > 14)
  {
    int *speed = cpu_speeds;
    int i;
    unsigned long bestmhz = 0;

    for (i = 0; i < sizeof(cpu_speeds) / sizeof(int); i++)
    {
      if (abs(mhz - bestmhz) > abs(mhz - *speed)) bestmhz = *speed;
      speed++;
    }

    mhz = bestmhz;
  }

  kprintf("speed: %d cycles/tick, %d MHz processor\n", cycles_per_tick, mhz);
  cpu.mhz = mhz;
}

void init_pit()
{
  struct tm tm;

  unsigned int cnt = PIT_CLOCK / TIMER_FREQ;
  _outp(TMR_CTRL, TMR_CH0 + TMR_BOTH + TMR_MD3);
  _outp(TMR_CNT0, (unsigned char) (cnt & 0xFF));
  _outp(TMR_CNT0, (unsigned char) (cnt >> 8));

  get_cmos_time(&tm);
  systemclock.tv_sec = mktime(&tm);
  upsince = systemclock.tv_sec;

  init_dpc(&timerdpc);
  set_interrupt_handler(INTR_TMR, timer_handler, NULL);
  enable_irq(IRQ_TMR);
}

void usleep(unsigned long us)
{
  if (cpu.features & CPU_FEATURE_TSC)
    tsc_delay(us * cycles_per_tick / (1000000 / TIMER_FREQ));
  else
    timed_delay(us * loops_per_tick / (1000000 / TIMER_FREQ));
}

unsigned int get_tick_count()
{
  return ticks;
}

time_t get_time()
{
  return systemclock.tv_sec;
}

time_t time(time_t *time)
{
  if (time)
  {
    *time = get_time();
    return *time;
  }
  else
    return get_time();
}
