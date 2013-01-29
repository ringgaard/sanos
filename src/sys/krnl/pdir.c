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

pte_t *pdir = (pte_t *) PAGEDIR_ADDRESS; // Page directory
pte_t *ptab = (pte_t *) PTBASE;          // Page tables

void map_page(void *vaddr, unsigned long pfn, unsigned long flags) {
  // Allocate page table if not already done
  if ((GET_PDE(vaddr) & PT_PRESENT) == 0) {
    unsigned long pdfn;

    pdfn = alloc_pageframe('PTAB');
    if (USERSPACE(vaddr)) {
      SET_PDE(vaddr, PTOB(pdfn) | PT_PRESENT | PT_WRITABLE | PT_USER);
    } else {
      SET_PDE(vaddr, PTOB(pdfn) | PT_PRESENT | PT_WRITABLE);
    }

    memset(ptab + PDEIDX(vaddr) * PTES_PER_PAGE, 0, PAGESIZE);
    register_page_table(pdfn);
  }

  // Map page frame into address space
  SET_PTE(vaddr, PTOB(pfn) | flags);
}

void unmap_page(void *vaddr) {
  SET_PTE(vaddr, 0);
  invlpage(vaddr);
}

unsigned long virt2phys(void *vaddr) {
  return ((GET_PTE(vaddr) & PT_PFNMASK) + PGOFF(vaddr));
}

unsigned long virt2pfn(void *vaddr) {
  return BTOP(GET_PTE(vaddr) & PT_PFNMASK);
}

pte_t get_page_flags(void *vaddr) {
  return GET_PTE(vaddr) & PT_FLAGMASK;
}

void set_page_flags(void *vaddr, unsigned long flags) {
  SET_PTE(vaddr, (GET_PTE(vaddr) & PT_PFNMASK) | flags);
  invlpage(vaddr);
}

int page_mapped(void *vaddr) {
  if ((GET_PDE(vaddr) & PT_PRESENT) == 0) return 0;
  if ((GET_PTE(vaddr) & PT_PRESENT) == 0) return 0;
  return 1;
}

int page_directory_mapped(void *vaddr) {
  return (GET_PDE(vaddr) & PT_PRESENT) != 0;
}

void unguard_page(void *vaddr) {
  SET_PTE(vaddr, (GET_PTE(vaddr) & ~PT_GUARD) | PT_USER);
  invlpage(vaddr);
}

void clear_dirty(void *vaddr) {
  SET_PTE(vaddr, GET_PTE(vaddr) & ~PT_DIRTY);
  invlpage(vaddr);
}

int mem_access(void *vaddr, int size, pte_t access) {
  unsigned long addr;
  unsigned long next;
  pte_t pte;
  
  addr = (unsigned long) vaddr;
  next = (addr & ~PAGESIZE) + PAGESIZE;
  while (1) {
    if ((GET_PDE(addr) & PT_PRESENT) == 0) return 0;
    pte = GET_PTE(addr);
    if ((pte & access) != access) {
      if (pte & PT_FILE) {
        if (fetch_page((void *) PAGEADDR(addr)) < 0) return 0;
        if ((GET_PTE(addr) & access) != access) return 0;
      } else {
        return 0;
      }
    }

    size -= next - addr;
    if (size <= 0) break;
    addr = next;
    next += PAGESIZE;
  }

  return 1;
}

int str_access(char *s, pte_t access) {
  pte_t pte;

  while (1) {
    if ((GET_PDE(s) & PT_PRESENT) == 0) return 0;
    pte = GET_PTE(s);
    if ((pte & access) != access) {
      if (pte & PT_FILE) {
        if (fetch_page((void *) PAGEADDR(s)) < 0) return 0;
        if ((GET_PTE(s) & access) != access) return 0;
      } else {
        return 0;
      }
    }

    while (1) {
      if (!*s) return 1;
      s++;
      if (PGOFF(s) == 0) break;
    }
  }
}

void init_pdir() {
  unsigned long i;

  // Clear identity mapping of the first 4 MB made by the os loader
  for (i = 0; i < PTES_PER_PAGE; i++) SET_PTE(PTOB(i), 0);
}

int pdir_proc(struct proc_file *pf, void *arg) {
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
  int gd = 0;
  int fi = 0;

  pprintf(pf, "virtaddr physaddr flags\n");
  pprintf(pf, "-------- -------- ------\n");

  vaddr = NULL;
  while (1) {
    if ((GET_PDE(vaddr) & PT_PRESENT) == 0) {
      vaddr += PTES_PER_PAGE * PAGESIZE;
    } else {
      pte = GET_PTE(vaddr);
      if (pte & PT_PRESENT) {
        ma++;
        
        if (pte & PT_WRITABLE) {
          rw++;
        } else {
          ro++;
        }

        if (pte & PT_USER) {
          us++;
        } else {
          su++;
        }

        if (pte & PT_ACCESSED) ac++;
        if (pte & PT_DIRTY) dt++;
        if (pte & PT_GUARD) gd++;
        if (pte & PT_FILE) fi++;

        pprintf(pf, "%08x %08x %c%c%c%c%c%c\n", 
                vaddr, PAGEADDR(pte), 
                (pte & PT_WRITABLE) ? 'w' : 'r',
                (pte & PT_USER) ? 'u' : 's',
                (pte & PT_ACCESSED) ? 'a' : ' ',
                (pte & PT_DIRTY) ? 'd' : ' ',
                (pte & PT_GUARD) ? 'g' : ' ',
                (pte & PT_FILE) ? 'f' : ' ');
      }

      vaddr += PAGESIZE;
    }

    if (!vaddr) break;
  }

  pprintf(pf, "\ntotal:%d usr:%d sys:%d rw: %d ro: %d acc: %d dirty: %d guard:%d file:%d\n", ma, us, su, rw, ro, ac, dt, gd, fi);
  return 0;
}

static print_virtmem(struct proc_file *pf, char *start, char *end, unsigned long tag) {
  char tagname[5];
  tag2str(tag, tagname);

  pprintf(pf, "%08x %08x %8dK %-4s\n", start, end - 1, (end - start) / 1024, tagname);
}

int virtmem_proc(struct proc_file *pf, void *arg) {
  char *vaddr;
  char *start;
  unsigned long curtag;
  int total = 0;

  pprintf(pf, "start    end           size type\n");
  pprintf(pf, "-------- -------- --------- ----\n");

  start = vaddr = NULL;
  curtag = 0;
  while (1) {
    if ((GET_PDE(vaddr) & PT_PRESENT) == 0) {
      if (start != NULL) {
        print_virtmem(pf, start, vaddr, curtag);
        start = NULL;
      }

      vaddr += PTES_PER_PAGE * PAGESIZE;
    } else {
      pte_t pte = GET_PTE(vaddr);
      unsigned long tag = pfdb[pte >> PT_PFNSHIFT].tag;

      if (pte & PT_PRESENT) {
        if (start == NULL)  {
          start = vaddr;
          curtag = tag;
        } else if (tag != curtag) {
          print_virtmem(pf, start, vaddr, curtag);
          start = vaddr;
          curtag = tag;
        }

        total += PAGESIZE;
      } else {
        if (start != NULL) {
          print_virtmem(pf, start, vaddr, curtag);
          start = NULL;
        }
      }

      vaddr += PAGESIZE;
    }

    if (!vaddr) break;
  }

  if (start) print_virtmem(pf, start, vaddr, curtag);
  pprintf(pf, "total             %8dK\n", total / 1024);

  return 0;
}

int pdir_stat(void *addr, int len, struct pdirstat *buf) {
  char *vaddr;
  char *end;
  pte_t pte;

  memset(buf, 0, sizeof(struct pdirstat));
  vaddr = (char *) addr;
  end = vaddr + len;
  while (vaddr < end) {
    if ((GET_PDE(vaddr) & PT_PRESENT) == 0) {
      vaddr += PTES_PER_PAGE * PAGESIZE;
      vaddr = (char *) ((unsigned long) vaddr & ~(PTES_PER_PAGE * PAGESIZE - 1));
    } else {
      pte = GET_PTE(vaddr);
      if (pte & PT_PRESENT) {
        buf->present++;
        
        if (pte & PT_WRITABLE) {
          buf->readwrite++;
        } else {
          buf->readonly++;
        }

        if (pte & PT_USER) {
          buf->user++;
        } else {
          buf->kernel++;
        }

        if (pte & PT_ACCESSED) buf->accessed++;
        if (pte & PT_DIRTY) buf->dirty++;
      }

      vaddr += PAGESIZE;
    }
  }

  return 0;
}
