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
#define HDCMD_DIAG		0x90
#define HDCMD_READ		0x20
#define HDCMD_WRITE		0x30
#define HDCMD_MULTREAD		0xC4
#define HDCMD_MULTWRITE		0xC5
#define HDCMD_SETMULT		0xC6
#define HDCMD_READDMA           0xC8
#define HDCMD_WRITEDMA          0xCA

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

#define HDDC_HD15               0x08  // Use 4 bits for head
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
#define BM_PRD_ADDR       4            // Offset to PRD addr reg

//#define BM_PRD_ADDR_LOW   4            // Offset to PRD addr reg low 16 bits
//#define BM_PRD_ADDR_HIGH  6            // Offset to PRD addr reg high 16 bits

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
//

struct hdparam 
{
  unsigned short config;	       // General configuration bits
  unsigned short cylinders;	       // Cylinders
  unsigned short reserved;
  unsigned short heads;		       // Heads
  unsigned short unfbytespertrk;       // Unformatted bytes/track
  unsigned short unfbytes;	       // Unformatted bytes/sector
  unsigned short sectors;	       // Sectors per track
  unsigned short vendorunique[3];
  char serial[20];		       // Serial number
  unsigned short buffertype;	       // Buffer type
  unsigned short buffersize;	       // Buffer size, in 512-byte units
  unsigned short necc;		       // ECC bytes appended
  char rev[8];		               // Firmware revision
  char model[40];		       // Model name
  unsigned char nsecperint;	       // Sectors per interrupt
  unsigned char resv0;                 // Reserved
  unsigned short usedmovsd;	       // Can use double word read/write?
  unsigned short caps;                 // Capabilities
  unsigned short resv1;                // Reserved
  unsigned short pio;                  // PIO data transfer cycle timing (0=slow, 1=medium, 2=fast)
  unsigned short dma;                  // DMA data transfer cycle timing (0=slow, 1=medium, 2=fast)
  unsigned short valid;                // Flag valid fields to follow
  unsigned short logcyl;               // Logical cylinders
  unsigned short loghead;              // Logical heads
  unsigned short logspt;               // Logical sector/track
  unsigned short cap0;                 // Capacity in sectors (32-bit)
  unsigned short cap1;
  unsigned short multisec;             // Multiple sector values
  unsigned short totalsec0;            // Total number of user sectors
  unsigned short totalsec1;            //  (LBA; 32-bit value)
  unsigned short dmasingle;            // Single-word DMA info
  unsigned short dmamulti;             // Multi-word DMA info
  unsigned short piomode;              // Advanced PIO modes
  unsigned short minmulti;             // Minimum multiword xfer
  unsigned short multitime;            // Recommended cycle time
  unsigned short minpio;               // Min PIO without flow ctl
  unsigned short miniodry;             // Min with IORDY flow
  unsigned short resv2[6];             // Reserved
  unsigned short queue_depth;          // Queue depth
  unsigned short resv3[4];             // Reserved
  unsigned short vermajor;             // Major version number
  unsigned short verminor;             // Minor version number
  unsigned short cmdset1;              // Command set supported
  unsigned short cmdset2;
  unsigned short cfsse;		       // Command set-feature supported extensions
  unsigned short cfs_enable_1;	       // Command set-feature enabled
  unsigned short cfs_enable_2;	       // Command set-feature enabled
  unsigned short csf_default;	       // Command set-feature default
  unsigned short dmaultra;	       // UltraDMA mode (0:5 = supported mode, 8:13 = selected mode)

  unsigned short word89;	       // Reserved (word 89)
  unsigned short word90;	       // Reserved (word 90)
  unsigned short curapmvalues;	       // Current APM values
  unsigned short word92;	       // Reserved (word 92)
  unsigned short hw_config;	       // Hardware config
  unsigned short words94_125[32];      // Reserved words 94-125
  unsigned short last_lun; 	       // Reserved (word 126)
  unsigned short word127;	       // Reserved (word 127)
  unsigned short dlf;		       // Device lock function
				       // 15:9	reserved
				       // 8	security level 1:max 0:high
				       // 7:6	reserved
				       // 5	enhanced erase
				       // 4	expire
				       // 3	frozen
				       // 2	locked
				       // 1	en/disabled
				       // 0	capability
					
  unsigned short csfo;		       // Current set features options
				       // 15:4 reserved
				       // 3	 auto reassign
				       // 2	 reverting
				       // 1	 read-look-ahead
				       // 0	 write cache
					 
  unsigned short words130_155[26];     // Reserved vendor words 130-155
  unsigned short word156;
  unsigned short words157_159[3];      // Reserved vendor words 157-159
  unsigned short words160_255[96];     // Reserved words 160-255
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
  int multsect;                         // Sectors per interrupt
  int udmamode;                         // UltraDMA mode

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
    if (*s++ != ' ' || (s != end && *s && *s != ' ')) *p++ = *(s - 1);
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

  start = clocks;
  while (1)
  {
    status = _inp(hdc->iobase + HDC_ALT_STATUS);
    if (status & HDCS_ERR) 
    {
      unsigned char error;
 
      error = _inp(hdc->iobase + HDC_ERR);
      hd_error("hdwait", error);

      return error;
    }

    if (!(status & HDCS_BSY) && ((status & mask) == mask)) return 0;
    if (time_before(start + timeout, clocks)) return -ETIMEOUT;

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

  _outp(hd->hdc->iobase + HDC_SECTORCNT, nsects);
  _outp(hd->hdc->iobase + HDC_SECTOR, (unsigned char) sector);
  _outp(hd->hdc->iobase + HDC_TRACKLSB, (unsigned char) track);
  _outp(hd->hdc->iobase + HDC_TRACKMSB, (unsigned char) (track >> 8));
  _outp(hd->hdc->iobase + HDC_DRVHD, (unsigned char) (head & 0xFF | hd->drvsel));
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

  _outpd(hdc->bmregbase + BM_PRD_ADDR, hdc->prds_phys);
}

static int hd_identify(struct hd *hd)
{
  // Ignore interrupt for identify command
  hd->hdc->dir = HD_XFER_IGNORE;

  // Issue read drive parameters command
  _outp(hd->hdc->iobase + HDC_DRVHD, hd->drvsel);
  _outp(hd->hdc->iobase + HDC_COMMAND, HDCMD_IDENTIFY);

  // Wait for data ready
  if (wait_for_object(&hd->hdc->ready, HDTIMEOUT_CMD) < 0) return -ETIMEOUT;

  // Read parameter data
  insw(hd->hdc->iobase + HDC_DATA, &(hd->param), SECTORSIZE / 2);

  // Fill in drive parameters
  hd->cyls = hd->param.cylinders;
  hd->heads = hd->param.heads;
  hd->sectors = hd->param.sectors;
  hd->use32bits = hd->param.usedmovsd != 0;
  hd->sectbufs = hd->param.buffersize;
  hd->multsect = hd->param.nsecperint;
  if (hd->multsect == 0) hd->multsect = 1;

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

static int hd_cmd(struct hd *hd, unsigned int cmd, unsigned int nsects)
{
  // Ignore interrupt for setmult command
  hd->hdc->dir = HD_XFER_IGNORE;

  // Issue setmult command
  _outp(hd->hdc->iobase + HDC_SECTORCNT, nsects);
  _outp(hd->hdc->iobase + HDC_DRVHD, hd->drvsel);
  _outp(hd->hdc->iobase + HDC_COMMAND, cmd);

  // Wait for data ready
  if (wait_for_object(&hd->hdc->ready, HDTIMEOUT_CMD) < 0) return -ETIMEOUT;

  // Check status
  if (hd->hdc->result < 0) return -EIO;

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
      kprintf("hd_read: no drdy (0x%02x)\n", result);
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
    _outp(hdc->iobase + HDC_COMMAND, hd->multsect > 1 ? HDCMD_MULTREAD : HDCMD_READ);

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
  int n;
  int result;
  char *bufp;

//kprintf("hdwrite: block %d\n", blkno);

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
      kprintf("hd_write: no drdy (0x%02x)\n", result);
      hdc->result = -EIO;
      break;
    }

    // Calculate maximum number of sectors we can transfer
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
    _outp(hdc->iobase + HDC_COMMAND, hd->multsect > 1 ? HDCMD_MULTWRITE : HDCMD_WRITE);

    // Wait for data ready
    if (!(_inp(hdc->iobase + HDC_ALT_STATUS) & HDCS_DRQ))
    {
      result = hd_wait(hdc, HDCS_DRQ, HDTIMEOUT_DRQ);
      if (result != 0)
      {
	kprintf("hd_write: no drq (0x%02x)\n", result);
	hdc->result = -EIO;
	break;
      }
    }
    
    // Write first sector(s)
    n = hd->multsect;
    if (n > nsects) n = nsects;
    while (n-- > 0)
    {
      pio_write_sect(hd, hdc->bufp);
      hdc->bufp += SECTORSIZE;
    }

//kprintf("wait\n");
    // Wait until data written
    wait_for_object(&hdc->ready, INFINITE);
    hdc->dir = HD_XFER_IDLE;
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
  unsigned char bmstat;

  if (count == 0) return 0;
  bufp = (char *) buffer;

  hd = (struct hd *) dev->privdata;
  hdc = hd->hdc;
  sectsleft = count / SECTORSIZE;
  wait_for_object(&hdc->lock, INFINITE);

  //kprintf("hdread block %d size %d buffer %p\n", blkno, count, buffer);

  while (sectsleft > 0)
  {
    // Wait for controller ready
    result = hd_wait(hdc, HDCS_DRDY, HDTIMEOUT_DRDY);
    if (result != 0) 
    {
      kprintf("hd_read: no drdy (0x%02x)\n", result);
      result = -EIO;
      break;
    }

    // Calculate maximum number of sectors we can transfer
    if (sectsleft > 256)
      nsects = 256;
    else
      nsects = sectsleft;

    if (nsects > MAX_DMA_XFER_SIZE / SECTORSIZE) nsects = MAX_DMA_XFER_SIZE / SECTORSIZE;

    // Prepare transfer
    result = 0;
    hdc->dir = HD_XFER_DMA;
    hdc->active = hd;
    reset_event(&hdc->ready);

    hd_setup_transfer(hd, blkno, nsects);
    
    // Setup DMA
    setup_dma_transfer(hdc, bufp, nsects * SECTORSIZE);
    _outp(hdc->bmregbase + BM_COMMAND_REG, BM_CR_MASK_WRITE | BM_CR_MASK_STOP);
    _outp(hdc->bmregbase + BM_STATUS_REG, _inp(hdc->bmregbase + BM_STATUS_REG) | BM_SR_MASK_INT | BM_SR_MASK_ERR);

    // Start read
    _outp(hdc->iobase + HDC_COMMAND, HDCMD_READDMA);
    _outp(hdc->bmregbase + BM_COMMAND_REG, BM_CR_MASK_WRITE | BM_CR_MASK_START);

    // Wait for interrupt
    wait_for_object(&hdc->ready, INFINITE);

    // Stop DMA channel and check DMA status
    _outp(hdc->bmregbase + BM_COMMAND_REG, BM_CR_MASK_WRITE | BM_CR_MASK_STOP);
    bmstat = _inp(hdc->bmregbase + BM_STATUS_REG);
    _outp(hdc->bmregbase + BM_STATUS_REG, bmstat | BM_SR_MASK_INT | BM_SR_MASK_ERR);
    if ((bmstat & (BM_SR_MASK_INT | BM_SR_MASK_ERR | BM_SR_MASK_ACT)) != BM_SR_MASK_INT)
    {
      kprintf("hd: dma error %02X\n", bmstat);
      result = -EIO;
      break;
    }

    // Check controller status
    if (hdc->status & HDCS_ERR)
    {
      unsigned char error;

      error = _inp(hdc->iobase + HDC_ERR);
      hd_error("hdread", error);

      kprintf("hd: read error (0x%02x)\n", hdc->status);
      result = -EIO;
      break;
    }

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
  unsigned char bmstat;

  if (count == 0) return 0;
  bufp = (char *) buffer;

  hd = (struct hd *) dev->privdata;
  hdc = hd->hdc;
  sectsleft = count / SECTORSIZE;
  wait_for_object(&hdc->lock, INFINITE);

  //kprintf("hdwrite block %d size %d buffer %p\n", blkno, count, buffer);

  while (sectsleft > 0)
  {
    // Wait for controller ready
    result = hd_wait(hdc, HDCS_DRDY, HDTIMEOUT_DRDY);
    if (result != 0) 
    {
      kprintf("hd_write: no drdy (0x%02x)\n", result);
      result = -EIO;
      break;
    }

    // Calculate maximum number of sectors we can transfer
    if (sectsleft > 256)
      nsects = 256;
    else
      nsects = sectsleft;

    if (nsects > MAX_DMA_XFER_SIZE / SECTORSIZE) nsects = MAX_DMA_XFER_SIZE / SECTORSIZE;

    // Prepare transfer
    result = 0;
    hdc->dir = HD_XFER_DMA;
    hdc->active = hd;
    reset_event(&hdc->ready);

    hd_setup_transfer(hd, blkno, nsects);

    // Setup DMA
    setup_dma_transfer(hdc, bufp, nsects * SECTORSIZE);
    _outp(hdc->bmregbase + BM_COMMAND_REG, BM_CR_MASK_READ | BM_CR_MASK_STOP);
    _outp(hdc->bmregbase + BM_STATUS_REG, _inp(hdc->bmregbase + BM_STATUS_REG) | BM_SR_MASK_INT | BM_SR_MASK_ERR);
    
    // Start write
    _outp(hdc->iobase + HDC_COMMAND, HDCMD_WRITEDMA);
    _outp(hdc->bmregbase + BM_COMMAND_REG, BM_CR_MASK_READ | BM_CR_MASK_START);

    // Wait for interrupt
    wait_for_object(&hdc->ready, INFINITE);

    // Stop DMA channel and check DMA status
    _outp(hdc->bmregbase + BM_COMMAND_REG, BM_CR_MASK_READ | BM_CR_MASK_STOP);
    bmstat = _inp(hdc->bmregbase + BM_STATUS_REG);
    _outp(hdc->bmregbase + BM_STATUS_REG, bmstat | BM_SR_MASK_INT | BM_SR_MASK_ERR);
    if ((bmstat & (BM_SR_MASK_INT | BM_SR_MASK_ERR | BM_SR_MASK_ACT)) != BM_SR_MASK_INT)
    {
      kprintf("hd: dma error %02X\n", bmstat);
      result = -EIO;
      break;
    }

    // Check controller status
    if (hdc->status & HDCS_ERR)
    {
      unsigned char error;

      error = _inp(hdc->iobase + HDC_ERR);
      hd_error("hdwrite", error);

      kprintf("hd: write error (0x%02x)\n", hdc->status);
      result = -EIO;
      break;
    }

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
  int nsects;
  int n;

  //kprintf("[hddpc]");
  switch (hdc->dir)
  {
    case HD_XFER_READ:
      // Read sector data
      nsects = hdc->active->multsect;
      if (nsects > hdc->nsects) nsects = hdc->nsects;
      for (n = 0; n < nsects; n++)
      {
        pio_read_sect(hdc->active, hdc->bufp);
        hdc->bufp += SECTORSIZE;
      }

      // Check status
      hdc->status = _inp(hdc->iobase + HDC_STATUS);
      if (hdc->status & HDCS_ERR)
      {
        unsigned char error;

        error = _inp(hdc->iobase + HDC_ERR);
        hd_error("hdread", error);

	kprintf("hd: read error (0x%02x)\n", hdc->status);
	hdc->result = -EIO;
	set_event(&hdc->ready);
      }

      // Signal event if we have read all sectors
      hdc->nsects -= nsects;
      if (hdc->nsects == 0) set_event(&hdc->ready);
      
      break;

    case HD_XFER_WRITE:
      // Check status
      hdc->status = _inp(hdc->iobase + HDC_STATUS);
      if (hdc->status & HDCS_ERR)
      {
        unsigned char error;

        error = _inp(hdc->iobase + HDC_ERR);
        hd_error("hdwrite", error);

	kprintf("hd: write error (0x%02x)\n", hdc->status);
	hdc->result = -EIO;
	set_event(&hdc->ready);
      }

      // Transfer next sector(s) or signal end of transfer
      nsects = hdc->active->multsect;
      if (nsects > hdc->nsects) nsects = hdc->nsects;
      hdc->nsects -= nsects;

      if (hdc->nsects > 0)
      {
	nsects = hdc->active->multsect;
        if (nsects > hdc->nsects) nsects = hdc->nsects;

	for (n = 0; n < nsects; n++)
	{
  	  pio_write_sect(hdc->active, hdc->bufp);
	  hdc->bufp += SECTORSIZE;
	}
      }
      else
	set_event(&hdc->ready);

      break;

    case HD_XFER_DMA:
      hdc->status = _inp(hdc->iobase + HDC_STATUS);
      set_event(&hdc->ready);
      break;

    case HD_XFER_IGNORE:
      // Read status to acknowledge interrupt
      hdc->status = _inp(hdc->iobase + HDC_STATUS);
      set_event(&hdc->ready);
      break;

    case HD_XFER_IDLE:
    default:
      // Read status to acknowledge interrupt
      hdc->status = _inp(hdc->iobase + HDC_STATUS);
      kprintf("unexpected intr from hdc\n");
  }
}

void hdc_handler(struct context *ctxt, void *arg)
{
  struct hdc *hdc = (struct hdc *) arg;

  if (hdc->xfer_dpc.flags & DPC_QUEUED) kprintf("hd: intr lost\n");
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

static int setup_hdc(struct hdc *hdc, int iobase, int irq, int bmregbase)
{
  memset(hdc, 0, sizeof(struct hdc));
  hdc->iobase = iobase;
  hdc->irq = irq;
  hdc->bmregbase = bmregbase;
  hdc->dir = HD_XFER_IGNORE;

  if (hdc->bmregbase)
  {
    // Allocate one page for PRD list
    hdc->prds = (struct prd *) kmalloc(PAGESIZE);
    hdc->prds_phys = (unsigned long) virt2phys(hdc->prds);
  }

  init_dpc(&hdc->xfer_dpc);
  init_mutex(&hdc->lock, 0);
  init_event(&hdc->ready, 0, 0);

#if 0 
  // Reset controller
  _outp(hdc->iobase + HDC_CONTROL, HDDC_SRST | HDDC_NIEN);
  sleep(10);
  _outp(hdc->iobase + HDC_CONTROL, HDDC_NIEN);
  sleep(10);
#endif

  // Enable interrupts
  set_interrupt_handler(IRQ2INTR(irq), hdc_handler, hdc);
  enable_irq(irq);
  _outp(hdc->iobase + HDC_CONTROL, HDDC_HD15);

#if 0
  // Performs internal diagnostic tests implemented by the drive
  _outp(hdc->iobase + HDC_COMMAND, HDCMD_DIAG);
  if (wait_for_object(&hdc->ready, HDTIMEOUT_CMD) >= 0)
  {
    hdc->status = _inp(hdc->iobase + HDC_STATUS);
    if (hdc->status & HDCS_ERR)
    {
      unsigned char error;

      error = _inp(hdc->iobase + HDC_ERR);
      hd_error("hddiag", error);
      return -EIO;
    }
  }
#endif

  return 0;
}

static void setup_hd(struct hd *hd, struct hdc *hdc, char *devname, int drvsel, char *partnames[HD_PARTITIONS])
{
  static int udma_speed[] = {16, 25, 33, 44, 66, 100};

  int i;
  struct master_boot_record mbr;
  devno_t devno;
  devno_t partdevno;
  int rc;

  memset(hd, 0, sizeof(struct hd));
  hd->hdc = hdc;
  hd->drvsel = drvsel;

  if (hd_identify(hd) < 0)
  {
    kprintf("hd: device %s not responding, ignored.\n", devname);
    return;
  }

  // Determine UDMA mode
  if (!hdc->bmregbase)
    hd->udmamode = -1;
  else if ((hd->param.valid & 4) &&  (hd->param.dmaultra & (hd->param.dmaultra >> 8) & 0x3F))
  {
    if ((hd->param.dmaultra >> 13) & 1)
      hd->udmamode = 5; // UDMA 100
    else if ((hd->param.dmaultra>> 12) & 1)
      hd->udmamode = 4; // UDMA 66
    else if ((hd->param.dmaultra>> 11) & 1)
      hd->udmamode = 3; // UDMA 44
    else if ((hd->param.dmaultra >> 10) & 1)
      hd->udmamode = 2; // UDMA 33
    else if ((hd->param.dmaultra >> 9) & 1)
      hd->udmamode = 1; // UDMA 25
    else
      hd->udmamode = 0; // UDMA 16
  }
  else
    hd->udmamode = -1;

  // Set multi-sector mode if drive supports it
  if (hd->multsect > 1)
  {
    rc = hd_cmd(hd, HDCMD_SETMULT, hd->multsect);
    if (rc < 0)
    {
      kprintf("hd: unable to set multi sector mode\n");
      hd->multsect = 1;
    }
  }

  // Make new device
  if (hd->udmamode != -1)
    devno = dev_make(devname, &harddisk_dma_driver, NULL, hd);
  else
    devno = dev_make(devname, &harddisk_intr_driver, NULL, hd);

  kprintf("%s: %s (%d MB)", devname, hd->param.model, hd->size);
  if (hd->lba) kprintf(", LBA");
  if (hd->udmamode != -1) kprintf(", UDMA%d", udma_speed[hd->udmamode]);
  if (hd->param.csfo & 2) kprintf(", read ahead");
  if (hd->param.csfo & 1) kprintf(", write cache");
  if (hd->udmamode == -1 && hd->multsect > 1) kprintf(", %d sects/intr", hd->multsect);
  kprintf("\n");

  // Create partitions
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
  struct unit *ide;
  int rc;

  numhd = syspage->biosdata[0x75];

  ide = lookup_unit_by_class(NULL, PCI_CLASS_STORAGE_IDE, PCI_SUBCLASS_MASK);
  if (ide)
  {
    bmiba = pci_unit_read(ide, PCI_CONFIG_BASE_ADDR_4) & 0xFFF0;
  }

  if (numhd >= 1) 
  {
    rc = setup_hdc(&hdctab[0], HDC0_IOBASE, HDC0_IRQ, ide ? bmiba : 0);
    if (rc < 0)
      kprintf("hd: error %d initializing primary ide controller\n", rc);
    else
    {
      if (numhd >= 1) setup_hd(&hdtab[0], &hdctab[0], "hd0", HD0_DRVSEL, hd0parts);
      if (numhd >= 2) setup_hd(&hdtab[1], &hdctab[0], "hd1", HD1_DRVSEL, hd1parts);
    }
  }

  if (numhd >= 3) 
  {
    rc = setup_hdc(&hdctab[1], HDC1_IOBASE, HDC1_IRQ, ide ? bmiba + 8: 0);
    if (rc < 0)
      kprintf("hd: error %d initializing secondary ide controller\n", rc);
    else
    {
      if (numhd >= 3) setup_hd(&hdtab[2], &hdctab[1], "hd2", HD0_DRVSEL, hd2parts);
      if (numhd >= 4) setup_hd(&hdtab[3], &hdctab[1], "hd3", HD1_DRVSEL, hd3parts);
    }
  }
}
