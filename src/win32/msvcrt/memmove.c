//
// memmove.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Memory copying
//

#include "msvcrt.h"

void *memmove(void *dst, const void *src, size_t count)
{
  void *ret = dst;

  TRACEX("memmove");
  if (dst <= src || (char *)dst >= ((char *)src + count)) 
  {
    while (count--) 
    {
      *(char *) dst = *(char *) src;
      dst = (char *) dst + 1;
      src = (char *) src + 1;
    }
  }
  else 
  {
    dst = (char *) dst + count - 1;
    src = (char *) src + count - 1;

    while (count--) 
    {
      *(char *) dst = *(char *) src;
      dst = (char *) dst - 1;
      src = (char *) src - 1;
    }
  }

  return ret;
}
