//
// pframe.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved
//
// Page frame database routines
//

#ifndef PFRAME_H
#define PFRAME_H

#define DMA_BUFFER_START 0x10000
#define DMA_BUFFER_PAGES 16

#define PFT_FREE 0
#define PFT_USED 1
#define PFT_SYS  2
#define PFT_BAD  3

struct pageframe
{
  unsigned short flags;
  unsigned short type;
  union
  {
    unsigned long locks;        // Number of locks
    unsigned long size;         // Size/buckets for kernel pages
    struct pageframe *next;     // Next free page frame for free pages
  };
};

extern struct pageframe *pfdb;

extern unsigned long freemem;
extern unsigned long totalmem;
extern unsigned long maxmem;

krnlapi unsigned long alloc_pageframe(int type);
krnlapi void free_pageframe(unsigned long pfn);

int memstat_proc(struct proc_file *pf, void *arg);
int physmem_proc(struct proc_file *pf, void *arg);

void init_pfdb();

#endif
