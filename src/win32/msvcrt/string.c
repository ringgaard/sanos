//
// string.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// String routines
//

#include "msvcrt.h"

char *strchr(const char *string, int ch)
{
  TRACEX("strchr");
  while (*string && *string != (char) ch) string++;

  if (*string == (char) ch) return (char *) string;

  return NULL;
}

int strncmp(const char *first, const char *last, size_t count)
{
  TRACEX("strncmp");
  if (!count) return 0;

  while (--count && *first && *first == *last)
  {
    first++;
    last++;
  }

  return *(unsigned char *) first - *(unsigned char *) last;
}

char *strncpy(char *dest, const char *source, size_t count)
{
  char *start = dest;

  TRACEX("strncpy");
  while (count && (*dest++ = *source++)) count--;
  
  if (count)
  {
    while (--count) *dest++ = '\0';
  }

  return start;
}

char *strrchr(const char *string, int ch)
{
  char *start = (char *) string;

  TRACEX("strrchr");
  while (*string++);

  while (--string != start && *string != (char) ch);

  if (*string == (char) ch) return (char *) string;

  return NULL;
}

char *strstr(const char * str1, const char *str2)
{
  char *cp = (char *) str1;
  char *s1, *s2;

  TRACEX("strstr");
  if (!*str2) return (char *) str1;

  while (*cp)
  {
    s1 = cp;
    s2 = (char *) str2;

    while (*s1 && *s2 && !(*s1 - *s2)) s1++, s2++;
    if (!*s2) return cp;
    cp++;
  }

  return NULL;
}

char *_strdup(const char *string)
{
  char *memory;

  TRACEX("_strdup");
  if (!string) return NULL;

  if (memory = _malloc(strlen(string) + 1)) return strcpy(memory, string);

  return NULL;
}

#ifdef DEBUG

void *memset(void *p, int c, size_t n)
{
  char *pb = (char *) p;
  char *pbend = pb + n;
  while (pb != pbend) *pb++ = c;
  return p;
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

#endif