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

static int exec_command(struct job *parent, char *cmdline) {
  struct inputfile *source = NULL;
  struct stkmark mark;
  struct parser parser;
  union node *node;

  pushstr(&source, cmdline);
  pushstkmark(NULL, &mark);
  parse_init(&parser, 0, source, &mark);

  while (!parent->shell->done && !(parser.tok & T_EOF)) {
    node = parse(&parser);
    if (!node) {
      if (parent->shell->debug) printf("line %d: tok=%d\n", source->lineno, parser.tok);
      break;
    }
    if (parent->shell->debug) print_node(node, stdout, 0);
    interp(parent, node);
  }
  
  popstkmark(&mark);

  return 0;
}

static void check_terminations(struct shell *shell) {
  struct job *job = shell->jobs;
  while (job) {
    struct job *next = job->next;
    if (job->handle != -1) {
      int rc = waitone(job->handle, 0);
      if (rc >= 0) {
        if (shell->debug) {
          fprintf(stderr, "Process %d terminated with exit code %d\n", job->handle, rc);
        }
        remove_job(job);
      }
    }
    job = next;
  }
}

static int run_command(struct job *parent, int argc, char *argv[]) {
  struct job *job;
  int i;
  int rc;

  job = create_job(parent);
  for (i = 0; i < argc; i++) {
    add_arg(&job->args, argv[i]);
  }
  rc = execute_job(job);
  if (rc == 0) {
    if (job->handle != -1) resume(job->handle);
    rc = wait_for_job(job);
  }
  remove_job(job);
  return rc;
}

static int run_shell(struct shell *shell) {
  char curdir[MAXPATH];
  char cmdline[1024];
  char *prompt = get_property(osconfig(), "shell", "prompt", "%s$ ");
  int rc;

  while (!shell->done) {
    check_terminations(shell);
    printf(prompt, getcwd(curdir, sizeof curdir));
    fflush(stdout);
    rc = readline(cmdline, sizeof cmdline);
    if (rc < 0) {
      if (errno != EINTR) break;
    } else {
      fflush(stdout);
      exec_command(shell->top, cmdline);
    }
  }
  return 0;
}

int main(int argc, char *argv[], char *envp[]) {
  struct shell shell;
  int rc;

  init_shell(&shell, argc, argv, envp);

  if (argc > 1) {
    rc = run_command(shell.top, argc - 1, argv + 1);
  } else {
    rc = run_shell(&shell);
  }

  if (rc == 0) rc = shell.top->exitcode;
  clear_shell(&shell);

  return rc;
}
