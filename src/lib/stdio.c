//
// stdio.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Standard I/O routines
//

#include <os.h>

#include <stdio.h>
#include <string.h>

int vfprintf(handle_t f, const char *fmt, va_list args)
{
  char buffer[1024];
  char *p;
  char *q;
  int n;

  n = vsprintf(buffer, fmt, args);

  p = buffer;
  while (*p)
  {
    q = p;
    while (*q && *q != '\n') q++;

    if (p != q) write(f, p, q - p);
    if (*q == '\n') 
    {
      write(f, "\r\n", 2);
      q++;
    }
    p = q;
  }

  return n;
}

int fprintf(handle_t f, const char *fmt,...)
{
  va_list args;
  int n;

  va_start(args, fmt);
  n = vfprintf(f, fmt, args);
  va_end(args);

  return n;
}

int vprintf(const char *fmt, va_list args)
{
  return vfprintf(stdout, fmt, args);
}

int printf(const char *fmt,...)
{
  va_list args;
  int n;

  va_start(args, fmt);
  n = vprintf(fmt, args);
  va_end(args);

  return n;
}

char *gets(char *buf)
{
  char *p = buf;
  char ch;

  while (1)
  {
    if (read(stdin, &ch, 1) == 1)
    {
      if (ch == 8)
      {
	if (p > buf)
	{
	  write(stdout, "\b \b", 3);
	  p--;
	}
      }
      else if (ch == '\r' || ch =='\n' || ch >= ' ')
      {
	write(stdout, &ch, 1);
        if (ch == '\n') break;
	if (ch != '\r') *p++ = ch;
      }
    }
  }

  *p = 0;
  return buf;
}

