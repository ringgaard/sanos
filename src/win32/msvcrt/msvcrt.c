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

#include <os.h>
#include <inifile.h>

#include "msvcrt.h"

crtapi int _adjust_fdiv = 0;
crtapi int __mb_cur_max = 1;

int _app_type = _UNKNOWN_APP;
int _error_mode;

char **__initenv = NULL;
int _commode = _IOCOMMIT;

static long holdrand = 1L;

int _fltused = 0x9875;

void _initterm(_PVFV *begin, _PVFV *end)
{
  TRACE("_initterm");
  while (begin < end)
  {
    if (*begin != NULL) (**begin)();
    ++begin;
  }
}

char ***__p___initenv()
{
  TRACE("__p___initenv");
  return &__initenv;
}

int *__p__commode()
{
  TRACE("__p__commode");
  return &_commode;
}

int *__p__fmode()
{
  TRACE("__p__fmode");
  return &fmode;
}

void __set_app_type(int type)
{
  TRACE("__set_app_type");
  _app_type = type;
}

void __setusermatherr(int (*errhandler)(struct _exception *))
{
  TRACE("__setusermatherr");
  syslog(LOG_DEBUG, "warning: __setusermatherr not implemented, ignored\n");
}

int _XcptFilter(unsigned long xcptnum, void *pxcptinfoptrs)
{
  syslog(LOG_ERR, "Exception %d catched in MSVCRT\n", xcptnum);
  return 0;
}

void _cexit()
{
  TRACE("_cexit");
  syslog(LOG_DEBUG, "warning: _cexit not implemented, ignored\n");
}

void _c_exit()
{
  TRACE("_c_exit");
  syslog(LOG_DEBUG, "warning: _c_exit not implemented, ignored\n");
}

void _amsg_exit(int rterrnum)
{
  TRACE("_amsg_exit");
  syslog(LOG_DEBUG, "warning: _amsg_exit(%d) not implemented, ignored\n", rterrnum);
}

_onexit_t __dllonexit(_onexit_t func, _PVFV **pbegin, _PVFV **pend)
{
  TRACE("__dllonexit");
  syslog(LOG_DEBUG, "warning: __dllonexit not implemented, ignored\n");
  return func;
}

_onexit_t _cdecl _onexit(_onexit_t func)
{
  TRACE("_onexit");
  syslog(LOG_DEBUG, "warning: _onexit not implemented, ignored\n");
  return func;
}

static int parse_args(char *args, char **argv)
{
  char *p;
  int argc;
  char *start;
  char *end;
  char *buf;
  int delim;

  p = args;
  argc = 0;
  while (*p)
  {
    while (*p == ' ') p++;
    if (!*p) break;

    if (*p == '"' || *p == '\'')
    {
      delim = *p++;
      start = p;
      while (*p && *p != delim) p++;
      end = p;
      if (*p == delim) p++;
    }
    else
    {
      start = p;
      while (*p && *p != ' ') p++;
      end = p;
    }

    if (argv)
    {
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

static char **build_env_block()
{
  struct section *env = find_section(config, "env");
  struct property *prop;
  int num = 0;
  char **envp;

  if (env)
  {
    prop = env->properties;
    while (prop)
    {
      num++;
      prop = prop->next;
    }
  }

  envp = malloc(sizeof(char *) * (num + 1));
  if (!envp) return NULL;

  if (env)
  {
    num = 0;
    prop = env->properties;
    while (prop)
    {
      int len = strlen(prop->name);
      if (prop->value) len += strlen(prop->value) + 1;
      envp[num] = malloc(len + 1);
      if (!envp[num]) return NULL;
      strcpy(envp[num], prop->name);
      if (prop->value)
      {
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

void __getmainargs(int *pargc, char ***pargv, char ***penvp, int dowildcard, _startupinfo *startinfo)
{
  TRACE("__getmainargs");

  // TODO: argv and envp should be freed on termination
  *pargc = parse_args(gettib()->args, NULL);
  *pargv = malloc(sizeof(char *) * *pargc);
  if (*pargv) parse_args(gettib()->args, *pargv);
  *penvp = build_env_block();
}

time_t _time(time_t *timer)
{
  time_t t;

  TRACE("_time");
  t = time(timer);
  return t;
}

int printf(const char *fmt, ...)
{
  va_list args;
  int n;
  char buffer[1024];

  TRACE("printf");
  va_start(args, fmt);
  n = vsprintf(buffer, fmt, args);
  va_end(args);
  return write(1, buffer, n);
}

int _vsnprintf(char *buffer, size_t size, const char *fmt, va_list args)
{
  int n;

  TRACE("_vsnprintf");
  // TODO: check buffer length
  n = vsprintf(buffer, fmt, args);
  if (n >= (int) size) panic("vsnprintf: overflow");

  return n;
}

int _snprintf(char *buffer, size_t size, const char *fmt, ...)
{
  va_list args;
  int n;

  TRACE("_snprintf");
  va_start(args, fmt);
  n = _vsnprintf(buffer, size, fmt, args);
  va_end(args);
  return n;
}

int sscanf(const char *buffer, const char *fmt, ...)
{
  TRACE("sscanf");

  if (strcmp(fmt, "%I64d") == 0)
  {
    unsigned __int64 *np;
    va_list args;

    va_start(args, fmt);
    np = va_arg(args, unsigned __int64 *);
    *np = atoi(buffer);
    va_end(args);

    return 1;
  }

  syslog(LOG_DEBUG, "sscanf '%s' format '%s'\n", buffer, fmt);
  panic("sscanf not implemented");
  return 0;
}

char *getenv(const char *option)
{
  TRACE("getenv");
  return get_property(config, "env", (char *) option, NULL);
}

double atof(const char *nptr)
{
  TRACE("atof");
  panic("atof not implemented");
  return 0;
}

int *__errno()
{
  TRACE("_errno");
  return &(gettib()->errnum);
}

unsigned long _beginthreadex(void *security, unsigned stack_size, unsigned (__stdcall *start_address)(void * ), void *arglist, unsigned initflag, unsigned *thrdaddr)
{
  struct tib *tib;
  handle_t hthread;

  TRACE("_beginthreadex");
  hthread = beginthread(start_address, stack_size, arglist, initflag, &tib);
  if (hthread >= 0 && thrdaddr && tib) *thrdaddr = tib->tid;
  return hthread;
}

void _endthreadex(unsigned retval)
{
  TRACE("_endthreadex");
  endthread(retval);
}

void abort()
{
  TRACE("abort");
  panic("program aborted");
}

void _exit(int status)
{
  TRACE("_exit");
  exit(status);
}

int _purecall()
{
  TRACE("_purecall");
  panic("pure virtual function call attempted");
  return 0;
}

int crt_raise(int sig)
{
  TRACE("raise");
  panic("raise not implemented");
  return 0;
}

void (*crt_signal(int sig, void (*func)(int)))(int)
{
  TRACE("signal");
  syslog(LOG_DEBUG, "warning: signal not implemented, ignored\n");
  return 0;
}

void _assert(void *expr, void *filename, unsigned lineno)
{
  TRACE("_assert");
  printf("Assertion failed: %s, file %s, line %d\n", expr, filename, lineno);
  abort();
}

int _getpid()
{
  return gettib()->pid;
}

char *_strdup(const char *s)
{
  char *t;
  int len;

  if (!s) return NULL;
  len = strlen(s);
  t = (char *) malloc(len + 1);
  memcpy(t, s, len + 1);
  return t;
}

void srand(unsigned int seed)
{
  holdrand = (long) seed;
}

int rand()
{
  return (((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}

size_t wcslen(const wchar_t *s)
{
  const wchar_t *eos = s;
  while (*eos++);
  return (int) (eos - s - 1);
}

int towlower(wint_t c)
{
  if (c < 256)
    return tolower(c);
  else
    return c;
}

int towupper(wint_t c)
{
  if (c < 256)
    return toupper(c);
  else
    return c;
}

void init_fileio();

int __stdcall dllmain(handle_t hmod, int reason, void *reserved)
{
  if (reason == DLL_PROCESS_ATTACH)
  {
    init_fileio();
  }

  return 1;
}
