//
// stdlib.c
//
// Standard library functions
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inifile.h>

static long holdrand = 1L;

#pragma function(abs)
#pragma function(labs)

int parse_args(char *args, char **argv)
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

void free_args(int argc, char **argv)
{
  int i;

  for (i = 0; i < argc; i++) free(argv[i]);
  if (argv) free(argv);
}

void abort()
{
  raise(SIGABRT);
}

int abs(int number)
{
  return number >= 0 ? number : -number;
}

div_t div(int numer, int denom)
{
  div_t result;

  result.quot = numer / denom;
  result.rem  = numer - (result.quot * denom);

  if (numer < 0 && result.rem > 0) 
  {
    result.quot++;
    result.rem -= denom;
  }

  return result;
}

char *getenv(const char *option)
{
  return get_property(osconfig, "env", (char *) option, NULL);
}

long labs(long n)
{
  return  n >= 0L ? n : -n;
}

ldiv_t ldiv(long numer, long denom)
{
  ldiv_t result;

  result.quot = numer / denom;
  result.rem = numer % denom;

  if (numer < 0 && result.rem > 0) 
  {
    result.quot++;
    result.rem -= denom;
  }

  return result;
}

void srand(unsigned int seed)
{
  holdrand = (long) seed;
}

int rand()
{
  return (((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}

int system(const char *command)
{
  char pgm[MAXPATH];
  char *p;
  char *q;
  int dotseen = 0;

  p = (char *) command;
  q = pgm;
  while (*p != 0 && *p != ' ')
  {
    if (*p == '.') dotseen = 1;
    if (*p == PS1 || *p == PS2) dotseen = 0;
    *q++ = *p++;
  }
  *q++ = 0;
  if (!dotseen) strcat(pgm, ".exe");

  return spawn(P_WAIT, pgm, command, NULL);
}
