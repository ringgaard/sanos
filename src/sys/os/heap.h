//
// heap.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Heap memory management routines
//

#ifndef HEAP_H
#define HEAP_H

void *heap_alloc(size_t size);
void *heap_realloc(void *mem, size_t size);
void *heap_calloc(size_t num, size_t size);
void heap_free(void *p);

#endif
