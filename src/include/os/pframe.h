//
// pframe.h
//
// Page frame database routines
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

#ifndef PFRAME_H
#define PFRAME_H

#define DMA_BUFFER_START 0x10000
#define DMA_BUFFER_PAGES 16

struct pageframe {
  unsigned long tag;
  union {
    unsigned long locks;        // Number of locks
    unsigned long size;         // Size/buckets for kernel pages
    handle_t owner;             // Reference to owner for file maps
    struct pageframe *next;     // Next free page frame for free pages
  };
};

extern struct pageframe *pfdb;

extern unsigned long freemem;
extern unsigned long totalmem;
extern unsigned long maxmem;

krnlapi unsigned long alloc_pageframe(unsigned long tag);
krnlapi unsigned long alloc_linear_pageframes(int pages, unsigned long tag);
krnlapi void free_pageframe(unsigned long pfn);
krnlapi void set_pageframe_tag(void *addr, unsigned int len, unsigned long tag);

void tag2str(unsigned long tag, char *str);
int memmap_proc(struct proc_file *pf, void *arg);
int memusage_proc(struct proc_file *pf, void *arg);
int memstat_proc(struct proc_file *pf, void *arg);
int physmem_proc(struct proc_file *pf, void *arg);

void init_pfdb();

#endif
