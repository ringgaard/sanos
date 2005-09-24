//
// crt0.c
//
// C runtime library entry point
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
#include <stdlib.h>
#include <atomic.h>
#include <crtbase.h>

void init_stdio();
void exit_stdio();

typedef void (__cdecl *proc_t)(void);
typedef int (__cdecl *func_t)(void);

int __instcount;

proc_t *atexit_begin = NULL;
proc_t *atexit_end = NULL;
proc_t *atexit_last = NULL;

//
// Pointers to initialization/termination functions
//
// NOTE: THE USE OF THE POINTERS DECLARED BELOW DEPENDS ON THE PROPERTIES
// OF C COMMUNAL VARIABLES. SPECIFICALLY, THEY ARE NON-NULL IFF THERE EXISTS
// A DEFINITION ELSEWHERE INITIALIZING THEM TO NON-NULL VALUES.
//

// C initializers

#pragma data_seg(".CRT$XIA")
func_t __xi_a[] = { NULL };

#pragma data_seg(".CRT$XIZ")
func_t __xi_z[] = { NULL };

// C++ initializers

#pragma data_seg(".CRT$XCA")
proc_t __xc_a[] = { NULL };

#pragma data_seg(".CRT$XCZ")
proc_t __xc_z[] = { NULL };

// C pre-terminators

#pragma data_seg(".CRT$XPA")
proc_t __xp_a[] = { NULL };

#pragma data_seg(".CRT$XPZ")
proc_t __xp_z[] = { NULL };

// C terminators

#pragma data_seg(".CRT$XTA")
proc_t __xt_a[] = { NULL };

#pragma data_seg(".CRT$XTZ")
proc_t __xt_z[] = { NULL };

#pragma data_seg()  // reset

#pragma comment(linker, "/merge:.CRT=.data")

int main(int argc, char *argv[]);

int atexit(proc_t exitfunc)
{
  if (atexit_end == atexit_last)
  {
    int size = atexit_end - atexit_begin;
    int newsize = size + 32;
    atexit_begin = (proc_t *) realloc(atexit_begin, newsize * sizeof(proc_t));
    if (atexit_begin == NULL) return -ENOMEM;
    atexit_end = atexit_begin + size;
    atexit_last = atexit_begin + newsize;
  }

  *atexit_end++ = exitfunc;
  return 0;
}

static void initterm(proc_t *begin, proc_t *end)
{
  while (begin < end)
  {
    if (*begin != NULL) (**begin)();
    ++begin;
  }
}

static int inittermi(func_t *begin, func_t *end)
{
  int rc = 0;

  while (begin < end && rc == 0)
  {
    if (*begin != NULL) rc = (**begin)();
    ++begin;
  }

  return rc;
}

static int initcrt()
{
  int rc;

  // Initialize stdio
  init_stdio();

  if (atomic_increment(&__instcount) == 1)
  {
    // Execute C initializers
    rc = inittermi(__xi_a, __xi_z);
    if (rc != 0) return rc;

    // Execute C++ initializers
    initterm(__xc_a, __xc_z);
  }

  return 0;
}

static void termcrt()
{
  struct job *job = gettib()->job;
  struct crtbase *crtbase = (struct crtbase *) job->crtbase;

  if (atomic_decrement(&__instcount) == 0)
  {
    // Execute atexit handlers
    if (atexit_begin)
    {
      while (--atexit_end >= atexit_begin) if (*atexit_end != NULL) (**atexit_end)();
      free(atexit_begin);
      atexit_begin = atexit_end = atexit_last = NULL;
    }

    // Execute C pre-terminators
    initterm(__xp_a, __xp_z);

    // Execute C terminators
    initterm(__xt_a, __xt_z);
  }

  // Flush stdout and stderr
  fflush(stdout);
  fflush(stderr);

  // Deallocate arguments
  free_args(crtbase->argc, crtbase->argv);
}

int mainCRTStartup()
{
  int rc;
  struct tib *tib = gettib();
  struct job *job = tib->job;
  struct crtbase *crtbase = (struct crtbase *) job->crtbase;

  crtbase->argc = parse_args(job->cmdline, NULL);
  crtbase->argv = (char **) malloc(crtbase->argc * sizeof(char *));
  parse_args(job->cmdline, crtbase->argv);

  rc = initcrt();
  if (rc == 0) 
  {
    gettib()->job->atexit = termcrt;
    rc = main(crtbase->argc, crtbase->argv);
  }

  return rc;
}
