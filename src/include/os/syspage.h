//
// syspage.h
//
// System page definitions
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

#ifndef SYSPAGE_H
#define SYSPAGE_H

#define GDT_TO_SEL(gdt) ((gdt) << 3)

#define GDT_NULL     0
#define GDT_KTEXT    1
#define GDT_KDATA    2
#define GDT_UTEXT    3
#define GDT_UDATA    4
#define GDT_TSS      5
#define GDT_TIB      6
#define GDT_AUX1     7
#define GDT_APM40    8
#define GDT_APMCS    9
#define GDT_APMCS16  10
#define GDT_APMDS    11
#define GDT_PNPTEXT  12
#define GDT_PNPDATA  13
#define GDT_PNPTHUNK 14
#define GDT_AUX2     15

#define MAXGDT 16
#define MAXIDT 64

#define SEL_NULL     GDT_TO_SEL(GDT_NULL)
#define SEL_KTEXT    GDT_TO_SEL(GDT_KTEXT)
#define SEL_KDATA    GDT_TO_SEL(GDT_KDATA)
#define SEL_UTEXT    GDT_TO_SEL(GDT_UTEXT)
#define SEL_UDATA    GDT_TO_SEL(GDT_UDATA)
#define SEL_TSS      GDT_TO_SEL(GDT_TSS)
#define SEL_TIB      GDT_TO_SEL(GDT_TIB)
#define SEL_AUX1     GDT_TO_SEL(GDT_AUX1)
#define SEL_APM40    GDT_TO_SEL(GDT_APM40)
#define SEL_APMCS    GDT_TO_SEL(GDT_APMCS)
#define SEL_APMCS16  GDT_TO_SEL(GDT_APMCS16)
#define SEL_APMDS    GDT_TO_SEL(GDT_APMDS)
#define SEL_PNPTEXT  GDT_TO_SEL(GDT_PNPTEXT)
#define SEL_PNPDATA  GDT_TO_SEL(GDT_PNPDATA)
#define SEL_PNPTHUNK GDT_TO_SEL(GDT_PNPTHUNK)
#define SEL_AUX2     GDT_TO_SEL(GDT_AUX2)

#define SEL_RPL0    0
#define SEL_RPL3    3

#define KRNLOPTS_POSOFS      0x1A   // Offset in os loader for kernel option address
#define KRNLOPTS_LEN         128    // Maximum length of kernel options

// APM install check flags

#define APM_16_BIT_SUPPORT      0x0001
#define APM_32_BIT_SUPPORT      0x0002
#define APM_IDLE_SLOWS_CLOCK    0x0004
#define APM_BIOS_DISABLED       0x0008
#define APM_BIOS_DISENGAGED     0x0010

// APM BIOS parameter block

struct apmparams {
  unsigned short version;      // APM version (BCD format)
  unsigned short flags;        // APM flags from install check
  unsigned short cseg32;       // APM 32-bit code segment (real mode segment base address)
  unsigned short entry;        // Offset of the entry point into the APM BIOS
  unsigned short cseg16;       // APM 16-bit code segment (real mode segment base address)
  unsigned short dseg;         // APM data segment (real mode segment base address)
  unsigned short cseg32len;    // APM BIOS 32-bit code segment length
  unsigned short cseg16len;    // APM BIOS 16-bit code segment length
  unsigned short dseglen;      // APM BIOS data segment length
};

// Memory map

#define MAX_MEMENTRIES    32

#define MEMTYPE_RAM       1
#define MEMTYPE_RESERVED  2
#define MEMTYPE_ACPI      3
#define MEMTYPE_NVS       4

#pragma pack(push, 1)

struct memmap {
  int count;              // Number of entries in memory map
  struct mementry {
    unsigned __int64 addr;     // Start of memory segment
    unsigned __int64 size;     // Size of memory segment
    unsigned long type;        // Type of memory segment
  } entry[MAX_MEMENTRIES];
};

// Boot parameter block

struct bootparams {
  int bootdrv;                 // Boot drive:
                               //  0x00 First floppy
                               //  0x01 Second floppy
                               //  0x80 First harddisk
                               //  0x81 Second harddisk
                               //  0xFD CD-ROM (1.44 MB Floppy Emulation)
                               //  0xFE PXE netboot
                               //  0xFF CD-ROM (No emulation)
  char *bootimg;               // Physical address of boot image
  char krnlopts[KRNLOPTS_LEN]; // Kernel options (set by mkdfs)
  struct apmparams apm;        // APM BIOS parameters
  struct memmap memmap;        // System memory map from BIOS
};

#pragma pack(pop)

// Loader parameter block

struct ldrparams {
  unsigned long heapstart;      // Start of boot heap
  unsigned long heapend;        // End of boot heap
  unsigned long memend;         // End of RAM
  int bootdrv;                  // Boot drive
  int bootpart;                 // Boot partition
  unsigned long initrd_size;    // Initial RAM disk size
};

// System page

struct syspage {
  struct tss tss;               // Task State Segment
  struct segment gdt[MAXGDT];   // Global Descriptor Table
  struct gate idt[MAXIDT];      // Interrupt Descriptor Table
  struct bootparams bootparams; // Boot parameter block
  struct ldrparams ldrparams;   // Loader parameter block
  unsigned char biosdata[256];  // Copy of BIOS data area
};

#ifdef KERNEL
#ifndef OSLDR
krnlapi extern struct syspage *syspage;
#endif
#endif

#define OSBASE      0x80000000
#define PTBASE      0x90000000
#define SYSBASE     0x90400000
#define PFDBBASE    0x90800000
#define HTABBASE    0x91000000
#define KHEAPBASE   0x92000000

#define KMODSIZE    0x10000000
#define KHEAPSIZE   0x6E000000
#define HTABSIZE    0x01000000

#define SYSPAGE_ADDRESS (SYSBASE + 0 * PAGESIZE)
#define PAGEDIR_ADDRESS (SYSBASE + 1 * PAGESIZE)
#define INITTCB_ADDRESS (SYSBASE + 2 * PAGESIZE)
#define OSVMAP_ADDRESS  (SYSBASE + 4 * PAGESIZE)
#define KMODMAP_ADDRESS (SYSBASE + 5 * PAGESIZE)
#define VIDBASE_ADDRESS (SYSBASE + 6 * PAGESIZE)

#define DMABUF_ADDRESS  (SYSBASE + 16 * PAGESIZE)  // 64K
#define INITRD_ADDRESS  (SYSBASE + 32 * PAGESIZE)  // 512K

#define TSS_ESP0 (SYSPAGE_ADDRESS + 4)

#define USERSPACE(addr) ((unsigned long)(addr) < OSBASE)
#define KERNELSPACE(addr) ((unsigned long)(addr) >= OSBASE)

#endif
