//
// vmm.c
//
// Virtual memory manager
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

#define VMAP_ENTRIES 1024
#define VMEM_START (64 * 1024)

struct rmap *vmap;

static int valid_range(void *addr, int size) {
  int pages = PAGES(size);

  if ((unsigned long) addr < VMEM_START) return 0;
  if (KERNELSPACE((unsigned long) addr + pages * PAGESIZE)) return 0;
  if (rmap_status(vmap, BTOP(addr), pages) != 1) return 0;
  return 1;
}

static unsigned long pte_flags_from_protect(int protect) {
  switch (protect) {
    case PAGE_NOACCESS: 
      return 0;

    case PAGE_READONLY:
    case PAGE_EXECUTE:
    case PAGE_EXECUTE_READ:
      return PT_USER;

    case PAGE_READWRITE:
    case PAGE_EXECUTE_READWRITE:
      return PT_USER | PT_WRITABLE;

    case PAGE_READONLY | PAGE_GUARD:
    case PAGE_EXECUTE | PAGE_GUARD:
    case PAGE_EXECUTE_READ | PAGE_GUARD:
      return PT_GUARD;

    case PAGE_READWRITE | PAGE_GUARD:
    case PAGE_EXECUTE_READWRITE | PAGE_GUARD:
      return PT_GUARD | PT_WRITABLE;
  }

  return 0xFFFFFFFF;
}

static int free_filemap(struct filemap *fm) {
  int rc;

  hunprotect(fm->file);
  rc = hfree(fm->file);
  if (rc < 0) return rc;
  
  rmap_free(vmap, BTOP(fm->addr), PAGES(fm->size));

  hunprotect(fm->self);
  rc = hfree(fm->self);
  if (rc < 0) return rc;

  return 0;
}

static int fetch_file_page(struct filemap *fm, void *addr) {
  struct file *filp;
  unsigned long pfn;
  unsigned long pos;
  int rc;

  filp = (struct file *) olock(fm->file, OBJECT_FILE);
  if (!filp) return -EBADF;

  pfn = alloc_pageframe('FMAP');
  if (pfn == 0xFFFFFFFF) {
    orel(filp);
    return -ENOMEM;
  }

  map_page(addr, pfn, PT_WRITABLE | PT_PRESENT);

  pos = (char *) addr - fm->addr;
  rc = pread(filp, addr, PAGESIZE, fm->offset + pos);
  if (rc < 0) {
    orel(filp);
    unmap_page(addr);
    free_pageframe(pfn);
    return rc;
  }

  pfdb[pfn].owner = fm->self;
  map_page(addr, pfn, fm->protect | PT_PRESENT);

  orel(filp);
  return 0;
}

static int save_file_page(struct filemap *fm, void *addr) {
  struct file *filp;
  unsigned long pos;
  int rc;

  filp = (struct file *) olock(fm->file, OBJECT_FILE);
  if (!filp) return -EBADF;

  pos = (char *) addr - fm->addr;
  rc = pwrite(filp, addr, PAGESIZE, fm->offset + pos);
  if (rc < 0) {
    orel(filp);
    return rc;
  }

  clear_dirty(addr);

  orel(filp);
  return 0;
}

void init_vmm() {
  vmap = (struct rmap *) kmalloc(VMAP_ENTRIES * sizeof(struct rmap));
  rmap_init(vmap, VMAP_ENTRIES);
  rmap_free(vmap, BTOP(VMEM_START), BTOP(OSBASE - VMEM_START));
}

void *vmalloc(void *addr, unsigned long size, int type, int protect, unsigned long tag, int *rc) {
  int pages = PAGES(size);
  unsigned long flags = pte_flags_from_protect(protect);
  int i;

  if (rc) *rc = 0;
  if (size == 0) {
    if (rc) *rc = -EINVAL;
    return NULL;
  }
  if ((type & MEM_COMMIT) != 0 && flags == 0xFFFFFFFF) {
    if (rc) *rc = -EINVAL;
    return NULL;
  }
  addr = (void *) PAGEADDR(addr);
  if (!addr && (type & MEM_COMMIT) != 0) type |= MEM_RESERVE;
  if (!tag) tag = 'VM';

  if (type & MEM_RESERVE) {
    if (addr == NULL) {
      if (type & MEM_ALIGN64K) {
        addr = (void *) PTOB(rmap_alloc_align(vmap, pages, 64 * 1024 / PAGESIZE));
      } else {
        addr = (void *) PTOB(rmap_alloc(vmap, pages));
      }

      if (addr == NULL) {
        if (rc) *rc = -ENOMEM;
        return NULL;
      }
    } else {
      if (rmap_reserve(vmap, BTOP(addr), pages)) {
        if (rc) *rc = -ENOMEM;
        return NULL;
      }
    }
  } else {
    if (!valid_range(addr, size)) {
      if (rc) *rc = -EFAULT;
      return NULL;
    }
  }

  if (type & MEM_COMMIT) {
    char *vaddr;
    unsigned long pfn;

    vaddr = (char *) addr;
    for (i = 0; i < pages; i++) {
      if (page_mapped(vaddr)) {
        set_page_flags(vaddr, flags | PT_PRESENT);
      } else {
        pfn = alloc_pageframe(tag);
        if (pfn == 0xFFFFFFFF) {
          if (rc) *rc = -ENOMEM;
          return NULL;
        }

        map_page(vaddr, pfn, flags | PT_PRESENT);
        memset(vaddr, 0, PAGESIZE);
      }
      vaddr += PAGESIZE;
    }
  }

  return addr;
}

void *vmmap(void *addr, unsigned long size, int protect, struct file *filp, off64_t offset, int *rc) {
  int pages = PAGES(size);
  unsigned long flags = pte_flags_from_protect(protect);
  struct filemap *fm;
  int i;
  char *vaddr;

  if (rc) *rc = 0;
  if (size == 0 || flags == 0xFFFFFFFF) {
    if (rc) *rc = -EINVAL;
    return NULL;
  }
  addr = (void *) PAGEADDR(addr);
  if (addr == NULL) {
    addr = (void *) PTOB(rmap_alloc(vmap, pages));
    if (addr == NULL) {
      if (rc) *rc = -ENOMEM;
      return NULL;
    }
  } else {
    if (rmap_reserve(vmap, BTOP(addr), pages)) {
      if (rc) *rc = -ENOMEM;
      return NULL;
    }
  }

  fm = (struct filemap *) kmalloc(sizeof(struct filemap));
  if (!fm) {
    rmap_free(vmap, BTOP(addr), pages);
    if (rc) *rc = -ENOMEM;
    return NULL;
  }
  init_object(&fm->object, OBJECT_FILEMAP);
  fm->self = halloc(&fm->object);
  fm->file = halloc(&filp->iob.object);
  if (fm->self < 0 || fm->file < 0) {
    if (rc) *rc = -ENFILE;
    return NULL;
  }
  hprotect(fm->self);
  hprotect(fm->file);
  fm->offset = offset;
  fm->pages = pages;
  fm->object.signaled = 1;
  fm->addr = addr;
  fm->size = size;
  fm->protect = flags | PT_FILE;

  vaddr = (char *) addr;
  flags = (flags & ~PT_USER) | PT_FILE;
  for (i = 0; i < pages; i++) {
    map_page(vaddr, fm->self, flags);
    vaddr += PAGESIZE;
  }

  return addr;
}

int vmsync(void *addr, unsigned long size) {
  struct filemap *fm = NULL;
  int pages = PAGES(size);
  int i, rc;
  char *vaddr;

  if (size == 0) return 0;
  addr = (void *) PAGEADDR(addr);
  if (!valid_range(addr, size)) return -EINVAL;

  vaddr = (char *) addr;
  for (i = 0; i < pages; i++) {
    if (page_directory_mapped(vaddr)) {
      pte_t flags = get_page_flags(vaddr);
      if ((flags & (PT_FILE | PT_PRESENT | PT_DIRTY)) == (PT_FILE | PT_PRESENT | PT_DIRTY)) {
        unsigned long pfn = BTOP(virt2phys(vaddr));
        struct filemap *newfm = (struct filemap *) hlookup(pfdb[pfn].owner);
        if (newfm != fm) {
          if (fm) {
            rc = unlock_filemap(fm);
            if (rc < 0) return rc;
          }
          fm = newfm;
          rc = wait_for_object(fm, INFINITE);
          if (rc < 0) return rc;
        }
        
        rc = save_file_page(fm, vaddr);
        if (rc < 0) return rc;
      }

      vaddr += PAGESIZE;
    }
  }

  if (fm) {
    rc = unlock_filemap(fm);
    if (rc < 0) return rc;
  }

  return 0;
}

int vmfree(void *addr, unsigned long size, int type) {
  struct filemap *fm = NULL;
  int pages = PAGES(size);
  int i, rc;
  char *vaddr;

  if (size == 0) return 0;
  addr = (void *) PAGEADDR(addr);
  if (!valid_range(addr, size)) return -EINVAL;

  if (type & (MEM_DECOMMIT | MEM_RELEASE)) {
    vaddr = (char *) addr;
    for (i = 0; i < pages; i++) {
      if (page_directory_mapped(vaddr)) {
        pte_t flags = get_page_flags(vaddr);
        unsigned long pfn = BTOP(virt2phys(vaddr));

        if (flags & PT_FILE) {
          handle_t h = (flags & PT_PRESENT) ? pfdb[pfn].owner : pfn;
          struct filemap *newfm = (struct filemap *) hlookup(h);
          if (newfm != fm) {
            if (fm) {
              if (fm->pages == 0) {
                rc = free_filemap(fm);
              } else {
                rc = unlock_filemap(fm);
              }
              if (rc < 0) return rc;
            }
            fm = newfm;
            rc = wait_for_object(fm, INFINITE);
            if (rc < 0) return rc;
          }
          fm->pages--;
          unmap_page(vaddr);
          if (flags & PT_PRESENT) free_pageframe(pfn);
        } else  if (flags & PT_PRESENT) {
          unmap_page(vaddr);
          free_pageframe(pfn);
        }
      }

      vaddr += PAGESIZE;
    }
  }

  if (fm) {
    if (fm->pages == 0) {
      rc = free_filemap(fm);
    } else {
      rc = unlock_filemap(fm);
    }
    if (rc < 0) return rc;
  } else if (type & MEM_RELEASE) {
    rmap_free(vmap, BTOP(addr), pages);
  }

  return 0;
}

void *vmrealloc(void *addr, unsigned long oldsize, unsigned long newsize, int type, int protect, unsigned long tag) {
  return NULL;
}

int vmprotect(void *addr, unsigned long size, int protect) {
  int pages = PAGES(size);
  int i;
  char *vaddr;
  unsigned long flags;

  if (size == 0) return 0;
  addr = (void *) PAGEADDR(addr);
  if (!valid_range(addr, size)) return -EINVAL;
  flags = pte_flags_from_protect(protect);
  if (flags == 0xFFFFFFFF) return -EINVAL;

  vaddr = (char *) addr;
  for (i = 0; i < pages; i++) {
    if (page_mapped(vaddr)) {
      set_page_flags(vaddr, (get_page_flags(vaddr) & ~PT_PROTECTMASK) | flags);
    }
    vaddr += PAGESIZE;
  }

  return 0;
}

int vmlock(void *addr, unsigned long size) {
  return -ENOSYS;
}

int vmunlock(void *addr, unsigned long size) {
  return -ENOSYS;
}

void *miomap(unsigned long addr, int size, int protect) {
  char *vaddr;
  int i;
  unsigned long flags = pte_flags_from_protect(protect);
  int pages = PAGES(size);

  vaddr = (char *) PTOB(rmap_alloc(vmap, pages));
  if (vaddr == NULL) return NULL;
  
  for (i = 0; i < pages; i++) {
    map_page(vaddr + PTOB(i), BTOP(addr) + i, flags | PT_PRESENT);
  }

  return vaddr;
}

void miounmap(void *addr, int size) {
  int i;
  int pages = PAGES(size);

  for (i = 0; i < pages; i++) unmap_page((char *) addr + PTOB(i));
  rmap_free(vmap, BTOP(addr), pages);
}

int guard_page_handler(void *addr) {
  unsigned long pfn;
  struct thread *t = self();

  if (!t->tib) return -EFAULT;
  
  if (addr < t->tib->stacklimit || addr >= t->tib->stacktop) return -EFAULT;
  if (t->tib->stacklimit <= t->tib->stackbase) return -EFAULT;

  pfn = alloc_pageframe('STK');
  if (pfn == 0xFFFFFFFF) return -ENOMEM;

  t->tib->stacklimit = (char *) t->tib->stacklimit - PAGESIZE;
  map_page(t->tib->stacklimit, pfn, PT_GUARD | PT_WRITABLE | PT_PRESENT);
  memset(t->tib->stacklimit, 0, PAGESIZE);

  return 0;
}

int fetch_page(void *addr) {
  struct filemap *fm;
  int rc;
  
  fm = (struct filemap *) olock(virt2pfn(addr), OBJECT_FILEMAP);
  if (!fm) return -EBADF;
  
  rc = wait_for_object(fm, INFINITE);
  if (rc < 0) {
    orel(fm);
    return rc;
  }

  if (!page_mapped(addr)) {
    rc = fetch_file_page(fm, addr);
    if (rc < 0) {
      unlock_filemap(fm);
      orel(fm);
      return rc;
    }
  }

  rc = unlock_filemap(fm);
  if (rc < 0) return rc;

  orel(fm);
  return 0;
}

int vmem_proc(struct proc_file *pf, void *arg) {
  return list_memmap(pf, vmap, BTOP(VMEM_START));
}

int mem_sysinfo(struct meminfo *info) {
  struct rmap *r;
  struct rmap *rlim;
  unsigned int free = 0;

  rlim = &vmap[vmap->offset];
  for (r = &vmap[1]; r <= rlim; r++) free += r->size;

  info->physmem_total = totalmem * PAGESIZE;
  info->physmem_avail = freemem * PAGESIZE;
  info->virtmem_total = OSBASE - VMEM_START;
  info->virtmem_avail = free * PAGESIZE;
  info->pagesize = PAGESIZE;

  return 0;
}
