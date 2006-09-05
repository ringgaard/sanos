//
// getopt.c
//
// Parse command options
//
// Copyright (C) 2005 Michael Ringgaard. All rights reserved.
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
#include <stdio.h>
#include <string.h>
#include <crtbase.h>

#define ERR(s, c) { if (opt->err) fprintf(stderr, "%s%s%c\n", argv[0], s, c); }

static struct opt *getoptvars()
{
  struct process *proc = gettib()->proc;
  struct crtbase *crtbase = (struct crtbase *) proc->crtbase;

  return &crtbase->opt;
}

int *_opterr()
{
  return &getoptvars()->err;
}

int *_optind()
{
  return &getoptvars()->ind;
}

int *_optopt()
{
  return &getoptvars()->opt;
}

char **_optarg()
{
  return &getoptvars()->arg;
}

int getopt(int argc, char **argv, char *opts)
{
  int c;
  char *cp;
  struct opt *opt = getoptvars();

  if (opt->sp == 1)
  {
    if (opt->ind >= argc || argv[opt->ind][0] != '-' || argv[opt->ind][1] == '\0')
      return -1;
    else if (strcmp(argv[opt->ind], "--") == 0)
    {
      opt->ind++;
      return -1;
    }
  }

  opt->opt = c = argv[opt->ind][opt->sp];
  if (c == ':' || (cp = strchr(opts, c)) == NULL) 
  {
    ERR(": illegal option -- ", c);
    if (argv[opt->ind][++(opt->sp)] == '\0') 
    {
      opt->ind++;
      opt->sp = 1;
    }

    return '?';
  }

  if (*++cp == ':') 
  {
    if (argv[opt->ind][opt->sp + 1] != '\0')
      opt->arg = &argv[opt->ind++][opt->sp + 1];
    else if (++(opt->ind) >= argc) 
    {
      ERR(": option requires an argument -- ", c);
      opt->sp = 1;
      return '?';
    } 
    else
      opt->arg = argv[opt->ind++];

    opt->sp = 1;
  } 
  else 
  {
    if (argv[opt->ind][++(opt->sp)] == '\0') 
    {
      opt->sp = 1;
      opt->ind++;
    }

    opt->arg = NULL;
  }

  return c;
}
