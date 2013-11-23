//
// stdlib.h
//
// Standard library routines
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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef STDLIB_H
#define STDLIB_H

#include <sys/types.h>

#define RAND_MAX     0x7fff

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#ifndef _DIV_T_DEFINED
#define _DIV_T_DEFINED

typedef struct _div_t {
  int quot;
  int rem;
} div_t;

typedef struct _ldiv_t {
  long quot;
  long rem;
} ldiv_t;

#endif

#ifdef  __cplusplus
extern "C" {
#endif

void abort();
int abs(int n);
double atof(const char *str);
int atoi(const char *string);
long atol(const char *string);
int atexit(void (*func)(void));
void *bsearch(const void *key, const void *base, size_t num, size_t width, int (*compare)(const void *, const void *));
div_t div(int numer, int denom);
osapi void exit(int status);
char *itoa(int val, char *buf, int radix);
long labs(long n);
ldiv_t ldiv(long numer, long denom);
char *ltoa(long val, char *buf, int radix);
void qsort(void *base, unsigned num, unsigned width, int (*comp)(const void *, const void *));
int rand();
void srand(unsigned int seed);
double strtod(const char *str, char **endptr);
float strtof(const char *str, char **endptr);
long double strtold(const char *str, char **endptr);
long strtol(const char *nptr, char **endptr, int ibase);
__int64 strtoll(const char *nptr, char **endptr, int ibase);
unsigned long strtoul(const char *nptr, char **endptr, int ibase);
int system(const char *command);
char *ultoa(unsigned long val, char *buf, int radix);

char *ecvt(double arg, int ndigits, int *decpt, int *sign);
char *ecvtbuf(double arg, int ndigits, int *decpt, int *sign, char *buf);
char *fcvt(double arg, int ndigits, int *decpt, int *sign);
char *fcvtbuf(double arg, int ndigits, int *decpt, int *sign, char *buf);

#ifdef USE_LOCAL_HEAP
osapi void *_lmalloc(size_t size);
osapi void *_lrealloc(void *mem, size_t size);
osapi void *_lcalloc(size_t num, size_t size);
osapi void _lfree(void *p);

#define malloc(n) _lmalloc(n)
#define realloc(p, n) _lrealloc((p), (n))
#define calloc(n, s) _lcalloc((n), (s))
#define free(p) _lfree(p)

#else
osapi void *malloc(size_t size);
osapi void *realloc(void *mem, size_t size);
osapi void *calloc(size_t num, size_t size);
osapi void free(void *p);
#endif

osapi int malloc_usable_size(void *p);

#ifndef NOGETOPTION
char *get_option(char *opts, char *name, char *buffer, int size, char *defval);
int get_num_option(char *opts, char *name, int defval);
#endif

#ifndef NOREADLINE
void add_to_history(char *line);
int readline(char *buf, int size);
#endif

void srandom(unsigned long x);
void srandomdev();
char *initstate(unsigned long seed, char *arg_state, long n);
char *setstate(char *arg_state);
long random();

char *realpath(const char *path, char *resolved);
char *mktemp(char *tmpl);
int mkstemp(char *tmpl);
int mkstemps(char *tmpl, int suffixlen);

osapi char *getenv(const char *name);
osapi int setenv(const char *name, const char *value, int rewrite);
osapi void unsetenv(const char *name);
osapi int putenv(const char *str);

#ifdef  __cplusplus
}
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#endif
