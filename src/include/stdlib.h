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

#ifndef STDLIB_H
#define STDLIB_H

#ifndef TYPES_H
#include <types.h>
#endif

#define offsetof(s, m) ((size_t)&(((s *) 0)->m))

int parse_args(char *args, char **argv);
void free_args(int argc, char **argv);

char *get_option(char *opts, char *name, char *buffer, int size, char *defval);
int get_num_option(char *opts, char *name, int defval);

int abs(int number);

char *strerror(int errnum);

void *bsearch(const void *key, const void *base, size_t num, size_t width, int (__cdecl *compare)(const void *, const void *));
void qsort(void *base, unsigned num, unsigned width, int (__cdecl *comp)(const void *, const void *));

long strtol(const char *nptr, char **endptr, int ibase);
unsigned long strtoul(const char *nptr, char **endptr, int ibase);

char *getenv(const char *option);

void abort();

#endif
