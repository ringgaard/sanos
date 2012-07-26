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

struct job top;

void indent(FILE *out, int level) {
  while (level-- > 0) fprintf(out, "  ");
}

void print_node(union node *node, FILE *out, int level);

void print_list(union node *node, FILE *out, int level) {
  while (node) {
    print_node(node, out, level);
    node = node->list.next;
  }
}

void print_redir(union node *rdir, FILE *out, int level) {
  union node *n;

  if (rdir) {
    indent(out, level); fprintf(out, " redir:\n");
    for (n = rdir; n; n = n->list.next) print_node(n, out, level + 1);
  }
}

void print_node(union node *node, FILE *out, int level) {
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

int main0(int argc, char *argv[], char *envp[]) {
  struct inputfile *source = NULL;
  struct stkmark mark;
  struct parser p;
  int fd;
  union node *node;

  if (argc <= 1) {
    fprintf(stderr, "usage: sh <source>\n");
    exit(1);
  }

  fd = open(argv[1], 0);
  if (fd < 0) {
    perror(argv[1]);
    exit(1);
  }

  pushfile(&source, fd);
  pushstkmark(NULL, &mark);
  parse_init(&p, 0, source, &mark);

  while (1) {
    printf("line %d:\n", source->lineno);
    node = parse(&p);

    if (node) {
      print_node(node, stdout, 0);
    } else if (p.tok & T_EOF) {
      break;
    } else if (!(p.tok & (T_NL | T_SEMI | T_BGND))) {
      fprintf(stderr, "syntax error (line %d)\n", source->lineno);
      if (p.tree) print_node(p.tree, stdout, 0);
      break;
    }

    if (p.tok & (T_NL | T_SEMI | T_BGND)) p.pushback = 0;
  }

  return 0;
}

int hash(char *str) {
  int h = 0;
  while (*str) h = 5 * h + *str++;
  return h;
}

void set_var(struct var **vars, char *name, char *value) {
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

struct arg *add_arg(struct stkmark *mark, struct args *args, char *value) {
  struct arg *arg = (struct arg *) stalloc(mark, sizeof(struct arg));
  arg->value = value ? value : "";
  if (args->last) args->last->next = arg;
  if (!args->first) args->first = arg;
  args->last = arg;
  args->num++;
  return arg;
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
    add_arg(mark, args, ststr(mark));
  }
  
  globfree(&globbuf);
  return 0;
}

static int expand_args(struct stkmark *mark, struct args *args, union node *node) {
  union node *n;
  int before;

  switch (node->type) {
    case N_SIMPLECMD:
      for (n = node->ncmd.args; n; n = n->list.next) expand_args(mark, args, n);
      break;
     
    case N_ARG:
      before = args->num;
      for (n = node->narg.list; n; n = n->list.next) {
        expand_args(mark, args, n);
      }
      if (args->num == before || ststrlen(mark) > 0) add_arg(mark, args, ststr(mark)); 
      break;

    case N_ARGSTR:
      if (node->nargstr.flags & S_GLOB) {
        if (expand_glob(mark, args, node->nargstr.text) < 0) return -1;
      } else {
        char *p = node->nargstr.text;
        while (*p) stputc(mark, *p++);
      }
      break;
     
     default:
       fprintf(stderr, "Unknown argument type: %d\n", node->type);
       return -1;
  }
  
  return 0;
}


static int exec_command(char *cmdline) {
  struct inputfile *source = NULL;
  struct stkmark mark;
  struct parser p;
  union node *node;

  pushstr(&source, cmdline);
  pushstkmark(NULL, &mark);
  parse_init(&p, 0, source, &mark);

  while (!(p.tok & T_EOF)) {
    struct arg *arg;
    struct args *args;
    
    printf("line %d: tok=%d\n", source->lineno, p.tok);
    node = parse(&p);
    if (!node) break;
    printf("analyze\n");

    print_node(node, stdout, 0);
    args = (struct args *) stalloc(&mark, sizeof(struct args));
      
    if (expand_args(&mark, args, node) < 0) break;
    for (arg = args->first; arg; arg = arg->next) printf("arg: %s\n", arg->value);
  }
  
  popstkmark(&mark);
  return 0;
}

void shell() {
  char curdir[MAXPATH];
  char cmdline[1024];
  char *prompt = get_property(osconfig, "shell", "prompt", "%s$ ");
  int rc;

  while (1) {
    printf(prompt, getcwd(curdir, sizeof curdir));
    rc = readline(cmdline, sizeof cmdline);
    if (rc < 0) {
      if (errno != EINTR) break;
    } else {
      if (stricmp(cmdline, "exit") == 0) break;
      fflush(stdout);
      exec_command(cmdline);
    }
  }
}

void setup_top_job(char *env[]) {
  int i;
  char *name;
  char *value;
  
  memset(&top, 0, sizeof(struct job));
  for (i = 0; env[i]; i++) {
    name = strdup(env[i]);
    value = strchr(name, '=');
    if (*value == '=') {
      *value++ = 0;
    } else {
      value = "";
    }
    set_var(&top.vars, name, value);
    free(name);
  }
}

int main(int argc, char *argv[], char *envp[]) {
  if (gettib()->proc->term->type == TERM_VT100) setvbuf(stdout, NULL, 0, 8192);
  setup_top_job(envp);

  shell();

  setbuf(stdout, NULL);

  return 0;
}
