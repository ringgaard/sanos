#ifndef MSVCRT_H
#define MSVCRT_H

#include <os.h>

#define NULL        ((void *)0)

#define ULONG_MAX   0xffffffffUL
#define LONG_MIN    (-2147483647L - 1)
#define LONG_MAX    2147483647L

// Error Codes

#define EPERM           1
#define ENOENT          2
#define ESRCH           3
#define EINTR           4
#define EIO             5
#define ENXIO           6
#define E2BIG           7
#define ENOEXEC         8
#define EBADF           9
#define ECHILD          10
#define EAGAIN          11
#define ENOMEM          12
#define EACCES          13
#define EFAULT          14
#define EBUSY           16
#define EEXIST          17
#define EXDEV           18
#define ENODEV          19
#define ENOTDIR         20
#define EISDIR          21
#define EINVAL          22
#define ENFILE          23
#define EMFILE          24
#define ENOTTY          25
#define EFBIG           27
#define ENOSPC          28
#define ESPIPE          29
#define EROFS           30
#define EMLINK          31
#define EPIPE           32
#define EDOM            33
#define ERANGE          34
#define EDEADLK         36
#define ENAMETOOLONG    38
#define ENOLCK          39
#define ENOSYS          40
#define ENOTEMPTY       41
#define EILSEQ          42

// Character types

#define _UPPER          0x1     // upper case letter
#define _LOWER          0x2     // lower case letter
#define _DIGIT          0x4     // digit[0-9]
#define _SPACE          0x8     // tab, carriage return, newline, vertical tab or form feed
#define _PUNCT          0x10    // punctuation character
#define _CONTROL        0x20    // control character
#define _BLANK          0x40    // space char
#define _HEX            0x80    // hexadecimal digit

#define _LEADBYTE       0x8000                      // multibyte leadbyte
#define _ALPHA          (0x0100 | _UPPER| _LOWER)  // alphabetic character

// File attributes

#define S_IFMT         0170000         // file type mask
#define S_IFDIR        0040000         // directory
#define S_IFCHR        0020000         // character special
#define S_IFIFO        0010000         // pipe
#define S_IFREG        0100000         // regular
#define S_IREAD        0000400         // read permission, owner
#define S_IWRITE       0000200         // write permission, owner
#define S_IEXEC        0000100         // execute/search permission, owner

typedef unsigned int size_t;

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

#define crtapi __declspec(dllexport)

crtapi void *bsearch(const void *key, const void *base, size_t num, size_t width, int (__cdecl *compare)(const void *, const void *));
crtapi void qsort(void *base, unsigned num, unsigned width, int (__cdecl *comp)(const void *, const void *));

crtapi void *memmove(void *dst, const void *src, size_t count);

crtapi char *strchr(const char *string, int ch);
crtapi int strncmp(const char *first, const char *last, size_t count);
crtapi char *strncpy(char *dest, const char *source, size_t count);
crtapi char *strrchr(const char *string, int ch);
crtapi char *strstr(const char * str1, const char *str2);
crtapi char *_strdup(const char *string);

crtapi long strtol(const char *nptr, char **endptr, int ibase);
crtapi unsigned long strtoul(const char *nptr, char **endptr, int ibase);

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

crtapi int _open(const char *filename, int oflag);
crtapi int _close(int handle);
crtapi int _read(int handle, void *buffer, unsigned int count);
crtapi int _write(int handle, const void *buffer, unsigned int count);
crtapi int _setmode(int handle, int mode);
crtapi int _stat(const char *path, struct _stat *buffer);
crtapi __int64 _stati64(const char *path, struct _stati64 *buffer);
crtapi __int64 _fstati64(int handle, struct _stati64 *buffer);
crtapi int _open_osfhandle(long osfhandle, int flags);
crtapi long _get_osfhandle(int filehandle);
crtapi __int64 _lseeki64(int handle, __int64 offset, int origin);

crtapi int _getdrive();
crtapi char *_getdcwd(int drive, char *buffer, int maxlen);
crtapi char *_fullpath(char *abspath, const char *relpath, size_t maxlen);
crtapi int _rename(const char *oldname, const char *newname);
crtapi int _access(const char *path, int mode);
crtapi int _mkdir(const char *dirname);

crtapi FILE *fopen(const char *filename, const char *mode);
crtapi int fclose(FILE *stream);
crtapi int fflush(FILE *stream);
crtapi int getc(FILE *stream);
crtapi int fputc(int c, FILE *stream);
crtapi char *fgets(char *string, int n, FILE *stream);
crtapi int fprintf(FILE *stream, const char *fmt, ...);
crtapi int vfprintf(FILE *stream, const char *fmt, va_list args);
crtapi int putchar(int c);

crtapi void _initterm(_PVFV *begin, _PVFV *end);
crtapi _onexit_t __dllonexit(_onexit_t func, _PVFV **pbegin, _PVFV **pend);
crtapi _onexit_t _cdecl _onexit(_onexit_t func);

crtapi int sprintf(char *buf, const char *fmt, ...);
crtapi int printf(const char *fmt, ...);
crtapi int vsprintf(char *buffer, const char *fmt, va_list args);
crtapi int _vsnprintf(char *buffer, size_t size, const char *fmt, va_list args);
crtapi int sscanf(const char *buffer, const char *fmt, ...);

crtapi char *getenv(const char *option);

crtapi void _ftol();
crtapi void _isnan();
crtapi double _copysign(double a, double b);
crtapi void _finite();
crtapi void _CIfmod();
crtapi unsigned int _control87(unsigned int newflags, unsigned int mask);

crtapi unsigned long _beginthreadex(void *security, unsigned stack_size, unsigned (__stdcall *start_address)(void * ), void *arglist, unsigned initflag, unsigned *thrdaddr);
crtapi void _endthreadex(unsigned retval);
crtapi void abort();
crtapi void exit(int status);
crtapi void _purecall();
crtapi void _assert(void *expr, void *filename, unsigned lineno);
crtapi int raise(int sig);
crtapi void (*signal(int sig, void (*func)(int)))(int);
crtapi void _except_handler3();

crtapi time_t time(time_t *timer);

crtapi int *_errno();
crtapi char *strerror(int errnum);

crtapi int _setjmp3(void *env);
crtapi void longjmp(void *env, int value);

#endif
