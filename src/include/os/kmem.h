//
// kmem.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Kernel memory page allocator
//

#ifndef KMEM_H
#define KMEM_H

extern struct rmap *osvmap;
extern struct rmap *kmodmap;

krnlapi void *alloc_pages(int pages);
krnlapi void *alloc_pages_align(int pages, int align);
krnlapi void free_pages(void *addr, int pages);

krnlapi void *iomap(unsigned long addr, int size);
krnlapi void iounmap(void *addr, int size);

void *alloc_module_mem(int pages);
void free_module_mem(void *addr, int pages);

void init_kmem();

#endif
