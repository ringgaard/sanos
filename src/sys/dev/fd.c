//
// fd.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Floppy disk driver
//

#include <os/krnl.h>

#define SECTORSIZE            512
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
// Transfer mode
//

#define FD_MODE_READ	0
#define FD_MODE_WRITE	1

//
// Motor status
//

#define FD_MOTOR_OFF		0
#define FD_MOTOR_DELAY		1
#define FD_MOTOR_ON		2

//
// DMA I/O ports
//

#define DMA1_CHAN		0x0A
#define DMA1_MODE		0x0B
#define DMA1_RESET	        0x0C

#define DMA1_CHAN2_ADDR	        0x04
#define DMA1_CHAN2_COUNT	0x05
#define DMA1_CHAN2_PAGE	        0x81

//
// DMA commands
//

#define DMA1_CHAN2_READ	        0x46
#define DMA1_CHAN2_WRITE	0x4A

//
// Drive geometry
//

struct fdgeometry
{
  unsigned char heads;         // Heads per drive (1.44M)
  unsigned char tracks;        // Number of tracks
  unsigned char spt;           // Sectors per track
  unsigned char gap3;          // Length of GAP3
};

//
// Result from FDC
//

struct fdresult
{
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

struct fdc
{
  struct mutex lock;	               // Controller mutex
  struct event intr;	               // Controller interrupt event
  struct event motor_change;           // Motor status change event
  struct dpc dpc;                      // DPC for fd interrupt
  unsigned char dor;                   // DOR register with motor status
  
  unsigned char bufp;                  // DMA buffer page
  unsigned char bufl;                  // DMA buffer address low
  unsigned char bufh;                  // DMA buffer address high
  char *dmabuf;                        // Virtual address of DMA buffer
};

//
// Floppy Disk Drive
//

struct fd
{
  struct fdc *fdc;
  struct fdgeometry *geom;
  int motor_status;
  int media_changed;
  unsigned int motor_timeout;
  unsigned char drive;
  unsigned char curtrack;
  unsigned char st0;
};

struct fdgeometry geom144 = {2, 80, 18, 0x1B}; // Drive geometry for 3 1/2" 1.44M drive

struct thread *fdmotor_task;
struct fdc fdc;
struct fd fddrives[NUMDRIVES];

//
// fd_command
//

static int fd_command(unsigned char cmd)
{
  int msr;
  int tmo;

  for (tmo = 0;tmo < 128; tmo++) 
  {
    msr = _inp(FDC_MSR);
    if ((msr & 0xc0) == 0x80)
    {
      _outp(FDC_DATA, cmd);
      return 0;
    }
    
    yield(); // delay
  }

  return -ETIMEOUT;
}

//
// fd_data
//

static int fd_data()
{
  int msr;
  int tmo;

  for (tmo = 0;tmo < 128; tmo++) 
  {
    msr = _inp(FDC_MSR);
    if ((msr & 0xd0) == 0xd0) return _inp(FDC_DATA);
    yield(); // delay
  }

  // Read timeout
  return -ETIMEOUT;
}

//
// fd_result
//

static int fd_result(struct fd *fd, struct fdresult *result, int sensei)
{
  unsigned char *status = (unsigned char *) result;
  int n;
  int data;

  // Read in command result bytes
  n = 0;
  while (n < 7 && (_inp(FDC_MSR) & (1<<4)) != 0) 
  {
    data = fd_data();
    if (data < 0) return data;
    status[n++] = data;
  }

  if (sensei) 
  {
    // Send a "sense interrupt status" command
    fd_command(CMD_SENSEI);
    fd->st0 = fd_data();
    fd->curtrack = fd_data();
  }

  // Check for disk changed
  if (_inp(FDC_DIR) & 0x80) fd->media_changed = 1;

  return 0;
}

//
// fd_motor_task
//

static void fd_motor_task(void *arg)
{
  int i;

  while (1)
  {
    wait_for_object(&fdc.motor_change, INFINITE);

    while (fdc.dor & 0xF0)
    {
      for (i = 0; i < NUMDRIVES; i++)
      {
	if (fddrives[i].motor_status == FD_MOTOR_DELAY && time_before(fddrives[i].motor_timeout, clocks))
	{
          //kprintf("fd: motor off\n");
	  fdc.dor &= ~(0x10 << i);
          _outp(FDC_DOR, fdc.dor);
	  fddrives[i].motor_status = FD_MOTOR_OFF;
	}
      }

      sleep(FD_MOTOR_TIMEOUT);
    }
  }
}

//
// fd_motor_on
//

static void fd_motor_on(struct fd *fd)
{
  if (fd->motor_status == FD_MOTOR_OFF)
  {
    //kprintf("fd: motor on\n");
    fd->fdc->dor |= 0x10 << fd->drive;
    _outp(FDC_DOR, fd->fdc->dor);
    sleep(FD_MOTOR_SPINUP_TIME);
    //kprintf("fd: motor spinned up\n");
  }

  fd->motor_status = FD_MOTOR_ON;
}

//
// fd_motor_off
//

static void fd_motor_off(struct fd *fd)
{
  if (fd->motor_status == FD_MOTOR_ON)
  {
    fd->motor_status = FD_MOTOR_DELAY;
    fd->motor_timeout = clocks + FD_MOTOR_TIMEOUT;
    set_event(&fd->fdc->motor_change);
  }
}

//
// blk2ths
//

static void blk2ths(struct fd *fd, blkno_t blkno, unsigned char *track, unsigned char *head, unsigned char *sector)
{
  *track = blkno / (fd->geom->heads * fd->geom->spt);
  *head = (blkno / fd->geom->spt) % fd->geom->heads;
  *sector = blkno % fd->geom->spt + 1;
}

//
// fd_recalibrate
//

static int fd_recalibrate(struct fd *fd)
{
  struct fdresult result;

  //kprintf("fd: recalibrate\n");
  reset_event(&fd->fdc->intr);
  fd_command(CMD_RECAL);
  fd_command(0x00);

  if (wait_for_object(&fd->fdc->intr, FD_RECAL_TIMEOUT) < 0) 
  {
    kprintf("fd: timeout waiting for calibrate to complete\n");
    return -ETIMEOUT;
  }

  if (fd_result(fd, &result, 1) < 0) return -ETIMEOUT;

  return 0;
}

//
// fd_transfer
//

static int fd_transfer(struct fd *fd, int mode, void *buffer, size_t count, blkno_t blkno)
{
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

  while (retries-- > 0) 
  {
    // Perform seek if necessary
    if (fd->curtrack != track)
    {
      reset_event(&fd->fdc->intr);

      //kprintf("fd: seek track %d head %d (curtrack %d)\n", track, head, fd->curtrack);
      fd_command(CMD_SEEK);
      fd_command((unsigned char) ((head << 2) | fd->drive));
      fd_command(track);
    
      if (wait_for_object(&fd->fdc->intr, FD_SEEK_TIMEOUT) < 0) 
      {
	kprintf("fd: timeout waiting for seek to complete\n");
	fd_recalibrate(fd);
	continue;
      }

      fd_result(fd, &result, 1);
      if (fd->st0 != 0x20 || fd->curtrack != track)
      {
	kprintf("fd: seek failed\n");
	continue;
      }

      //kprintf("fd: set curtrack to %d\n", fd->curtrack);

      // Let head settle for 15ms
      sleep(15);
    }

    // Program data rate (500K/s)
    _outp(FDC_CCR,0);

    // Set up DMA
    _outp(DMA1_CHAN, 0x06);

    if (mode == FD_MODE_READ) 
    {
      _outp(DMA1_RESET, DMA1_CHAN2_READ);
      _outp(DMA1_MODE, DMA1_CHAN2_READ);
    } 
    else 
    {
      _outp(DMA1_RESET, DMA1_CHAN2_WRITE);
      _outp(DMA1_MODE, DMA1_CHAN2_WRITE);
    }

    // Setup DMA transfer
    _outp(DMA1_CHAN2_ADDR, fd->fdc->bufl);
    _outp(DMA1_CHAN2_ADDR, fd->fdc->bufh);
    _outp(DMA1_CHAN2_PAGE, fd->fdc->bufp);
    _outp(DMA1_CHAN2_COUNT, ((count - 1) & 0xFF));
    _outp(DMA1_CHAN2_COUNT, ((count - 1) >> 8));
    _outp(DMA1_CHAN, 0x02);

    reset_event(&fd->fdc->intr);

    // Perform transfer
    if (mode == FD_MODE_READ)
      fd_command(CMD_READ);
    else
      fd_command(CMD_WRITE);

    fd_command((unsigned char) ((head << 2) | fd->drive));
    fd_command(track);
    fd_command(head);
    fd_command(sector);
    fd_command(0x02); // 512 bytes/sector
    fd_command(fd->geom->spt);
    fd_command(fd->geom->gap3);
    fd_command(0xff); // DTL = unused
    
    if (wait_for_object(&fd->fdc->intr, FD_XFER_TIMEOUT) < 0) 
    {
      kprintf("fd: timeout waiting for transfer to complete\n");
      fd_recalibrate(fd);
      continue;
    }

    fd_result(fd, &result, 0);

    if ((result.st0 & 0xc0) == 0) 
    {
      // Successful transfer
      if (mode == FD_MODE_READ) memcpy(buffer, fd->fdc->dmabuf, count);
      return count;
    }
    else if ((result.st0 & 0xc0) == 0x40 && (result.st1 & 0x04) == 0x04)
    {
      fd_recalibrate(fd);
    }
    else
    {
      kprintf("fd: xfer error, st0 %02X st1 %02X st2 %02X THS=%d/%d/%d\n", result.st0, result.st1, result.st2, result.track, result.head, result.sector);

      // Recalibrate before retrying.
      fd_recalibrate(fd);
    } 
  }
  
  return -ETIMEOUT;
}

//
// fd_ioctl
//

static int fd_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  struct fd *fd = (struct fd *) dev->privdata;

  switch (cmd)
  {
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

static int fd_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  struct fd *fd = (struct fd *) dev->privdata;
  int left;
  int result;
  char *buf;

  if (wait_for_object(&fd->fdc->lock, FD_BUSY_TIMEOUT) < 0) return -EBUSY;
  fd_motor_on(fd);

  left = count;
  buf = (char *) buffer;
  while (left > 0)
  {
    result = fd_transfer(fd, FD_MODE_READ, buf, left, blkno);
    if (result <= 0)
    {
      kprintf("fd: error %d reading from floppy\n", result);
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

static int fd_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  struct fd *fd = (struct fd *) dev->privdata;
  int left;
  int result;
  char *buf;

  if (wait_for_object(&fd->fdc->lock, FD_BUSY_TIMEOUT) < 0) return -EBUSY;
  fd_motor_on(fd);

  left = count;
  buf = (char *) buffer;
  while (left > 0)
  {
    result = fd_transfer(fd, FD_MODE_WRITE, buf, left, blkno);
    if (result <= 0)
    {
      kprintf("fd: error %d writing to floppy\n", result);
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

static void fd_dpc(void *arg)
{
  struct fdc *fdc = (struct fdc *) arg;
  set_event(&fdc->intr);
}

//
// fd_handler
//

static void fd_handler(struct context *ctxt, void *arg)
{
  queue_irq_dpc(&fdc.dpc, fd_dpc, arg);
  eoi(IRQ_FD);
}

struct driver floppy_driver =
{
  "floppy",
  DEV_TYPE_BLOCK,
  fd_ioctl,
  fd_read,
  fd_write
};

//
// init_fd
//

static void init_drive(char *devname, struct fd *fd, struct fdc *fdc, int drive, struct fdgeometry *geom)
{
  fd->fdc = fdc;
  fd->geom = geom;
  fd->drive = drive;
  fd->curtrack = 0xFF;

  // Specify drive timings
  fd_command(CMD_SPECIFY);
  fd_command(0xdf);  // SRT = 3ms, HUT = 240ms 
  fd_command(0x02);  // HLT = 16ms, ND = 0

  
  dev_make(devname, &floppy_driver, NULL, fd);

  fd_recalibrate(fd);

  kprintf("%s: %u blks (%d KB) THS=%u/%u/%u\n", devname, 
    fd->geom->tracks * fd->geom->heads * fd->geom->spt, 
    fd->geom->tracks * fd->geom->heads * fd->geom->spt * SECTORSIZE / K,
    fd->geom->tracks, fd->geom->heads, fd->geom->spt);
}

void init_fd()
{
  int i;
  int numfd;

  numfd = (syspage->biosdata[0x10] >> 6) + 1;

  memset(&fdc, 0, sizeof(struct fdc));
  memset(&fddrives, 0, sizeof(struct fd) * NUMDRIVES);

  init_dpc(&fdc.dpc);
  init_mutex(&fdc.lock, 0);
  init_event(&fdc.intr, 0, 0);
  init_event(&fdc.motor_change, 0, 0);
  fdc.dor = 0x0C; // TODO: select drive in DOR on transfer

  fdc.dmabuf = (char *) DMABUF_ADDRESS;
  fdc.bufp = (DMA_BUFFER_START >> 16) & 0xFF;
  fdc.bufh = (DMA_BUFFER_START >> 8) & 0xFF;
  fdc.bufl = DMA_BUFFER_START & 0xFF;

  for (i = 0; i < DMA_BUFFER_PAGES; i++) map_page(fdc.dmabuf + i * PAGESIZE, BTOP(DMA_BUFFER_START) + i, PT_WRITABLE | PT_PRESENT);

  set_interrupt_handler(INTR_FD, fd_handler, &fdc);
  enable_irq(IRQ_FD); 

  if (numfd >= 1) init_drive("fd0", &fddrives[0], &fdc, 0, &geom144);
  if (numfd >= 2) init_drive("fd1", &fddrives[1], &fdc, 1, &geom144);
  if (numfd >= 3) init_drive("fd2", &fddrives[2], &fdc, 2, &geom144);
  if (numfd >= 4) init_drive("fd3", &fddrives[3], &fdc, 3, &geom144);

  fdmotor_task = create_kernel_thread(fd_motor_task, NULL, PRIORITY_NORMAL, "fdmotor");
}
