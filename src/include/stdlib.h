//
// stdlib.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Standard library routines
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
