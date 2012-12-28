//
// job.c
//
// Shell jobs
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

void run_atexit_handlers();

static int hash(char *str) {
  int h = 0;
  while (*str) h = 5 * h + *str++;
  return h;
}

static void copy_vars(struct job *job) {
  struct var **vptr;
  struct var *var;

  // Find scope for variables
  struct job *scope = get_var_scope(job);

  // Initialize variables from parent scope
  vptr = &job->vars.list;
  var = scope->vars.list;
  while (var) {
    struct var *v = (struct var *) malloc(sizeof(struct var));
    v->name = strdup(var->name);
    v->value = strdup(var->value);
    v->hash = var->hash;
    v->next = NULL;
    *vptr = v;
    vptr = &v->next;
    var = var->next;
  }
  job->vars.inherit = 0;
}

static void assign_var(struct var **vars, char *name, char *value) {
  int h = hash(name);
  struct var **vptr = vars;
  struct var *v = *vptr;
  
  while (v) {
    if (v->hash == h && strcmp(v->name, name) == 0) {
      int len = strlen(value);
      v->value = (char *) realloc(v->value, len + 1);
      memcpy(v->value, value, len);
      v->value[len] = 0;
      return;
    }
    vptr = &v->next;
    v = *vptr;
  }

  v = *vptr = (struct var *) malloc(sizeof(struct var));
  v->name = strdup(name);
  v->value = strdup(value);
  v->hash = h;
  v->next = NULL;
}

static void remove_var(struct var **vars, char *name) {
  int h = hash(name);
  struct var **vptr = vars;
  struct var *v = *vptr;
  
  while (v) {
    if (v->hash == h && strcmp(v->name, name) == 0) {
      *vptr = v->next;
      free(v->name);
      free(v->value);
      free(v);
      return;
    }
    vptr = &v->next;
    v = *vptr;
  }
}

void set_var(struct job *job, char *name, char *value) {
  // Performed defered initialization of variables
  if (job->vars.inherit) copy_vars(job);

  if (value) {
    // Assign new value to variable.
    assign_var(&job->vars.list, name, value);
  } else {
    // Remove variable
    remove_var(&job->vars.list, name);
  }
}

struct job *get_var_scope(struct job *job) {
  while (job->parent && job->vars.inherit) job = job->parent;
  return job;
}

struct job *get_arg_scope(struct job *job) {
  while (job->parent && job->args.inherit) job = job->parent;
  return job;
}

char *get_var(struct job *job, char *name) {
  int h;
  struct var *var;

  // Find scope for variables
  struct job *scope = get_var_scope(job);
  
  // Find variable in scope
  h = hash(name);
  var = scope->vars.list;
  while (var) {
    if (var->hash == h && strcmp(var->name, name) == 0) return var->value;
    var = var->next;
  }
  return NULL;
}

static void delete_vars(struct vars *vars) {
  struct var *var = vars->list;
  while (var) {
    struct var *next = var->next;
    free(var->name);
    free(var->value);
    free(var);
    var = next;
  }
  vars->list = NULL;
}

void init_args(struct args *args) {
  args->first = args->last = NULL;
  args->num = 0;
  args->inherit = 0;
}

void delete_args(struct args *args) {
  struct arg *arg = args->first;
  while (arg) {
    struct arg *next = arg->next;
    free(arg->value);
    free(arg);
    arg = next;
  }
  args->first = args->last = NULL;
  args->num = 0;
}

struct arg *add_arg(struct args *args, char *value) {
  struct arg *arg = (struct arg *) malloc(sizeof(struct arg));
  arg->value = strdup(value ? value : "");
  arg->next = NULL;
  if (args->last) args->last->next = arg;
  if (!args->first) args->first = arg;
  args->last = arg;
  args->num++;
  return arg;
}

char *get_command_line(struct args *args) {
  struct arg *arg;
  char *cmdline;
  char *cmd;
  int cmdlen;

  // Compute command line size.
  arg = args->first;
  cmdlen = args->num;
  while (arg) {
    int escape = 0;
    char *p = arg->value;
    while (*p) {
      if (*p == ' ') escape = 1;
      p++;
    }
    if (escape) cmdlen += 2;
    cmdlen += strlen(arg->value);

    arg = arg->next;
  }

  // Build command line
  cmdline = malloc(cmdlen);
  cmd = cmdline;
  arg = args->first;
  while (arg) {
    int arglen;
    int escape = 0;
    char *p = arg->value;
    while (*p) {
      if (*p == ' ') escape = 1;
      p++;
    }
    if (arg != args->first) *cmd++ = ' ';
    if (escape) *cmd++ = '"';
    arglen = strlen(arg->value);
    memcpy(cmd, arg->value, arglen);
    cmd += arglen;
    if (escape) *cmd++ = '"';

    arg = arg->next;
  }
  *cmd = 0;

  return cmdline;
}

struct job *create_job(struct job *parent) {
  struct job *job;
  int i;

  // Allocate job
  job = (struct job *) malloc(sizeof(struct job));
  memset(job, 0, sizeof(struct job));
  job->parent = parent;
  job->shell = parent->shell;
  job->handle = -1;
  job->args.inherit = 1;

  // Insert job in job list for shell
  if (job->shell->jobs) {
    struct job *prev = job->shell->jobs;
    while (prev->next != NULL) prev = prev->next;
    prev->next = job;
  } else {
    job->shell->jobs = job;
  }

  // Defer initialization of variables
  job->vars.inherit = 1;

  // Defer initialization of I/O handles.
  for (i = 0; i < STD_HANDLES; i++) job->fd[i] = -1;

  return job;
}

void detach_job(struct job *job) {
  int i;

  // Check if job has already been detached
  if (!job->parent) return;

  // Detach variables
  if (job->vars.inherit) copy_vars(job);

  // Detach I/O handles
  for (i = 0; i < STD_HANDLES; i++) {
    if (job->fd[i] != -1) {
      close(job->fd[i]);
      job->fd[i] = -1;
    }
  }

  // Detach from parent
  job->parent = NULL;
}

void remove_job(struct job *job) {
  int i;

  // Delete arguments
  delete_args(&job->args);

  // Delete variables
  delete_vars(&job->vars);

  // Close I/O handles
  for (i = 0; i < STD_HANDLES; i++) {
    if (job->fd[i] != -1) close(job->fd[i]);
  }

  // Close process handle
  if (job->handle != -1) close(job->handle);

  // Remove job from joblist
  if (job->shell->jobs == job) {
    job->shell->jobs = job->next;
  } else {
    struct job *j = job->shell->jobs;
    struct job *prev = NULL;
    while (j != job) {
      prev = j;
      j = j->next;
    }
    if (j == job) prev->next = j->next;
  }

  // Deallocate job
  free(job);
}

void init_shell(struct shell *shell, int argc, char *argv[], char *env[]) {
  int i;
  char *name;
  char *value;
  struct job *top;
  
  // Create top job
  top = (struct job *) malloc(sizeof(struct job));
  memset(top, 0, sizeof(struct job));
  top->shell = shell;
  top->handle = -1;
  for (i = 0; i < STD_HANDLES; i++) top->fd[i] = -1;
  shell->top = shell->jobs = top;
  shell->done = 0;
  shell->debug = 0;
  shell->term = gettib()->proc->term;

  // Setup command line parameters
  for (i = 0; i < argc; i++) {
    add_arg(&top->args, argv[i]);
  }

  // Copy environment variables
  for (i = 0; env[i]; i++) {
    name = strdup(env[i]);
    value = strchr(name, '=');
    if (*value == '=') {
      *value++ = 0;
    } else {
      value = "";
    }
    assign_var(&top->vars.list, name, value);
    free(name);
  }

  // Set up shell-level standard I/O handles
  shell->fd[0] = fdin;
  shell->fd[1] = fdout;
  shell->fd[2] = fderr;
}

void clear_shell(struct shell *shell) {
  // Remove all jobs
  while (shell->jobs) remove_job(shell->jobs);
  
  // Clear job list.
  shell->jobs = shell->top = NULL;
}

int get_fd(struct job *job, int h, int own) {
  // Try to get I/O handle from job
  struct shell *shell = job->shell;
  int fd = job->fd[h];
  if (fd != -1) {
    if (own) {
      job->fd[h] = -1;
      return fd;
    } else {
      return dup(fd);
    }
  }

  // Try to get I/O handle from ancestors  
  job = job->parent;
  while (job) {
    fd = job->fd[h];
    if (fd != -1) {
      if (own) fd = dup(fd);
      return fd;
    }
    job = job->parent;
  }
  
  // Return shell-level I/O handle
  fd = shell->fd[h];
  if (own) fd = dup(fd);
  return fd;
}

void set_fd(struct job *job, int h, int fd) {
  if (job->fd[h] != -1) close(job->fd[h]);
  job->fd[h] = fd;
}

static char **get_env(struct job *job) {
  struct var *v;
  char **env;
  int n;

  // Find scope for variables
  struct job *scope = job;
  while (scope->vars.inherit && scope->parent != NULL) scope = scope->parent;

  // Build environment string for scope
  if (!scope->vars.list) return NULL;
  for (n = 0, v = scope->vars.list; v; n++, v = v->next);
  env = (char **) malloc((n + 1) * sizeof(char *));
  if (!env) return NULL;
  env[n] = NULL;
  for (n = 0, v = scope->vars.list; v; n++, v = v->next) {
    int nlen = strlen(v->name);
    int vlen = strlen(v->value);
    int len = nlen + vlen + 1;
    char *buf = env[n] = (char *) malloc(len + 1);
    if (buf) {
      memcpy(buf, v->name, nlen);
      buf[nlen] = '=';
      memcpy(buf + nlen + 1, v->value, vlen);
      buf[len] = 0;
    }
  }
  return env;
}

static void free_env(char **env) {
  int n;
  
  if (!env) return;
  for (n = 0; env[n]; n++) free(env[n]);
  free(env);
}

int wait_for_job(struct job *job) {
  if (job->handle != -1) {
    while (1) {
      job->exitcode = waitone(job->handle, INFINITE);
      if (job->exitcode >= 0 || errno != EINTR) break;
    }
    job->shell->lastrc = job->exitcode;
  }
  return 0;
}

static int run_external_command(struct job *job) {
  char *cmdline;
  char **env;
  int i;
  struct tib *tib;
  struct process *proc;

  // Build command line
  cmdline = get_command_line(&job->args);
  
  // Create environment
  env = get_env(job);

  // Run command
  job->handle = spawn(P_SUSPEND | P_DETACH, NULL, cmdline, env, &tib);
  free(cmdline);
  free_env(env);
  if (job->handle < 0) {
    perror("error");
    return 1;
  }
  tib->proc->term = job->shell->term;
  job->shell->lastpid = tib->pid;

  // Setup standard I/O for program
  proc = tib->proc;
  for (i = 0; i < 3; i++) proc->iob[i] = get_fd(job, i, 1);

  return 0;
}

static main_t lookup_internal(char *name) {
  char procname[MAX_COMMAND_LEN + 5];
  
  if (strlen(name) > MAX_COMMAND_LEN) return NULL;
  strcpy(procname, "cmd_");
  strcat(procname, name);
  return (main_t) dlsym(getmodule(NULL), procname);
}

static void exit_internal(int status) {
  struct process *proc = gettib()->proc;
  struct crtbase *crtbase = (struct crtbase *) proc->crtbase;

  // Execute atexit handlers
  run_atexit_handlers();

  // Deallocate arguments
  free_args(crtbase->argc, crtbase->argv);
}

static void __stdcall enter_internal(void *args) {
  struct process *proc = gettib()->proc;
  struct job *job = (struct job *) args;
  struct crtbase *crtbase = (struct crtbase *) proc->crtbase;

  // Initialize arguments
  crtbase->argc = parse_args(proc->cmdline, NULL);
  crtbase->argv = (char **) malloc((crtbase->argc + 1) * sizeof(char *));
  parse_args(proc->cmdline, crtbase->argv);
  crtbase->argv[crtbase->argc] = NULL;
  crtbase->opt.err = 1;
  crtbase->opt.ind = 1;
  crtbase->opt.sp = 1;
  crtbase->opt.place = "";

  // Run internal command.
  gettib()->proc->atexit = exit_internal;
  exit(job->main(crtbase->argc, crtbase->argv));
}

static int run_internal_command(struct job *job) {
  int i;
  struct tib *tib;
  struct process *proc;

  // Create process for internal command
  char *name = job->args.first->value;
  job->handle = beginthread(enter_internal, 0, job, CREATE_SUSPENDED | CREATE_NEW_PROCESS | CREATE_NO_ENV | CREATE_DETACHED, name, &tib);
  if (job->handle < 0) return 1;
  proc = tib->proc;
  proc->cmdline = get_command_line(&job->args);
  proc->env = get_env(job);
  proc->ident = strdup(name);
  proc->term = job->shell->term;
  for (i = 0; i < 3; i++) proc->iob[i] = get_fd(job, i, 1);
  job->shell->lastpid = tib->pid;

  return 0;
}

static builtin_t lookup_builtin(char *name) {
  char procname[MAX_COMMAND_LEN + 9];

  if (strlen(name) > MAX_COMMAND_LEN) return NULL;
  strcpy(procname, "builtin_");
  strcat(procname, name);
  return (builtin_t) dlsym(getmodule(NULL), procname);
}

int execute_job(struct job *job) {
  builtin_t cmd;
  int rc;

  // Ignore empty commands
  if (job->args.num == 0) return 0;

  // Run builtin command
  cmd = lookup_builtin(job->args.first->value);
  if (cmd != NULL) {
    rc = cmd(job);
    job->exitcode = rc;
    job->shell->lastrc = rc;
    return rc;
  }

  // Launch program
  job->main = lookup_internal(job->args.first->value);
  if (job->main != NULL) {
    rc = run_internal_command(job);
  } else {
    rc = run_external_command(job);
  }
  if (rc != 0) return rc;
  
  return 0;
}

