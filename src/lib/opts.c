//
// opts.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Option string parsing
//

#include <types.h>
#include <stdarg.h>
#include <string.h>

char *get_option(char *opts, char *name, char *buffer, int size, char *defval)
{
  char *eq;
  char *p;

  if (!opts) 
  {
    if (!defval) return NULL;
    strncpy(buffer, defval, size);
    return buffer;
  }

  while (*opts)
  {
    while (*opts == ' ') opts++;
    p = opts;
    while (*p != 0 && *p != ',') p++;

    eq = opts;
    while (*eq != 0 && *eq != '=') eq++;

    if (*eq == '=')
    {
      if (strncmp(name, opts, eq - opts) == 0)
      {
	if (!buffer) return eq + 1;

	if (p - (eq + 1) > size)
	  strncpy(buffer, eq + 1, size);
	else
	  strncpy(buffer, eq + 1, p - (eq + 1));

	return buffer;
      }
    }
    else
    {
      if (buffer) *buffer = 0;
      return buffer;
    }

    opts = p;
    if (*opts == ',') opts++;
  }

  if (!defval) return NULL;
  if (buffer) strncpy(buffer, defval, size);
  return buffer;
}

int get_num_option(char *opts, char *name, int defval)
{
  char buffer[32];
  char *p;
  int value;

  p = get_option(opts, name, buffer, 32, NULL);
  if (!p) return defval;

  while (*p == ' ') p++;
  
  if (p[0] == '0' && p[1] == 'x')
  {
    p += 2;
    value = 0;
    while (*p)
    {
      if (*p >= '0' && *p <= '9')
	value = (value << 4) | (*p - '0');
      else if (*p >= 'A' && *p <= 'F')
	value = (value << 4) | (*p - 'A' + 10);
      else if (*p >= 'a' && *p <= 'f')
	value = (value << 4) | (*p - 'a' + 10);
      else
	return defval;

      p++;
    }

    return value;
  }

  return atoi(p);
}
