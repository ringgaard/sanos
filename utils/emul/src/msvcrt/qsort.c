#include "msvcrt.h"

#define CUTOFF 8

static void shortsort(char *lo, char *hi, unsigned width, int (__cdecl *comp)(const void *, const void *));
static void swap(char *p, char *q, unsigned int width);

void qsort(void *base, unsigned num, unsigned width, int (__cdecl *comp)(const void *, const void *))
{
  char *lo, *hi;
  char *mid;
  char *loguy, *higuy;
  unsigned size;
  char *lostk[30], *histk[30];
  int stkptr;

  if (num < 2 || width == 0) return;
  stkptr = 0;

  lo = base;
  hi = (char *) base + width * (num-1);

recurse:
  size = (hi - lo) / width + 1;        /* number of el's to sort */

  if (size <= CUTOFF) 
  {
    shortsort(lo, hi, width, comp);
  }
  else 
  {
    mid = lo + (size / 2) * width;
    swap(mid, lo, width);

    loguy = lo;
    higuy = hi + width;

    for (;;) 
    {
      do  { loguy += width; } while (loguy <= hi && comp(loguy, lo) <= 0);
      do  { higuy -= width; } while (higuy > lo && comp(higuy, lo) >= 0);
      if (higuy < loguy) break;
      swap(loguy, higuy, width);
    }

    swap(lo, higuy, width);

    if ( higuy - 1 - lo >= hi - loguy ) 
    {
      if (lo + width < higuy) 
      {
	lostk[stkptr] = lo;
	histk[stkptr] = higuy - width;
	++stkptr;
      }

      if (loguy < hi) 
      {
	lo = loguy;
	goto recurse;
      }
    }
    else
    {
      if (loguy < hi) 
      {
	  lostk[stkptr] = loguy;
	  histk[stkptr] = hi;
	  ++stkptr;
      }

      if (lo + width < higuy) 
      {
	  hi = higuy - width;
	  goto recurse;
      }
    }
  }

  --stkptr;
  if (stkptr >= 0) 
  {
    lo = lostk[stkptr];
    hi = histk[stkptr];
    goto recurse;
  }
  else
    return;
}

static void shortsort(char *lo, char *hi, unsigned width, int (__cdecl *comp)(const void *, const void *))
{
  char *p, *max;

  while (hi > lo) 
  {
    max = lo;
    for (p = lo+width; p <= hi; p += width) if (comp(p, max) > 0) max = p;
    swap(max, hi, width);
    hi -= width;
  }
}

static void swap(char *a, char *b, unsigned width)
{
  char tmp;

  if (a != b)
  {
    while (width--) 
    {
      tmp = *a;
      *a++ = *b;
      *b++ = tmp;
    }
  }
}
