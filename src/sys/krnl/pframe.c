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

unsigned long freemem;        // Number of pages free memory
unsigned long totalmem;       // Total number of pages of memory (bad pages excluded)
unsigned long maxmem;         // First unavailable memory page
struct pageframe *pfdb;       // Page frame database      
struct pageframe *freelist;   // List of free pages

void panic(char *msg);

unsigned long alloc_pageframe(int type)
{
  struct pageframe *pf;

  if (freemem == 0) panic("out of memory");
  pf = freelist;
  freelist = pf->next;
  freemem--;

  pf->type = type;
  pf->flags = 0;
  pf->next = 0;

  return pf - pfdb;
}

void free_pageframe(unsigned long pfn)
{
  struct pageframe *pf;

  pf = pfdb + pfn;
  pf->type = PFT_FREE;
  pf->next = freelist;
  freelist = pf;
  freemem++;
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

  pprintf(pf, ".=free *=user page X=locked user page, #=kernel page -=reserved page\n");
  for (n = 0; n < maxmem; n++)
  {
    if (n % 64 == 0)
    {
      if (n > 0) pprintf(pf, "\n");
      pprintf(pf, "%08X ", PTOB(n));
    }

    switch (pfdb[n].type)
    {
      case PFT_FREE:
	pprintf(pf, ".");
	break;

      case PFT_USED:
	pprintf(pf, pfdb[n].locks ? "X" : "*");
	break;

      case PFT_SYS:
	if (pfdb[n].size == 0 || pfdb[n].size >= PAGESHIFT)
	  pprintf(pf, "#");
	else
	  pprintf(pf, "%c", '0' + (PAGESHIFT - pfdb[n].size));
	break;

      case PFT_BAD:
	pprintf(pf, "-");
	break;

      default:
	pprintf(pf, "?");
    }
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
  totalmem = maxmem - (1024 - 640) * K / PAGESIZE - DMA_BUFFER_PAGES;
  freemem = 0;

  pfdb = (struct pageframe *) PFDBBASE;
  memset(pfdb, 0, pfdbpages * PAGESIZE);

  // Add interval [0:640K] as FREE pages
  for (i = 0; i < 640 * K / PAGESIZE; i++) pfdb[i].type = PFT_FREE;

  // Add interval [640K:1MB] as BAD pages
  for (i = 640 * K / PAGESIZE; i < syspage->bootparams.heapstart / PAGESIZE; i++) pfdb[i].type = PFT_BAD;

  // Add interval [BOOTHEAPBASE:heap] to pfdb as SYS pages
  for (i = syspage->bootparams.heapstart / PAGESIZE; i < heap / PAGESIZE; i++) pfdb[i].type = PFT_SYS;

  // Add interval [heap:maxmem] as FREE pages
  for (i = heap / PAGESIZE; i < totalmem; i++) pfdb[i].type = PFT_FREE;

  // Reserve DMA buffers at 0x10000 (used by floppy driver)
  for (i = DMA_BUFFER_START / PAGESIZE; i < DMA_BUFFER_START / PAGESIZE + DMA_BUFFER_PAGES; i++) pfdb[i].type = PFT_BAD;

  // Insert all free pages into free list
  pf = pfdb + maxmem;
  do
  {
    pf--;

    if (pf->type == PFT_FREE)
    {
      pf->next = freelist;
      freelist = pf;
      freemem++;
    }
  } 
  while (pf > pfdb);
}

