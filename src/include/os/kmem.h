//
// kmem.h
//
// Kernel memory page allocator
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

#ifndef KMEM_H
#define KMEM_H

extern struct rmap *osvmap;
extern struct rmap *kmodmap;

krnlapi void *alloc_pages(int pages, unsigned long tag);
krnlapi void *alloc_pages_align(int pages, int align, unsigned long tag);
krnlapi void *alloc_pages_linear(int pages, unsigned long tag);
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
