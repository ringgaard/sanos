//
// hd.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Harddisk driver
//

#include <os/krnl.h>

#define SECTORSIZE              512

#define HD_CONTROLLERS		2
#define HD_DRIVES		4
#define HD_PARTITIONS		4

#define MAX_PRDS                (PAGESIZE / 8)
#define MAX_DMA_XFER_SIZE       ((MAX_PRDS - 1) * PAGESIZE)

#define HDC0_IOBASE		0x01F0
#define HDC1_IOBASE		0x0170

#define HDC0_IRQ                14
#define HDC1_IRQ                15

#define HD0_DRVSEL              0xA0
#define HD1_DRVSEL              0xB0

//
// Controller registers
//

#define HDC_DATA		0x0000
#define HDC_ERR			0x0001
#define HDC_FEATURE		0x0001
#define HDC_SECTORCNT		0x0002
#define HDC_SECTOR		0x0003
#define HDC_TRACKLSB		0x0004
#define HDC_TRACKMSB		0x0005
#define HDC_DRVHD		0x0006
#define HDC_STATUS		0x0007
#define HDC_COMMAND		0x0007
#define HDC_DEVCTRL		0x0008
#define HDC_ALT_STATUS		0x0206
#define HDC_CONTROL		0x0206

//
// Controller commands
//

#define HDCTL_RST		0x04   // Controller reset
#define HDCTL_4BITHD		0x08   // Use 4 bits for head id

//
// Drive commands
//

#define HDCMD_NULL		0x00
#define HDCMD_IDENTIFY		0xEC
#define HDCMD_RESET		0x10
#define HDCMD_READ		0x20
#define HDCMD_WRITE		0x30
#define HDCMD_READ_DMA          0xC8
#define HDCMD_WRITE_DMA         0xCA

//
// Controller status
//

#define HDCS_ERR		0x01   // Error
#define HDCS_IDX		0x02   // Index
#define HDCS_CORR		0x04   // Corrected data
#define HDCS_DRQ		0x08   // Data request
#define HDCS_DSC		0x10   // Drive seek complete
#define HDCS_DWF		0x20   // Drive write fault
#define HDCS_DRDY		0x40   // Drive ready
#define HDCS_BSY		0x80   // Controller busy

//
// Device control 
//

#define HDDC_HD15               0x08  // Bit should always be set to one
#define HDDC_SRST               0x04  // Soft reset
#define HDDC_NIEN               0x02  // Disable interrupts

//
// Controller error conditions 
//

#define HDCE_AMNF		0x01   // Address mark not found
#define HDCE_TK0NF		0x02   // Track 0 not found
#define HDCE_ABRT		0x04   // Abort
#define HDCE_MCR		0x08   // Media change requested
#define HDCE_IDNF		0x10   // Sector id not found
#define HDCE_MC			0x20   // Media change
#define HDCE_UNC		0x40   // Uncorrectable data error
#define HDCE_BBK		0x80   // Bad block

//
// Timeouts (in ms)
//

#define HDTIMEOUT_DRDY		5000
#define HDTIMEOUT_DRQ		5000
#define HDTIMEOUT_CMD		1000

//
// Buffer type
//

#define HDTYPE_SINGLE           1      // Single port, single sector buf
#define HDTYPE_DUAL             2      // Dual port, multiple sector buf
#define HDTYPE_DUAL_CACHE       3      // Above plus track cache

//
// Transfer type
//

#define HD_XFER_IDLE            0
#define HD_XFER_READ            1
#define HD_XFER_WRITE           2
#define HD_XFER_DMA             3
#define HD_XFER_IGNORE          4

//
// Bus master registers
//

#define BM_COMMAND_REG    0            // Offset to command reg
#define BM_STATUS_REG     2            // Offset to status reg
#define BM_PRD_ADDR_LOW   4            // Offset to PRD addr reg low 16 bits
#define BM_PRD_ADDR_HIGH  6            // Offset to PRD addr reg high 16 bits

//
// Bus master command register flags
//

#define BM_CR_MASK_READ    0x00        // Read from memory
#define BM_CR_MASK_WRITE   0x08        // Write to memory
#define BM_CR_MASK_START   0x01        // Start transfer
#define BM_CR_MASK_STOP    0x00        // Stop transfer

//
// Bus master status register flags
//

#define BM_SR_MASK_SIMPLEX 0x80        // Simplex only
#define BM_SR_MASK_DRV1    0x40        // Drive 1 can do dma
#define BM_SR_MASK_DRV0    0x20        // Drive 0 can do dma
#define BM_SR_MASK_INT     0x04        // INTRQ signal asserted
#define BM_SR_MASK_ERR     0x02        // Error
#define BM_SR_MASK_ACT     0x01        // Active

//
// Parameters returned by read drive parameters command

struct hdparam 
{
  // Drive information
  unsigned short config;	       // General configuration bits
  unsigned short cylinders;	       // Cylinders
  unsigned short reserved;
  unsigned short heads;		       // Heads
  unsigned short unfbytespertrk;       // Unformatted bytes/track
  unsigned short unfbytes;	       // Unformatted bytes/sector
  unsigned short sectors;	       // Sectors per track
  unsigned short vendorunique[3];

  // Controller information
  char serial[20];		       // Serial number
  unsigned short buffertype;	       // Buffer type
  unsigned short buffersize;	       // Buffer size, in 512-byte units
  unsigned short necc;		       // ECC bytes appended
  char rev[8];		               // Firmware revision
  char model[40];		       // Model name
  unsigned short nsecperint;	       // Sectors per interrupt
  unsigned short usedmovsd;	       // Can use double word read/write?
  unsigned short caps;                 // Capabilities
  unsigned short resv1;                // Reserved
  unsigned short pio;                  // PIO data transfer cycle timing
  unsigned short resv2;                // Reserved
  unsigned short valid;                // Flag valid fields to follow
  unsigned short logcyl;               // Logical cylinders
  unsigned short loghead;              // Logical heads
  unsigned short logspt;               // Logical sector/track
  unsigned short cap0;                 // Capacity in sectors (32-bit)
  unsigned short cap1;
  unsigned short multisec;             // Multiple sector values
  unsigned short totalsec0;            // Total number of user sectors
  unsigned short totalsec1;            //  (LBA; 32-bit value)
  unsigned short resv3;                // Reserved
  unsigned short multimode;            // Multiword DMA
  unsigned short piomode;              // Advanced PIO modes
  unsigned short minmulti;             // Minimum multiword xfer
  unsigned short multitime;            // Recommended cycle time
  unsigned short minpio;               // Min PIO without flow ctl
  unsigned short miniodry;             // Min with IORDY flow
  unsigned short resv4[10];            // Reserved
  unsigned short ver;                  // Major version number
  unsigned short verminor;             // Minor version number
  unsigned short cmdset;               // Command set supported
  unsigned short cmdset2;
  char pad[346];
};

struct hd;

struct prd
{
  void *addr;
  int len;
};

struct hdc 
{
  struct mutex lock;	               // Controller mutex
  struct event ready;	               // Controller interrupt event
  struct dpc xfer_dpc;                 // DPC for data transfer
  
  int status;                          // Controller status
  int bmstat;                          // Busmaster status

  int iobase;	                       // I/O port registers base address
  int irq;                             // IRQ for controller
  int bmregbase;                       // Busmaster register base

  char *bufp;                          // Buffer pointer for next transfer
  int nsects;                          // Number of sectors left to transfer
  int result;                          // Result of transfer
  int dir;                             // Transfer direction
  struct hd *active;                   // Active drive for transfer

  struct prd *prds;                    // PRD list for DMA transfer
  unsigned long prds_phys;             // Physical address of PRD list
};

struct partition
{
  devno_t dev;
  unsigned int start;
  unsigned int len;
  unsigned short bootid;
  unsigned short systid;
};

struct hd 
{
  struct hdc *hdc;		        // Controller
  struct hdparam param;		        // Drive parameter block
  int drvsel;                           // Drive select on controller
  int use32bits;                        // Use 32 bit transfers
  int sectbufs;                         // Number of sector buffers
  int lba;                              // Use LBA mode

  // Geometry
  unsigned int blks;		        // Number of blocks on drive
  unsigned int size;		        // Size in MB

  unsigned int cyls;	                // Number of cylinders
  unsigned int heads;		        // Number of heads
  unsigned int sectors;                 // Sectors per track

  struct partition parts[HD_PARTITIONS]; // Partition info
};

static struct hdc hdctab[HD_CONTROLLERS];
static struct hd hdtab[HD_DRIVES];

static char *hd0parts[HD_PARTITIONS] = {"hd0a", "hd0b", "hd0c", "hd0d"};
static char *hd1parts[HD_PARTITIONS] = {"hd1a", "hd1b", "hd1c", "hd1d"};
static char *hd2parts[HD_PARTITIONS] = {"hd2a", "hd2b", "hd2c", "hd2d"};
static char *hd3parts[HD_PARTITIONS] = {"hd3a", "hd3b", "hd3c", "hd3d"};

static void insw(int port, void *buf, int count)
{
  __asm
  {
    mov edx, port
    mov edi, buf
    mov ecx, count
    rep insw
  }
}

static void outsw(int port, void *buf, int count)
{
  __asm
  {
    mov edx, port
    mov esi, buf
    mov ecx, count
    rep outsw
  }
}

static void insd(int port, void *buf, int count)
{
  __asm
  {
    mov edx, port
    mov edi, buf
    mov ecx, count
    rep insd
  }
}

static void outsd(int port, void *buf, int count)
{
  __asm
  {
    mov edx, port
    mov esi, buf
    mov ecx, count
    rep outsd
  }
}

static void hd_fixstring(unsigned char *s, int len)
{
  unsigned char *p = s;
  unsigned char *end = s + len;

  // Convert from big-endian to host byte order
  for (p = end ; p != s;) 
  {
    
    unsigned short *pp = (unsigned short *) (p -= 2);
    *pp = ((*pp & 0x00FF) << 8) | ((*pp & 0xFF00) >> 8);
  }

  // Strip leading blanks
  while (s != end && *s == ' ') ++s;

  // Compress internal blanks and strip trailing blanks
  while (s != end && *s) 
  {
    if (*s++ != ' ' || (s != end && *s && *s != ' ')) *p++ = *(s-1);
  }

  // Wipe out trailing garbage
  while (p != end) *p++ = '\0';
}

static void hd_error(char *func, unsigned char error)
{
  kprintf("%s: ", func);
  if (error & HDCE_BBK)   kprintf("bad block  ");
  if (error & HDCE_UNC)   kprintf("uncorrectable data  ");
  if (error & HDCE_MC)    kprintf("media change  ");
  if (error & HDCE_IDNF)  kprintf("id not found  ");
  if (error & HDCE_MCR)   kprintf("media change requested  ");
  if (error & HDCE_ABRT)  kprintf("abort  ");
  if (error & HDCE_TK0NF) kprintf("track 0 not found  ");
  if (error & HDCE_AMNF)  kprintf("address mark not found  ");
  kprintf("\n");
}

static int hd_wait(struct hdc *hdc, unsigned char mask, unsigned int timeout)
{
  unsigned int start;
  unsigned char status;

  start = get_tick_count();
  while (1)
  {
    status = _inp((unsigned short) (hdc->iobase + HDC_ALT_STATUS));
    if (status & HDCS_ERR) 
    {
      unsigned char error;
 
      error = _inp((unsigned short) (hdc->iobase + HDC_ERR));
      hd_error("hdwait", error);

      return error;
    }

    if (!(status & HDCS_BSY) && ((status & mask) == mask)) return 0;
    if (get_tick_count() - start >= timeout) return -ETIMEOUT;

    yield();
  }
}

static void hd_setup_transfer(struct hd *hd, blkno_t blkno, int nsects)
{
  unsigned int track;
  unsigned int head;
  unsigned int sector;

  if (hd->lba)
  {
    track = (blkno >> 8) & 0xFFFF;
    head = ((blkno >> 24) & 0xF) | 0x40;
    sector = blkno & 0xFF;
  }
  else 
  {
    track = blkno / (hd->heads * hd->sectors);
    head = (blkno / hd->sectors) % hd->heads;
    sector = blkno % hd->sectors + 1;
  }

  _outp((unsigned short) (hd->hdc->iobase + HDC_SECTORCNT), nsects);
  _outp((unsigned short) (hd->hdc->iobase + HDC_SECTOR), (unsigned char) sector);
  _outp((unsigned short) (hd->hdc->iobase + HDC_TRACKLSB), (unsigned char) track);
  _outp((unsigned short) (hd->hdc->iobase + HDC_TRACKMSB), (unsigned char) (track >> 8));
  _outp((unsigned short) (hd->hdc->iobase + HDC_DRVHD), (unsigned char) (head & 0xFF | hd->drvsel));
}

static void pio_read_sect(struct hd *hd, char *buffer)
{
  struct hdc *hdc = hd->hdc;

  if (hd->use32bits)
    insd(hdc->iobase + HDC_DATA, buffer, SECTORSIZE / 4);
  else
    insw(hdc->iobase + HDC_DATA, buffer, SECTORSIZE / 2);
}

static void pio_write_sect(struct hd *hd, char *buffer)
{
  struct hdc *hdc = hd->hdc;

  if (hd->use32bits)
    outsd(hdc->iobase + HDC_DATA, buffer, SECTORSIZE / 4);
  else
    outsw(hdc->iobase + HDC_DATA, buffer, SECTORSIZE / 2);
}

static void setup_dma_transfer(struct hdc *hdc, char *buffer, int count)
{
  int i;
  int len;
  char *next;

  i = 0;
  next = (char *) ((unsigned long) buffer & ~PAGESIZE) + PAGESIZE;
  while (1)
  {
    if (i == MAX_PRDS) panic("hd dma transfer too large");

    hdc->prds[i].addr = virt2phys(buffer);
    len = next - buffer;
    if (count > len)
    {
      hdc->prds[i].len = len;
      count -= len;
      buffer = next;
      next += PAGESIZE;
      i++;
    }
    else
    {
      hdc->prds[i].len = count | 0x80000000;
      break;
    }
  }

  _outpw((unsigned short) (hdc->bmregbase + BM_PRD_ADDR_LOW), (unsigned short) (hdc->prds_phys & 0xFFFF));
  _outpw((unsigned short) (hdc->bmregbase + BM_PRD_ADDR_HIGH), (unsigned short) (hdc->prds_phys >> 16));
}

static int hd_identify(struct hd *hd)
{
  // Ignore interrupt for identify command
  hd->hdc->dir = HD_XFER_IGNORE;

  // Issue read drive parameters command
  _outp(hd->hdc->iobase + HDC_DRVHD, hd->drvsel);
  _outp(hd->hdc->iobase + HDC_COMMAND, HDCMD_IDENTIFY);

  // Wait for data ready
  if (wait_for_object(&hd->hdc->ready, HDTIMEOUT_CMD) < 0) 
  {
    return -ETIMEOUT;
  }

  // Read parameter data
  insw(hd->hdc->iobase + HDC_DATA, &(hd->param), SECTORSIZE / 2);

  // Read status
  hd->hdc->status = _inp((unsigned short) (hd->hdc->iobase + HDC_STATUS));

  // Fill in drive parameters
  hd->cyls = hd->param.cylinders;
  hd->heads = hd->param.heads;
  hd->sectors = hd->param.sectors;
  hd->use32bits = hd->param.usedmovsd != 0;
  hd->sectbufs = hd->param.buffersize;
  hd_fixstring(hd->param.model, sizeof(hd->param.model));
  hd_fixstring(hd->param.rev, sizeof(hd->param.rev));
  hd_fixstring(hd->param.serial, sizeof(hd->param.serial));

  // Determine LBA or CHS mode
  if (hd->param.totalsec0 == 0 && hd->param.totalsec1 == 0)
  {
    hd->lba = 0;
    hd->blks = hd->cyls * hd->heads * hd->sectors;
    if (hd->cyls == 0 && hd->heads == 0 && hd->sectors == 0) return -EIO;
    if (hd->cyls == 0xFFFF && hd->heads == 0xFFFF && hd->sectors == 0xFFFF) return -EIO;
  }
  else
  {
    hd->lba = 1;
    hd->blks = (hd->param.totalsec1 << 16) | hd->param.totalsec0;
    if (hd->blks == 0 || hd->blks == 0xFFFFFFFF) return -EIO;
  }
  hd->size = hd->blks / (M / SECTORSIZE);

  return 0;
}

static int hd_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  struct hd *hd = (struct hd *) dev->privdata;
  struct geometry *geom;

  switch (cmd)
  {
    case IOCTL_GETDEVSIZE:
      return hd->blks;

    case IOCTL_GETBLKSIZE:
      return SECTORSIZE;

    case IOCTL_GETGEOMETRY:
      if (!args || size != sizeof(struct geometry)) return -EINVAL;
      geom = (struct geometry *) args;
      geom->cyls = hd->cyls;
      geom->heads = hd->heads;
      geom->spt = hd->sectors;
      geom->sectorsize = SECTORSIZE;
      geom->sectors = hd->blks;

      return 0;
  }

  return -ENOSYS;
}

static int hd_read_intr(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  struct hd *hd;
  struct hdc *hdc;
  int sectsleft;
  int nsects;
  int result;
  char *bufp;

//kprintf("hdread: block %d\n", blkno);
//kprintf("R");

  if (count == 0) return 0;
  bufp = (char *) buffer;
  hd = (struct hd *) dev->privdata;
  hdc = hd->hdc;
  sectsleft = count / SECTORSIZE;
  wait_for_object(&hdc->lock, INFINITE);

  while (sectsleft > 0)
  {
    // Wait for controller ready
    result = hd_wait(hdc, HDCS_DRDY, HDTIMEOUT_DRDY);
    if (result != 0) 
    {
      kprintf("hd_read: no drdy (%d)\n", result);
      hdc->result = -EIO;
      break;
    }

    // Calculate maximum number of sectors we can transfer
    if (sectsleft > 256)
      nsects = 256;
    else
      nsects = sectsleft;

    // Prepare transfer
    hdc->bufp = bufp;
    hdc->nsects = nsects;
    hdc->result = 0;
    hdc->dir = HD_XFER_READ;
    hdc->active = hd;
    reset_event(&hdc->ready);

    hd_setup_transfer(hd, blkno, nsects);
    _outp((unsigned short) (hdc->iobase + HDC_COMMAND), HDCMD_READ);

    // Wait until data read
    wait_for_object(&hdc->ready, INFINITE);
    if (hdc->result < 0) break;

    // Advance to next
    sectsleft -= nsects;
    bufp += nsects * SECTORSIZE;
  }

  // Cleanup
  hdc->dir = HD_XFER_IDLE;
  hdc->active = NULL;
  result = hdc->result;
  release_mutex(&hdc->lock);
  return result == 0 ? count : result;
}

static int hd_write_intr(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  struct hd *hd;
  struct hdc *hdc;
  int sectsleft;
  int nsects;
  int result;
  char *bufp;

//kprintf("hdwrite: block %d\n", blkno);
//kprintf("W");

  if (count == 0) return 0;
  bufp = (char *) buffer;
  hd = (struct hd *) dev->privdata;
  hdc = hd->hdc;
  sectsleft = count / SECTORSIZE;
  wait_for_object(&hdc->lock, INFINITE);

  while (sectsleft > 0)
  {
//kprintf("%d sects left\n", sectsleft);
    // Wait for controller ready
    result = hd_wait(hdc, HDCS_DRDY, HDTIMEOUT_DRDY);
    if (result != 0) 
    {
      kprintf("hd_write: no drdy (%d)\n", result);
      hdc->result = -EIO;
      break;
    }

    // Calculate maximum number of sectors we can transfer
//hd->sectbufs = 1;
    if (sectsleft > 256)
      nsects = 256;
    else
      nsects = sectsleft;

//kprintf("write %d sects\n", nsects);
    // Prepare transfer
    hdc->bufp = bufp;
    hdc->nsects = nsects;
    hdc->result = 0;
    hdc->dir = HD_XFER_WRITE;
    hdc->active = hd;
    reset_event(&hdc->ready);

    hd_setup_transfer(hd, blkno, nsects);
    _outp((unsigned short) (hdc->iobase + HDC_COMMAND), HDCMD_WRITE);

    // Wait for data ready
    if (!(_inp((unsigned short)(hdc->iobase + HDC_ALT_STATUS)) & HDCS_DRQ))
    {
      result = hd_wait(hdc, HDCS_DRQ, HDTIMEOUT_DRQ);
      if (result != 0)
      {
	kprintf("hd_write: no drq (%d)\n", result);
	hdc->result = -EIO;
	break;
      }
    }
    
    // Write first sector
    pio_write_sect(hd, hdc->bufp);

//kprintf("wait\n");
    // Wait until data written
    wait_for_object(&hdc->ready, INFINITE);
    if (hdc->result < 0) break;

//kprintf("ready\n");

    // Advance to next
    sectsleft -= nsects;
    bufp += nsects * SECTORSIZE;
  }

//kprintf("finito\n");
  // Cleanup
  hdc->dir = HD_XFER_IDLE;
  hdc->active = NULL;
  result = hdc->result;
  release_mutex(&hdc->lock);
  return result == 0 ? count : result;
}

static int hd_read_dma(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  struct hd *hd;
  struct hdc *hdc;
  int sectsleft;
  int nsects;
  int result;
  char *bufp;

  if (count == 0) return 0;
  bufp = (char *) buffer;

  hd = (struct hd *) dev->privdata;
  hdc = hd->hdc;
  sectsleft = count / SECTORSIZE;
  wait_for_object(&hdc->lock, INFINITE);

  while (sectsleft > 0)
  {
    // Wait for controller ready
    result = hd_wait(hdc, HDCS_DRDY, HDTIMEOUT_DRDY);
    if (result != 0) 
    {
      kprintf("hd_read_dma: no drdy (%d)\n", result);
      result = -EIO;
      break;
    }

    // Calculate maximum number of sectors we can transfer
    if (sectsleft > 256)
      nsects = 256;
    else
      nsects = sectsleft;

    if (nsects > MAX_DMA_XFER_SIZE / SECTORSIZE) nsects = MAX_DMA_XFER_SIZE / SECTORSIZE;

    // Setup DMA
    _outp((unsigned short) (hdc->bmregbase + BM_STATUS_REG), hdc->bmstat | BM_SR_MASK_INT | BM_SR_MASK_ERR);
    _outp((unsigned short) (hdc->bmregbase + BM_COMMAND_REG), BM_CR_MASK_STOP);
    setup_dma_transfer(hdc, bufp, nsects * SECTORSIZE);
    _outp((unsigned short) (hdc->bmregbase + BM_COMMAND_REG), BM_CR_MASK_WRITE);

    // Prepare transfer
    result = 0;
    hdc->dir = HD_XFER_DMA;
    hdc->active = hd;
    reset_event(&hdc->ready);

    hd_setup_transfer(hd, blkno, nsects);
    _outp((unsigned short) (hdc->iobase + HDC_COMMAND), HDCMD_READ_DMA);
    _outp((unsigned short) (hdc->bmregbase + BM_COMMAND_REG), BM_CR_MASK_WRITE | BM_CR_MASK_START);

    // Wait for interrupt
    wait_for_object(&hdc->ready, INFINITE);

    // Now wait for the PCI BM DMA channel to flush the data
    while (1)
    {
      if (_inp((unsigned short) (hdc->bmregbase + BM_STATUS_REG)) & BM_SR_MASK_INT) break;
    }

    // Check status
    hdc->status = _inp((unsigned short) (hdc->iobase + HDC_STATUS));
    if (hdc->status & HDCS_ERR)
    {
      unsigned char error;

      error = _inp((unsigned short) (hdc->iobase + HDC_ERR));
      hd_error("hdread", error);

      kprintf("hd: dma read error (%d)\n", hdc->status);
      result = -EIO;
      break;
    }

    // Stop DMA channel
    _outp((unsigned short) (hdc->bmregbase + BM_COMMAND_REG), BM_CR_MASK_STOP);

    // Advance to next
    sectsleft -= nsects;
    bufp += nsects * SECTORSIZE;
  }

  // Cleanup
  hdc->dir = HD_XFER_IDLE;
  hdc->active = NULL;
  release_mutex(&hdc->lock);
  return result == 0 ? count : result;
}

static int hd_write_dma(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  struct hd *hd;
  struct hdc *hdc;
  int sectsleft;
  int nsects;
  int result;
  char *bufp;

  if (count == 0) return 0;
  bufp = (char *) buffer;

  hd = (struct hd *) dev->privdata;
  hdc = hd->hdc;
  sectsleft = count / SECTORSIZE;
  wait_for_object(&hdc->lock, INFINITE);

  while (sectsleft > 0)
  {
    // Wait for controller ready
    result = hd_wait(hdc, HDCS_DRDY, HDTIMEOUT_DRDY);
    if (result != 0) 
    {
      kprintf("hd_write_dma: no drdy (%d)\n", result);
      result = -EIO;
      break;
    }

    // Calculate maximum number of sectors we can transfer
    if (sectsleft > 256)
      nsects = 256;
    else
      nsects = sectsleft;

    if (nsects > MAX_DMA_XFER_SIZE / SECTORSIZE) nsects = MAX_DMA_XFER_SIZE / SECTORSIZE;

    // Setup DMA
    _outp((unsigned short) (hdc->bmregbase + BM_STATUS_REG), hdc->bmstat | BM_SR_MASK_INT | BM_SR_MASK_ERR);
    _outp((unsigned short) (hdc->bmregbase + BM_COMMAND_REG), BM_CR_MASK_STOP);
    setup_dma_transfer(hdc, bufp, nsects * SECTORSIZE);
    _outp((unsigned short) (hdc->bmregbase + BM_COMMAND_REG), BM_CR_MASK_READ);

    // Prepare transfer
    result = 0;
    hdc->dir = HD_XFER_DMA;
    hdc->active = hd;
    reset_event(&hdc->ready);

    hd_setup_transfer(hd, blkno, nsects);
    _outp((unsigned short) (hdc->iobase + HDC_COMMAND), HDCMD_WRITE_DMA);
    _outp((unsigned short) (hdc->bmregbase + BM_COMMAND_REG), BM_CR_MASK_READ | BM_CR_MASK_START);

    // Wait for interrupt
    wait_for_object(&hdc->ready, INFINITE);

    // Now wait for the PCI BM DMA channel to flush the data
    while (1)
    {
      if (_inp((unsigned short) (hdc->bmregbase + BM_STATUS_REG)) & BM_SR_MASK_INT) break;
    }

    // Check status
    hdc->status = _inp((unsigned short) (hdc->iobase + HDC_STATUS));
    if (hdc->status & HDCS_ERR)
    {
      unsigned char error;

      error = _inp((unsigned short) (hdc->iobase + HDC_ERR));
      hd_error("hdwrite", error);

      kprintf("hd: dma write error (%d)\n", hdc->status);
      result = -EIO;
      break;
    }

    // Stop DMA channel
    _outp((unsigned short) (hdc->bmregbase + BM_COMMAND_REG), BM_CR_MASK_STOP);

    // Advance to next
    sectsleft -= nsects;
    bufp += nsects * SECTORSIZE;
  }

  // Cleanup
  hdc->dir = HD_XFER_IDLE;
  hdc->active = NULL;
  release_mutex(&hdc->lock);
  return result == 0 ? count : result;
}

void hd_dpc(void *arg)
{
  struct hdc *hdc = (struct hdc *) arg;

//kprintf("[hddpc]");
  switch (hdc->dir)
  {
    case HD_XFER_READ:
      // Read sector data
      pio_read_sect(hdc->active, hdc->bufp);

      // Check status
      hdc->status = _inp((unsigned short) (hdc->iobase + HDC_STATUS));
      if (hdc->status & HDCS_ERR)
      {
        unsigned char error;

        error = _inp((unsigned short) (hdc->iobase + HDC_ERR));
        hd_error("hdread", error);

	kprintf("hd: read error (%d)\n", hdc->status);
	hdc->result = -EIO;
	set_event(&hdc->ready);
      }

      // Signal event if we have read all sectors
      hdc->bufp += SECTORSIZE;
      hdc->nsects--;
      if (hdc->nsects == 0) set_event(&hdc->ready);
      
      break;

    case HD_XFER_WRITE:
      // Check status
      hdc->status = _inp((unsigned short) (hdc->iobase + HDC_STATUS));
      if (hdc->status & HDCS_ERR)
      {
        unsigned char error;

        error = _inp((unsigned short) (hdc->iobase + HDC_ERR));
        hd_error("hdwrite", error);

	kprintf("hd: write error (%d)\n", hdc->status);
	hdc->result = -EIO;
	set_event(&hdc->ready);
      }

      // Transfer next sector or signal end of transfer
      hdc->bufp += SECTORSIZE;
      hdc->nsects--;
      if (hdc->nsects > 0) 
	pio_write_sect(hdc->active, hdc->bufp);
      else
	set_event(&hdc->ready);

      break;

    case HD_XFER_DMA:
      set_event(&hdc->ready);
      break;

    case HD_XFER_IGNORE:
      // Read status to acknowledge interrupt
      hdc->status = _inp((unsigned short) (hdc->iobase + HDC_STATUS));
      set_event(&hdc->ready);
      break;

    case HD_XFER_IDLE:
    default:
      // Read status to acknowledge interrupt
      hdc->status = _inp((unsigned short) (hdc->iobase + HDC_STATUS));
      kprintf("unexpected intr from hdc\n");
  }
}

void hdc_handler(struct context *ctxt, void *arg)
{
  struct hdc *hdc = (struct hdc *) arg;

  queue_irq_dpc(&hdc->xfer_dpc, hd_dpc, hdc);
  eoi(hdc->irq);
}

static int part_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  struct partition *part = (struct partition *) dev->privdata;

  switch (cmd)
  {
    case IOCTL_GETDEVSIZE:
      return part->len;

    case IOCTL_GETBLKSIZE:
      return dev_ioctl(part->dev, IOCTL_GETBLKSIZE, NULL, 0);
  }

  return -ENOSYS;
}

static int part_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  struct partition *part = (struct partition *) dev->privdata;
  if (blkno + count / SECTORSIZE > part->len) return -EFAULT;
  return dev_read(part->dev, buffer, count, blkno + part->start);
}

static int part_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  struct partition *part = (struct partition *) dev->privdata;
  if (blkno + count / SECTORSIZE > part->len) return -EFAULT;
  return dev_write(part->dev, buffer, count, blkno + part->start);
}

struct driver harddisk_dma_driver =
{
  "idedisk/dma",
  DEV_TYPE_BLOCK,
  hd_ioctl,
  hd_read_dma,
  hd_write_dma
};

struct driver harddisk_intr_driver =
{
  "idedisk",
  DEV_TYPE_BLOCK,
  hd_ioctl,
  hd_read_intr,
  hd_write_intr
};

struct driver partition_driver =
{
  "partition", 
  DEV_TYPE_BLOCK,
  part_ioctl,
  part_read,
  part_write
};

static void setup_hdc(struct hdc *hdc, int iobase, int irq, int bmregbase)
{
  memset(hdc, 0, sizeof(struct hdc));
  hdc->iobase = iobase;
  hdc->irq = irq;
  hdc->bmregbase = bmregbase;
  hdc->dir = HD_XFER_IGNORE;

  if (hdc->bmregbase)
  {
    // Save upper three bits of bus master status
    hdc->bmstat = _inp((unsigned short) (hdc->bmregbase + BM_STATUS_REG)) & 0xE0;

    // Allocate one page for PRD list
    hdc->prds = (struct prd *) kmalloc(PAGESIZE);
    hdc->prds_phys = (unsigned long) virt2phys(hdc->prds);
  }

  init_dpc(&hdc->xfer_dpc);
  init_mutex(&hdc->lock, 0);
  init_event(&hdc->ready, 0, 0);

  // Enable interrupts
  set_interrupt_handler(IRQ2INTR(irq), hdc_handler, hdc);
  enable_irq(irq);
  _outp((unsigned short) (iobase + HDC_CONTROL), HDDC_HD15);
}

static void setup_hd(struct hd *hd, struct hdc *hdc, char *devname, int drvsel, char *partnames[HD_PARTITIONS])
{
  int i;
  struct master_boot_record mbr;
  devno_t devno;
  devno_t partdevno;

  memset(hd, 0, sizeof(struct hd));
  hd->hdc = hdc;
  hd->drvsel = drvsel;

  if (hd_identify(hd) < 0)
  {
    kprintf("hd: device %s not responding, ignored.\n", devname);
    return;
  }

  if (hdc->bmregbase)
    devno = dev_make(devname, &harddisk_dma_driver, NULL, hd);
  else
    devno = dev_make(devname, &harddisk_intr_driver, NULL, hd);

  if (hd->lba)
    kprintf("%s: %s (%d MB) %u blks (%d bits, %s, %d bufs)\n", devname, hd->param.model, hd->size, hd->blks, hd->use32bits ? 32 : 16, hdc->bmregbase ? "dma" : "intr", hd->param.buffersize);
  else
    kprintf("%s: %s (%d MB) CHS=%u/%u/%u (%d bits, %s, %d bufs)\n", devname, hd->param.model, hd->size, hd->cyls, hd->heads, hd->sectors, hd->use32bits ? 32 : 16, hdc->bmregbase ? "dma" : "intr", hd->param.buffersize);
  
  dev_read(devno, &mbr, SECTORSIZE, 0);

  if (mbr.signature != MBR_SIGNATURE)
  {
    kprintf("%s: illegal boot sector signature\n", devname);
  }
  else
  {
    for (i = 0; i < HD_PARTITIONS; i++)
    {
      if (mbr.parttab[i].systid != 0)
      {
	hd->parts[i].dev = devno;
	hd->parts[i].bootid = mbr.parttab[i].bootid;
	hd->parts[i].systid = mbr.parttab[i].systid;
	hd->parts[i].start = mbr.parttab[i].relsect;
	hd->parts[i].len = mbr.parttab[i].numsect;

	partdevno = dev_make(partnames[i], &partition_driver, NULL, &hd->parts[i]);
 
	kprintf("%s: partition %d on %s, %dMB (type %02x)\n", partnames[i], i, devname, mbr.parttab[i].numsect / (M / SECTORSIZE), mbr.parttab[i].systid);
      }
    }
  }
}

void init_hd()
{
  int bmiba;
  int numhd;
  struct pci_dev *idedev;

  numhd = syspage->biosdata[0x75];

  idedev = lookup_pci_device_class(PCI_CLASS_STORAGE_IDE, PCI_SUBCLASS_MASK);
  if (idedev)
  {
    bmiba = pci_config_read(idedev->bus->busno, idedev->devno, idedev->funcno, PCI_CONFIG_BASE_ADDR_4) & 0xFFF0;
  }

  if (numhd >= 1) setup_hdc(&hdctab[0], HDC0_IOBASE, HDC0_IRQ, idedev ? bmiba : 0);
  if (numhd >= 3) setup_hdc(&hdctab[1], HDC1_IOBASE, HDC1_IRQ, idedev ? bmiba + 8: 0);

  if (numhd >= 1) setup_hd(&hdtab[0], &hdctab[0], "hd0", HD0_DRVSEL, hd0parts);
  if (numhd >= 2) setup_hd(&hdtab[1], &hdctab[0], "hd1", HD1_DRVSEL, hd1parts);
  if (numhd >= 3) setup_hd(&hdtab[2], &hdctab[1], "hd2", HD0_DRVSEL, hd2parts);
  if (numhd >= 4) setup_hd(&hdtab[3], &hdctab[1], "hd3", HD1_DRVSEL, hd3parts);
}
