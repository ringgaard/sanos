//
// pdir.c
//
// Page directory 
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
// 3. Neither the name of Michael Ringgaard nor the names of its contributors
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

pte_t *pdir = (pte_t *) PAGEDIR_ADDRESS; // Page directory
pte_t *ptab = (pte_t *) PTBASE;          // Page tables

void flushtlb()
{
  __asm { mov eax, cr3 }
  __asm { mov cr3, eax }
}

void invlpage(void *addr)
{
  if (cpu.family < CPU_FAMILY_486)
  {
    __asm { mov eax, cr3 }
    __asm { mov cr3, eax }
  }
  else
  {
    __asm { mov eax, addr }
    __asm { invlpg [eax] }
  }
}

void map_page(void *vaddr, unsigned long pfn, unsigned long flags)
{
  // Allocate page table if not already done
  if ((pdir[PDEIDX(vaddr)] & PT_PRESENT) == 0)
  {
    if (USERSPACE(vaddr))
      pdir[PDEIDX(vaddr)] = PTOB(alloc_pageframe(PFT_SYS)) | PT_PRESENT | PT_WRITABLE | PT_USER;
    else
      pdir[PDEIDX(vaddr)] = PTOB(alloc_pageframe(PFT_SYS)) | PT_PRESENT | PT_WRITABLE;

    memset(ptab + PDEIDX(vaddr) * PTES_PER_PAGE, 0, PAGESIZE);
  }

  // Map page frame into address space
  ptab[PTABIDX(vaddr)] = PTOB(pfn) | flags;
}

void unmap_page(void *vaddr)
{
  ptab[PTABIDX(vaddr)] = 0;
  invlpage(vaddr);
}

void *virt2phys(void *vaddr)
{
  return (void *) ((ptab[PTABIDX(vaddr)] & PT_PFNMASK) + PGOFF(vaddr));
}

pte_t get_page_flags(void *vaddr)
{
  return ptab[PTABIDX(vaddr)] & PT_FLAGMASK;
}

void set_page_flags(void *vaddr, unsigned long flags)
{
  ptab[PTABIDX(vaddr)] = (ptab[PTABIDX(vaddr)] & PT_PFNMASK) | flags;
}

int page_guarded(void *vaddr)
{
  if ((pdir[PDEIDX(vaddr)] & PT_PRESENT) == 0) return 0;
  if ((ptab[PTABIDX(vaddr)] & PT_GUARD) == 0) return 0;
  return 1;
}

int page_mapped(void *vaddr)
{
  if ((pdir[PDEIDX(vaddr)] & PT_PRESENT) == 0) return 0;
  if ((ptab[PTABIDX(vaddr)] & PT_PRESENT) == 0) return 0;
  return 1;
}

int mem_mapped(void *vaddr, int size)
{
  int len;
  unsigned long addr;
  unsigned long next;

  addr = (unsigned long) vaddr;
  next = (addr & ~PAGESIZE) + PAGESIZE;
  while (1)
  {
    if ((pdir[PDEIDX(addr)] & PT_PRESENT) == 0) return 0;
    if ((ptab[PTABIDX(addr)] & PT_PRESENT) == 0) return 0;
    len = next - addr;
    if (size > len)
    {
      size -= len;
      addr = next;
      next += PAGESIZE;
    }
    else
      break;
  }

  return 1;
}

int str_mapped(char *s)
{
  while (1)
  {
    if ((pdir[PDEIDX(s)] & PT_PRESENT) == 0) return 0;
    if ((ptab[PTABIDX(s)] & PT_PRESENT) == 0) return 0;

    while (1)
    {
      if (!*s) return 1;
      s++;
      if (PGOFF(s) == 0) break;
    }
  }
}

void init_pdir()
{
  unsigned long i;

  // Clear identity mapping of the first 4 MB made by the os loader
  for (i = 0; i < PTES_PER_PAGE; i++) ptab[i] = 0;
}

int pdir_proc(struct proc_file *pf, void *arg)
{
  char *vaddr;
  pte_t pte;
  int lines = 0;

  int ma = 0;
  int us = 0;
  int su = 0;
  int ro = 0;
  int rw = 0;
  int ac = 0;
  int dt = 0;

  pprintf(pf, "virtaddr physaddr flags\n");
  pprintf(pf, "-------- -------- ----- \n");

  vaddr = NULL;
  while (1)
  {
    if ((pdir[PDEIDX(vaddr)] & PT_PRESENT) == 0)
      vaddr += PTES_PER_PAGE * PAGESIZE;
    else
    {
      pte = ptab[PTABIDX(vaddr)];
      if (pte & PT_PRESENT) 
      {
	ma++;
	
	if (pte & PT_WRITABLE)
	  rw++;
	else
	  ro++;

	if (pte & PT_USER) 
	  us++;
	else
	  su++;

	if (pte & PT_ACCESSED) ac++;
	if (pte & PT_DIRTY) dt++;

	pprintf(pf, "%08x %08x %c%c%c%c\n", 
	        vaddr, PAGEADDR(pte), 
		(pte & PT_WRITABLE) ? 'w' : 'r',
		(pte & PT_USER) ? 'u' : 's',
		(pte & PT_ACCESSED) ? 'a' : ' ',
		(pte & PT_DIRTY) ? 'd' : ' ');

      }

      vaddr += PAGESIZE;
    }

    if (!vaddr) break;
  }

  pprintf(pf, "\ntotal: %d user: %d sys: %d r/w: %d r/o: %d accessed: %d dirty: %d\n", ma, us, su, rw, ro, ac, dt);
  return 0;
}
