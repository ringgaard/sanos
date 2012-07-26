//
// opts.c
//
// Option string parsing
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

#include <string.h>
#include <stdlib.h>

char *get_option(char *opts, char *name, char *buffer, int size, char *defval) {
  char *eq;
  char *p;

  if (!opts) {
    if (!defval) return NULL;
    strncpy(buffer, defval, size);
    return buffer;
  }

  while (*opts) {
    while (*opts == ' ') opts++;
    p = opts;
    while (*p != 0 && *p != ',') p++;

    eq = opts;
    while (*eq != 0 && *eq != '=') eq++;

    if (*eq == '=') {
      if (strncmp(name, opts, eq - opts) == 0) {
        if (!buffer) return eq + 1;

        if (p - (eq + 1) > size) {
          strncpy(buffer, eq + 1, size);
        } else {
          memcpy(buffer, eq + 1, p - (eq + 1));
          buffer[p - (eq + 1)] = 0;
        }

        return buffer;
      }
    } else {
      if (strncmp(name, opts, p - opts) == 0) {
        if (!buffer) return "";
        *buffer = 0;
        return buffer;
      }
    }

    opts = p;
    if (*opts == ',') opts++;
  }

  if (!defval) return NULL;
  if (buffer) strncpy(buffer, defval, size);
  return buffer;
}

int get_num_option(char *opts, char *name, int defval) {
  char buffer[32];
  char *p;
  int value;

  p = get_option(opts, name, buffer, 32, NULL);
  if (!p) return defval;

  while (*p == ' ') p++;
  
  if (p[0] == '0' && p[1] == 'x') {
    p += 2;
    value = 0;
    while (*p) {
      if (*p >= '0' && *p <= '9') {
        value = (value << 4) | (*p - '0');
      } else if (*p >= 'A' && *p <= 'F') {
        value = (value << 4) | (*p - 'A' + 10);
      } else if (*p >= 'a' && *p <= 'f') {
        value = (value << 4) | (*p - 'a' + 10);
      } else {
        return defval;
      }

      p++;
    }

    return value;
  }

  return atoi(p);
}
