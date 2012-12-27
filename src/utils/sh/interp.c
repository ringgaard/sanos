//
// interp.c
//
// Shell command interpreter
//
// Copyright (C) 2012 Michael Ringgaard. All rights reserved.
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

#include "sh.h"

static int expand_glob(struct stkmark *mark, struct args *args, char *pattern) {
  glob_t globbuf;
  unsigned i;

  if (glob(pattern, GLOB_NOCHECK | GLOB_NOESCAPE, NULL, &globbuf) < 0) {
    fprintf(stderr, "errror expanding glob pattern %s\n", pattern);
    return 1;
  }

  if (args) {
    for (i = 0; i < globbuf.gl_pathc; ++i) {
      stputstr(mark, globbuf.gl_pathv[i]);
      add_arg(args, ststr(mark));
    }
  } else {
    for (i = 0; i < globbuf.gl_pathc; ++i) {
      if (i > 0) stputc(mark, ' ');
      stputstr(mark, globbuf.gl_pathv[i]);
    }
  }
  
  globfree(&globbuf);
  return 0;
}

static int expand_args(struct stkmark *mark, struct job *job, struct args *args, union node *node) {
  union node *n;
  int before;
  int rc;
  char *value;

  switch (node->type) {
    case N_SIMPLECMD:
      for (n = node->ncmd.args; n; n = n->list.next) {
        rc = expand_args(mark, job, args, n);
        if (rc != 0) return rc;
      }
      break;

    case N_ARG:
      if (args) {
        before = args->num;
        for (n = node->narg.list; n; n = n->list.next) {
          rc = expand_args(mark, job, args, n);
          if (rc != 0) return rc;
        }
        if (args->num == before || ststrlen(mark) > 0) add_arg(args, ststr(mark));
      } else {
        for (n = node->narg.list; n; n = n->list.next) {
          rc = expand_args(mark, job, args, n);
          if (rc != 0) return rc;
        }
      }
      break;

    case N_ARGSTR:
      if (node->nargstr.flags & S_GLOB) {
        if (expand_glob(mark, args, node->nargstr.text) < 0) return 1;
      } else {
        stputstr(mark, node->nargstr.text);
      }
      break;

    case N_ARGPARAM:
      value = get_var(job, node->nargparam.name);
      if (value) stputstr(mark, value);
      break;

    default:
      fprintf(stderr, "Unsupported argument type: %d\n", node->type);
      return 1;
  }
  
  return 0;
}

static int interp_vars(struct stkmark *mark, struct job *job, union node *node) {
  struct arg *arg;
  union node *n;
  int rc;

  // Expand variable assignments  
  struct args args;
  init_args(&args);
  for (n = node->ncmd.vars; n; n = n->list.next) {
    rc = expand_args(mark, job, &args, n);
    if (rc != 0) {
      delete_args(&args);
      return rc;
    }
  }

  // Assign variables to job
  arg = args.first;
  while (arg) {
    char *name = arg->value;
    char *value = strchr(arg->value, '=');
    if (*value == '=') *value++ = 0;
    set_var(job, name, value);
    arg = arg->next;
  }
  delete_args(&args);
  return 0;
}

static int interp_redir(struct stkmark *mark, struct job *job, union node *node) {
  union node *n;
  int rc, dir, act, fd, f, flags, oflags;
  char *arg;

  // Open files for redirection
  rc = 0;
  for (n = node; n; n = n->nredir.next) {
    fd = n->nredir.fd;
    flags = n->nredir.flags;
    dir = flags & R_DIR;
    act = flags & R_ACT;

    rc = expand_args(mark, job, NULL, n->nredir.list);
    if (rc != 0) break;
    arg = ststr(mark);
    if (!arg || fd < 0 || fd >= STD_HANDLES || act != R_OPEN) {
      rc = 1;
      break;
    }

    oflags = 0;
    if (dir == R_IN) {
      oflags |= O_RDONLY;
    } else if (dir == R_OUT) {
      oflags |= O_WRONLY;
    } else if (dir == (R_IN | R_OUT)) {
      oflags |= O_RDWR;
    }

    if (dir & R_OUT) {
      oflags |= (flags & R_APPEND) ? O_APPEND : O_TRUNC;
      oflags |= (flags & R_CLOBBER) ? (O_CREAT | O_EXCL) : O_CREAT;
    }

    f = open(arg, oflags, 0666);
    if (f < 0) {
      perror(arg);
      rc = 1;
      break;
    }
    set_fd(job, fd, f);
  }
  return rc;
}

static struct job *setup_command(struct job *parent, union node *node) {
  struct stkmark mark;
  struct job *job;

  // Create new job
  job = create_job(parent);

  // Assign variables
  pushstkmark(NULL, &mark);
  if (node->ncmd.vars) {
    if (interp_vars(&mark, job, node) != 0) {
      popstkmark(&mark);
      remove_job(job);
      return NULL;
    }
  }

  // Perform I/O redirection
  if (node->ncmd.rdir) {
    if (interp_redir(&mark, job, node->ncmd.rdir) != 0) {
      popstkmark(&mark);
      remove_job(job);
      return NULL;
    }
  }

  // Expand command arguments
  if (expand_args(&mark, job, &job->args, node) != 0) {
    popstkmark(&mark);
    remove_job(job);
    return NULL;
  }

  popstkmark(&mark);
  return job;
}

static int interp_simple_command(struct job *parent, union node *node) {
  int rc;
  struct job *job;
  int i;

  // If there are no arguments just assign variables to parent job
  if (!node->ncmd.args) {
    struct stkmark mark;
    pushstkmark(NULL, &mark);
    rc = interp_vars(&mark, parent, node);
    popstkmark(&mark);
    return rc;
  }
  
  // Setup job for command
  job = setup_command(parent, node);
  if (job == NULL) return 1;

  // Execute job
  rc = execute_job(job);
  if (rc != 0) {
    remove_job(job);
    return rc;
  }
  if (job->handle != -1) resume(job->handle);
  for (i = 0; i < STD_HANDLES; i++) {
    if (job->fd[i] != -1) {
      close(job->fd[i]);
      job->fd[i] = -1;
    }
  }

  if (node->ncmd.flags & S_BGND) {
    // Detach background job
    detach_job(job);
    return 0;
  } else {
    // Wait for command to complete
    rc = wait_for_job(job);
    remove_job(job);
    return rc;
  }
}

static int interp_pipeline(struct job *parent, union node *node) {
  union node *n;
  struct job *job;
  int pipefd[2];
  int out;

  // Create job for pipeline and get stdin and stdout for whole pipeline
  job = create_job(parent);
  out = get_fd(job, 1, 1);
  pipefd[0] = pipefd[1] = -1;

  for (n = node->npipe.cmds; n; n = n->list.next) {
    int first = (n == node->npipe.cmds);
    int last = (n->list.next == NULL);

    // Set input to be the output of the previous pipe
    if (!first) {
      set_fd(job, 0, pipefd[0]);
      pipefd[0] = -1;
    }

    // Create pipe for output
    if (last) {
      set_fd(job, 1, out);
      out = -1;
    } else {
      pipe(pipefd);
      set_fd(job, 1, pipefd[1]);
      pipefd[1] = -1;
      n->list.flags |= S_BGND;
    }

    // Interpret pipeline element
    if (interp(job, n) != 0) {
      if (out != -1) close(out);
      if (pipefd[0] != -1) close(pipefd[0]);
      if (pipefd[1] != -1) close(pipefd[1]);
      remove_job(job);
      return 1;
    }
  }

  remove_job(job);
  return 0;
}

int interp(struct job *parent, union node *node) {
  switch (node->type) {
    case N_SIMPLECMD: return interp_simple_command(parent, node);
    case N_PIPELINE: return interp_pipeline(parent, node);
     default:
       fprintf(stderr, "Error: not supported (node type %d)\n", node->type);
       return 1;
  }
}

