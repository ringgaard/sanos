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

void printf(const char *fmt,...)
{
  va_list args;
  char buffer[1024];
  char *p;
  char *q;

  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);

  p = buffer;
  while (*p)
  {
    q = p;
    while (*q && *q != '\n') q++;

    if (p != q) write(stdout, p, q - p);
    if (*q == '\n') 
    {
      write(stdout, "\r\n", 2);
      q++;
    }
    p = q;
  }
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
	  write(stdout, &ch, 1);
	  p--;
	}
      }
      else 
      {
	write(stdout, &ch, 1);
	if (ch != 13)
	{
	  if (ch == 10) break;
	  *p++ = ch;
	}
      }
    }
  }

  *p = 0;
  return buf;
}

