//
// string.c
//
// String routines
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of Michael Ringgaard nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#include <types.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#pragma function(memset)
#pragma function(memcmp)
#pragma function(memcpy)

#pragma function(strcpy)
#pragma function(strlen)
#pragma function(strcat)
#pragma function(strcmp)

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

int strnicmp(const char *s1, const char *s2, size_t count)
{
  int f, l;

  do 
  {
      if (((f = (unsigned char)(*(s1++))) >= 'A') && (f <= 'Z')) f -= 'A' - 'a';
      if (((l = (unsigned char)(*(s2++))) >= 'A') && (l <= 'Z')) l -= 'A' - 'a';
  } while (--count && f && (f == l));

  return f - l;
}

char *strchr(const char *s, int ch)
{
  while (*s && *s != (char) ch) s++;
  if (*s == (char) ch) return (char *) s;
  return NULL;
}

char *strrchr(const char *s, int ch)
{
  char *start = (char *) s;

  while (*s++);
  while (--s != start && *s != (char) ch);
  if (*s == (char) ch) return (char *) s;

  return NULL;
}

char *strstr(const char *str1, const char *str2)
{
  char *cp = (char *) str1;
  char *s1, *s2;

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

void *memchr(const void *buf, int ch, size_t count)
{
  while (count && (*(unsigned char *)buf != (unsigned char)ch)) 
  {
    buf = (unsigned char *) buf + 1;
    count--;
  }

  return (count ? (void *) buf : NULL);
}

long atol(const char *nptr)
{
  int c;
  long total;
  int sign;

  while (isspace((int)(unsigned char) *nptr)) ++nptr;

  c = (int)(unsigned char) *nptr++;
  sign = c;
  if (c == '-' || c == '+') c = (int)(unsigned char) *nptr++;

  total = 0;
  while (isdigit(c)) 
  {
    total = 10 * total + (c - '0');
    c = (int)(unsigned char) *nptr++;
  }

  if (sign == '-')
    return -total;
  else
    return total;
}

int atoi(const char *nptr)
{
  return (int) atol(nptr);
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
