//
// pdir.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Page directory routines
//

#ifndef PDIR_H
#define PDIR_H

#define PAGESIZE       4096
#define PAGESHIFT      12
#define PTES_PER_PAGE  (PAGESIZE / sizeof(pte_t))

typedef unsigned long pte_t;

#define PT_PRESENT   0x001
#define PT_WRITABLE  0x002
#define	PT_USER      0x004
#define PT_ACCESSED  0x020
#define PT_DIRTY     0x040

#define PT_GUARD     0x200

#define PT_FLAGMASK  (PT_PRESENT | PT_WRITABLE | PT_USER | PT_ACCESSED | PT_DIRTY)

#define PT_PFNMASK   0xFFFFF000
#define PT_PFNSHIFT  12

#define PDEIDX(vaddr)  (((unsigned long) vaddr) >> 22)
#define PTEIDX(vaddr)  ((((unsigned long) vaddr) >> 12) & 0x3FF)
#define PGOFF(vaddr)   (((unsigned long) vaddr) & 0xFFF) 
#define PTABIDX(vaddr) (((unsigned long) vaddr) >> 12)

#define PAGES(x) (((unsigned long)(x) + (PAGESIZE - 1)) >> PAGESHIFT)
#define PAGEADDR(x) ((unsigned long)(x) & ~(PAGESIZE - 1))

#define PTOB(x) ((unsigned long)(x) << PAGESHIFT)
#define BTOP(x) ((unsigned long)(x) >> PAGESHIFT)

extern pte_t *pdir;
extern pte_t *ptab;

krnlapi void flushtlb();
krnlapi void map_page(void *vaddr, unsigned long pfn, unsigned long flags);
krnlapi void unmap_page(void *vaddr);
krnlapi void *virt2phys(void *vaddr);
krnlapi pte_t get_page_flags(void *vaddr);
krnlapi void set_page_flags(void *vaddr, unsigned long flags);
krnlapi int page_guarded(void *vaddr);
krnlapi int page_mapped(void *vaddr);
krnlapi int mem_mapped(void *vaddr, int size);
krnlapi int str_mapped(char *s);

void init_pdir();
int pdir_proc(struct proc_file *pf, void *arg);

#endif