//
// vmm.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Virtual memory manager
//

#ifndef VMM_H
#define VMM_H

extern struct rmap *vmap;

void init_vmm();

krnlapi void *mmap(void *addr, unsigned long size, int type, int protect);
krnlapi int munmap(void *addr, unsigned long size, int type);
krnlapi void *mremap(void *addr, unsigned long oldsize, unsigned long newsize, int type, int protect);
krnlapi int mprotect(void *addr, unsigned long size, int protect);
krnlapi int mlock(void *addr, unsigned long size);
krnlapi int munlock(void *addr, unsigned long size);

int guard_page_handler(void *addr);
int vmem_proc(struct proc_file *pf, void *arg);

#endif
