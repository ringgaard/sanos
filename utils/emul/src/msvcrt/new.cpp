extern "C"
{
#include "os.h"
}

void *operator new(unsigned int size)
{
  void *p;

  p = malloc(size);
  if (p) memset(p, 0, size);
  return p;
}

void operator delete(void *p)
{
  if (p) free(p);
}
