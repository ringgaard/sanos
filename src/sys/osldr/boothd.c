//
// boothd.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Boot Harddisk driver
//

#include <os/krnl.h>

#define SECTORSIZE              512

#define HDC0_IOBASE		0x01F0
#define HDC1_IOBASE		0x0170

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
// Parameters returned by read drive parameters command

struct hdparam 
{
  // Drive information
  short config;		               // General configuration bits
  unsigned short cylinders;	       // Cylinders
  short reserved;
  unsigned short heads;		       // Heads
  short unfbytespertrk;	               // Unformatted bytes/track
  short unfbytes;		       // Unformatted bytes/sector
  unsigned short sectors;	       // Sectors per track
  short vendorunique[3];

  // Controller information
  char serial[20];		       // Serial number
  short buffertype;		       // Buffer type
  short buffersize;		       // Buffer size, in 512-byte units
  short necc;			       // ECC bytes appended
  char rev[8];		               // Firmware revision
  char model[40];		       // Model name
  short nsecperint;		       // Sectors per interrupt
  short usedmovsd;		       // Can use double word read/write?
  char pad[418];
};

struct hd 
{
  int iobase;	                        // I/O port registers base address
  struct hdparam param;		        // Drive parameter block
  int drvsel;                           // Drive select on controller
  int status;                           // Controller status

  // Geometry
  unsigned int blks;		        // Number of blocks on drive
  unsigned int size;		        // Size in MB

  unsigned int cyls;	                // Number of cylinders
  unsigned int heads;		        // Number of heads
  unsigned int sectors;                 // Sectors per track
};

static struct hd hd;

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

static void boothd_error(char *func, unsigned char error)
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

static int boothd_wait(unsigned char mask)
{
  unsigned char status;

  while (1)
  {
    status = _inp((unsigned short) (hd.iobase + HDC_ALT_STATUS));
    if (status & HDCS_ERR) 
    {
       unsigned char error;
 
       error = _inp((unsigned short) (hd.iobase + HDC_ERR));
       boothd_error("boothd_wait", error);

       return error;
    }

    if (!(status & HDCS_BSY) && ((status & mask) == mask)) return 0;
  }
}

static void boothd_setup_transfer(blkno_t blkno, int nsects)
{
  unsigned int track;
  unsigned int head;
  unsigned int sector;

  track = blkno / (hd.heads * hd.sectors);
  head = (blkno / hd.sectors) % hd.heads;
  sector = blkno % hd.sectors + 1;

  _outp((unsigned short) (hd.iobase + HDC_SECTORCNT), nsects);
  _outp((unsigned short) (hd.iobase + HDC_SECTOR), (unsigned char) sector);
  _outp((unsigned short) (hd.iobase + HDC_TRACKLSB), (unsigned char) track);
  _outp((unsigned short) (hd.iobase + HDC_TRACKMSB), (unsigned char) (track >> 8));
  _outp((unsigned short) (hd.iobase + HDC_DRVHD), ((unsigned char) head & 0x0F) | hd.drvsel);
}

static int boothd_identify()
{
  int result;

  // Issue read drive parameters command
  _outp(hd.iobase + HDC_DRVHD, hd.drvsel);
  _outp(hd.iobase + HDC_COMMAND, HDCMD_IDENTIFY);

  // Wait for data ready
  if (!(_inp((unsigned short) (hd.iobase + HDC_ALT_STATUS)) & HDCS_DRQ))
  {
    result = boothd_wait(HDCS_DRQ);
    if (result != 0) return result;
  }

  // Read parameter data
  insw(hd.iobase + HDC_DATA, &(hd.param), SECTORSIZE / 2);

  // Fill in drive parameters
  hd.cyls = hd.param.cylinders;
  hd.heads = hd.param.heads;
  hd.sectors = hd.param.sectors;
  hd.blks = hd.cyls * hd.heads * hd.sectors;
  hd.size = hd.blks / (M / SECTORSIZE);

  // Read status
  hd.status = _inp((unsigned short) (hd.iobase + HDC_STATUS));

  return 0;
}

int boothd_read(void *buffer, size_t count, blkno_t blkno)
{
  char *hdbuf;
  int i;
  int result;
  int nsects;

  if (count == 0) return 0;
  nsects = count / SECTORSIZE;

  // Issue read sectors command
  result = boothd_wait(HDCS_DRDY);
  if (result != 0) 
  {
    kprintf("boothd_read: no drdy (%d)\n", result);
    return -1;
  }

  boothd_setup_transfer(blkno, nsects);
  _outp((unsigned short) (hd.iobase + HDC_COMMAND), HDCMD_READ);

  hdbuf = (char *) buffer;
  for (i = 0; i < nsects; i++) 
  {
    // Wait for data ready
    if (!(_inp((unsigned short)(hd.iobase + HDC_ALT_STATUS)) & HDCS_DRQ))
    {
      result = boothd_wait(HDCS_DRQ);
      if (result != 0)
      {
	kprintf("boothd_read: no drq (%d)\n", result);
        return -1;
      }
    }

    // Read sector data
    insw(hd.iobase + HDC_DATA, hdbuf, SECTORSIZE / 2);

    // Check status
    hd.status = _inp((unsigned short) (hd.iobase + HDC_STATUS));
    if (hd.status & HDCS_ERR)
    {
      kprintf("boothd_read: read error (%d)\n", hd.status);
      return -1;
    }

    hdbuf += SECTORSIZE;
  }

  return count;
}

void init_boothd(int bootdrv)
{
  memset(&hd, 0, sizeof(struct hd));
  switch (bootdrv & 0x7F)
  {
    case 0:
      hd.iobase = HDC0_IOBASE;
      hd.drvsel = HD0_DRVSEL;
      break;

    case 1:
      hd.iobase = HDC0_IOBASE;
      hd.drvsel = HD1_DRVSEL;
      break;

    case 2:
      hd.iobase = HDC1_IOBASE;
      hd.drvsel = HD0_DRVSEL;
      break;

    case 3:
      hd.iobase = HDC1_IOBASE;
      hd.drvsel = HD1_DRVSEL;
      break;
  }

  boothd_identify();

  _outp((unsigned short) (hd.iobase + HDC_CONTROL), HDDC_HD15 | HDDC_NIEN);

  kprintf("%d: %u blks (%d MB) CHS=%u/%u/%u\n", hd.drvsel, hd.blks, hd.size, hd.cyls, hd.heads, hd.sectors);
}

void uninit_boothd()
{
}
