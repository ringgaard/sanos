//
// stdlib.c
//
// Standard library functions
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

#include <os.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inifile.h>

#define SHELL "sh.exe"

static long holdrand = 1L;

typedef void (__cdecl *proc_t)(void);

static proc_t *atexit_begin = NULL;
static proc_t *atexit_end = NULL;
static proc_t *atexit_last = NULL;

#pragma function(abs)
#pragma function(labs)

int atexit(proc_t exitfunc) {
  if (atexit_end == atexit_last) {
    int size = atexit_end - atexit_begin;
    int newsize = size + 32;
    atexit_begin = (proc_t *) realloc(atexit_begin, newsize * sizeof(proc_t));
    if (atexit_begin == NULL) return -ENOMEM;
    atexit_end = atexit_begin + size;
    atexit_last = atexit_begin + newsize;
  }

  *atexit_end++ = exitfunc;
  return 0;
}

void run_atexit_handlers() {
  if (atexit_begin) {
    while (--atexit_end >= atexit_begin) if (*atexit_end != NULL) (**atexit_end)();
    free(atexit_begin);
    atexit_begin = atexit_end = atexit_last = NULL;
  }
}

void abort() {
  raise(SIGABRT);
#ifdef __GNUC__
  while (1);
#endif
}

int abs(int number) {
  return number >= 0 ? number : -number;
}

div_t div(int numer, int denom) {
  div_t result;

  result.quot = numer / denom;
  result.rem  = numer - (result.quot * denom);

  if (numer < 0 && result.rem > 0) {
    result.quot++;
    result.rem -= denom;
  }

  return result;
}

long labs(long n) {
  return  n >= 0L ? n : -n;
}

ldiv_t ldiv(long numer, long denom) {
  ldiv_t result;

  result.quot = numer / denom;
  result.rem = numer % denom;

  if (numer < 0 && result.rem > 0) {
    result.quot++;
    result.rem -= denom;
  }

  return result;
}

void srand(unsigned int seed) {
  holdrand = (long) seed;
}

int rand() {
  return (((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}

int system(const char *command) {
  char *cmdline;
  int cmdlen;
  int rc;

  if (!command) {
    errno = EINVAL;
    return -1;
  }

  cmdlen = strlen(SHELL) + 1 + strlen(command);
  cmdline = malloc(cmdlen + 1);
  if (!cmdline) {
    errno = ENOMEM;
    return -1;
  }

  strcpy(cmdline, SHELL);
  strcat(cmdline, " ");
  strcat(cmdline, command);

  rc = spawn(P_WAIT, SHELL, cmdline, NULL, NULL);
  free(cmdline);
  return rc;
}

char *realpath(const char *path, char *resolved) {
  int rc;
  char *buffer = NULL;

  if (!resolved) resolved = buffer = malloc(MAXPATH);
  rc = canonicalize(path, resolved, MAXPATH);
  if (rc < 0) {
    resolved = NULL;
    if (buffer) free(buffer);
  }
  return resolved;
}

int getrusage(int who, struct rusage *usage) {
  // TODO implement
  errno = ENOSYS;
  return -1;
}

char *setlocale(int category, const char *locale) {
  // TODO implement
  return NULL;
}

struct lconv *localeconv(void) {
  //  TODO implement
  return NULL;
}

