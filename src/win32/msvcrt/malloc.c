//
// malloc.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Heap allocation
//

#include "msvcrt.h"

void *_malloc(size_t size)
{
  TRACEX("_malloc");
  return malloc(size);
}

void *_calloc(size_t num, size_t size)
{
  TRACEX("_calloc");
  return calloc(num, size);
}

void *_realloc(void *mem, size_t size)
{
  TRACEX("_realloc");
  return realloc(mem, size);
}

void _free(void *mem)
{
  TRACEX("_free");
  free(mem);
}

