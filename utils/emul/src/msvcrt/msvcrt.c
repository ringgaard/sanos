#include <os.h>
#include "msvcrt.h"

crtapi int _adjust_fdiv = 0;
crtapi int __mb_cur_max = 1;

int errno = 0;
int _fltused = 0x9875;

void _initterm(_PVFV *begin, _PVFV *end)
{
  syslog(LOG_DEBUG, "_initterm\n");

  while (begin < end)
  {
    if (*begin != NULL) (**begin)();
    ++begin;
  }
}

_onexit_t __dllonexit(_onexit_t func, _PVFV **pbegin, _PVFV **pend)
{
  syslog(LOG_DEBUG, "warning: __dllonexit not implemented, ignored\n");
  return func;
}

_onexit_t _cdecl _onexit(_onexit_t func)
{
  panic("_onexit not implemented");
  return NULL;
}

int sprintf(char *buf, const char *fmt, ...)
{
  va_list args;
  int n;

  va_start(args, fmt);
  n = vformat(buf, fmt, args);
  va_end(args);

  return n;
}

int printf(const char *fmt, ...)
{
  va_list args;
  int n;
  char buffer[1024];

  va_start(args, fmt);
  n = vformat(buffer, fmt, args);
  va_end(args);
  print_string(buffer);
  return n;
}

int vsprintf(char *buffer, const char *fmt, va_list args)
{
  return vformat(buffer, fmt, args);
}

int _vsnprintf(char *buffer, size_t size, const char *fmt, va_list args)
{
  // TODO: check buffer length
  return vformat(buffer, fmt, args);
}

int sscanf(const char *buffer, const char *fmt, ...)
{
  panic("sscanf not implemented");
  return 0;
}

char *getenv(const char *option)
{
  syslog(LOG_DEBUG, "getenv(%s) requested\n", option);
  return NULL;
}

int *_errno()
{
  // TODO: implement per-thread error number variable
  return &errno;
}

char *strerror(int errnum)
{
  return "general error";
}

unsigned long _beginthreadex(void *security, unsigned stack_size, unsigned (__stdcall *start_address)(void * ), void *arglist, unsigned initflag, unsigned *thrdaddr)
{
  handle_t hthread;

  //panic("_beginthreadex called");
  hthread = begin_thread(start_address, stack_size, arglist, initflag != 0, thrdaddr);
  return (unsigned long) hthread;
}

void _endthreadex(unsigned retval)
{
  end_thread(retval);
}

void abort()
{
  panic("program aborted");
}

void exit(int status)
{
  syslog(LOG_DEBUG, "exit code %d\n", status);
  panic("program exited");
}

void _purecall()
{
  panic("pure virtual function call attempted");
}

int raise(int sig)
{
  panic("raise not implemented");
  return 0;
}

void (*signal(int sig, void (*func)(int)))(int)
{
  syslog(LOG_DEBUG, "warning: signal not implemented, ignored\n");
  return 0;
}

void _except_handler3()
{
  panic("_except_handler3 not implemented");
}

void _assert(void *expr, void *filename, unsigned lineno)
{
  printf("Assertion failed: %s, file %s, line %d\n", expr, filename, lineno);
  abort();
}

void init_fileio();

int __stdcall dllmain(handle_t hmod, int reason, void *reserved)
{
  init_fileio();
  return 1;
}
