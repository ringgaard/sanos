#include "msvcrt.h"

int _setjmp3(void *env)
{
  //syslog(LOG_DEBUG, "setjmp called\n");
  return 0;
}

void longjmp(void *env, int value)
{
  panic("longjmp not implemented");
}





