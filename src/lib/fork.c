//
// fork.c
//
// Unix process creation routines
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
#include <unistd.h>
#include <atomic.h>
#include <sys/wait.h>

static int forkpid = 0;

//
// This implementation of vfork()/exec() has been inspired by Shaun 
// Jackman's <sjackman@gmail.com> implementation for BusyBox. It uses 
// setjmp()/longjmp() to emulate the behaviour of vfork()/exec().
//

static char **copyenv(char **env)
{
  int n;
  char **newenv;

  if (!env) return NULL;
  for (n = 0; env[n]; n++);
  newenv = (char **) malloc((n + 1) * sizeof(char *));
  if (newenv)
  {
    newenv[n] = NULL;
    for (n = 0; env[n]; n++) newenv[n] = strdup(env[n]);
  }

  return newenv;
}

static void freeenv(char **env)
{
  int n;
  
  if (!env) return;
  for (n = 0; env[n]; n++) free(env[n]);
  free(env);
}

static resume_fork(struct tib *tib, int pid)
{
  int i;
  struct job *job = tib->job;
  struct _forkctx *fc = (struct _forkctx *) tib->forkctx;

  // Restore standard handles
  for (i = 0; i < 3; i++)
  {
    close(job->iob[i]);
    job->iob[i] = fc->fd[i];
  }

  // Restore environment variables
  freeenv(job->env);
  job->env = fc->env;

  // Unlink fork context from chain
  tib->forkctx = fc->prev;

  // Return control to the parent process path with pid handle
  longjmp(fc->jmp, pid);
}

struct _forkctx *_vfork(struct _forkctx *fc)
{
  int i;
  struct tib *tib = gettib();
  struct job *job = tib->job;

  // Assign fork pid
  fc->pid = atomic_increment(&forkpid);

  // Save and duplicate standard handles
  for (i = 0; i < 3; i++)
  {
    fc->fd[i] = job->iob[i];
    job->iob[i] = dup(job->iob[i]);
  }

  // Save and duplicate environment variables
  fc->env = job->env;
  job->env = copyenv(job->env);

  // Link fork context into chain
  fc->prev = tib->forkctx;
  tib->forkctx = fc;

  return fc;
}

void fork_exit(int status)
{
  struct tib *tib = gettib();

  // If no vfork() is in progress just return
  struct _forkctx *fc = (struct _forkctx *) tib->forkctx;
  if (!fc) return;

  // Add a fake zombie for job
  setchildstat(0x40000000 | fc->pid, status & 0xFF);

  // Send a SIGCHLD to account for the virtual process exit
  sendsig(self(), SIGCHLD);

  // Return control to the parent process path with fork pid
  resume_fork(tib, 0x40000000 | fc->pid);
}

pid_t wait(int *stat_loc)
{
  struct job *job = gettib()->job;

  while (1)
  {
    int pid = getchildstat(-1, stat_loc);
    if (pid != -1) return pid;
    sigsuspend(NULL);
  }
}

pid_t waitpid(pid_t pid, int *stat_loc, int options)
{
  struct job *job = gettib()->job;
  int rc;
  handle_t h;

  // Use wait() if pid is -1 (TODO pid==0 meaning wait for any child in process group)
  if (pid <= 0)
  {
    if (options & WNOHANG) 
      return getchildstat(-1, stat_loc);
    else
      return wait(stat_loc);
  }

  while (1)
  {
    // Check for zombies
    rc = getchildstat(pid, stat_loc);
    if (rc != -1 || (options & WNOHANG)) return rc;

    // Get handle for job from pid
    h = getjobhandle(pid);
    if (h == NOHANDLE)
    {
      errno = ECHILD;
      return -1;
    }

    // Wait for main thread in job to terminate
    rc = waitone(h, INFINITE);
    close(h);
  }
}
 
int execve(const char *path, char *argv[], char *env[])
{
  struct tib *tib = gettib();
  struct job *job = tib->job;
  char *cmdline;
  char *p, *q;
  int i, len, pid;
  handle_t child;
  struct tib *ctib;

  // If no vfork() is in progress return error
  struct _forkctx *fc = (struct _forkctx *) tib->forkctx;
  if (!fc)
  {
    errno = EPERM;
    return -1;
  }

  // Build command line from arguments
  len = 0;
  for (i = 0; argv[i]; i++)
  {
    // Add two extra chars for quoting args with spaces
    len += strlen(argv[i]) + 1;
    if (strchr(argv[i], ' ')) len += 2;
  }

  cmdline = q = malloc(len);
  if (!cmdline)
  {
    errno = ENOMEM;
    return -1;
  }

  for (i = 0; argv[i]; i++)
  {
    int sp = strchr(argv[i], ' ') != NULL;

    if (i > 0) *q++ = ' ';
    if (sp) *q++ = '"';
    p = argv[i];
    while (*p) *q++ = *p++;
    if (sp) *q++ = '"';
  }
  *q = 0;

  // Spawn new child job
  child = spawn(P_SUSPEND | P_CHILD, path, cmdline, env, &ctib);
  free(cmdline);
  if (child < 0) return -1;
  pid = ctib->job->id;
  resume(child);
  close(child);

  // Return control to the parent process path with pid
  resume_fork(tib, pid);
  return 0;
}

int execv(const char *path, char *argv[])
{
  return execve(path, argv, NULL);
}

int execl(const char *path, char *arg0, ...)
{
  return execve(path, &arg0, NULL);
}
