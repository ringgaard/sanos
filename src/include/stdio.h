//
// stdio.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Standard I/O routines
//

#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

typedef void FILE;

#define EOF     (-1)

#define BUFSIZ  4096

#define STREAM_OFFSET 1024

#define fileno(stream)   (((int) (stream)) - STREAM_OFFSET)
#define strmno(fd)       ((FILE *) ((fd) + STREAM_OFFSET))

#define stdin  (strmno(gettib()->in))
#define stdout (strmno(gettib()->out))
#define stderr (strmno(gettib()->err))

FILE *fopen(const char *filename, const char *mode);
int fflush(FILE *stream);
int fclose(FILE *stream);

int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);

int feof(FILE *stream);

int fgetc(FILE *stream);
int fputc(int c, FILE *stream);

char *fgets(char *buf, int n, FILE *stream);
int fputs(const char *string, FILE *stream);

size_t fread(void *buffer, size_t size, size_t num, FILE *stream);
size_t fwrite(const void *buffer, size_t size, size_t num, FILE *stream);

char *gets(char *buf);
int puts(const char *string);

void perror(const char *message);

int vfprintf(FILE *stream, const char *fmt, va_list args);
int fprintf(FILE *stream, const char *fmt, ...);

int vprintf(const char *fmt, va_list args);
int printf(const char *fmt, ...);

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);

#define getc(stream)     fgetc(stream)
#define putc(c, stream)  fputc(stream)
#define getchar()        getc(stdin)
#define putchar(c)       putc((c), stdout)

#endif
