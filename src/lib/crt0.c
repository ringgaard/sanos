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
#include <crtbase.h>
#include <atomic.h>

typedef void (__cdecl *proc_t)(void);
typedef int (__cdecl *func_t)(void);

int __instcount;

#if !defined(__GNUC__) && !defined(__TINYC__)

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

#if _MSC_FULL_VER >= 140050214
#pragma comment(linker, "/merge:.CRT=.rdata")
#else
#pragma comment(linker, "/merge:.CRT=.data")
#endif

#endif

int main(int argc, char *argv[], char *envp[]);

void run_atexit_handlers();

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

  if (atomic_increment(&__instcount) == 1)
  {
#if !defined(__GNUC__) && !defined(__TINYC__)
    // Execute C initializers
    rc = inittermi(__xi_a, __xi_z);
    if (rc != 0) return rc;

    // Execute C++ initializers
    initterm(__xc_a, __xc_z);
#endif
  }

  return 0;
}

static void termcrt(int status)
{
  struct process *proc = gettib()->proc;
  struct crtbase *crtbase = (struct crtbase *) proc->crtbase;

  // Check for vfork() exit
  if (crtbase->fork_exit) crtbase->fork_exit(status);

  // Call termination handlers when last instance exits
  if (atomic_decrement(&__instcount) == 0)
  {
    // Execute atexit handlers
    run_atexit_handlers();

#if !defined(__GNUC__) && !defined(__TINYC__)
    // Execute C pre-terminators
    initterm(__xp_a, __xp_z);

    // Execute C terminators
    initterm(__xt_a, __xt_z);
#endif
  }

  // Deallocate arguments
  free_args(crtbase->argc, crtbase->argv);
}

#ifdef __GNUC__

// Dummy __main and __alloca and routine for GCC
// TODO: implement proper constructor handling

void __main()
{
}

void _alloca()
{
}

#endif

int mainCRTStartup()
{
  int rc;
  struct tib *tib = gettib();
  struct process *proc = tib->proc;
  struct crtbase *crtbase = (struct crtbase *) proc->crtbase;

  crtbase->argc = parse_args(proc->cmdline, NULL);
  crtbase->argv = (char **) malloc(crtbase->argc * sizeof(char *));
  parse_args(proc->cmdline, crtbase->argv);
  crtbase->opt.err = 1;
  crtbase->opt.ind = 1;
  crtbase->opt.sp = 1;

  rc = initcrt();
  if (rc == 0) 
  {
    gettib()->proc->atexit = termcrt;
    rc = main(crtbase->argc, crtbase->argv, environ ? environ : (char **) &environ);
  }

  return rc;
}
