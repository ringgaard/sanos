//
// string.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// String routines
//

#ifndef STRING_H
#define STRING_H

#ifndef TYPES_H
#include <types.h>
#endif

#ifndef STDARG_H
#include <stdarg.h>
#endif

char *strncpy(char *dest, const char *source, size_t count);
int strncmp(const char *s1, const char *s2, size_t count);
char *strdup(char *s);
int stricmp(const char *s1, const char *s2);
char *strchr(const char *s, int ch);
char *strrchr(const char *s, int ch);
char *strstr(const char *s1, const char *s2);

long atol(const char *s);
int atoi(const char *s);

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);

void *memmove(void *dst, const void *src, size_t count);

// Intrinsic functions

void *memcpy(void *, const void *, size_t);
int memcmp(const void *, const void *, size_t);
void *memset(void *, int, size_t);

char *strcpy(char *, const char *);
char *strcat(char *, const char *);
int strcmp(const char *, const char *);
size_t strlen(const char *);

#endif
