//
// pdir.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Page frame database 
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
  __asm { mov eax, cr3 }
  __asm { mov cr3, eax }
}

void map_page(void *vaddr, unsigned long pfn, unsigned long flags)
{
  // Allocate page table if not already done
  if ((pdir[PDEIDX(vaddr)] & PT_PRESENT) == 0)
  {
    if (vaddr < (void *) OSBASE)
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
  int i;
  int len;
  unsigned long addr;
  unsigned long next;

  i = 0;
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

void init_pdir()
{
  unsigned long i;

  // Clear identity mapping of the first 4 MB made by the os loader
  for (i = 0; i < PTES_PER_PAGE; i++) ptab[i] = 0;
}
