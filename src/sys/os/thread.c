//
// thread.c
//
// Thread routines
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
#include <atomic.h>
#include <inifile.h>

#include <os/syscall.h>
#include <os/seg.h>
#include <os/tss.h>
#include <os/syspage.h>

#include "heap.h"

// environ.c
char **initenv(struct section *sect);
char **copyenv(char **env);
void freeenv(char **env);

struct critsect proc_lock;
int nextprocid = 1;

void init_threads(hmodule_t hmod, struct term *initterm) {
  struct tib *tib = gettib();
  struct peb *peb = getpeb();
  struct process *proc;

  mkcs(&proc_lock);

  proc = malloc(sizeof(struct process));
  if (!proc) panic("unable to allocate initial process");
  memset(proc, 0, sizeof(struct process));

  proc->id = nextprocid++;
  proc->threadcnt = 1;
  proc->hndl = dup(self());
  proc->iob[0] = 0;
  proc->iob[1] = 1;
  proc->iob[2] = 2;
  proc->term = initterm;
  proc->hmod = hmod;
  proc->cmdline = NULL;
  proc->env = initenv(find_section(osconfig(), "env"));
  proc->facility = LOG_DAEMON;
  proc->ident = strdup("os");

  tib->proc = proc;
  peb->firstproc = peb->lastproc = proc;
}

handle_t mkthread(void (__stdcall *startaddr)(struct tib *), unsigned long stacksize, char *name, struct tib **ptib) {
  return syscall(SYSCALL_MKTHREAD, &startaddr);
}

void __stdcall threadstart(struct tib *tib) {
  // Call thread routine
  if (tib->flags & CREATE_POSIX) {
    endthread((int) ((void *(*)(void *)) (tib->startaddr))(tib->startarg));
  } else {
    ((void (__stdcall *)(void *)) (tib->startaddr))(tib->startarg);
    endthread(0);
  }
}

static struct process *mkproc(struct process *parent, int hndl, int flags) {
  struct peb *peb = getpeb();
  struct process *proc;
  int i;

  proc = malloc(sizeof(struct process));
  if (!proc) return NULL;
  memset(proc, 0, sizeof(struct process));

  if (flags & CREATE_DETACHED) {
    for (i = 0; i < 3; i++) proc->iob[i] = NOHANDLE;
  } else {
    for (i = 0; i < 3; i++) proc->iob[i] = dup(parent->iob[i]);
    proc->term = parent->term;
  }

  proc->hndl = dup(hndl);
  proc->facility = LOG_USER;
  if ((flags & CREATE_NO_ENV) == 0) proc->env = copyenv(parent->env);

  enter(&proc_lock);
  proc->id = nextprocid++;
  if (flags & CREATE_CHILD) proc->parent = parent;
  proc->prevproc = peb->lastproc;
  if (!peb->firstproc) peb->firstproc = proc;
  if (peb->lastproc) peb->lastproc->nextproc = proc;
  peb->lastproc = proc;
  leave(&proc_lock);

  return proc;
}

handle_t beginthread(void (__stdcall *startaddr)(void *), unsigned int stacksize, void *arg, int flags, char *name, struct tib **ptib) {
  struct process *parent = gettib()->proc;
  struct tib *tib;
  struct process *proc;
  handle_t h;

  h = mkthread(threadstart, stacksize, name, &tib);
  if (h < 0) return h;

  tib->startaddr = (void *) startaddr;
  tib->startarg = arg;
  tib->flags = flags;

  if (flags & CREATE_NEW_PROCESS) {
    proc = mkproc(parent, h, flags);
    if (!proc) {
      errno = ENOMEM;
      return -1;
    }
  } else {
    proc = parent;
  }

  atomic_add(&proc->threadcnt, 1);
  tib->proc = proc;
  tib->pid = proc->id;

  if (!(flags & CREATE_SUSPENDED)) resume(h);

  if (ptib) *ptib = tib;
  return h;
}

handle_t self() {
  return gettib()->hndl;
}

struct tib *getthreadblock(handle_t thread) {
  int rc;

  rc = syscall(SYSCALL_GETTHREADBLOCK, &thread);
  if (rc < 0) return NULL;
  return (struct tib *) rc;
}

int suspend(handle_t thread) {
  return syscall(SYSCALL_SUSPEND, &thread);
}

int resume(handle_t thread) {
  return syscall(SYSCALL_RESUME, &thread);
}

void endproc(struct process *proc, int status) {
  struct peb *peb = getpeb();
  struct process *parent;
  struct process *p;
  struct zombie *z;
  int i;

  int ppid = -1;

  enter(&proc_lock);

  // Add zombie to parent process
  parent = proc->parent;
  if (parent) {
    z = (struct zombie *) malloc(sizeof(struct zombie));
    if (z) {
      z->pid = proc->id;
      z->status = status;
      z->next = parent->zombies;
      parent->zombies = z;

      ppid = parent->id;
    }
  }

  // Free zombies for process
  while (proc->zombies) {
    z = proc->zombies->next;
    free(proc->zombies);
    proc->zombies = z;
  }

  // Remove process from process list
  if (proc->nextproc) proc->nextproc->prevproc = proc->prevproc;
  if (proc->prevproc) proc->prevproc->nextproc = proc->nextproc;
  if (proc == peb->firstproc) peb->firstproc = proc->nextproc;
  if (proc == peb->lastproc) peb->lastproc = proc->prevproc;
  
  // Orphan child processes
  for (p = peb->firstproc; p; p = p->nextproc) if (p->parent == proc) p->parent = NULL;
  
  leave(&proc_lock);

  // Close standard handles
  for (i = 0; i < 3; i++) if (proc->iob[i] >= 0) close(proc->iob[i]);

  // Cleanup process resources
  if (proc->hmod) dlclose(proc->hmod);
  if (proc->cmdline) free(proc->cmdline);
  if (proc->ident) free(proc->ident);
  if (proc->env) freeenv(proc->env);
  if (proc->heap) heap_destroy(proc->heap);

  close(proc->hndl);

  free(proc);

  // Notify parent about child termination
  if (ppid != -1) kill(ppid, SIGCHLD);
}

void endthread(int status) {
  struct process *proc;

  proc = gettib()->proc;
  if (atomic_add(&proc->threadcnt, -1) == 0) endproc(proc, status);

  syscall(SYSCALL_ENDTHREAD, &status);
}

tid_t gettid() {
  return gettib()->tid;
}

pid_t getpid() {
  return gettib()->pid;
}

pid_t getppid() {
  struct tib *tib = gettib();
  int id;

  enter(&proc_lock);
  
  if (tib && tib->proc && tib->proc->parent) {
    id = tib->proc->parent->id;
  } else {
    id = -1;
    errno = ESRCH;
  }

  leave(&proc_lock);

  return id;
}

int getchildstat(pid_t pid, int *status) {
  struct process *proc = gettib()->proc;
  struct zombie *z;
  int rc;

  enter(&proc_lock);

  z = proc->zombies;
  if (pid == -1) {
    if (z) proc->zombies = z->next;
  } else {
    struct zombie *pz = NULL;
    while (z) {
      if (z->pid == pid) {
        if (pz) {
          pz->next = z->next;
        } else {
          proc->zombies = z->next;
        }

        break;
      }
      pz = z;
      z = z->next;
    }
  }

  if (z) {
    rc = z->pid;
    if (status) *status = z->status;
    free(z);
  } else {
    rc = -1;
  }

  leave(&proc_lock);

  return rc;
}

int setchildstat(pid_t pid, int status) {
  struct process *proc = gettib()->proc;
  struct zombie *z;

  enter(&proc_lock);
  z = (struct zombie *) malloc(sizeof(struct zombie));
  z->pid = pid;
  z->status = status;
  z->next = proc->zombies;
  proc->zombies = z;
  leave(&proc_lock);

  return 0;
}

handle_t getprochandle(pid_t pid) {
  handle_t h = NOHANDLE;
  struct process *p;

  enter(&proc_lock);
  for (p = getpeb()->firstproc; p; p = p->nextproc) {
    if (p->id == pid) {
      h = dup(p->hndl);
      break;
    }
  }
  leave(&proc_lock);

  if (h == NOHANDLE) errno = ESRCH;
  return h;
}

int getcontext(handle_t thread, void *context) {
  return syscall(SYSCALL_GETCONTEXT, &thread);
}

int setcontext(handle_t thread, void *context) {
  return syscall(SYSCALL_SETCONTEXT, &thread);
}

int getprio(handle_t thread) {
  return syscall(SYSCALL_GETPRIO, &thread);
}

int setprio(handle_t thread, int priority) {
  return syscall(SYSCALL_SETPRIO, &thread);
}

int msleep(int millisecs) {
  return syscall(SYSCALL_MSLEEP, &millisecs);
}

struct tib *gettib() {
  struct tib *tib;

  __asm {
    mov eax, fs:[TIB_SELF_OFFSET]
    mov [tib], eax
  }

  return tib;
}

void exit(int status) {
  struct process *proc = gettib()->proc;

  if (proc->atexit) proc->atexit(status, 0);
  endthread(status);
}

void _exit(int status) {
  struct process *proc = gettib()->proc;

  if (proc->atexit) proc->atexit(status, 1);
  endthread(status);
}

static void __stdcall spawn_program(void *args) {
  struct process *proc = gettib()->proc;
  int rc;

  rc = exec(proc->hmod, proc->cmdline, proc->env);
  exit(rc);
}

static char *procname(const char *name) {
  char *procname;
  char *p = (char *) name;
  char *start = p;
  char *end = NULL;

  while (*p) {
    if (*p == PS1 || *p == PS2) start = p + 1;
    if (*p == '.') end = p;
    p++;
  }
  if (!end || end < start) end = p;

  procname = malloc(end - start + 1);
  if (!procname) return NULL;
  memcpy(procname, start, end - start);
  procname[end - start] = 0;

  return procname;
}

int spawn(int mode, const char *pgm, const char *cmdline, char **env, struct tib **tibptr) {
  hmodule_t hmod;
  handle_t hthread;
  struct tib *tib;
  struct process *proc;
  int flags;
  int rc;
  char pgmbuf[MAXPATH];
  char *name;

  if (!pgm) {
    const char *p = cmdline;
    char *q = pgmbuf;

    if (!cmdline) {
      errno = EINVAL;
      return -1;
    }

    while (*p != 0 && *p != ' ') {
      if (q - pgmbuf == MAXPATH - 1) break;
      *q++ = *p++;
    }
    *q++ = 0;
    pgm = pgmbuf;
  }

  hmod = dlopen(pgm, RTLD_EXE | RTLD_SCRIPT);
  if (!hmod) return -1;

  flags = CREATE_SUSPENDED | CREATE_NEW_PROCESS;
  if (mode & P_DETACH) flags |= CREATE_DETACHED;
  if (mode & P_CHILD) flags |= CREATE_CHILD;
  if (env) flags |= CREATE_NO_ENV;
  
  name = procname(pgm);

  hthread = beginthread(spawn_program, 0, NULL, flags, name, &tib);
  if (hthread < 0) {
    free(name);
    dlclose(hmod);
    return -1;
  }

  proc = tib->proc;
  proc->hmod = hmod;
  proc->cmdline = strdup(cmdline);
  proc->ident = name;
  if (env) proc->env = copyenv(env);

  if (mode & (P_NOWAIT | P_SUSPEND)) {
    if (tibptr) *tibptr = tib;
    if ((mode & P_SUSPEND) == 0) resume(hthread);
    return hthread;
  } else {
    rc = resume(hthread);
    if (rc >= 0) {
      while (1) {
        rc = waitone(hthread, INFINITE);
        if (rc >= 0 || errno != EINTR) break;
      }
      close(hthread);
    }

    return rc;
  }
}
