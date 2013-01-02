//
// node.c
//
// Shell AST nodes
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

static void indent(FILE *out, int level) {
  while (level-- > 0) fprintf(out, "  ");
}

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

void print_expr(struct expr *expr, FILE *out, int level) {
  if (expr) {
    int op = expr->op & ~A_ASSIGN;
    indent(out, level);
    if (op > 0xFF) {
      fprintf(out, "expr <%x>%s", op, expr->op & A_ASSIGN ? "=" : "");
    } else {
      fprintf(out, "expr %c%s", op, expr->op & A_ASSIGN ? "=" : "");
    }

    if (expr->num != 0) fprintf(out, " num=%d", expr->num);
    if (expr->var) fprintf(out, " var=%s", expr->var);
    fprintf(out, "\n");
    if (expr->left) print_expr(expr->left, out, level + 1);
    if (expr->right) print_expr(expr->right, out, level + 1);
  }
}

void print_node(union node *node, FILE *out, int level) {
  union node *n;

  switch(node->type) {
    case N_SIMPLECMD:
      indent(out, level); fprintf(out, "CMD\n");
      if (node->ncmd.flags != 0) {
        indent(out, level); fprintf(out, " flags: %x\n", node->ncmd.flags);
      } 
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

    case N_ARG:
      indent(out, level); fprintf(out, "%s\n", "ARG");
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
      indent(out, level); fprintf(out, "FUNCTION\n");
      indent(out, level); fprintf(out, " name: %s\n", node->nfunc.name);
      if (node->nfunc.cmds) {
        indent(out, level); fprintf(out, " cmds:\n");
        print_list(node->nfunc.cmds, out, level + 1);
      }
      break;

    case N_ARGARITH:
      indent(out, level); fprintf(out, "ARGARITH\n");
      print_expr(node->nargarith.expr, out, level + 1);
      break;

    default:
      indent(out, level); fprintf(out, "UNKNOWN %d\n", node->type);
  }
}

int list_size(union node *node) {
  int n = 0;
  while (node) {
    n++;
    node = node->list.next;
  }
  return n;
}

static union node *copy_list(struct stkmark *mark, union node *node) {
  union node *list = NULL;
  union node **nptr = &list;
  while (node) {
    union node *n = copy_node(mark, node);
    *nptr = n;
    nptr = &n->list.next;
    node = node->list.next;
  }
  return list;
}

static struct expr *copy_expr(struct stkmark *mark, struct expr *expr) {
  struct expr *e;
  
  if (!expr) return NULL;
  e = stalloc(mark, sizeof(struct expr));
  e->op = expr->op;
  e->var = ststrdup(mark, expr->var);
  e->left = copy_expr(mark, expr->left);
  e->right = copy_expr(mark, expr->right);
  return e;
}

union node *copy_node(struct stkmark *mark, union node *node) {
  union node *n = stalloc(mark, sizeof(union node));
  memcpy(n, node, sizeof(union node));

  switch (node->type) {
    case N_SIMPLECMD:
      n->ncmd.rdir = copy_list(mark, node->ncmd.rdir);
      n->ncmd.args = copy_list(mark, node->ncmd.args);
      n->ncmd.vars = copy_list(mark, node->ncmd.vars);
      break;

    case N_PIPELINE:
      n->npipe.cmds = copy_list(mark, node->npipe.cmds);
      break;

    case N_AND:
    case N_OR:
    case N_NOT:
      n->nandor.cmd0 = copy_node(mark, node->nandor.cmd0);
      n->nandor.cmd1 = copy_node(mark, node->nandor.cmd1);
      break;

    case N_SUBSHELL:
    case N_CMDLIST:
      n->ngrp.rdir = copy_list(mark, node->ngrp.rdir);
      n->ngrp.cmds = copy_list(mark, node->ngrp.cmds);
      break;

    case N_FOR:
      n->nfor.rdir = copy_list(mark, node->nfor.rdir);
      n->nfor.cmds = copy_list(mark, node->nfor.cmds);
      n->nfor.args = copy_list(mark, node->nfor.args);
      n->nfor.varn = ststrdup(mark, node->nfor.varn);
      break;

    case N_CASE:
      n->ncase.rdir = copy_list(mark, node->ncase.rdir);
      n->ncase.list = copy_list(mark, node->ncase.list);
      n->ncase.word = copy_list(mark, node->ncase.word);
      break;

    case N_CASENODE:
      n->ncasenode.pats = copy_list(mark, node->ncasenode.pats);
      n->ncasenode.cmds = copy_list(mark, node->ncasenode.cmds);
      break;

    case N_IF:
      n->nif.rdir = copy_list(mark, node->nif.rdir);
      n->nif.cmd0 = copy_list(mark, node->nif.cmd0);
      n->nif.cmd1 = copy_list(mark, node->nif.cmd1);
      n->nif.test = copy_list(mark, node->nif.test);
      break;

    case N_WHILE:
    case N_UNTIL:
      n->nloop.rdir = copy_list(mark, node->nloop.rdir);
      n->nloop.cmds = copy_list(mark, node->nloop.cmds);
      n->nloop.test = copy_list(mark, node->nloop.test);
      break;

    case N_FUNCTION:
      n->nfunc.cmds = copy_list(mark, node->nfunc.cmds);
      n->nfunc.name = ststrdup(mark, node->nfunc.name);
      break;

    case N_ARG:
      n->narg.list = copy_list(mark, node->narg.list);
      n->narg.text = ststrdup(mark, node->narg.text);
      break;

    case N_REDIR:
      n->nredir.list = copy_list(mark, node->nredir.list);
      n->nredir.data = copy_list(mark, node->nredir.data);
      break;

    case N_ARGSTR:
      n->nargstr.text = ststrdup(mark, node->nargstr.text);
      break;

    case N_ARGCMD:
      n->nargcmd.list = copy_list(mark, node->nargcmd.list);
      break;

    case N_ARGPARAM:
      n->nargparam.name = ststrdup(mark, node->nargparam.name);
      n->nargparam.word = copy_list(mark, node->nargparam.word);
      break;

    case N_ARGARITH:
      n->nargarith.expr = copy_expr(mark, node->nargarith.expr);
      break;
  }

  return n;
}

