//
// serial.c
//
// RS-232 serial driver
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

#define QUEUE_SIZE      4096
#define ISR_LIMIT       256

//
// UART registers
//

#define UART_RX         0       // Receive buffer
#define UART_TX         0       // Transmit buffer
#define UART_DLL        0       // Divisor Latch Low

#define UART_DLH        1       // Divisor Latch High
#define UART_IER        1       // Interrupt Enable Register

#define UART_IIR        2       // Interrupt ID Register
#define UART_FCR        2       // FIFO Control Register

#define UART_LCR        3       // Line Control Register
#define UART_MCR        4       // Modem Control Register
#define UART_LSR        5       // Line Status Register
#define UART_MSR        6       // Modem Status Register
#define UART_SCR        7       // Scratch Register

//
// Interrupt Enable Register
//

#define IER_ERXRDY      0x01
#define IER_ETXRDY      0x02
#define IER_ERLS        0x04
#define IER_EMSC        0x08

//
// Interrupt Identification Register
//

#define IIR_IMASK       0x0F
#define IIR_NOPEND      0x01
#define IIR_MLSC        0x00
#define IIR_TXRDY       0x02
#define IIR_RXRDY       0x04
#define IIR_RLS         0x06
#define IIR_RXTOUT      0x0C
#define IIR_FIFO_MASK   0xC0    // Set if FIFOs are enabled

//
// FIFO Control Register
//

#define FCR_ENABLE      0x01
#define FCR_RCV_RST     0x02
#define FCR_XMT_RST     0x04
#define FCR_DMA_MODE    0x08
#define FCR_TRIGGER_1   0x00
#define FCR_TRIGGER_4   0x40
#define FCR_TRIGGER_8   0x80
#define FCR_TRIGGER_14  0xC0

//
// Line Control Register
//

#define LCR_DLAB        0x80
#define LCR_SBREAK      0x40
#define LCR_PZERO       0x30
#define LCR_PONE        0x20
#define LCR_PEVEN       0x10
#define LCR_PODD        0x00
#define LCR_PENAB       0x08
#define LCR_STOPB       0x04
#define LCR_8BITS       0x03
#define LCR_7BITS       0x02
#define LCR_6BITS       0x01
#define LCR_5BITS       0x00

//
// Modem Control Register
//

#define MCR_LOOPBACK    0x10
#define MCR_IENABLE     0x08
#define MCR_DRS         0x04
#define MCR_RTS         0x02
#define MCR_DTR         0x01

//
// Line Status Register
//

#define LSR_RCV_FIFO    0x80
#define LSR_TSRE        0x40
#define LSR_TXRDY       0x20
#define LSR_BI          0x10
#define LSR_FE          0x08
#define LSR_PE          0x04
#define LSR_OE          0x02
#define LSR_RXRDY       0x01
#define LSR_RCV_MASK    0x1F

//
// Modem Status Register
//

#define MSR_DCD         0x80
#define MSR_RI          0x40
#define MSR_DSR         0x20
#define MSR_CTS         0x10
#define MSR_DDCD        0x08
#define MSR_TERI        0x04
#define MSR_DDSR        0x02
#define MSR_DCTS        0x01

//
// UART types
//

#define UART_UNKNOWN    0
#define UART_8250       1
#define UART_16450      2
#define UART_16550      3
#define UART_16550A     4

static char *uart_name[] = {"(unknown)", "8250", "16450", "16550", "16550A"};
static int serial_default_irq[4] = {4, 3, 11, 10};
static int serial_default_iobase[4] = {0x3F8, 0x2F8, 0x3E8, 0x2E8};

int next_serial_portno = 1;

struct fifo {
  unsigned char queue[QUEUE_SIZE];
  int count;
  int head;
  int tail;
};

struct serial_port {
  int iobase;                            // I/O base address
  int irq;                               // Interrupt number
  int type;                              // UART type
  struct serial_config cfg;              // Configuration

  struct fifo rxq;                       // Receive queue
  struct sem rx_sem;                     // Receive queue semaphore
  struct mutex rx_lock;                  // Receiver lock
  int rx_queue_rel;                      // Receiver queue release count

  struct fifo txq;                       // Transmit queue
  struct sem tx_sem;                     // Transmit queue semaphore
  int tx_queue_rel;                      // Transmit queue release count
  struct mutex tx_lock;                  // Transmit lock
  int tx_busy;                           // Transmitter busy

  struct interrupt intr;                 // Serial interrupt object
  struct dpc dpc;                        // Serial DPC
  struct event event;                    // Line or modem status event
  int mlsc;                              // Model line status changed
  int rls;                               // Receiver line status changed
  int linestatus;                        // Line status
  
  unsigned char mcr;                     // Modem control register
  unsigned char msr;                     // Modem status register
};

void serial_dpc(void *arg);
static void drain_tx_queue(struct serial_port *sp);

static void fifo_clear(struct fifo *f) {
  f->head = 0;
  f->tail = 0;
  f->count = 0;
}

__inline static int fifo_put(struct fifo *f, unsigned char c) {
  if (f->count >= QUEUE_SIZE) return -ENOSPC;
  f->queue[f->head++] = c;
  f->count++;
  if (f->head >= QUEUE_SIZE) f->head = 0;

  return 0;
}

__inline static unsigned char fifo_get(struct fifo *f) {
  unsigned char c;

  if (f->count == 0) return 0;
  c = f->queue[f->tail++];
  f->count--;
  if (f->tail == QUEUE_SIZE) f->tail = 0;

  return c;
}

__inline static int fifo_empty(struct fifo *f) {
  return f->count == 0;
}

__inline static int fifo_full(struct fifo *f) {
  return f->count == QUEUE_SIZE;
}

static void check_uart_type(struct serial_port *sp) {
  unsigned char b;
  unsigned char t1;
  unsigned char t2;
  unsigned char t3;

  // Look to see if we can find any sort of FIFO response
  outp(sp->iobase + UART_FCR, FCR_ENABLE);
  b = (inp((unsigned short) (sp->iobase + UART_IIR)) & IIR_FIFO_MASK) >> 6;

  switch(b) {
    case 0:
      // No FIFO response is a 16450 or 8250.  The 8250
      // doesn't have a scratchpad register though.  We
      // make this test attempt to restore the original
      // scratchpad state
      sp->type = UART_16450;
      t3 = inp((unsigned short) (sp->iobase + UART_SCR));
      outp(sp->iobase + UART_SCR, 0xA5);
      t1 = inp((unsigned short) (sp->iobase + UART_SCR));
      outp(sp->iobase + UART_SCR, 0x5A);
      t2 = inp((unsigned short) (sp->iobase + UART_SCR));
      outp(sp->iobase + UART_SCR, t3);
      if (t1 != 0xA5 || t2 != 0x5A) sp->type = UART_8250;
      break;

    case 1:
      sp->type = UART_UNKNOWN;
      break;

    case 2:
      // This is the sort of broken response we get from an
      // early 16550 part with a broken FIFO
      sp->type = UART_16550;
      break;

    case 3:
      // We have a 16550A - working FIFOs
      sp->type = UART_16550A;
      break;
  }
}

static void serial_config(struct serial_port *sp) {
  int divisor;
  unsigned char lcr;

  // Set baudrate
  if (sp->cfg.speed == 0) {
    divisor = (1843200 / (16 * 9600));
  } else {
    divisor = (1843200 / (16 * sp->cfg.speed));
  }

  outp(sp->iobase + UART_LCR, LCR_DLAB);
  outp(sp->iobase + UART_DLH, (divisor >> 8) & 0xFF);
  outp(sp->iobase + UART_DLL, divisor & 0xFF);

  // Set line control register
  lcr = 0;

  switch (sp->cfg.parity) {
    case PARITY_NONE:  lcr |= 0; break;
    case PARITY_EVEN:  lcr |= LCR_PEVEN; break;
    case PARITY_ODD:   lcr |= LCR_PODD; break;
    case PARITY_MARK:  lcr |= LCR_PONE; break;
    case PARITY_SPACE: lcr |= LCR_PZERO; break;
  }

  switch (sp->cfg.stopbits) {
    case 1: lcr |= 0; break;
    case 2: lcr |= LCR_STOPB; break;
  }

  switch (sp->cfg.databits) {
    case 5: lcr |= LCR_5BITS; break;
    case 6: lcr |= LCR_6BITS; break;
    case 7: lcr |= LCR_7BITS; break;
    case 8: lcr |= LCR_8BITS; break;
  }

  outp(sp->iobase + UART_LCR, lcr);
}

static int serial_ioctl(struct dev *dev, int cmd, void *args, size_t size) {
  struct serial_port *sp = (struct serial_port *) dev->privdata;
  struct serial_status  *ss;

  switch (cmd) {
    case IOCTL_GETDEVSIZE:
      return 0;

    case IOCTL_GETBLKSIZE:
      return 1;

    case IOCTL_SERIAL_SETCONFIG:
      if (!args || size != sizeof(struct serial_config)) return -EINVAL;
      memcpy(&sp->cfg, args, sizeof(struct serial_config));
      serial_config(sp);
      return 0;

    case IOCTL_SERIAL_GETCONFIG:
      if (!args || size != sizeof(struct serial_config)) return -EINVAL;
      memcpy(args, &sp->cfg, sizeof(struct serial_config));
      return 0;

    case IOCTL_SERIAL_WAITEVENT:
      if (!args && size == 0) {
        return wait_for_object(&sp->event, INFINITE);
      } else if (args && size == 4) {
        return wait_for_object(&sp->event, *(unsigned int *) args);
      } else {
        return -EINVAL;
      }

    case IOCTL_SERIAL_STAT:
      if (!args || size != sizeof(struct serial_status)) return -EINVAL;
      ss = (struct serial_status *) args;
      ss->linestatus = sp->linestatus;
      sp->linestatus = 0;
      ss->modemstatus = inp((unsigned short) (sp->iobase + UART_MSR)) & 0xFF;
      ss->rx_queue_size = sp->rxq.count;
      ss->tx_queue_size = sp->txq.count;
      return 0;

    case IOCTL_SERIAL_DTR:
      if (!args || size != 4) return -EINVAL;
      
      if (*(int *) args) {
        sp->mcr |= MCR_DTR;
      } else {
        sp->mcr &= ~MCR_DTR;
      }

      outp(sp->iobase + UART_MCR, sp->mcr);
      return 0;

    case IOCTL_SERIAL_RTS:
      if (!args || size != 4) return -EINVAL;

      if (*(int *) args) {
        sp->mcr |= MCR_RTS;
      } else {
        sp->mcr &= ~MCR_RTS;
      }

      outp(sp->iobase + UART_MCR, sp->mcr);
      return 0;

    case IOCTL_SERIAL_FLUSH_TX_BUFFER:
      cli();
      fifo_clear(&sp->txq);
      set_sem(&sp->tx_sem, QUEUE_SIZE);
      sp->tx_queue_rel = 0;
      if (sp->type == UART_16550A) outp(sp->iobase + UART_FCR, FCR_ENABLE | FCR_XMT_RST | FCR_TRIGGER_14);
      sti();
      return 0;

    case IOCTL_SERIAL_FLUSH_RX_BUFFER:
      cli();
      fifo_clear(&sp->rxq);
      set_sem(&sp->rx_sem, 0);
      sp->rx_queue_rel = 0;
      if (sp->type == UART_16550A) outp(sp->iobase + UART_FCR, FCR_ENABLE | FCR_RCV_RST | FCR_TRIGGER_14);
      sti();
      return 0;
  }
  
  return -ENOSYS;
}

static int serial_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  struct serial_port *sp = (struct serial_port *) dev->privdata;
  unsigned int n;
  unsigned char *bufp;

  if (wait_for_object(&sp->rx_lock, sp->cfg.rx_timeout) < 0) return -ETIMEOUT;

  bufp = (unsigned char *) buffer;
  for (n = 0; n < count; n++) {
    // Wait until rx queue is not empty
    if (wait_for_object(&sp->rx_sem, n == 0 ? sp->cfg.rx_timeout : 0) < 0) break;

    // Remove next char from receive queue
    cli();
    *bufp++ = fifo_get(&sp->rxq);
    sti();

    //kprintf("serial: read %02X\n", bufp[-1]);
  }

  release_mutex(&sp->rx_lock);
  return n;
}

static int serial_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  struct serial_port *sp = (struct serial_port *) dev->privdata;
  unsigned int n;
  unsigned char *bufp;

  if (wait_for_object(&sp->tx_lock, sp->cfg.tx_timeout) < 0) return -ETIMEOUT;

  bufp = (unsigned char *) buffer;
  for (n = 0; n < count; n++) {
    // Wait until tx queue is not full
    if (wait_for_object(&sp->tx_sem, sp->cfg.tx_timeout) < 0) break;

    // Insert next char in transmit queue
    cli();
    fifo_put(&sp->txq, *bufp++);
    sti();

    //kprintf("serial: write %02X\n", bufp[-1]);
    //kprintf("fifo put: h:%d t:%d c:%d\n", sp->txq.head, sp->txq.tail, sp->txq.count);

    // If transmitter idle then queue a DPC to restart transmission
    //if (!sp->tx_busy) queue_dpc(&sp->dpc, serial_dpc, sp);

    // If transmitter idle then restart transmission
    if (!sp->tx_busy) drain_tx_queue(sp);
  }

  release_mutex(&sp->tx_lock);
  return count;
}

static void drain_tx_queue(struct serial_port *sp) {
  unsigned char lsr;
  unsigned char b;
  int count;

  count = 0;
  while (1) {
    cli();

    // Is UART ready to transmit next byte
    lsr = inp((unsigned short) (sp->iobase + UART_LSR));
    sp->linestatus |= (lsr & (LSR_OE | LSR_PE | LSR_FE | LSR_BI));
    //kprintf("drain_tx_queue: lsr=%02X\n", lsr);

    if (!(lsr & LSR_TXRDY)) {
      sti();
      break;
    }

    // Is tx queue empty
    if (fifo_empty(&sp->txq)) {
      sti();
      break;
    }

    // Get next byte from queue
    b = fifo_get(&sp->txq);
    //kprintf("fifo get: h:%d t:%d c:%d\n", sp->txq.head, sp->txq.tail, sp->txq.count);

    //kprintf("serial: xmit %02X (drain)\n", b);
    outp(sp->iobase + UART_TX, b);
    sp->tx_busy = 1;
    count++;
    sti();
  }

  // Release transmitter queue resources
  if (count > 0) release_sem(&sp->tx_sem, count);
}

static void serial_dpc(void *arg) {
  int tqr;
  int rqr;
  int mlsc;
  int rls;
  struct serial_port *sp = (struct serial_port *) arg;

  //kprintf("[sdpc]");

  // Release transmitter and receiver queue resources and
  // signal line or modem status change
  cli();
  tqr = sp->tx_queue_rel;
  sp->tx_queue_rel = 0;
  rqr = sp->rx_queue_rel;
  sp->rx_queue_rel = 0;
  mlsc = sp->mlsc;
  sp->mlsc = 0;
  rls = sp->rls;
  sp->rls = 0;
  sti();

  if (tqr > 0) release_sem(&sp->tx_sem, tqr);
  if (rqr > 0) release_sem(&sp->rx_sem, rqr);
  if (mlsc || rls) set_event(&sp->event);
}

static void serial_transmit(struct serial_port *sp) {
  unsigned char lsr;
  unsigned char b;

  while (1) {
    // Is UART ready to transmit next byte
    lsr = inp((unsigned short) (sp->iobase + UART_LSR));
    sp->linestatus |= (lsr & (LSR_OE | LSR_PE | LSR_FE | LSR_BI));
    //kprintf("serial_transmit: lsr=%02X\n", lsr);

    if (!(lsr & LSR_TXRDY)) break;

    // Is tx queue empty
    if (fifo_empty(&sp->txq)) {
      sp->tx_busy = 0;
      break;
    }

    // Get next byte from queue
    b = fifo_get(&sp->txq);
    //kprintf("fifo get: h:%d t:%d c:%d\n", sp->txq.head, sp->txq.tail, sp->txq.count);
    //kprintf("serial: xmit %02X\n", b);
    outp(sp->iobase + UART_TX, b);
    sp->tx_busy = 1;
    sp->tx_queue_rel++;
  }
}

static void serial_receive(struct serial_port *sp) {
  unsigned char lsr;
  unsigned char b;

  while (1) {
    // Is there any data ready in the UART
    lsr = inp((unsigned short) (sp->iobase + UART_LSR));
    sp->linestatus |= (lsr & (LSR_OE | LSR_PE | LSR_FE | LSR_BI));
    //kprintf("serial_receive: lsr=%02X\n", lsr);
    if (!(lsr & LSR_RXRDY)) break;

    // Get next byte from UART and insert in rx queue
    b = inp((unsigned short) (sp->iobase + UART_RX));
    //kprintf("serial: receive %02X\n", b);
    if (fifo_put(&sp->rxq, b) < 0) {
      kprintf("serial: rx queue overflow\n");
      sp->linestatus |= LINESTAT_OVERFLOW;
    } else {
     sp->rx_queue_rel++;
    }
  }
}

static int serial_handler(struct context *ctxt, void *arg) {
  struct serial_port *sp = (struct serial_port *) arg;
  unsigned char iir;
  unsigned char lsr;
  int boguscnt = ISR_LIMIT;

  while (1) {
    lsr = inp((unsigned short) (sp->iobase + UART_LSR));

    // If receiver ready drain FIFO
    if (lsr & LSR_RXRDY) serial_receive(sp);

    // If transmitter ready send next bytes from tx queue
    if (lsr & LSR_TXRDY) serial_transmit(sp);

    // Get interrupt identification register
    iir = inp((unsigned short) (sp->iobase + UART_IIR));
    //kprintf("[sisr %d %x]", sp->irq, iir);

    if (iir & IIR_NOPEND) break;

    switch (iir & IIR_IMASK) {
      case IIR_MLSC:
        // Modem status changed
        sp->msr = inp((unsigned short) (sp->iobase + UART_MSR));
        sp->mlsc = 1;
        break;

      case IIR_RLS:
        // Line status changed
        sp->linestatus |= (lsr & (LSR_OE | LSR_PE | LSR_FE | LSR_BI));
        sp->rls = 1;
        break;
    }

    if (--boguscnt < 0) {
      kprintf("serial: Too much work at interrupt, iir=0x%02x\n", iir);
      break;
    }
  }

  // Set OUT2 to enable interrupts
  outp(sp->iobase + UART_MCR, sp->mcr);

  queue_irq_dpc(&sp->dpc, serial_dpc, sp);
  eoi(sp->irq);

  return 0;
}

struct driver serial_driver = {
  "serial",
  DEV_TYPE_STREAM,
  serial_ioctl,
  serial_read,
  serial_write
};

static void init_serial_port(char *devname, int iobase, int irq, struct unit *unit) {
  struct serial_port *sp;
  dev_t devno;

  sp = (struct serial_port *) kmalloc(sizeof(struct serial_port));
  memset(sp, 0, sizeof(struct serial_port));

  sp->iobase = iobase;
  sp->irq = irq;

  sp->cfg.speed = 115200;
  sp->cfg.databits = 8;
  sp->cfg.parity = PARITY_NONE;
  sp->cfg.stopbits = 1;
  sp->cfg.rx_timeout = INFINITE;
  sp->cfg.tx_timeout = INFINITE;

  init_dpc(&sp->dpc);
  sp->dpc.flags |= DPC_NORAND;

  init_event(&sp->event, 0, 0);

  init_sem(&sp->tx_sem, QUEUE_SIZE);
  init_mutex(&sp->tx_lock, 0);

  init_sem(&sp->rx_sem, 0);
  init_mutex(&sp->rx_lock, 0);

  // Disable interrupts
  outp(sp->iobase + UART_IER, 0);

  // Determine UART type
  check_uart_type(sp);

  // Set baudrate, parity, databits and stopbits
  serial_config(sp);

  // Enable FIFO
  if (sp->type == UART_16550A) {
    outp(sp->iobase + UART_FCR, FCR_ENABLE | FCR_RCV_RST | FCR_XMT_RST | FCR_TRIGGER_14);
  }

  // Turn on DTR, RTS and OUT2
  sp->mcr = MCR_DTR | MCR_RTS | MCR_IENABLE;  
  outp(sp->iobase + UART_MCR, sp->mcr);

  // Create device
  devno = dev_make(devname, &serial_driver, unit, sp);

  // Enable interrupts
  register_interrupt(&sp->intr, IRQ2INTR(sp->irq), serial_handler, sp);
  enable_irq(sp->irq);
  outp((unsigned short) (sp->iobase + UART_IER), IER_ERXRDY | IER_ETXRDY | IER_ERLS | IER_EMSC);

  kprintf(KERN_INFO "%s: %s iobase 0x%x irq %d\n", device(devno)->name, uart_name[sp->type], sp->iobase, sp->irq);
}

int __declspec(dllexport) serial(struct unit *unit) {
  int iobase;
  int irq;
  char devname[8];

  iobase = get_unit_iobase(unit);
  irq = get_unit_irq(unit);

  if (iobase < 0) iobase = serial_default_iobase[next_serial_portno - 1];
  if (irq < 0) irq = serial_default_irq[next_serial_portno - 1];

  sprintf(devname, "com%d", next_serial_portno++);
  init_serial_port(devname, iobase, irq, unit);

  return 0;
}

void init_serial() {
  int port;
  int iobase;

  // Fallback to BIOS settings if PnP BIOS not present
  if (next_serial_portno == 1) {
    for (port = 0; port < 4; port++) {
      iobase = syspage->biosdata[port * 2] + (syspage->biosdata[port * 2 + 1] << 8);

      if (iobase != 0) {
        switch (port) {
          case 0: init_serial_port("com1", iobase, serial_default_irq[0], NULL); break;
          case 1: init_serial_port("com2", iobase, serial_default_irq[1], NULL); break;
          case 2: init_serial_port("com3", iobase, serial_default_irq[2], NULL); break;
          case 3: init_serial_port("com4", iobase, serial_default_irq[3], NULL); break;
        }
      }
    }
  }
}
