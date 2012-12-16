//
// sh.c
//
// Shell
//
// Copyright (C) 2011 Michael Ringgaard. All rights reserved.
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

static void indent(FILE *out, int level) {
  while (level-- > 0) fprintf(out, "  ");
}

static void print_node(union node *node, FILE *out, int level);

static void print_list(union node *node, FILE *out, int level) {
  while (node) {
    print_node(node, out, level);
    node = node->list.next;
  }
}

static void print_redir(union node *rdir, FILE *out, int level) {
  union node *n;

  if (rdir) {
    indent(out, level); fprintf(out, " redir:\n");
    for (n = rdir; n; n = n->list.next) print_node(n, out, level + 1);
  }
}

static void print_node(union node *node, FILE *out, int level) {
  union node *n;

  switch(node->type) {
    case N_SIMPLECMD:
      indent(out, level); fprintf(out, "CMD\n");
      if (node->ncmd.vars) {
        indent(out, level); fprintf(out, " vars:\n");
        for (n = node->ncmd.vars; n; n = n->list.next) print_node(n, out, level + 1);
      }
      if (node->ncmd.args) {
        indent(out, level); fprintf(out, " args:\n");
        for (n = node->ncmd.args; n; n = n->list.next) print_node(n, out, level + 1);
      }
      if (node->ncmd.rdir) {
        indent(out, level); fprintf(out, " rdir:\n");
        for (n = node->ncmd.rdir; n; n = n->list.next) print_node(n, out, level + 1);
      }
      break;

    case N_PIPELINE:
      indent(out, level); fprintf(out, "PIPE\n");
      for (n = node->npipe.cmds; n; n = n->list.next) print_node(n, out, level + 1);
      break;

    case N_ASSIGN:
    case N_ARG:
      indent(out, level); fprintf(out, "%s\n", node->type == N_ASSIGN ? "ASSIGN" : "ARG");
      for (n = node->narg.list; n; n = n->list.next) print_node(n, out, level + 1);
      break;
    
    case N_ARGSTR:
      indent(out, level); fprintf(out, "ARGSTR\n");
      indent(out, level); fprintf(out, " flags: %x\n", node->nargstr.flags);
      indent(out, level); fprintf(out, " text: [%s]\n", node->nargstr.text);
      break;
    
    case N_ARGPARAM:
      indent(out, level); fprintf(out, "ARGPARAM\n");
      indent(out, level); fprintf(out, " flags: %x\n", node->nargparam.flags);
      indent(out, level); fprintf(out, " name: %s\n", node->nargparam.name);
      if (node->nargparam.word) {
        indent(out, level); fprintf(out, " word:\n");
        print_node(node->nargparam.word, out, level + 1);
      }
      if (node->nargparam.num != -1) {
        indent(out, level); fprintf(out, " num: %d\n", node->nargparam.num);
      }
      break;
    
    case N_ARGCMD:
      indent(out, level); fprintf(out, "ARGCMD\n");
      indent(out, level); fprintf(out, " flags: %x\n", node->nargcmd.flags);
      if (node->nargcmd.list) print_list(node->nargcmd.list, out, level + 1);
      break;
    
    case N_IF:
      indent(out, level); fprintf(out, "IF\n");
      indent(out, level); fprintf(out, " test:\n");
      print_list(node->nif.test, out, level + 1);
      if (node->nif.cmd0) {
        indent(out, level); fprintf(out, " then:\n");
        print_list(node->nif.cmd0, out, level + 1);
      }
      if (node->nif.cmd1) {
        indent(out, level); fprintf(out, " else:\n");
        print_list(node->nif.cmd1, out, level + 1);
      }
      print_redir(node->nif.rdir, out, level);
      break;
    
    case N_FOR:
      indent(out, level); fprintf(out, "FOR\n");
      indent(out, level); fprintf(out, " varn: %s\n", node->nfor.varn);

      if (node->nfor.args) {
        indent(out, level); fprintf(out, " args:\n");
        print_list(node->nfor.args, out, level + 1);
      }

      indent(out, level); fprintf(out, " do:\n");
      print_list(node->nfor.cmds, out, level + 1);
      print_redir(node->nfor.rdir, out, level);
      break;
    
    case N_WHILE:
    case N_UNTIL:
      indent(out, level); fprintf(out, "%s\n", node->type == N_WHILE ? "WHILE" : "UNTIL");
      indent(out, level); fprintf(out, " test:\n");
      print_list(node->nloop.test, out, level + 1);
      indent(out, level); fprintf(out, " do:\n");
      print_list(node->nloop.cmds, out, level + 1);
      print_redir(node->nfor.rdir, out, level);
      break;
    
    case N_CASE:
      indent(out, level); fprintf(out, "CASE\n");
      indent(out, level); fprintf(out, " word:\n");
      print_node(node->ncase.word, out, level + 1);
      indent(out, level); fprintf(out, " in:\n");
      print_list(node->ncase.list, out, level + 1);
      print_redir(node->ncase.rdir, out, level);
      break;
    
    case N_CASENODE:
      indent(out, level); fprintf(out, "CASENODE\n");
      indent(out, level); fprintf(out, " pats:\n");
      print_list(node->ncasenode.pats, out, level + 1);
      indent(out, level); fprintf(out, " cmds:\n");
      print_list(node->ncasenode.cmds, out, level + 1);
      break;
    
    case N_NOT:
      indent(out, level); fprintf(out, "NOT\n");
      indent(out, level); fprintf(out, " cmd0:\n");
      print_list(node->nandor.cmd0, out, level + 1);
      break;

    case N_AND:
      indent(out, level); fprintf(out, "AND\n");
      indent(out, level); fprintf(out, " cmd0:\n");
      print_list(node->nandor.cmd0, out, level + 1);
      indent(out, level); fprintf(out, " cmd1:\n");
      print_list(node->nandor.cmd1, out, level + 1);
      break;

    case N_OR:
      indent(out, level); fprintf(out, "OR\n");
      indent(out, level); fprintf(out, " cmd0:\n");
      print_list(node->nandor.cmd0, out, level + 1);
      indent(out, level); fprintf(out, " cmd1:\n");
      print_list(node->nandor.cmd1, out, level + 1);
      break;
    
    case N_SUBSHELL:
      indent(out, level); fprintf(out, "SUBSHELL\n");
      indent(out, level); fprintf(out, " cmds:\n");
      print_list(node->ngrp.cmds, out, level + 1);
      print_redir(node->ngrp.rdir, out, level);
      break;

    case N_CMDLIST:
      indent(out, level); fprintf(out, "CMDLIST\n");
      indent(out, level); fprintf(out, " cmds:\n");
      print_list(node->ngrp.cmds, out, level + 1);
      print_redir(node->ngrp.rdir, out, level);
      break;
    
    case N_REDIR:
      indent(out, level); fprintf(out, "REDIR\n");
      indent(out, level); fprintf(out, " flags: %x\n", node->nredir.flags);
      indent(out, level); fprintf(out, " fd: %x\n", node->nredir.fd);
      if (node->nredir.list) {
        indent(out, level); fprintf(out, " list:\n");
        print_list(node->nredir.list, out, level + 1);
      }
      if (node->nredir.data) {
        indent(out, level); fprintf(out, " data:\n");
        print_list(node->nredir.data, out, level + 1);
      }
      break;
    
    case N_FUNCTION:
    case N_ARGARITH:
      // TODO:  IMPLEMENT  !!!

    default:
      indent(out, level); fprintf(out, "UNKNOWN %d\n", node->type);
  }
}

static int hash(char *str) {
  int h = 0;
  while (*str) h = 5 * h + *str++;
  return h;
}

static void set_var(struct var **vars, char *name, char *value) {
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

static struct arg *add_arg(struct args *args, char *value) {
  struct arg *arg = (struct arg *) malloc(sizeof(struct arg));
  arg->value = strdup(value ? value : "");
  arg->next = NULL;
  if (args->last) args->last->next = arg;
  if (!args->first) args->first = arg;
  args->last = arg;
  args->num++;
  return arg;
}

static struct job *create_job(struct job *parent) {
  struct job *job;
  struct var *pv;
  struct var *last;

  // Allocate job
  job = (struct job *) malloc(sizeof(struct job));
  memset(job, 0, sizeof(struct job));
  job->parent = parent;
  job->shell = parent->shell;

  // Insert job in job list for shell
  if (job->shell->jobs) {
    struct job *prev = job->shell->jobs;
    while (prev->next != NULL) prev = prev->next;
    prev->next = job;
  } else {
    job->shell->jobs = job;
  }

  // Copy variables from parent
  pv = parent->vars;
  last = NULL;
  while (pv) {
    struct var *v = (struct var *) malloc(sizeof(struct var));
    v->name = strdup(pv->name);
    v->value = strdup(pv->value);
    v->hash = pv->hash;
    v->next = NULL;
    if (last != NULL) last->next = v;
    last = v;
    pv = pv->next;
  }

  // Copy file handles from parent
  job->fd[0] = parent->fd[0];
  job->fd[1] = parent->fd[1];
  job->fd[2] = parent->fd[2];

  return job;
}

static void remove_job(struct job *job) {
  struct arg *a;
  struct var *v;

  // Free arguments
  a = job->args.first;
  while (a) {
    struct arg *next = a->next;
    free(a->value);
    free(a);
    a = next;
  }

  // Free variables
  v = job->vars;
  while (v) {
    struct var *next = v->next;
    free(v->name);
    free(v->value);
    free(v);
    v = next;
  }
  
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

static void init_shell(struct shell *shell, int argc, char *argv[], char *env[]) {
  int i;
  char *name;
  char *value;
  struct job *top;
  
  // Create top job.
  top = (struct job *) malloc(sizeof(struct job));
  memset(top, 0, sizeof(struct job));
  top->shell = shell;
  shell->top = shell->jobs = top;

  // Setup command line parameters.
  for (i = 0; i < argc; i++) {
    add_arg(&top->args, argv[i]);
  }

  // Copy environment variables.
  for (i = 0; env[i]; i++) {
    name = strdup(env[i]);
    value = strchr(name, '=');
    if (*value == '=') {
      *value++ = 0;
    } else {
      value = "";
    }
    set_var(&top->vars, name, value);
    free(name);
  }

  // Copy file handles.
  top->fd[0] = fdin;
  top->fd[1] = fdout;
  top->fd[2] = fderr;
}

static int expand_glob(struct stkmark *mark, struct args *args, char *pattern) {
  glob_t globbuf;
  unsigned i;

  if (glob(pattern, GLOB_NOCHECK | GLOB_NOESCAPE, NULL, &globbuf) < 0) {
    fprintf(stderr, "errror expanding glob pattern %s\n", pattern);
    return -1;
  }
  
  for (i = 0; i < globbuf.gl_pathc; ++i) {
    stputstr(mark, globbuf.gl_pathv[i]);
    add_arg(args, ststr(mark));
  }
  
  globfree(&globbuf);
  return 0;
}

static int expand_args(struct stkmark *mark, struct job *job, union node *node) {
  union node *n;
  int before;

  switch (node->type) {
    case N_SIMPLECMD:
      for (n = node->ncmd.args; n; n = n->list.next) expand_args(mark, job, n);
      break;
     
    case N_ARG:
      before = job->args.num;
      for (n = node->narg.list; n; n = n->list.next) expand_args(mark, job, n);
      if (job->args.num == before || ststrlen(mark) > 0) add_arg(&job->args, ststr(mark));
      break;

    case N_ARGSTR:
      if (node->nargstr.flags & S_GLOB) {
        if (expand_glob(mark, &job->args, node->nargstr.text) < 0) return -1;
      } else {
        stputstr(mark, node->nargstr.text);
      }
      break;
     
    default:
      fprintf(stderr, "Unsupported argument type: %d\n", node->type);
      return -1;
  }
  
  return 0;
}

static char *get_command_line(struct args *args) {
  struct arg *arg;
  char *cmdline;
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

  // Build command line.
  char *cmdline = malloc(cmdlen);
  char *cmd = cmdline;
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

static int run_external_command(struct job *job) {
  char *cmdline;

  // Build command line
  cmdline = get_command_line(&job->args);

  // Run command
  job->exitcode = spawn(P_WAIT, NULL, cmdline, NULL, NULL);
  if (job->exitcode < 0) perror("error");
  free(cmdline);

  return job->exitcode;
}

static void exit_builtin(int status) {
  struct process *proc = gettib()->proc;
  struct crtbase *crtbase = (struct crtbase *) proc->crtbase;

  // Execute atexit handlers
  run_atexit_handlers();

  // Deallocate arguments
  free_args(crtbase->argc, crtbase->argv);
}

static void __stdcall enter_builtin(void *args) {
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

  // Run builtin command.
  gettib()->proc->atexit = exit_builtin;
  return job->builtin(crtbase->argc, crtbase->argv);
}

static int run_builtin_command(struct job *job) {
  // Create process for builtin command
  char *name = job->args.first->value;
  struct tib *tib;
  job->handle = beginthread(enter_builtin, 0, job, CREATE_SUSPENDED | CREATE_NEW_PROCESS, name, &tib);
  if (job->handle < 0) return -1;
  tib->proc->cmdline = get_command_line(&job->args);
  tib->proc->ident = strdup(name);

  // Run command and wait for termination
  job->exitcode = resume(job->handle);
  if (job->exitcode >= 0) {
    while (1) {
      job->exitcode = waitone(job->handle, INFINITE);
      if (job->exitcode >= 0 || errno != EINTR) break;
    }
    close(job->handle);
  }
  
  return job->exitcode;
}

shellcmd(echo) {
  int i;

  for (i = 1; i < argc; i++) {
    if (i > 1) fputc(' ', stdout);
    fputs(argv[i], stdout);
  }
  fputs("\n", stdout);

  return 0;
}

static int execute_simple_command(struct job *parent, union node *node) {
  int rc = 0;
  struct stkmark mark;
  struct job *job;

  // Create new job
  job = create_job(parent);

  // Expand command arguments
  pushstkmark(NULL, &mark);
  rc = expand_args(&mark, job, node);
  popstkmark(&mark);
  if (rc < 0) goto done;

  // Ignore empty commands
  if (job->args.num == 0) goto done;

  // Run program
  job->builtin = (builtin_t) dlsym(getmodule(NULL), job->args.first->value);
  if (job->builtin != NULL) {
    // Run builtin command
    run_builtin_command(job);
  } else {
    // Run external program
    run_external_command(job);
  }

  //struct arg *arg;
  //for (arg = job->args.first; arg; arg = arg->next) printf("arg: %s\n", arg->value);
  
done:
  remove_job(job);
  
  return rc;
}

static int execute(struct job *parent, union node *node) {
  switch (node->type) {
    case N_SIMPLECMD: return execute_simple_command(parent, node);
     default:
       fprintf(stderr, "Error: not supported (node type %d)\n", node->type);
       return -1;
  }
}

static int exec_command(struct job *parent, char *cmdline) {
  struct inputfile *source = NULL;
  struct stkmark mark;
  struct parser p;
  union node *node;

  pushstr(&source, cmdline);
  pushstkmark(NULL, &mark);
  parse_init(&p, 0, source, &mark);

  while (!(p.tok & T_EOF)) {
    //printf("line %d: tok=%d\n", source->lineno, p.tok);
    node = parse(&p);
    if (!node) break;
    //print_node(node, stdout, 0);
    execute(parent, node);
  }
  
  popstkmark(&mark);

  return 0;
}

static int run_shell(struct shell *shell) {
  char curdir[MAXPATH];
  char cmdline[1024];
  char *prompt = get_property(osconfig(), "shell", "prompt", "nsh %s$ ");
  int rc;

  while (1) {
    printf(prompt, getcwd(curdir, sizeof curdir));
    fflush(stdout);
    rc = readline(cmdline, sizeof cmdline);
    if (rc < 0) {
      if (errno != EINTR) break;
    } else {
      if (stricmp(cmdline, "exit") == 0) break;
      fflush(stdout);
      exec_command(shell->top, cmdline);
    }
  }
  return 0;
}

int main(int argc, char *argv[], char *envp[]) {
  struct shell shell;

  if (gettib()->proc->term->type == TERM_VT100) setvbuf(stdout, NULL, 0, 8192);
  init_shell(&shell, argc, argv, envp);

  int rc = run_shell(&shell);

  setbuf(stdout, NULL);
  //clear_job(&top);

  return rc;
}
