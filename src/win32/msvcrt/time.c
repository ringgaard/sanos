//
// time.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Time routines
//

#include "msvcrt.h"

time_t _time(time_t *timer)
{
  time_t t;

  TRACE("_time");
  t = time();
  if (timer) *timer = t;
  return t;
}
