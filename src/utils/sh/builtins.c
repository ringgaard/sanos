//
// builtins.c
//
// Built-in shell commands
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

builtin(debug) {
  job->shell->debug = 1;
  return 0;
}

builtin(exit) {
  int rc = 0;
  if (job->args.num > 1) rc = atoi(job->args.first->next->value);
  job->shell->done = 1;
  return rc;
}

builtin(chdir) {
  char *path;

  if (job->args.num > 1) {
    path = job->args.first->next->value;
  } else {
    path = "/";
  }

  if (chdir(path) < 0) {
    fprintf(stderr, "%s: %s\n", path, strerror(errno));
    return 1;
  }

  return 0;
}

builtin(cd) {
  return builtin_chdir(job);
}

builtin(set) {
  struct var *var;
  struct arg *arg;

  if (job->args.num > 1) {
    // Set shell arguments
    struct job *scope = get_arg_scope(job);
    struct args *args = &scope->args;
    char *argv0 = strdup(args->first->value); 
    args->first->value = NULL;
    delete_args(args);
    args->inherit = 1;
    add_arg(args, argv0);
    free(argv0);
    arg = job->args.first->next;
    while (arg) {
      add_arg(args, arg->value);
      arg = arg->next;
    }
  } else {
    // Print variables
    struct job *scope = get_var_scope(job);
    var = scope->vars.list;
    while (var) {
      printf("%s=%s\n", var->name, var->value);
      var = var->next;
    }
  }

  return 0;
}

