//
// syspage.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// System page defintions
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
#define GDT_PNPTEXT  7
#define GDT_PNPDATA  8
#define GDT_PNPTHUNK 9
#define GDT_AUX1     10
#define GDT_AUX2     11

#define MAXGDT 16
#define MAXIDT 64

#define SEL_NULL     GDT_TO_SEL(GDT_NULL)
#define SEL_KTEXT    GDT_TO_SEL(GDT_KTEXT)
#define SEL_KDATA    GDT_TO_SEL(GDT_KDATA)
#define SEL_UTEXT    GDT_TO_SEL(GDT_UTEXT)
#define SEL_UDATA    GDT_TO_SEL(GDT_UDATA)
#define SEL_TSS      GDT_TO_SEL(GDT_TSS)
#define SEL_TIB      GDT_TO_SEL(GDT_TIB)
#define SEL_PNPTEXT  GDT_TO_SEL(GDT_PNPTEXT)
#define SEL_PNPDATA  GDT_TO_SEL(GDT_PNPDATA)
#define SEL_PNPTHUNK GDT_TO_SEL(GDT_PNPTHUNK)
#define SEL_AUX1     GDT_TO_SEL(GDT_AUX1)
#define SEL_AUX2     GDT_TO_SEL(GDT_AUX2)

#define SEL_RPL0    0
#define SEL_RPL3    3

struct bootparams
{
  unsigned long heapstart;
  unsigned long heapend;
  unsigned long memend;
  int bootdrv;
  int bootpart;
};

struct syspage
{
  struct tss tss;
  struct segment gdt[MAXGDT];
  struct gate idt[MAXIDT];
  struct bootparams bootparams;
  unsigned char biosdata[256];
};

#ifndef OSLDR
krnlapi extern struct syspage *syspage;
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

#define DMABUF_ADDRESS  (SYSBASE + 16 * PAGESIZE)

#define TSS_ESP0 (SYSPAGE_ADDRESS + 4)

#endif
