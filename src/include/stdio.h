//
// stdio.h
//
// Standard I/O routines
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

#ifndef STDIO_H
#define STDIO_H

#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
typedef char *va_list;
#endif

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

// TODO: libc implement: implement new stdio with buffering

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
