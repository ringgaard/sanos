//
// msvcrt.c
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

#include "msvcrt.h"

int _adjust_fdiv = 0;
int __mb_cur_max = 1;

int _app_type = _UNKNOWN_APP;
int _error_mode;

char **__initenv = NULL;
int _commode = _IOCOMMIT;

static long holdrand = 1L;

//int _fltused = 0x9875;

int _sys_nerr = 90;

#if _MSC_VER >= 1400
#pragma function(wcslen)
#pragma function(wcscpy)
#pragma function(wcscat)
#pragma function(wcscmp)
#endif

void _initterm(_PVFV *begin, _PVFV *end) {
  TRACE("_initterm");
  while (begin < end)
  {
    if (*begin != NULL) (**begin)();
    ++begin;
  }
}

char ***__p___initenv() {
  TRACE("__p___initenv");
  return &__initenv;
}

int *__p__commode() {
  TRACE("__p__commode");
  return &_commode;
}

int *__p__fmode() {
  TRACE("__p__fmode");
  return &fmode;
}

void __set_app_type(int type) {
  TRACE("__set_app_type");
  _app_type = type;
}

void __setusermatherr(int (*errhandler)(struct _exception *)) {
  TRACE("__setusermatherr");
  syslog(LOG_DEBUG, "warning: __setusermatherr not implemented, ignored");
}

int _XcptFilter(unsigned long xcptnum, void *pxcptinfoptrs) {
  syslog(LOG_ERR, "Exception %d catched in MSVCRT", xcptnum);
  return 0;
}

void _cexit() {
  TRACE("_cexit");
  syslog(LOG_DEBUG, "warning: _cexit not implemented, ignored");
}

void _c_exit() {
  TRACE("_c_exit");
  syslog(LOG_DEBUG, "warning: _c_exit not implemented, ignored");
}

void _amsg_exit(int rterrnum) {
  TRACE("_amsg_exit");
  syslog(LOG_DEBUG, "warning: _amsg_exit(%d) not implemented, ignored", rterrnum);
}

_onexit_t __dllonexit(_onexit_t func, _PVFV **pbegin, _PVFV **pend) {
  TRACE("__dllonexit");
  //syslog(LOG_DEBUG, "warning: __dllonexit not implemented, ignored");
  return func;
}

_onexit_t __cdecl _onexit(_onexit_t func) {
  TRACE("_onexit");
  syslog(LOG_DEBUG, "warning: _onexit not implemented, ignored");
  return func;
}

static int parse_args(char *args, char **argv) {
  char *p;
  int argc;
  char *start;
  char *end;
  char *buf;
  int delim;

  p = args;
  argc = 0;
  while (*p) {
    while (*p == ' ') p++;
    if (!*p) break;

    if (*p == '"' || *p == '\'') {
      delim = *p++;
      start = p;
      while (*p && *p != delim) p++;
      end = p;
      if (*p == delim) p++;
    } else {
      start = p;
      while (*p && *p != ' ') p++;
      end = p;
    }

    if (argv) {
      buf = (char *) malloc(end - start + 1);
      if (!buf) break;
      memcpy(buf, start, end - start);
      buf[end - start] = 0;
      argv[argc] = buf;
    }
    
    argc++;
  }

  return argc;
}

static char **build_env_block() {
  struct section *env = find_section(osconfig(), "env");
  struct property *prop;
  int num = 0;
  char **envp;

  if (env) {
    prop = env->properties;
    while (prop) {
      num++;
      prop = prop->next;
    }
  }

  envp = malloc(sizeof(char *) * (num + 1));
  if (!envp) return NULL;

  if (env) {
    num = 0;
    prop = env->properties;
    while (prop) {
      int len = strlen(prop->name);
      if (prop->value) len += strlen(prop->value) + 1;
      envp[num] = malloc(len + 1);
      if (!envp[num]) return NULL;
      strcpy(envp[num], prop->name);
      if (prop->value) {
        strcpy(envp[num] + strlen(envp[num]), "=");
        strcpy(envp[num] + strlen(envp[num]), prop->value);
      }
      num++;
      prop = prop->next;
    }
  }

  envp[num] = NULL;

  return envp;
}

void __getmainargs(int *pargc, char ***pargv, char ***penvp, int dowildcard, _startupinfo *startinfo) {
  TRACE("__getmainargs");

  // TODO: argv and envp should be freed on termination
  *pargc = parse_args(gettib()->proc->cmdline, NULL);
  *pargv = malloc(sizeof(char *) * *pargc);
  if (*pargv) parse_args(gettib()->proc->cmdline, *pargv);
  *penvp = build_env_block();
}

time_t _time(time_t *timer) {
  time_t t;

  TRACE("_time");
  t = time(timer);
  return t;
}

int printf(const char *fmt, ...) {
  va_list args;
  int n;
  char buffer[1024];

  TRACE("printf");
  va_start(args, fmt);
  n = vsprintf(buffer, fmt, args);
  va_end(args);
  return write(1, buffer, n);
}

int _vsnprintf(char *buffer, size_t size, const char *fmt, va_list args) {
  int n;

  TRACE("_vsnprintf");
  // TODO: check buffer length
  n = vsprintf(buffer, fmt, args);
  if (n >= (int) size) panic("vsnprintf: overflow");

  return n;
}

int _snprintf(char *buffer, size_t size, const char *fmt, ...) {
  va_list args;
  int n;

  TRACE("_snprintf");
  va_start(args, fmt);
  n = _vsnprintf(buffer, size, fmt, args);
  va_end(args);
  return n;
}

int sscanf(const char *buffer, const char *fmt, ...) {
  TRACE("sscanf");

  if (strcmp(fmt, "%I64d") == 0) {
    unsigned __int64 *np;
    va_list args;

    va_start(args, fmt);
    np = va_arg(args, unsigned __int64 *);
    *np = atoi(buffer);
    va_end(args);

    return 1;
  }

  syslog(LOG_DEBUG, "sscanf '%s' format '%s'", buffer, fmt);
  panic("sscanf not implemented");
  return 0;
}

char *_getenv(const char *option) {
  TRACE("getenv");
  return getenv(option);
}

int *__errno() {
  TRACE("_errno");
  return &(gettib()->errnum);
}

char *_strerror(int errnum)
{
  return strerror(errnum);
}

unsigned long _beginthreadex(void *security, unsigned stack_size, unsigned (__stdcall *start_address)(void * ), void *arglist, unsigned initflag, unsigned *thrdaddr) {
  struct tib *tib;
  handle_t hthread;

  TRACE("_beginthreadex");
  hthread = beginthread((void (__stdcall *)(void *)) start_address, stack_size, arglist, initflag, NULL, &tib);
  if (hthread >= 0 && thrdaddr && tib) *thrdaddr = tib->tid;
  return hthread;
}

void _endthreadex(unsigned retval) {
  TRACE("_endthreadex");
  endthread(retval);
}

void abort() {
  TRACE("abort");
  raise(SIGABRT);
}

void _exit(int status) {
  TRACE("_exit");
  exit(status);
}

int crt_raise(int sig) {
  TRACE("raise");

  raise(sig);
  return 0;
}

void (*crt_signal(int sig, void (*func)(int)))(int) {
  TRACE("signal");

  return signal(sig, func);
}

void _assert(void *expr, void *filename, unsigned lineno) {
  TRACE("_assert");
  printf("Assertion failed: %s, file %s, line %d\n", expr, filename, lineno);
  abort();
}

int _getpid() {
  return getpid();
}

void _ftime(struct timeb *timeptr) {
  struct timeval tv;

  gettimeofday(&tv, NULL);
  timeptr->dstflag = 0;
  timeptr->timezone = 0;
  timeptr->time = tv.tv_sec;
  timeptr->millitm = (unsigned short) (tv.tv_usec / 1000);
}

char *_strdup(const char *s) {
  char *t;
  int len;

  if (!s) return NULL;
  len = strlen(s);
  t = (char *) malloc(len + 1);
  memcpy(t, s, len + 1);
  return t;
}

void srand(unsigned int seed) {
  holdrand = (long) seed;
}

int rand() {
  return (((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}

int convert_filename_to_unicode(const char *src, wchar_t *dst, int maxlen) {
  wchar_t *end = dst + maxlen;
  while (*src) {
    if (dst == end)  {
      errno = ENAMETOOLONG;
      return -1;
    }

    *dst++ = (unsigned char) *src++;
  }
  
  if (dst == end) {
    errno = ENAMETOOLONG;
    return -1;
  }

  *dst = 0;
  return 0;
}

int convert_filename_from_unicode(const wchar_t *src, char *dst, int maxlen) {
  char *end = dst + maxlen;
  while (*src) {
    if (dst == end) {
      errno = ENAMETOOLONG;
      return -1;
    }

    if (*src & 0xFF00) {
      errno = EINVAL;
      return -1;
    }

    *dst++ = (unsigned char) *src++;
  }
  
  if (dst == end) {
    errno = ENAMETOOLONG;
    return -1;
  }

  *dst = 0;
  return 0;
}

size_t wcslen(const wchar_t *s) {
  const wchar_t *eos = s;
  while (*eos++);
  return (int) (eos - s - 1);
}

// The MSVC8 compiler complains about the definitions for the
// instrinsic functions wcscpy and wcscat. For now we just 
// ignore the warning.

#pragma warning(disable: 4142) // warning C4142: benign redefinition of type

wchar_t *wcscpy(wchar_t *dst, const wchar_t *src) {
  wchar_t *cp = dst;
  while (*cp++ = *src++);
  return dst;
}

wchar_t *wcscat(wchar_t *dst, const wchar_t *src) {
  wchar_t *cp = dst;
  while (*cp) cp++;
  while (*cp++ = *src++);
  return dst;
}

int wcscmp(const wchar_t *s1, const wchar_t *s2) {
  int ret = 0;
  while (!(ret = *(unsigned short *) s1 - *(unsigned short *) s2) && *s2) ++s1, ++s2;

  if (ret < 0) {
    ret = -1;
  } else if (ret > 0) {
    ret = 1;
  }

  return ret;
}

int _wcsicmp(const wchar_t *s1, const wchar_t *s2) {
  wchar_t f, l;

  do {
    f = ((*s1 <= 'Z') && (*s1 >= 'A')) ? *s1 + 'a' - 'A' : *s1;
    l = ((*s2 <= 'Z') && (*s2 >= 'A')) ? *s2 + 'a' - 'A' : *s2;
    s1++;
    s2++;
  } while ((f) && (f == l));

  return (int) (f - l);
}

wchar_t *wcschr(const wchar_t *s, wchar_t ch) {
  while (*s && *s != (wchar_t) ch) s++;
  if (*s == (wchar_t) ch) return (wchar_t *) s;
  return NULL;
}

int towlower(wint_t c) {
  if (c < 256) {
    return tolower(c);
  } else {
    return c;
  }
}

int towupper(wint_t c) {
  if (c < 256) {
    return toupper(c);
  } else {
    return c;
  }
}

int iswctype(wint_t c, int mask) {
  if (c < 256) {
    return 0;
  } else {
    return _isctype(c, mask);
  }
}

void init_fileio();

int __stdcall dllmain(handle_t hmod, int reason, void *reserved) {
  if (reason == DLL_PROCESS_ATTACH) {
    init_fileio();
  }

  return 1;
}
