//
// new.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Object new and delete operators
//

extern "C"
{
#include "os.h"

void *memset(void *, int, size_t);
}

void *operator new(unsigned int size)
{
  void *p;

  TRACE("new");
  p = malloc(size);
  if (p) memset(p, 0, size);
  return p;
}

void operator delete(void *p)
{
  TRACE("delete");
  if (p) free(p);
}
