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

struct critsect job_lock;
int nextjobid = 1;

void init_threads(hmodule_t hmod, struct term *initterm)
{
  struct tib *tib = gettib();
  struct job *job;

  mkcs(&job_lock);

  job = malloc(sizeof(struct job));
  if (!job) panic("unable to allocate initial job");
  memset(job, 0, sizeof(struct job));

  job->id = nextjobid++;
  job->threadcnt = 1;
  job->hndl = dup(self());
  job->parent = job;
  job->in = 0;
  job->out = 1;
  job->err = 2;
  job->term = initterm;
  job->hmod = hmod;
  job->cmdline = NULL;
  job->env = initenv(find_section(osconfig, "env"));
  job->facility = LOG_DAEMON;
  job->ident = strdup("os");

  tib->job = job;
  peb->firstjob = peb->lastjob = job;
}

handle_t mkthread(void (__stdcall *startaddr)(struct tib *), unsigned long stacksize, struct tib **ptib)
{
  return syscall(SYSCALL_MKTHREAD, &startaddr);
}

void __stdcall threadstart(struct tib *tib)
{
  // Set usermode segment selectors
  __asm
  {
    mov	ax, SEL_UDATA + SEL_RPL3
    mov	ds, ax
    mov	es, ax
  }

  // Call thread routine
  if (tib->flags & CREATE_POSIX)
  {
    endthread((int) ((void *(*)(void *)) (tib->startaddr))(tib->startarg));
  }
  else
  {
    ((void (__stdcall *)(void *)) (tib->startaddr))(tib->startarg);
    endthread(0);
  }
}

static struct job *mkjob(struct job *parent, int hndl, int detached, int noenv)
{
  struct job *job;

  job = malloc(sizeof(struct job));
  if (!job) return NULL;
  memset(job, 0, sizeof(struct job));

  if (detached)
  {
    job->in = job->out = job->err = NOHANDLE;
  }
  else
  {
    job->in = dup(parent->in);
    job->out = dup(parent->out);
    job->err = dup(parent->err);
    job->term = parent->term;
  }

  job->hndl = dup(hndl);
  job->facility = LOG_USER;
  if (!noenv) job->env = copyenv(parent->env);

  enter(&job_lock);
  job->id = nextjobid++;
  job->parent = parent;
  job->prevjob = peb->lastjob;
  if (!peb->firstjob) peb->firstjob = job;
  if (peb->lastjob) peb->lastjob->nextjob = job;
  peb->lastjob = job;
  leave(&job_lock);

  return job;
}

handle_t beginthread(void (__stdcall *startaddr)(void *), unsigned int stacksize, void *arg, int flags, struct tib **ptib)
{
  struct job *parent = gettib()->job;
  struct tib *tib;
  struct job *job;
  handle_t h;

  h = mkthread(threadstart, stacksize, &tib);
  if (h < 0) return h;

  tib->startaddr = (void *) startaddr;
  tib->startarg = arg;
  tib->flags = flags;

  if (flags & CREATE_NEW_JOB)
  {
    job = mkjob(parent, h, flags & CREATE_DETACHED, flags & CREATE_NO_ENV);
    if (!job) 
    {
      errno = ENOMEM;
      return -1;
    }
  }
  else
    job = parent;

  atomic_add(&job->threadcnt, 1);
  tib->job = job;
  tib->pid = job->id;

  if (!(flags & CREATE_SUSPENDED)) resume(h);

  if (ptib) *ptib = tib;
  return h;
}

handle_t self()
{
  return gettib()->hndl;
}

struct tib *getthreadblock(handle_t thread)
{
  int rc;

  rc = syscall(SYSCALL_GETTHREADBLOCK, &thread);
  if (rc < 0) return NULL;
  return (struct tib *) rc;
}

int suspend(handle_t thread)
{
  return syscall(SYSCALL_SUSPEND, &thread);
}

int resume(handle_t thread)
{
  return syscall(SYSCALL_RESUME, &thread);
}

void endjob(struct job *job)
{
  struct job *j;

  enter(&job_lock);
  if (job->nextjob) job->nextjob->prevjob = job->prevjob;
  if (job->prevjob) job->prevjob->nextjob = job->nextjob;
  if (job == peb->firstjob) peb->firstjob = job->nextjob;
  if (job == peb->lastjob) peb->lastjob = job->prevjob;
  for (j = peb->firstjob; j; j = j->nextjob) if (j->parent == job) j->parent = peb->firstjob;
  leave(&job_lock);

  if (job->in >= 0) close(job->in);
  if (job->out >= 0) close(job->out);
  if (job->err >= 0) close(job->err);

  if (job->hmod) dlclose(job->hmod);
  if (job->cmdline) free(job->cmdline);
  if (job->ident) free(job->ident);
  if (job->env) freeenv(job->env);
  if (job->heap) heap_destroy(job->heap);

  close(job->hndl);

  free(job);
}

void endthread(int retval)
{
  struct job *job;

  job = gettib()->job;
  if (atomic_add(&job->threadcnt, -1) == 0) endjob(job);

  syscall(SYSCALL_ENDTHREAD, &retval);
}

tid_t gettid()
{
  return gettib()->tid;
}

pid_t getpid()
{
  return gettib()->pid;
}

pid_t getppid()
{
  int id;

  enter(&job_lock);
  id = gettib()->job->parent->id;
  leave(&job_lock);

  return id;
}

handle_t getjobhandle(pid_t pid)
{
  handle_t h = NOHANDLE;
  struct job *j;

  enter(&job_lock);
  for (j = peb->firstjob; j; j = j->nextjob) 
  {
    if (j->id == pid)
    {
      h = dup(j->hndl);
      break;
    }
  }
  leave(&job_lock);

  if (h == NOHANDLE) errno = ESRCH;
  return h;
}

int getcontext(handle_t thread, void *context)
{
  return syscall(SYSCALL_GETCONTEXT, &thread);
}

int setcontext(handle_t thread, void *context)
{
  return syscall(SYSCALL_SETCONTEXT, &thread);
}

int getprio(handle_t thread)
{
  return syscall(SYSCALL_GETPRIO, &thread);
}

int setprio(handle_t thread, int priority)
{
  return syscall(SYSCALL_SETPRIO, &thread);
}

void msleep(int millisecs)
{
  syscall(SYSCALL_MSLEEP, &millisecs);
}

struct tib *gettib()
{
  struct tib *tib;

  __asm
  {
    mov eax, fs:[TIB_SELF_OFFSET]
    mov [tib], eax
  }

  return tib;
}

void exit(int status)
{
  struct job *job = gettib()->job;

  if (job->atexit) job->atexit(status);
  endthread(status);
}

static void __stdcall spawn_program(void *args)
{
  struct job *job = gettib()->job;
  int rc;

  rc = exec(job->hmod, job->cmdline, job->env);
  exit(rc);
}

static char *procname(const char *name)
{
  char *procname;
  char *p = (char *) name;
  char *start = p;
  char *end = NULL;

  while (*p)
  {
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

int spawn(int mode, const char *pgm, const char *cmdline, char **env, struct tib **tibptr)
{
  hmodule_t hmod;
  handle_t hthread;
  struct tib *tib;
  struct job *job;
  int flags;
  int rc;
  char pgmbuf[MAXPATH];

  if (!pgm)
  {
    char *p = (char *) cmdline;
    char *q = pgmbuf;
    int dotseen = 0;

    if (!cmdline)
    {
      errno = EINVAL;
      return -1;
    }

    while (*p != 0 && *p != ' ')
    {
      if (*p == '.') dotseen = 1;
      if (*p == PS1 || *p == PS2) dotseen = 0;
      if (q - pgmbuf == MAXPATH - 1) break;
      *q++ = *p++;
    }
    *q++ = 0;
    if (!dotseen && strlen(pgmbuf) + 5 < MAXPATH) strcat(pgmbuf, ".exe");
    pgm = pgmbuf;
  }

  hmod = dlopen(pgm, RTLD_NOSHARE);
  if (!hmod) return -1;

  flags = CREATE_SUSPENDED | CREATE_NEW_JOB;
  if (mode & P_DETACH) flags |= CREATE_DETACHED;
  if (env) flags |= CREATE_NO_ENV;

  hthread = beginthread(spawn_program, 0, NULL, flags, &tib);
  if (hthread < 0)
  {
    dlclose(hmod);
    return -1;
  }

  job = tib->job;
  job->hmod = hmod;
  job->cmdline = strdup(cmdline);
  job->ident = procname(pgm);
  if (env) job->env = copyenv(env);

  if (mode & (P_NOWAIT | P_SUSPEND))
  {
    if (tibptr) *tibptr = tib;
    if ((mode & P_SUSPEND) == 0) resume(hthread);
    return hthread;
  }
  else
  {
    rc = resume(hthread);
    if (rc >= 0)
    {
      rc = waitone(hthread, INFINITE);
      close(hthread);
    }

    return rc;
  }
}
