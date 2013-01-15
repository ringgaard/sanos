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

#include <sys/types.h>

#ifndef _FPOS_T_DEFINED
#define _FPOS_T_DEFINED
typedef long fpos_t;
#endif

#ifndef SEEK_SET
#define SEEK_SET   0
#define SEEK_CUR   1
#define SEEK_END   2
#endif

#define L_tmpnam       256
#define FILENAME_MAX   256
#define EOF            (-1)
#define BUFSIZ         512

struct _iobuf {
  char *ptr;
  int cnt;
  char *base;
  int flag;
  handle_t file;
  int charbuf;
  int bufsiz;
  int phndl;
};

typedef struct _iobuf FILE;

#define stdin   __getstdfile(0)
#define stdout  __getstdfile(1)
#define stderr  __getstdfile(2)

#define _IORD           0x0001
#define _IOWR           0x0002

#define _IOFBF          0x0000
#define _IOLBF          0x0040
#define _IONBF          0x0004

#define _IOOWNBUF       0x0008
#define _IOEXTBUF       0x0100
#define _IOTMPBUF       0x1000

#define _IOEOF          0x0010
#define _IOERR          0x0020
#define _IOSTR          0x0040
#define _IORW           0x0080

#define _IOCRLF         0x8000

#ifdef  __cplusplus
extern "C" {
#endif

int filbuf(FILE *stream);
int flsbuf(int, FILE *stream);

FILE *fdopen(int fd, const char *mode);
FILE *freopen(const char *filename, const char *mode, FILE *stream);
FILE *fopen(const char *filename, const char *mode);

FILE *popen(const char *command, const char *mode);
int pclose(FILE *stream);

void clearerr(FILE *stream);
int fclose(FILE *stream);
int fflush(FILE *stream);

int fgetc(FILE *stream);
int fputc(int c, FILE *stream);

char *fgets(char *string, int n, FILE *stream);
int fputs(const char *string, FILE *stream);

char *gets(char *buf);
int puts(const char *string);

size_t fread(void *buffer, size_t size, size_t num, FILE *stream);
size_t fwrite(const void *buffer, size_t size, size_t num, FILE *stream);

int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
void rewind(FILE *stream);
int fsetpos(FILE *stream, const fpos_t *pos);
int fgetpos(FILE *stream, fpos_t *pos);

void perror(const char *message);

void setbuf(FILE *stream, char *buffer);
int setvbuf(FILE *stream, char *buffer, int type, size_t size);

int ungetc(int c, FILE *stream);

int fready(FILE *stream);

int remove(const char *filename);
osapi int rename(const char *oldname, const char *newname);

FILE *tmpfile();
char *tmpnam(char *string);
char *tempnam(const char *dir, const char *prefix);

int vfprintf(FILE *stream, const char *fmt, va_list args);
int fprintf(FILE *stream, const char *fmt, ...);

int vprintf(const char *fmt, va_list args);
int printf(const char *fmt, ...);

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);

int vsnprintf(char *buf, size_t count, const char *fmt, va_list args);
int snprintf(char *buf, size_t count, const char *fmt, ...);

int fscanf(FILE *stream, const char *fmt, ...);
int scanf(const char *fmt, ...);
int sscanf(const char *buffer, const char *fmt, ...);

int vfscanf(FILE *stream, const char *fmt, va_list args);
int vscanf(const char *fmt, va_list args);
int vsscanf(const char *buffer, const char *fmt, va_list args);

FILE *__getstdfile(int n);

#ifdef  __cplusplus
}
#endif

#define feof(stream)     ((stream)->flag & _IOEOF)
#define ferror(stream)   ((stream)->flag & _IOERR)
#define fileno(stream)   ((stream)->file)

#define getc(stream)     (--(stream)->cnt >= 0 ? 0xff & *(stream)->ptr++ : filbuf(stream))
#define putc(c, stream)  (--(stream)->cnt >= 0 ? 0xff & (*(stream)->ptr++ = (char) (c)) :  flsbuf((c), (stream)))
#define getchar()        getc(stdin)
#define putchar(c)       putc((c), stdout)

#endif
