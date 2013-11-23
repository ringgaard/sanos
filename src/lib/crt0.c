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

int __instcount = 0;

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

static char *next_arg(char *p, char *arg, int *size) {
  int n = 0;
  
  // Skip whitespace
  while (*p == ' ') p++;
  if (!*p) return NULL;

  if (*p == '"') {
    // Escaped argument
    p++;
    while (*p) {
      if (*p == '"') {
        p++;
        if (*p != '"') break;
      }
      if (arg) *arg++ = *p;
      n++;
      p++;
    }
  } else {
    // Regular argument
    while (*p && *p != ' ') {
      if (arg) *arg++ = *p;
      n++;
      p++;
    }
  }
  if (size) *size = n;
  if (arg) *arg = 0;
  return p;
}

static char **parse_cmdline(char *cmdline, int *argc) {
  char *p;
  int size;
  int argsize;
  int n;
  char *arg;
  char **argv;

  // Compute number of arguments and size of argument buffer
  p = cmdline;
  n = 0;
  size = sizeof(int) + sizeof(char *);
  for (;;) {
    p = next_arg(p, NULL, &argsize);
    if (!p) break;
    size += sizeof(char *) + argsize + 1;
    n++;
  }
  *argc = n;

  // Allocate argument buffer
  arg = vmalloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE, 'ARGV');
  if (!arg) return NULL;

  // Store buffer size in first word
  *((int *) arg) = size;
  arg += sizeof(int);

  // Include null termination of argument vector
  argv = (char **) arg;
  argv[n] = NULL;
  arg += (n + 1) * sizeof(char *);

  // Build argument vector
  p = cmdline;
  n = 0;
  size = sizeof(int) + sizeof(char *);
  for (;;) {
    p = next_arg(p, arg, &argsize);
    if (!p) break;
    argv[n++] = arg;
    arg += argsize + 1;
  }

  return argv;
}

struct crtbase *init_crtbase() {
  struct process *proc = gettib()->proc;
  struct crtbase *crtbase = (struct crtbase *) proc->crtbase;

  crtbase->argv = parse_cmdline(proc->cmdline, &crtbase->argc);
  crtbase->opt.err = 1;
  crtbase->opt.ind = 1;
  crtbase->opt.sp = 1;
  crtbase->opt.place = "";
  
  return crtbase;
}

void term_crtbase(struct crtbase *crtbase) {
  int *argbuf;
  int size;

  argbuf = (int *) crtbase->argv;
  if (argbuf) {
    argbuf--;
    size = *argbuf;
    vmfree(argbuf, size, MEM_RELEASE);    
  }
}

static void initterm(proc_t *begin, proc_t *end) {
  while (begin < end) {
    if (*begin != NULL) (**begin)();
    ++begin;
  }
}

static int inittermi(func_t *begin, func_t *end) {
  int rc = 0;

  while (begin < end && rc == 0) {
    if (*begin != NULL) rc = (**begin)();
    ++begin;
  }

  return rc;
}

static int initcrt() {
  int rc;

  if (atomic_increment(&__instcount) == 1) {
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

static void termcrt(int status, int fast) {
  struct process *proc = gettib()->proc;
  struct crtbase *crtbase = (struct crtbase *) proc->crtbase;

  // Check for vfork() exit
  if (crtbase->fork_exit) crtbase->fork_exit(status);

  // Call termination handlers when last instance exits
  if (atomic_decrement(&__instcount) == 0) {
    // Execute atexit handlers
    if (!fast) run_atexit_handlers();

#if !defined(__GNUC__) && !defined(__TINYC__)
    // Execute C pre-terminators
    initterm(__xp_a, __xp_z);

    // Execute C terminators
    initterm(__xt_a, __xt_z);
#endif
  }

  // Clean runtime library
  term_crtbase(crtbase);
}

// Default entry point for DLLs
int __stdcall DllMain(hmodule_t hmod, int reason, void *reserved) {
  return TRUE;
}

// Default entry point for EXEs
int mainCRTStartup() {
  struct crtbase *crtbase = init_crtbase();
  int rc = initcrt();
  if (rc == 0) {
    gettib()->proc->atexit = termcrt;
    rc = main(crtbase->argc, crtbase->argv, environ ? environ : (char **) &environ);
  }

  return rc;
}
