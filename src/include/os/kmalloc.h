//
// kmalloc.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Kernel heap allocator
//

#ifndef KMALLOC_H
#define KMALLOC_H

struct bucket 
{
  void *mem;	        // List of chunks of memory
  unsigned long elems;	// # chunks available in this bucket
  unsigned long pages;  // # pages used for this bucket size
  unsigned long size;   // Size of this kind of chunk
};

extern struct bucket buckets[PAGESHIFT];

krnlapi void *kmalloc(int size);
krnlapi void *krealloc(void *addr, int newsize);
krnlapi void kfree(void *addr);

void init_malloc();

void dump_malloc();

void *malloc(size_t size);
void free(void *addr);

#endif
