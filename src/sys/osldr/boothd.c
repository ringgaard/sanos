//
// boothd.c
//
// Boot Harddisk driver
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

#include <os.h>
#include <string.h>
#include <sys/types.h>
#include <os/iop.h>

void kprintf(const char *fmt,...);

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

struct hd 
{
  int iobase;	                        // I/O port registers base address
  struct hdparam param;		        // Drive parameter block
  int drvsel;                           // Drive select on controller
  int status;                           // Controller status

  // Geometry
  unsigned int blks;		        // Number of blocks on drive
  unsigned int size;		        // Size in MB
  int lba;                              // Use LBA mode

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

  if (hd.lba)
  {
    track = (blkno >> 8) & 0xFFFF;
    head = ((blkno >> 24) & 0xF) | 0x40;
    sector = blkno & 0xFF;
  }
  else 
  {
    track = blkno / (hd.heads * hd.sectors);
    head = (blkno / hd.sectors) % hd.heads;
    sector = blkno % hd.sectors + 1;
  }

  _outp((unsigned short) (hd.iobase + HDC_SECTORCNT), nsects);
  _outp((unsigned short) (hd.iobase + HDC_SECTOR), (unsigned char) sector);
  _outp((unsigned short) (hd.iobase + HDC_TRACKLSB), (unsigned char) track);
  _outp((unsigned short) (hd.iobase + HDC_TRACKMSB), (unsigned char) (track >> 8));
  _outp((unsigned short) (hd.iobase + HDC_DRVHD), (unsigned char) (head | hd.drvsel));
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
  insw(hd.iobase + HDC_DATA, &hd.param, SECTORSIZE / 2);

  // Fill in drive parameters
  hd.cyls = hd.param.cylinders;
  hd.heads = hd.param.heads;
  hd.sectors = hd.param.sectors;

  // Determine LBA or CHS mode
  if (hd.param.totalsec0 == 0 && hd.param.totalsec1 == 0)
  {
    hd.lba = 0;
    hd.blks = hd.cyls * hd.heads * hd.sectors;
  }
  else
  {
    hd.lba = 1;
    hd.blks = (hd.param.totalsec1 << 16) | hd.param.totalsec0;
  }
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
    kprintf("boothd_read: no drdy in wait (%d)\n", result);
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

    // Check status
    hd.status = _inp((unsigned short) (hd.iobase + HDC_STATUS));
    if (hd.status & HDCS_ERR)
    {
      kprintf("boothd_read: read error (%d)\n", hd.status);
      return -1;
    }

    // Read sector data
    insw(hd.iobase + HDC_DATA, hdbuf, SECTORSIZE / 2);
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

  //kprintf("%d: %u blks (%d MB) CHS=%u/%u/%u\n", hd.drvsel, hd.blks, hd.size, hd.cyls, hd.heads, hd.sectors);
}

void uninit_boothd()
{
}
