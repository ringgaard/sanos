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

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define RAND_MAX     0x7fff

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#ifndef _DIV_T_DEFINED
#define _DIV_T_DEFINED

typedef struct _div_t 
{
  int quot;
  int rem;
} div_t;

typedef struct _ldiv_t 
{
  long quot;
  long rem;
} ldiv_t;

typedef struct _uldiv_t 
{
  unsigned long quot;
  unsigned long rem;
} uldiv_t;

#endif

int parse_args(char *args, char **argv);
void free_args(int argc, char **argv);

char *get_option(char *opts, char *name, char *buffer, int size, char *defval);
int get_num_option(char *opts, char *name, int defval);

int readline(int f, char *buf, int size);

int abs(int number);

double modf(double x, double *iptr);
char *ecvt(double arg, int ndigits, int *decpt, int *sign);
char *ecvtbuf(double arg, int ndigits, int *decpt, int *sign, char *buf);
char *fcvt(double arg, int ndigits, int *decpt, int *sign);
char *fcvtbuf(double arg, int ndigits, int *decpt, int *sign, char *buf);

char *strerror(int errnum);

void *bsearch(const void *key, const void *base, size_t num, size_t width, int (__cdecl *compare)(const void *, const void *));
void qsort(void *base, unsigned num, unsigned width, int (__cdecl *comp)(const void *, const void *));

long strtol(const char *nptr, char **endptr, int ibase);
unsigned long strtoul(const char *nptr, char **endptr, int ibase);

char *getenv(const char *option);

void abort();

#if 0
// TODO: libc implement
int abs(int n);
double atof(const char *string);
int atoi(const char *string);
long atol(const char *string);
int atexit(void(*func)(void));
void *bsearch(const void *keyval, const void *base, size_t num, size_t width, int(*cmp) (const void *keyval, const void *elem));
void  *calloc(size_t num, size_t size);
div_t div(int numer, int denom);
void exit(int status);
void free(void *memblock);
char *_getcmd(void);
char *getenv(const char *varname);
char *itoa(int value, char *string, int radix);
long labs(long n);
ldiv_t ldiv(long numer, long denom);
char *ltoa(long value, char *string, int radix);
void *malloc(size_t size);
int putenv(const char *envptr);
void qsort(void *base, size_t num, size_t width, int(*cmp) (const void *elem1, const void *elem2));
int rand(void);
void *realloc(void *memblock, size_t size);
void srand(unsigned int seed);
double strtod(const char *string, char **endptr);
long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
int system(const char *command);
uldiv_t uldiv(unsigned long numer, unsigned long denom);
char *ultoa(unsigned long value, char *string, int radix);
#endif

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

#endif
