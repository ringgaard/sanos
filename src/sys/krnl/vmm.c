//
// vmm.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Virtual memory manager
//

#include <os/krnl.h>

#define VMAP_ENTRIES 1024
#define VMEM_START (64 * K)

struct rmap *vmap;

static int valid_range(void *addr, int size)
{
  int pages = PAGES(size);

  if ((unsigned long) addr < VMEM_START) return 0;
  if ((unsigned long) addr + pages * PAGESIZE >= OSBASE) return 0;
  if (rmap_status(vmap, BTOP(addr), pages) != 1) return 0;
  return 1;
}

void init_vmm()
{
  vmap = (struct rmap *) kmalloc(VMAP_ENTRIES * sizeof(struct rmap));
  rmap_init(vmap, VMAP_ENTRIES);
  rmap_free(vmap, BTOP(VMEM_START), BTOP(OSBASE - VMEM_START));
}

void *mmap(void *addr, unsigned long size, int type, int protect)
{
  int pages = PAGES(size);
  int i;

//kprintf("mmap(%p,%d,%x,%x)\n", addr, size, type, protect);

  if (size == 0) return NULL;
  addr = (void *) PAGEADDR(addr);
  if (!addr && (type & MEM_COMMIT)) type |= MEM_RESERVE;

  if (type & MEM_RESERVE)
  {
    if (addr == NULL)
    {
      addr = (void *) PTOB(rmap_alloc(vmap, pages));
      if (addr == NULL) return NULL;
    }
    else
    {
      if (rmap_reserve(vmap, BTOP(addr), pages)) return NULL;
    }
  }
  else
  {
    if (!valid_range(addr, size)) return NULL;
  }

  if (type & MEM_COMMIT)
  {
    char *vaddr;
    unsigned long flags;
    unsigned long pfn;

    switch (protect & ~PAGE_GUARD)
    {
      case PAGE_NOACCESS:
	flags = 0;
	break;

      case PAGE_READONLY:
      case PAGE_EXECUTE:
      case PAGE_EXECUTE_READ:
	flags = PT_USER;
	break;

      case PAGE_READWRITE:
      case PAGE_EXECUTE_READWRITE:
	flags = PT_USER | PT_WRITABLE;
	break;

      default:
	return NULL;
    }

    vaddr = (char *) addr;
    for (i = 0; i < pages; i++)
    {
      if (!page_mapped(vaddr))
      {
	if (protect & PAGE_GUARD)
	{
          map_page(vaddr, 0, flags | PT_GUARD);
	}
	else
	{
	  pfn = alloc_pageframe(PFT_USED);
	  map_page(vaddr, pfn, flags | PT_PRESENT);
	  memset(vaddr, 0, PAGESIZE);
	}
      }
      vaddr += PAGESIZE;
    }
  }

//kprintf("mmap returned %p\n", addr);
  return addr;
}

int munmap(void *addr, unsigned long size, int type)
{
  int pages = PAGES(size);
  int i;
  char *vaddr;

  if (size == 0) return 0;
  addr = (void *) PAGEADDR(addr);
  if (!valid_range(addr, size)) return -EINVAL;

  if (type == MEM_DECOMMIT || type == MEM_RELEASE)
  {
    vaddr = (char *) addr;
    for (i = 0; i < pages; i++)
    {
      if (page_mapped(vaddr))
      {
	unsigned long pfn = BTOP(virt2phys(vaddr));
	unmap_page(vaddr);
	free_pageframe(pfn);
      }
      vaddr += PAGESIZE;
    }
  }
  
  if (type == MEM_RELEASE)
  {
    rmap_free(vmap, BTOP(addr), pages);
  }
  
  return 0;
}

void *mremap(void *addr, unsigned long oldsize, unsigned long newsize, int type, int protect)
{
  return NULL;
}

int mprotect(void *addr, unsigned long size, int protect)
{
  int pages = PAGES(size);
  int i;
  char *vaddr;
  unsigned long flags;

  if (size == 0) return 0;
  addr = (void *) PAGEADDR(addr);
  if (!valid_range(addr, size)) return -EINVAL;

  switch (protect)
  {
    case PAGE_NOACCESS:
      flags = 0;
      break;

    case PAGE_READONLY:
    case PAGE_EXECUTE:
    case PAGE_EXECUTE_READ:
      flags = PT_USER;
      break;

    case PAGE_READWRITE:
    case PAGE_EXECUTE_READWRITE:
      flags = PT_USER | PT_WRITABLE;
      break;

    default:
      return -EINVAL;
  }

  vaddr = (char *) addr;
  for (i = 0; i < pages; i++)
  {
    if (page_mapped(vaddr))
    {
      ptab[PTABIDX(vaddr)] &= ~(PT_USER | PT_WRITABLE);
      ptab[PTABIDX(vaddr)] |= flags;
    }
    vaddr += PAGESIZE;
  }
  flushtlb();

  return 0;
}

int mlock(void *addr, unsigned long size)
{
  return -ENOSYS;
}

int munlock(void *addr, unsigned long size)
{
  return -ENOSYS;
}

int guard_page_handler(void *addr)
{
  unsigned long flags;
  unsigned long pfn;

  flags = get_page_flags(addr);
  pfn = alloc_pageframe(PFT_USED);
  map_page(addr, pfn, (flags & ~PT_GUARD) | PT_PRESENT);
  memset(addr, 0, PAGESIZE);
  return 0;
}
