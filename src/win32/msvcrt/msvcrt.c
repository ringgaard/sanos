//
// msvcrt.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// C runtime library
//

#include <os.h>
#include <inifile.h>

#include "msvcrt.h"

crtapi int _adjust_fdiv = 0;
crtapi int __mb_cur_max = 1;

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

_onexit_t __dllonexit(_onexit_t func, _PVFV **pbegin, _PVFV **pend)
{
  TRACE("__dllonexit");
  syslog(LOG_DEBUG, "warning: __dllonexit not implemented, ignored\n");
  return func;
}

_onexit_t _cdecl _onexit(_onexit_t func)
{
  TRACE("_onexit");
  panic("_onexit not implemented");
  return NULL;
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
  TRACE("_vsnprintf");
  // TODO: check buffer length
  return vsprintf(buffer, fmt, args);
}

int sscanf(const char *buffer, const char *fmt, ...)
{
  TRACE("sscanf");
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

int *_errno()
{
  TRACE("_errno");
  return &(gettib()->errnum);
}

char *strerror(int errnum)
{
  TRACE("strerror");
  return "general error";
}

unsigned long _beginthreadex(void *security, unsigned stack_size, unsigned (__stdcall *start_address)(void * ), void *arglist, unsigned initflag, unsigned *thrdaddr)
{
  TRACE("_beginthreadex");
  return beginthread(start_address, stack_size, arglist, initflag != 0, thrdaddr);
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

void _purecall()
{
  TRACE("_purecall");
  panic("pure virtual function call attempted");
}

int raise(int sig)
{
  TRACE("raise");
  panic("raise not implemented");
  return 0;
}

void (*signal(int sig, void (*func)(int)))(int)
{
  TRACE("signal");
  syslog(LOG_DEBUG, "warning: signal not implemented, ignored\n");
  return 0;
}

void _except_handler3()
{
  TRACE("_except_handler3");
  panic("_except_handler3 not implemented");
}

void _assert(void *expr, void *filename, unsigned lineno)
{
  TRACE("_assert");
  printf("Assertion failed: %s, file %s, line %d\n", expr, filename, lineno);
  abort();
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
