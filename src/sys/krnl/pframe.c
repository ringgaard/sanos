//
// pframe.c
//
// Page frame database routines
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

#define MAX_MEMTAGS           128

unsigned long freemem;        // Number of pages free memory
unsigned long totalmem;       // Total number of pages of memory (bad pages excluded)
unsigned long maxmem;         // First unavailable memory page
struct pageframe *pfdb;       // Page frame database      
struct pageframe *freelist;   // List of free pages

void panic(char *msg);

unsigned long alloc_pageframe(unsigned long tag)
{
  struct pageframe *pf;

  if (freemem == 0) panic("out of memory");

  pf = freelist;
  freelist = pf->next;
  freemem--;

  pf->tag = tag;
  pf->next = NULL;

  return pf - pfdb;
}

unsigned long alloc_linear_pageframes(int pages, unsigned long tag)
{
  struct pageframe *pf;
  struct pageframe *prevpf;

  if (pages == 1) return alloc_pageframe(tag);

  if ((int) freemem < pages) return 0xFFFFFFFF;

  prevpf = NULL;
  pf = freelist;
  while (pf)
  {
    if (pf - pfdb + pages < (int) maxmem)
    {
      int n;

      for (n = 0; n < pages; n++)
      {
	if (pf[n].tag != 'FREE') break;
	if (n != 0 && pf[n - 1].next != &pf[n]) break;
      }

      if (n == pages)
      {
	if (prevpf)
	  prevpf->next = pf[pages - 1].next;
	else
	  freelist = pf[pages - 1].next;

        for (n = 0; n < pages; n++)
	{
	  pf[n].tag = tag;
	  pf[n].next = NULL;
	}

	freemem -= pages;

	return pf - pfdb;
      }
    }

    prevpf = pf;
    pf = pf->next;
  }

  return 0xFFFFFFFF;
}

void free_pageframe(unsigned long pfn)
{
  struct pageframe *pf;

  pf = pfdb + pfn;
  pf->tag = 'FREE';
  pf->next = freelist;
  freelist = pf;
  freemem++;
}

void set_pageframe_tag(void *addr, unsigned int len, unsigned long tag)
{
  char *vaddr = (char *) addr;
  char *vend = vaddr + len;

  while (vaddr < vend)
  {
    unsigned long pfn = virt2phys(vaddr) >> PAGESHIFT;
    pfdb[pfn].tag = tag;
    vaddr += PAGESIZE;
  }
}

int memusage_proc(struct proc_file *pf, void *arg)
{
  unsigned int num_memtypes = 0;
  struct { unsigned long tag; int pages; } memtype[MAX_MEMTAGS];
  unsigned long tag;
  unsigned int n;
  unsigned int m;

  for (n = 0; n < maxmem; n++)
  {
    tag = pfdb[n].tag;

    m = 0;
    while (m < num_memtypes && tag != memtype[m].tag) m++;

    if (m < num_memtypes)
      memtype[m].pages++;
    else if (m < MAX_MEMTAGS)
    {
      memtype[m].tag = tag;
      memtype[m].pages = 1;
      num_memtypes++;
    }
  }

  for (n = 0; n < num_memtypes; n++)
  {
    char tagname[5];
    char *p = tagname;

    tag = memtype[n].tag;
    if (tag & 0xFF000000) *p++ = (char) ((tag >> 24) & 0xFF);
    if (tag & 0x00FF0000) *p++ = (char) ((tag >> 16) & 0xFF);
    if (tag & 0x0000FF00) *p++ = (char) ((tag >> 8) & 0xFF);
    if (tag & 0x000000FF) *p++ = (char) (tag & 0xFF);
    *p++ = 0;

    pprintf(pf, "%-4s    %8d KB\n", tagname, memtype[n].pages * (PAGESIZE / K));
  }

  return 0;
}

int memstat_proc(struct proc_file *pf, void *arg)
{
  pprintf(pf, "Memory %dMB total, %dKB used, %dKB free, %dKB reserved\n", 
	  maxmem * PAGESIZE / M, 
	  (totalmem - freemem) * PAGESIZE / K, 
	  freemem * PAGESIZE / K, (maxmem - totalmem) * PAGESIZE / K);
  
  return 0;
}

int physmem_proc(struct proc_file *pf, void *arg)
{
  unsigned int n;

  for (n = 0; n < maxmem; n++)
  {
    if (n % 64 == 0)
    {
      if (n > 0) pprintf(pf, "\n");
      pprintf(pf, "%08X ", PTOB(n));
    }

    if (pfdb[n].tag == 'FREE')
      pprintf(pf, ".");
    else if (pfdb[n].tag == 0)
      pprintf(pf, "?");
    else
      pprintf(pf, "X");
  }

  pprintf(pf, "\n");
  return 0;
}

void init_pfdb()
{
  unsigned long heap;
  unsigned long pfdbpages;
  unsigned long i;
  unsigned long memend;
  pte_t *pt;
  struct pageframe *pf;

  // Calculates number of pages needed for page frame database
  memend = syspage->bootparams.memend;
  heap = syspage->bootparams.heapend;
  pfdbpages = PAGES((memend / PAGESIZE) * sizeof(struct pageframe));
  if ((pfdbpages + 2) * PAGESIZE + heap >= memend) panic("not enough memory for page table database");

  // Intialize page tables for mapping the page frame database into kernel space
  pdir[PDEIDX(PFDBBASE)] = heap | PT_PRESENT | PT_WRITABLE;
  pdir[PDEIDX(PFDBBASE) + 1] = (heap + PAGESIZE) | PT_PRESENT | PT_WRITABLE;
  pt = (pte_t *) heap;
  heap += 2 * PAGESIZE;
  memset(pt, 0, 2 * PAGESIZE);

  // Allocate and map pages for page frame database
  for (i = 0; i < pfdbpages; i++)
  {
    pt[i] = heap | PT_PRESENT | PT_WRITABLE;
    heap += PAGESIZE;
  }

  // Initialize page frame database
  maxmem = syspage->bootparams.memend / PAGESIZE;
  totalmem = maxmem - (1024 - 640) * K / PAGESIZE;
  freemem = 0;

  pfdb = (struct pageframe *) PFDBBASE;
  memset(pfdb, 0, pfdbpages * PAGESIZE);

  // Add interval [0:640K] as free pages
  for (i = 0; i < 640 * K / PAGESIZE; i++) pfdb[i].tag = 'FREE';

  // Add interval [640K:1MB] as reserved pages
  for (i = 640 * K / PAGESIZE; i < syspage->bootparams.heapstart / PAGESIZE; i++) pfdb[i].tag = 'RESV';

  // Add interval [heapstart:heap] to pfdb as page table pages
  for (i = syspage->bootparams.heapstart / PAGESIZE; i < heap / PAGESIZE; i++) pfdb[i].tag = 'PTAB';

  // Add interval [heap:maxmem] as free pages
  for (i = heap / PAGESIZE; i < maxmem; i++) pfdb[i].tag = 'FREE';

  // Reserve DMA buffers at 0x10000 (used by floppy driver)
  for (i = DMA_BUFFER_START / PAGESIZE; i < DMA_BUFFER_START / PAGESIZE + DMA_BUFFER_PAGES; i++) pfdb[i].tag = 'DMA';

  // Fixup tags for pfdb and syspage and intial tcb
  set_pageframe_tag(pfdb, pfdbpages * PAGESIZE, 'PFDB');
  set_pageframe_tag(syspage, PAGESIZE, 'SYS');
  set_pageframe_tag(self(), TCBSIZE, 'TCB');

  // Insert all free pages into free list
  pf = pfdb + maxmem;
  do
  {
    pf--;

    if (pf->tag == 'FREE')
    {
      pf->next = freelist;
      freelist = pf;
      freemem++;
    }
  } 
  while (pf > pfdb);
}
