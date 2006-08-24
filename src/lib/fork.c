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
#include <sys/wait.h>

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
  struct job *job = tib->job;
  struct _forkctx *fc = (struct _forkctx *) tib->forkctx;

  // Restore standard handles
  close(job->in);
  close(job->out);
  close(job->err);

  job->in = fc->fd[0];
  job->out = fc->fd[1];
  job->err = fc->fd[2];

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
  struct tib *tib = gettib();
  struct job *job = tib->job;

  // Save and duplicate standard handles
  fc->fd[0] = job->in;
  fc->fd[1] = job->out;
  fc->fd[2] = job->err;

  job->in = dup(job->in);
  job->out = dup(job->out);
  job->err = dup(job->err);

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
  struct job *job = tib->job;

  // If no vfork() is in progress just return
  struct _forkctx *fc = (struct _forkctx *) tib->forkctx;
  if (!fc) return;

  // Return control to the parent process path with synthetic status encoded pid
  resume_fork(tib, (status & 0xFF) | 0x40000000);
}

int waitpid(int pid, int *stat_loc, int options)
{
  int rc;
  handle_t h;

  // We can only wait for specific pids
  if (pid == -1)
  {
    errno = ENOSYS;
    return -1;
  }

  // Handle synthetic status encoded handles from exit()
  if (pid & 0x40000000)
  {
    if (stat_loc) *stat_loc = pid & 0xFF;
    return pid;
  }

  // Get handle for job from pid
  h = getjobhandle(pid);
  if (h == NOHANDLE)
  {
    errno = ECHILD;
    return -1;
  }

  // Wait for main thread in job to terminate
  rc = waitone(h, (options & WNOHANG) ? 0 : INFINITE);
  close(h);
  if (rc < 0) return 0;

  // Termination, return status
  if (stat_loc) *stat_loc = rc & 0xFF;
  return pid;
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

  // Spawn new job
  child = spawn(P_SUSPEND, path, cmdline, env, &ctib);
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
