//
// string.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// String routines
//

#include <types.h>
#include <stdarg.h>
#include <string.h>

char *strncpy(char *dest, const char *source, size_t count)
{
  char *start = dest;

  while (count && (*dest++ = *source++)) count--;
  if (count) while (--count) *dest++ = '\0';
  return start;
}

int strncmp(const char *s1, const char *s2, size_t count)
{
  if (!count) return 0;

  while (--count && *s1 && *s1 == *s2)
  {
    s1++;
    s2++;
  }

  return *(unsigned char *) s1 - *(unsigned char *) s2;
}

int stricmp(const char *s1, const char *s2)
{
  char f,l;

  do 
  {
    f = ((*s1 <= 'Z') && (*s1 >= 'A')) ? *s1 + 'a' - 'A' : *s1;
    l = ((*s2 <= 'Z') && (*s2 >= 'A')) ? *s2 + 'a' - 'A' : *s2;
    s1++;
    s2++;
  } while ((f) && (f == l));

  return (int) (f - l);
}

char *strchr(const char *s, int ch)
{
  while (*s && *s != (char) ch) s++;
  if (*s == (char) ch) return (char *) s;
  return NULL;
}

void *memmove(void *dst, const void *src, size_t count)
{
  void * ret = dst;

  if (dst <= src || (char *) dst >= ((char *) src + count)) 
  {
    //
    // Non-Overlapping Buffers
    // copy from lower addresses to higher addresses
    //
    while (count--) 
    {
      *(char *) dst = *(char *) src;
      dst = (char *) dst + 1;
      src = (char *) src + 1;
    }
  }
  else 
  {
    //
    // Overlapping Buffers
    // copy from higher addresses to lower addresses
    //
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

int atoi(const char *s)
{
  int n;
  int sign;

  if (!s) return -1;
  while (*s == ' ') s++;

  sign = *s;
  if (*s == '-' || *s == '+') s++;

  n = 0;
  if (*s < '0' || *s > '9') return -1;
  while (*s >= '0' && *s <= '9') n = 10 * n + (*s++ - '0');

  return sign == '-' ? -n : n;
}

/////////////////////////////////////////////////////////////////////
//
// intrinsic functions
//

#ifdef DEBUG

void *memset(void *p, int c, size_t n)
{
  char *pb = (char *) p;
  char *pbend = pb + n;
  while (pb != pbend) *pb++ = c;
  return p;
}

int memcmp(const void *dst, const void *src, size_t n)
{
  if (!n) return 0;

  while (--n && *(char *) dst == *(char *) src)
  {
    dst = (char *) dst + 1;
    src = (char *) src + 1;
  }

  return *((unsigned char *) dst) - *((unsigned char *) src);
}

void *memcpy(void *dst, const void *src, size_t n)
{
  void *ret = dst;

  while (n--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *) dst + 1;
    src = (char *) src + 1;
  }

  return ret;
}

char *strcpy(char *dst, const char *src)
{
  char *cp = dst;
  while (*cp++ = *src++);
  return dst;
}

size_t strlen(const char *s)
{
  const char *eos = s;
  while (*eos++);
  return (int) (eos - s - 1);
}

int strcmp(const char *src, const char *dst)
{
  int ret = 0 ;
  while (!(ret = *(unsigned char *) src - *(unsigned char *) dst) && *dst) ++src, ++dst;

  if (ret < 0)
    ret = -1 ;
  else if (ret > 0)
    ret = 1 ;

  return ret;
}

char *strcat(char *dst, const char *src)
{
  char *cp = dst;
  while(*cp) cp++;
  while (*cp++ = *src++);
  return dst;
}

#endif
