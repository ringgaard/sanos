//
// setjmp.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Non-local goto
//

#include "msvcrt.h"

int _setjmp3(void *env)
{
  TRACE("_setjmp3");
  //syslog(LOG_DEBUG, "setjmp called\n");
  return 0;
}

void longjmp(void *env, int value)
{
  TRACE("longjmp");
  panic("longjmp not implemented");
}
