#include "msvcrt.h"

void *bsearch(const void *key, const void *base, size_t num, size_t width, int (__cdecl *compare)(const void *, const void *))
{
  char *lo = (char *) base;
  char *hi = (char *) base + (num - 1) * width;
  char *mid;
  unsigned int half;
  int result;

  while (lo <= hi)
  {
    if (half = num / 2)
    {
      mid = lo + (num & 1 ? half : (half - 1)) * width;
      if (!(result = (*compare)(key,mid)))
        return mid;
      else if (result < 0)
      {
	hi = mid - width;
	num = num & 1 ? half : half-1;
      }
      else    
      {
	lo = mid + width;
	num = half;
      }
    }
    else if (num)
      return ((*compare)(key,lo) ? NULL : lo);
    else
      break;
  }

  return NULL;
}
