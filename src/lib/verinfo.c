//
// verinfo.c
//
// Module version information
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
// 3. Neither the name of the project nor the names of its contributors
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

#ifdef KERNEL
#include <os/krnl.h>
#else
#include <os.h>
#include <string.h>
#include <sys/types.h>
#include <moddb.h>
#include <verinfo.h>
#endif

#define ALIGN(p) ((char *)(((unsigned long) (p) + 3) & ~3))

#define TYPE_VALUE_BINARY 0
#define TYPE_VALUE_TEXT   1

#pragma pack(push, 1)

struct stringinfo {
  unsigned short length;
  unsigned short value_length;
  unsigned short type;
  wchar_t key[0];
};

struct version_resource {
  unsigned short length;
  unsigned short value_length;
  unsigned short type;
  wchar_t key[16];
  unsigned short padding1;
  struct verinfo fixed;
  char value[0];
};

#pragma pack(pop)

static int streq(wchar_t *s1, char *s2) {
  while (*s1 && *s2) if (*s1++ != *s2++) return 0;
  return !*s1 && !*s2;
}

static char *getchild(char *start, char *end, char *name, int *size) {
  char *p = start;
  while (p < end) {
    struct stringinfo *si = (struct stringinfo *) p;
    if (si->length == 0) break;
    if (streq(si->key, name)) {
      end = p + si->length;
      p += 3 * sizeof(unsigned short) + (strlen(name) + 1) * sizeof(wchar_t);
      p = ALIGN(p);
      *size = end - p;
      return p;
    }

    p += si->length;
    p = ALIGN(p);
  }

  return NULL;
}

static struct version_resource *getver(hmodule_t hmod) {
  int rc;
  struct version_resource *ver;

  rc = get_resource_data(hmod, INTRES(16), INTRES(1), 0, (void **) &ver);
  if (rc < 0) return NULL;
  if (rc < sizeof(struct version_resource)) return NULL;
  if (ver->fixed.signature != VER_SIGNATURE) return NULL;
  
  return ver;
}

struct verinfo *get_version_info(hmodule_t hmod) {
  struct version_resource *ver;
  
  ver = getver(hmod);
  if (ver == NULL) return NULL;
  return &ver->fixed;
}

int get_version_value(hmodule_t hmod, char *name, char *buf, int size) {
  struct version_resource *ver;
  char *start;
  char *end;
  int len;
  char *p;
  wchar_t *q;
  
  ver = getver(hmod);
  if (ver == NULL) return -EINVAL;

  start = (char *) ver->value;
  end = (char *) ver + ver->length;

  p = getchild(start, end, "StringFileInfo", &len);
  if (!p) return -ENOENT;

  p = getchild(p, p + len, "00000000", &len);
  if (!p) return -ENOENT;

  p = getchild(p, p + len, name, &len);
  if (!p) return -ENOENT;

  q = (wchar_t *) p;
  p = buf;
  while (*q) {
    if (--size == 0) return -E2BIG;
    *p++ = (char) *q++;
  }
  *p = 0;

  return p - buf;
}
