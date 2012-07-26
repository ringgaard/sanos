//
// fd.c
//
// Floppy disk driver
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

#define NUMDRIVES             4

#define FD_MOTOR_TIMEOUT      3000
#define FD_MOTOR_SPINUP_TIME  1000
#define FD_RECAL_TIMEOUT      4000
#define FD_SEEK_TIMEOUT       15000
#define FD_XFER_TIMEOUT       15000
#define FD_BUSY_TIMEOUT       15000

//
// FDC I/O ports
//

#define FDC_DOR    0x3F2        // Digital Output Register
#define FDC_MSR    0x3F4        // Main Status Register (input)
#define FDC_DRS    0x3F4        // Data Rate Select Register (output)
#define FDC_DATA   0x3F5        // Data Register
#define FDC_DIR    0x3F7        // Digital Input Register (input)
#define FDC_CCR    0x3F7        // Configuration Control Register

//
// FDC Commands
//

#define CMD_SPECIFY 0x03        // Specify drive timings
#define CMD_WRITE   0xC5        // Write data (+ MT,MFM)
#define CMD_READ    0xE6        // Read data (+ MT,MFM,SK)
#define CMD_RECAL   0x07        // Recalibrate
#define CMD_SENSEI  0x08        // Sense interrupt status
#define CMD_FORMAT  0x4D        // Format track (+ MFM)
#define CMD_SEEK    0x0F        // Seek track
#define CMD_VERSION 0x10        // FDC version

//
// FDC versions
//

#define FDC_NEC765     0x80      // Standard uPD765A controller
#define FDC_82077      0x90      // Extended uPD765B controller

//
// Transfer mode
//

#define FD_MODE_READ    0
#define FD_MODE_WRITE   1

//
// Motor status
//

#define FD_MOTOR_OFF            0
#define FD_MOTOR_DELAY          1
#define FD_MOTOR_ON             2

//
// DMA I/O ports
//

#define DMA1_CHAN               0x0A
#define DMA1_MODE               0x0B
#define DMA1_RESET              0x0C

#define DMA1_CHAN2_ADDR         0x04
#define DMA1_CHAN2_COUNT        0x05
#define DMA1_CHAN2_PAGE         0x81

//
// DMA commands
//

#define DMA1_CHAN2_READ         0x46
#define DMA1_CHAN2_WRITE        0x4A

//
// Drive geometry
//

struct fdgeometry {
  char *name;
  unsigned char heads;         // Heads per drive (1.44M)
  unsigned char tracks;        // Number of tracks
  unsigned char spt;           // Sectors per track
  unsigned char gap3;          // Length of GAP3
};

//
// Result from FDC
//

struct fdresult {
  unsigned char st0;
  unsigned char st1;
  unsigned char st2;
  unsigned char st3;
  unsigned char track;
  unsigned char head;
  unsigned char sector;
  unsigned char size;
};

//
// Floppy Disk Controller
//

struct fdc {
  struct mutex lock;                   // Controller mutex
  struct event done;                   // Controller interrupt event
  struct interrupt intr;               // Interrupt object
  struct dpc dpc;                      // DPC for fd interrupt
  unsigned char dor;                   // DOR register with motor status

  int type;                            // FDC controller type
  char *name;                          // FDC controller name
  
  unsigned char bufp;                  // DMA buffer page
  unsigned char bufl;                  // DMA buffer address low
  unsigned char bufh;                  // DMA buffer address high
  char *dmabuf;                        // Virtual address of DMA buffer
};

//
// Floppy Disk Drive
//

struct fd {
  struct fdc *fdc;
  struct fdgeometry *geom;
  struct timer motortimer;
  int motor_status;
  int drive_initialized;
  int media_changed;
  unsigned char drive;
  unsigned char curtrack;
  unsigned char st0;
};

struct fdgeometry geom144 = {"3 1/2\" 1.44M", 2, 80, 18, 0x1B};

int fd_init;
struct thread *fdmotor_task;
struct fdc fdc;
struct fd fddrives[NUMDRIVES];

//
// fd_command
//

static int fd_command(unsigned char cmd) {
  int msr;
  unsigned int tmo;

  tmo = ticks + 1*HZ;
  while (1) {
    msr = inp(FDC_MSR);
    if ((msr & 0xc0) == 0x80) {
      outp(FDC_DATA, cmd);
      return 0;
    }
    
    if (time_before(tmo, ticks)) break;
    yield(); // delay
  }

  if (!fd_init) kprintf(KERN_WARNING "fd: command timeout\n");
  return -ETIMEOUT;
}

//
// fd_data
//

static int fd_data() {
  int msr;
  unsigned int tmo;

  tmo = ticks + 5*HZ;
  while (1) {
    msr = inp(FDC_MSR);
    if ((msr & 0xd0) == 0xd0) return inp(FDC_DATA) & 0xFF;
    if (time_before(tmo, ticks)) break;
    yield(); // delay
  }

  if (!fd_init) kprintf(KERN_WARNING "fd: data timeout\n");
  return -ETIMEOUT;
}

//
// fd_result
//

static int fd_result(struct fd *fd, struct fdresult *result, int sensei) {
  unsigned char *status = (unsigned char *) result;
  int n;
  int data;

  // Read in command result bytes
  n = 0;
  while (n < 7 && (inp(FDC_MSR) & (1 << 4)) != 0) {
    data = fd_data();
    if (data < 0) return data;
    status[n++] = data;
  }

  if (sensei) {
    // Send a "sense interrupt status" command
    fd_command(CMD_SENSEI);
    fd->st0 = fd_data();
    fd->curtrack = fd_data();
  }

  // Check for disk changed
  if (inp(FDC_DIR) & 0x80) fd->media_changed = 1;

  return 0;
}

//
// fd_motor_timeout
//

static void fd_motor_timeout(void *arg) {
  struct fd *fd = (struct fd *) arg;

  //kprintf("fd: motor off\n");
  if (wait_for_object(&fd->fdc->lock, 0) < 0) return;
  fd->fdc->dor &= ~(0x10 << fd->drive);
  outp(FDC_DOR, fd->fdc->dor);
  fd->motor_status = FD_MOTOR_OFF;
  release_mutex(&fd->fdc->lock);
}

//
// fd_motor_on
//

static void fd_motor_on(struct fd *fd) {
  if (fd->motor_status == FD_MOTOR_OFF) {
    //kprintf("fd: motor on\n");
    fd->fdc->dor |= 0x10 << fd->drive;
    outp(FDC_DOR, fd->fdc->dor);
    fd->motor_status = FD_MOTOR_ON;
    msleep(FD_MOTOR_SPINUP_TIME);
    //kprintf("fd: motor spinned up\n");
  } else {
    fd->motor_status = FD_MOTOR_ON;
  }
}

//
// fd_motor_off
//

static void fd_motor_off(struct fd *fd) {
  if (fd->motor_status == FD_MOTOR_ON) {
    fd->motor_status = FD_MOTOR_DELAY;
    mod_timer(&fd->motortimer, ticks + FD_MOTOR_TIMEOUT / MSECS_PER_TICK);
  }
}

//
// blk2ths
//

static void blk2ths(struct fd *fd, blkno_t blkno, unsigned char *track, unsigned char *head, unsigned char *sector) {
  *track = blkno / (fd->geom->heads * fd->geom->spt);
  *head = (blkno / fd->geom->spt) % fd->geom->heads;
  *sector = blkno % fd->geom->spt + 1;
}

//
// fd_recalibrate
//

static int fd_recalibrate(struct fd *fd) {
  struct fdresult result;

  kprintf("fd: recalibrate\n");
  reset_event(&fd->fdc->done);
  fd_command(CMD_RECAL);
  fd_command(0x00);

  if (wait_for_object(&fd->fdc->done, FD_RECAL_TIMEOUT) < 0) {
    kprintf(KERN_WARNING "fd: timeout waiting for calibrate to complete\n");
    return -ETIMEOUT;
  }

  if (fd_result(fd, &result, 1) < 0) return -ETIMEOUT;

  return 0;
}

//
// fd_initialize
//

static int fd_initialize(struct fd *fd) {
  // Specify drive timings
  fd_command(CMD_SPECIFY);
  fd_command(0xdf);  // SRT = 3ms, HUT = 240ms 
  fd_command(0x02);  // HLT = 16ms, ND = 0
  
  //fd_recalibrate(fd);

  fd->drive_initialized = 1;
  return 0;
}

//
// fd_transfer
//

static int fd_transfer(struct fd *fd, int mode, void *buffer, size_t count, blkno_t blkno) {
  struct fdresult result;
  int retries = 5;
  unsigned char track, head, sector;
  unsigned int remaining;

  if (!buffer) return -EINVAL;

  // The FDC can read multiple sides at once but not multiple tracks
  blk2ths(fd, blkno, &track, &head, &sector);
  remaining = ((fd->geom->spt + 1 - sector) + fd->geom->spt * (fd->geom->heads - head - 1)) * SECTORSIZE;
  if (remaining < count) count = remaining;
  
  if (mode == FD_MODE_WRITE) memcpy(fd->fdc->dmabuf, buffer, count);

  while (retries-- > 0) {
    // Perform seek if necessary
    if (fd->curtrack != track) {
      reset_event(&fd->fdc->done);

      //kprintf("fd: seek track %d head %d (curtrack %d)\n", track, head, fd->curtrack);
      fd_command(CMD_SEEK);
      fd_command((unsigned char) ((head << 2) | fd->drive));
      fd_command(track);
    
      if (wait_for_object(&fd->fdc->done, FD_SEEK_TIMEOUT) < 0) {
        kprintf(KERN_WARNING "fd: timeout waiting for seek to complete\n");
        fd_recalibrate(fd);
        continue;
      }

      fd_result(fd, &result, 1);
      if ((fd->st0 &0xE0) != 0x20 || fd->curtrack != track) {
        kprintf(KERN_ERR "fd: seek failed, st0 0x%02x, current %d, target %d\n", fd->st0, fd->curtrack, track);
        continue;
      }

      //kprintf("fd: set curtrack to %d\n", fd->curtrack);

      // Let head settle for 15ms
      msleep(15);
    }

    // Program data rate (500K/s)
    outp(FDC_CCR,0);

    // Set up DMA
    outp(DMA1_CHAN, 0x06);

    if (mode == FD_MODE_READ) {
      outp(DMA1_RESET, DMA1_CHAN2_READ);
      outp(DMA1_MODE, DMA1_CHAN2_READ);
    } else {
      outp(DMA1_RESET, DMA1_CHAN2_WRITE);
      outp(DMA1_MODE, DMA1_CHAN2_WRITE);
    }

    // Setup DMA transfer
    outp(DMA1_CHAN2_ADDR, fd->fdc->bufl);
    outp(DMA1_CHAN2_ADDR, fd->fdc->bufh);
    outp(DMA1_CHAN2_PAGE, fd->fdc->bufp);
    outp(DMA1_CHAN2_COUNT, ((count - 1) & 0xFF));
    outp(DMA1_CHAN2_COUNT, ((count - 1) >> 8));
    outp(DMA1_CHAN, 0x02);

    reset_event(&fd->fdc->done);

    // Perform transfer
    if (mode == FD_MODE_READ) {
      fd_command(CMD_READ);
    } else {
      fd_command(CMD_WRITE);
    }

    fd_command((unsigned char) ((head << 2) | fd->drive));
    fd_command(track);
    fd_command(head);
    fd_command(sector);
    fd_command(0x02); // 512 bytes/sector
    fd_command(fd->geom->spt);
    fd_command(fd->geom->gap3);
    fd_command(0xff); // DTL = unused
    
    if (wait_for_object(&fd->fdc->done, FD_XFER_TIMEOUT) < 0) {
      kprintf(KERN_WARNING "fd: timeout waiting for transfer to complete\n");
      fd_recalibrate(fd);
      continue;
    }

    fd_result(fd, &result, 0);

    if ((result.st0 & 0xc0) == 0) {
      // Successful transfer
      if (mode == FD_MODE_READ) memcpy(buffer, fd->fdc->dmabuf, count);
      return count;
    } else {
      kprintf(KERN_ERR "fd: xfer error, st0 %02X st1 %02X st2 %02X THS=%d/%d/%d\n", result.st0, result.st1, result.st2, result.track, result.head, result.sector);

      // Recalibrate before retrying.
      fd_recalibrate(fd);
    } 
  }
  
  return -ETIMEOUT;
}

//
// fd_ioctl
//

static int fd_ioctl(struct dev *dev, int cmd, void *args, size_t size) {
  struct fd *fd = (struct fd *) dev->privdata;

  switch (cmd) {
    case IOCTL_GETDEVSIZE:
      return fd->geom->heads * fd->geom->tracks * fd->geom->spt;

    case IOCTL_GETBLKSIZE:
      return SECTORSIZE;
  }
  
  return -ENOSYS;
}

//
// fd_read
//

static int fd_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  struct fd *fd = (struct fd *) dev->privdata;
  int left;
  int result;
  char *buf;

  if (wait_for_object(&fd->fdc->lock, FD_BUSY_TIMEOUT) < 0) return -EBUSY;
  if (!fd->drive_initialized) fd_initialize(fd);
  fd_motor_on(fd);

  left = count;
  buf = (char *) buffer;
  while (left > 0) {
    result = fd_transfer(fd, FD_MODE_READ, buf, left, blkno);
    if (result <= 0) {
      kprintf(KERN_ERR "fd: error %d reading from floppy\n", result);
      fd_motor_off(fd);
      release_mutex(&fd->fdc->lock);
      return result;
    }

    left -= result;
    buf += result;
    blkno += result / SECTORSIZE;
  }

  fd_motor_off(fd);
  release_mutex(&fd->fdc->lock);

  return count;
}

//
// fd_write
//

static int fd_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  struct fd *fd = (struct fd *) dev->privdata;
  int left;
  int result;
  char *buf;

  if (wait_for_object(&fd->fdc->lock, FD_BUSY_TIMEOUT) < 0) return -EBUSY;
  if (!fd->drive_initialized) fd_initialize(fd);
  fd_motor_on(fd);

  left = count;
  buf = (char *) buffer;
  while (left > 0) {
    result = fd_transfer(fd, FD_MODE_WRITE, buf, left, blkno);
    if (result <= 0) {
      kprintf(KERN_ERR "fd: error %d writing to floppy\n", result);
      fd_motor_off(fd);
      release_mutex(&fd->fdc->lock);
      return result;
    }

    left -= result;
    buf += result;
    blkno += result / SECTORSIZE;
  }

  fd_motor_off(fd);
  release_mutex(&fd->fdc->lock);

  return count;
}

//
// fd_dpc
//

static void fd_dpc(void *arg) {
  struct fdc *fdc = (struct fdc *) arg;
  set_event(&fdc->done);
}

//
// fd_handler
//

static int fd_handler(struct context *ctxt, void *arg) {
  queue_irq_dpc(&fdc.dpc, fd_dpc, arg);
  eoi(IRQ_FD);
  return 0;
}

struct driver floppy_driver = {
  "floppy",
  DEV_TYPE_BLOCK,
  fd_ioctl,
  fd_read,
  fd_write
};

//
// init_fd
//

static void init_drive(char *devname, struct fd *fd, struct fdc *fdc, int drive, struct fdgeometry *geom) {
  fd->fdc = fdc;
  fd->geom = geom;
  fd->drive = drive;
  fd->curtrack = 0xFF;
  fd->drive_initialized = 0;
  init_timer(&fd->motortimer, fd_motor_timeout, fd);

  dev_make(devname, &floppy_driver, NULL, fd);

  kprintf(KERN_INFO "%s: %s, %d KB, THS=%u/%u/%u\n", devname, fd->geom->name,
    fd->geom->tracks * fd->geom->heads * fd->geom->spt * SECTORSIZE / 1024,
    fd->geom->tracks, fd->geom->heads, fd->geom->spt);
}

void init_fd() {
  int i;
  unsigned char fdtypes;
  int first_floppy;
  int second_floppy;
  //int version;
  //char *name;

  fdtypes = read_cmos_reg(0x10);
  first_floppy = (fdtypes >> 4) & 0x0F;
  second_floppy = fdtypes & 0x0F;
  if (!first_floppy && !second_floppy) return;

#if 0
  // FIXME: the version command times out when no floppy is inserted
  fd_init = 1;
  if (fd_command(CMD_VERSION) < 0) return;
  version = fd_data();
  if (version < 0) return;
  fd_init = 0;

  switch (version)
  {
    case FDC_NEC765:
      name = "NEC765 compatible";
      break;

    case FDC_82077:
      name = "82077 compatible";
      break;

    case 0x81:
      name = "Type 0x81";
      break;

    default:
      kprintf("fd: unknown fdc type 0x%02x\n", version);
      return;
  }
#endif

  memset(&fdc, 0, sizeof(struct fdc));
  memset(&fddrives, 0, sizeof(struct fd) * NUMDRIVES);

  //fdc.type = version;
  //fdc.name = name;

  init_dpc(&fdc.dpc);
  init_mutex(&fdc.lock, 0);
  init_event(&fdc.done, 0, 0);
  fdc.dor = 0x0C; // TODO: select drive in DOR on transfer

  fdc.dmabuf = (char *) DMABUF_ADDRESS;
  fdc.bufp = (DMA_BUFFER_START >> 16) & 0xFF;
  fdc.bufh = (DMA_BUFFER_START >> 8) & 0xFF;
  fdc.bufl = DMA_BUFFER_START & 0xFF;

  for (i = 0; i < DMA_BUFFER_PAGES; i++) map_page(fdc.dmabuf + i * PAGESIZE, BTOP(DMA_BUFFER_START) + i, PT_WRITABLE | PT_PRESENT);

  register_interrupt(&fdc.intr, INTR_FD, fd_handler, &fdc);
  enable_irq(IRQ_FD); 

  //kprintf("fdc: %s\n", fdc.name);

  // FIXME: support other geometries: 0=unknown, 1=360K 5 1/4", 2=1.2M 5 1/4", 3=720K 3 1/2", 4=1.44M 3 1/2"
  if (first_floppy == 0x4) init_drive("fd0", &fddrives[0], &fdc, 0, &geom144);
  if (second_floppy == 0x4) init_drive("fd1", &fddrives[1], &fdc, 1, &geom144);
}
