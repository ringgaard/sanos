//
// bootfd.c
//
// Boot floppy disk driver
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
#include <os/seg.h>
#include <os/tss.h>
#include <os/syspage.h>
#include <os/pic.h>
#include <os/trap.h>

void panic(char *msg);

#define SECTORSIZE            512
#define NUMDRIVES             4

//
// Place DMA buffer between boot image and os loader. This gives the following
// low memory layout:
//
// 0x00000000 BIOS data area
// 0x00007C00 Boot sector + boot disk image (up to 512K)
// 0x00088000 32K DMA buffer
// 0x00090000 OS loader
// 0x000A0000 Start of BIOS ROM area and top of stack
//

#define BOOT_DMA_BUF          0x88000

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
// DMA I/O ports
//

#define DMA1_STATUS		0x08
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
  unsigned char drive;
  unsigned char curtrack;
};

struct fdgeometry geom144 = {2, 80, 18, 0x1B}; // Drive geometry for 3 1/2" 1.44M drive

struct fdc fdc;
struct fd fd;
volatile int fddone;

struct selector bootidtsel;
struct gate bootidt[MAXIDT];

//
// iodelay
//

static void iodelay(int usecs)
{
  int n;

  for (n = 0; n < usecs; n++) 
  {
    _inp(0x80);
    _inp(0x80);
  }
}

//
// fd_command
//

static void fd_command(unsigned char cmd)
{
  int msr;
  int tmo;

  for (tmo = 0;tmo < 10000000; tmo++) 
  {
    msr = _inp(FDC_MSR);
    if ((msr & 0xc0) == 0x80)
    {
      _outp(FDC_DATA, cmd);
      return;
    }
  }
  panic("fd_command: timeout\n");
}

//
// fd_result
//

static unsigned char fd_result()
{
  int msr;
  int tmo;

  for (tmo = 0;tmo < 10000000; tmo++) 
  {
    msr = _inp(FDC_MSR);
    if ((msr & 0xd0) == 0xd0) return _inp(FDC_DATA);
  }

  // Read timeout
  panic("fd_result: timeout\n");
  return -1;   
}

//
// fd_wait
//

static void fd_wait(int status)
{
  while (!fddone) iodelay(1);
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
// fd_transfer
//

static int fd_transfer(struct fd *fd, void *buffer, size_t count, blkno_t blkno)
{
  struct fdresult result;
  int retries = 10;
  unsigned char track, head, sector;
  unsigned int remaining;

  if (!buffer) return -EINVAL;

  // The FDC can read multiple sides at once but not multiple tracks
  blk2ths(fd, blkno, &track, &head, &sector);
  remaining = ((fd->geom->spt + 1 - sector) + fd->geom->spt * (fd->geom->heads - head - 1)) * SECTORSIZE;
  if (remaining < count) count = remaining;

//kprintf("fd: t=%d h=%d s=%d count=%d sectors\n", track, head, sector, count / SECTORSIZE);
  
  while (retries > 0) 
  {
    // Perform seek if necessary
    if (fd->curtrack != track)
    {
//kprintf("fd: seek %d\n", track);

      fddone = 0;

      fd_command(CMD_SEEK);
      fd_command((unsigned char) ((head << 2) | fd->drive));
      fd_command(track);

      fd_wait(0x80);

      fd_command(CMD_SENSEI);
      result.st0 = fd_result();
      result.track = fd_result();

      if (result.track != track) panic("seek failed");
      fd->curtrack = track;
    }

    // Program data rate (500K/s)
    _outp(FDC_CCR,0);

    // Set up DMA
    _outp(DMA1_CHAN, 0x06);
    _outp(DMA1_RESET, DMA1_CHAN2_READ);
    _outp(DMA1_MODE, DMA1_CHAN2_READ);

    // Setup DMA transfer
    _outp(DMA1_CHAN2_ADDR, fd->fdc->bufl);
    _outp(DMA1_CHAN2_ADDR, fd->fdc->bufh);
    _outp(DMA1_CHAN2_PAGE, fd->fdc->bufp);
    _outp(DMA1_CHAN2_COUNT, ((count - 1) & 0xFF));
    _outp(DMA1_CHAN2_COUNT, ((count - 1) >> 8));
    _outp(DMA1_CHAN, 0x02);

//kprintf("fd: xfer %d\n", count);
    // Perform transfer
    fddone = 0;
    fd_command(CMD_READ);
    fd_command((unsigned char) ((head << 2) | fd->drive));
    fd_command(track);
    fd_command(head);
    fd_command(sector);
    fd_command(0x02); // 512 bytes/sector
    fd_command(fd->geom->spt);
    fd_command(fd->geom->gap3);
    fd_command(0xff); // DTL = unused
    
    fd_wait(0x80);

    result.st0 = fd_result();
    result.st1 = fd_result();
    result.st2 = fd_result();
    result.track = fd_result();
    result.head = fd_result();
    result.sector = fd_result();
    result.size = fd_result();

    if ((result.st0 & 0xc0) == 0) 
    {
      // Successful transfer
      memcpy(buffer, fd->fdc->dmabuf, count);
      return count;
    }
    else // if ((result.st0 & 0xc0) == 0x40 && (result.st1 & 0x04) == 0x04) 
    {
//kprintf("fd: recal st0 %02X st1 %02X st2 %02X track %d head %d sector %d size %d\n", 
//	result.st0, result.st1, result.st2, 
//	result.track, result.head, result.sector, result.size);

      // Recalibrate before retrying.
      fddone = 0;
      fd->curtrack = 0;
      fd_command(CMD_RECAL);
      fd_command(0x00);

      fd_wait(0x80);

//kprintf("fd: sensei\n");
      fd_command(CMD_SENSEI);
      result.st0 = fd_result();
      result.track = fd_result();
    } 

    retries--;
  }
  
  return -ETIMEOUT;
}

//
// bootfd_read
//

int bootfd_read(void *buffer, size_t count, blkno_t blkno)
{
  int left;
  int result;
  char *buf;

//kprintf("fd: read %d\n", count);
  left = count;
  buf = (char *) buffer;
  while (left > 0)
  {
    result = fd_transfer(&fd, buf, left, blkno);
    if (result < 0) return result;

    left -= result;
    buf += result;
    blkno += result / SECTORSIZE;
  }

  return count;
}

//
// fd_isr
//

static void __declspec(naked) fd_isr()
{
  __asm
  {
    cld
    push    eax
    mov	    eax, 1
    mov     [fddone], eax
    mov     al, PIC_EOI_FD
    out     PIC_MSTR_CTRL, al
    pop	    eax
    iretd
  }
}

//
// init_boot_intrs
//

static void init_boot_intrs()
{
  // Initialize master PIC
  _outp(PIC_MSTR_CTRL, PIC_MSTR_ICW1);
  _outp(PIC_MSTR_MASK, PIC_MSTR_ICW2);
  _outp(PIC_MSTR_MASK, PIC_MSTR_ICW3);
  _outp(PIC_MSTR_MASK, PIC_MSTR_ICW4);
  _outp(PIC_MSTR_MASK, ~(1 << IRQ_FD));

  // Setup IDT for FDC
  bootidt[INTR_FD].offset_low = (unsigned short) (((unsigned long) fd_isr) & 0xFFFF);
  bootidt[INTR_FD].selector = SEL_KTEXT;
  bootidt[INTR_FD].access = D_PRESENT | D_INT | D_DPL0;
  bootidt[INTR_FD].offset_high = (unsigned short) (((unsigned long) fd_isr) >> 16);

  // Load boot IDT
  bootidtsel.limit = (sizeof(struct gate) * MAXIDT) - 1;
  bootidtsel.dt = bootidt;
  __asm { lidt bootidtsel }

  // Enable interrupts
  sti();
}

//
// init_bootfd
//

void init_bootfd(int bootdrv)
{
  memset(&fdc, 0, sizeof(struct fdc));
  memset(&fd, 0, sizeof(struct fd));

  fdc.dor = 0x0C; // TODO: select drive in DOR on transfer
  fdc.dmabuf = (char *) BOOT_DMA_BUF;
  fdc.bufp = (BOOT_DMA_BUF >> 16) & 0xFF;
  fdc.bufh = (BOOT_DMA_BUF >> 8) & 0xFF;
  fdc.bufl = BOOT_DMA_BUF & 0xFF;

  fd.fdc = &fdc;
  fd.geom = &geom144;
  fd.drive = bootdrv;
  fd.curtrack = 0xFF;

  // Start motor on drive
  fdc.dor |= 0x10 << fd.drive;
  _outp(FDC_DOR, fdc.dor);

  // Initialize interrupts
  init_boot_intrs();
}

//
// uninit_bootfd
//

void uninit_bootfd()
{
  // Stop motor
  fdc.dor &= 0x0F;
  _outp(FDC_DOR, fdc.dor);

  // Disable interrupts
  cli();
}
