#include "msvcrt.h"

void *_malloc(size_t size)
{
  return malloc(size);
}

void *_calloc(size_t num, size_t size)
{
  return calloc(num, size);
}

void *_realloc(void *mem, size_t size)
{
  return realloc(mem, size);
}

void _free(void *mem)
{
  free(mem);
}

