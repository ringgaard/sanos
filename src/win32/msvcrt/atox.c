//
// atox.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// String to number conversion
//

#include "msvcrt.h"

long atol(const char *nptr)
{
  int c;
  long total;
  int sign;

  TRACE("atol");

  while (isspace((int)(unsigned char) *nptr)) ++nptr;

  c = (int)(unsigned char) *nptr++;
  sign = c;
  if (c == '-' || c == '+') c = (int)(unsigned char) *nptr++;

  total = 0;
  while (isdigit(c)) 
  {
    total = 10 * total + (c - '0');
    c = (int)(unsigned char) *nptr++;
  }

  if (sign == '-')
    return -total;
  else
    return total;
}

int atoi(const char *nptr)
{
  TRACE("atoi");
  return (int) atol(nptr);
}

double atof(const char *nptr)
{
  TRACE("atof");
  panic("atof not implemented");
  return 0;
}

