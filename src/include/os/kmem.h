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

int list_memmap(struct proc_file *pf, struct rmap *rmap, unsigned int startpos);
int kmem_proc(struct proc_file *pf, void *arg);
int kmodmem_proc(struct proc_file *pf, void *arg);

void init_kmem();

#endif
