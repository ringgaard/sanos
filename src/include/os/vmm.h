//
// vmm.h
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

#ifndef VMM_H
#define VMM_H

extern struct rmap *vmap;

void init_vmm();

krnlapi void *vmalloc(void *addr, unsigned long size, int type, int protect, unsigned long tag, int *rc);
krnlapi void *vmmap(void *addr, unsigned long size, int protect, struct file *filp, off64_t offset, int *rc);
krnlapi int vmsync(void *addr, unsigned long size);
krnlapi int vmfree(void *addr, unsigned long size, int type);
krnlapi void *vmrealloc(void *addr, unsigned long oldsize, unsigned long newsize, int type, int protect, unsigned long tag);
krnlapi int vmprotect(void *addr, unsigned long size, int protect);
krnlapi int vmlock(void *addr, unsigned long size);
krnlapi int vmunlock(void *addr, unsigned long size);

krnlapi void *miomap(unsigned long addr, int size, int protect);
krnlapi void miounmap(void *addr, int size);

int guard_page_handler(void *addr);
int fetch_page(void *addr);

int vmem_proc(struct proc_file *pf, void *arg);
int mem_sysinfo(struct meminfo *info);

#endif
