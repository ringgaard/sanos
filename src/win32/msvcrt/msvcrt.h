//
// msvcrt.h
//
// C runtime library
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

#ifndef MSVCRT_H
#define MSVCRT_H

#include <os.h>
#include <stdarg.h>
#include <inifile.h>
#include <win32.h>

// Application types

#define _UNKNOWN_APP    0
#define _CONSOLE_APP    1
#define _GUI_APP        2

// I/O stream flags

#define _IOREAD         0x0001
#define _IOWRT          0x0002

#define _IOFBF          0x0000
#define _IOLBF          0x0040
#define _IONBF          0x0004

#define _IOMYBUF        0x0008
#define _IOEOF          0x0010
#define _IOERR          0x0020
#define _IOSTRG         0x0040
#define _IORW           0x0080

#define _IOAPPEND       0x0200

#define _IOYOURBUF      0x0100
#define _IOSETVBUF      0x0400
#define _IOFEOF         0x0800
#define _IOFLRTN        0x1000
#define _IOCTRLZ        0x2000
#define _IOCOMMIT       0x4000

#define _IOFREE         0x10000

// File modes (fmode)

#define _O_TEXT         0x4000  // File mode is text (translated)
#define _O_BINARY       0x8000  // File mode is binary (untranslated)

// Math exceptions

#define _DOMAIN     1   // Argument domain error
#define _SING       2   // Argument singularity
#define _OVERFLOW   3   // Overflow range error
#define _UNDERFLOW  4   // Underflow range error
#define _TLOSS      5   // Total loss of precision
#define _PLOSS      6   // Partial loss of precision

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#ifndef _FPOS_T_DEFINED
#define _FPOS_T_DEFINED
typedef long fpos_t;
#endif

#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#endif

typedef unsigned short wint_t;
typedef unsigned short wctype_t;

struct _exception 
{
  int type;       // Exception type - see below
  char *name;     // Name of function where error occured
  double arg1;    // First argument to function
  double arg2;    // Second argument (if any) to function
  double retval;  // Value to be returned by function
};

typedef struct
{
  int newmode;
} _startupinfo;

struct _stat
{
  unsigned int st_dev;
  unsigned short st_ino;
  unsigned short st_mode;
  short st_nlink;
  short st_uid;
  short st_gid;
  unsigned int st_rdev;
  long st_size;
  time_t st_atime;
  time_t st_mtime;
  time_t st_ctime;
};

struct _stati64 
{
  unsigned int st_dev;
  unsigned short st_ino;
  unsigned short st_mode;
  short st_nlink;
  short st_uid;
  short st_gid;
  unsigned int st_rdev;
  __int64 st_size;
  time_t st_atime;
  time_t st_mtime;
  time_t st_ctime;
};

struct _iobuf 
{
  char *ptr;
  int cnt;
  char *base;
  int flag;
  handle_t file;
  int charbuf;
  int bufsiz;
  char *tmpfname;
};

typedef struct _iobuf FILE;

typedef void (__cdecl *_PVFV)(void);
typedef int (__cdecl * _onexit_t)(void);

#define EOF     (-1)

void *memset(void *p, int c, size_t n);
void *memcpy(void *, const void *, size_t);
char *strcpy(char *, const char *);
size_t strlen(const char *);
int strcmp(const char *src, const char *dst);

#define crtapi __declspec(dllexport)

crtapi void *bsearch(const void *key, const void *base, size_t num, size_t width, int (__cdecl *compare)(const void *, const void *));
crtapi void qsort(void *base, unsigned num, unsigned width, int (__cdecl *comp)(const void *, const void *));

crtapi void *memmove(void *dst, const void *src, size_t count);

crtapi char *_strdup(const char *string);

crtapi int _isctype(int c, int mask);
crtapi int isspace(int c);
crtapi int isupper(int c);
crtapi int islower(int c);
crtapi int isdigit(int c);
crtapi int isxdigit(int c);
crtapi int ispunct(int c);
crtapi int isalpha(int c);
crtapi int isalnum(int c);
crtapi int isprint(int c);
crtapi int isgraph(int c);
crtapi int iscntrl(int c);

crtapi long atol(const char *nptr);
crtapi int atoi(const char *nptr);
crtapi double atof(const char *nptr);

crtapi int toupper(int c);
crtapi int tolower(int c);

crtapi void *_malloc(size_t size);
crtapi void _free(void *mem);
crtapi void *_calloc(size_t num, size_t size);
crtapi void *_realloc(void *mem, size_t size);

crtapi int _pipe(int *phandles, unsigned int psize, int textmode);
crtapi int _dup2(int handle1, int handle2);
crtapi int _open(const char *filename, int oflag);
crtapi int _close(int handle);
crtapi int _commit(int handle);
crtapi int _read(int handle, void *buffer, unsigned int count);
crtapi int _write(int handle, const void *buffer, unsigned int count);
crtapi int _setmode(int handle, int mode);
crtapi int _stat(const char *path, struct _stat *buffer);
crtapi __int64 _stati64(const char *path, struct _stati64 *buffer);
crtapi int _fstat(int handle, struct _stat *buffer);
crtapi __int64 _fstati64(int handle, struct _stati64 *buffer);
crtapi int _open_osfhandle(long osfhandle, int flags);
crtapi long _get_osfhandle(int filehandle);
crtapi __int64 _lseeki64(int handle, __int64 offset, int origin);

crtapi int _getdrive();
crtapi char *_getdcwd(int drive, char *buffer, int maxlen);
crtapi char *_fullpath(char *abspath, const char *relpath, size_t maxlen);
crtapi int _unlink(const char *filename);
crtapi int remove(const char *path);
crtapi int _rename(const char *oldname, const char *newname);
crtapi int _access(const char *path, int mode);
crtapi int _chmod(const char *filename, int pmode);
crtapi int _mkdir(const char *dirname);
crtapi int _chdir(const char *dirname);
crtapi char *_getcwd(char *buffer, int maxlen);

crtapi FILE *_fdopen(int handle, const char *mode);
crtapi FILE *fopen(const char *filename, const char *mode);
crtapi FILE *freopen(const char *path, const char *mode, FILE *stream);
crtapi int fclose(FILE *stream);
crtapi int fflush(FILE *stream);
crtapi size_t fread(void *buffer, size_t size, size_t num, FILE *stream);
crtapi size_t fwrite(const void *buffer, size_t size, size_t num, FILE *stream);
crtapi int fputs(const char *string, FILE *stream);
crtapi int fseek(FILE *stream, long offset, int whence);
crtapi int fgetpos(FILE *stream, fpos_t *pos);
crtapi void clearerr(FILE *stream);
crtapi int getc(FILE *stream);
crtapi int fputc(int c, FILE *stream);
crtapi int fgetc(FILE *stream);
crtapi char *fgets(char *string, int n, FILE *stream);
crtapi int fprintf(FILE *stream, const char *fmt, ...);
crtapi int vfprintf(FILE *stream, const char *fmt, va_list args);
crtapi int putchar(int c);
crtapi int _fileno(FILE *stream);
crtapi void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);

crtapi char ***__p___initenv();
crtapi int *__p__commode();
crtapi int *__p__fmode();
crtapi void __set_app_type(int type);
crtapi void __setusermatherr(int (*errhandler)(struct _exception *));
crtapi int _XcptFilter(unsigned long xcptnum, void *pxcptinfoptrs);
crtapi void _cexit();
crtapi void _c_exit();
crtapi void _amsg_exit(int rterrnum);
crtapi void __getmainargs(int *pargc, char ***pargv, char ***penvp, int dowildcard, _startupinfo *startinfo);

crtapi void _initterm(_PVFV *begin, _PVFV *end);
crtapi _onexit_t __dllonexit(_onexit_t func, _PVFV **pbegin, _PVFV **pend);
crtapi _onexit_t _cdecl _onexit(_onexit_t func);

crtapi int sprintf(char *buf, const char *fmt, ...);
crtapi int printf(const char *fmt, ...);
crtapi int vsprintf(char *buffer, const char *fmt, va_list args);
crtapi int _vsnprintf(char *buffer, size_t size, const char *fmt, va_list args);
crtapi int _snprintf(char *buffer, size_t size, const char *fmt, ...);
crtapi int sscanf(const char *buffer, const char *fmt, ...);

crtapi char *getenv(const char *option);

crtapi int _isnan(double x);
crtapi double _copysign(double a, double b);
crtapi int _finite(double x);
crtapi void _CIfmod();
crtapi unsigned int _control87(unsigned int newflags, unsigned int mask);
crtapi unsigned int _controlfp(unsigned int newflags, unsigned int mask);

crtapi unsigned long _beginthreadex(void *security, unsigned stack_size, unsigned (__stdcall *start_address)(void * ), void *arglist, unsigned initflag, unsigned *thrdaddr);
crtapi void _endthreadex(unsigned retval);
crtapi void abort();
crtapi void _exit(int status);
crtapi void _assert(void *expr, void *filename, unsigned lineno);
crtapi int _getpid();
crtapi int crt_raise(int sig);
crtapi void (*crt_signal(int sig, void (*func)(int)))(int);
crtapi void srand(unsigned int seed);
crtapi int rand();

crtapi time_t _time(time_t *timer);

crtapi int *__errno();

#endif
