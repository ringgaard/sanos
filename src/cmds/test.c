//
// test.c
//
// Evaluate expression
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <shlib.h>
#include <unistd.h>
#include <sys/stat.h>

struct parser {
  char **argv;
  int pos;
  char *current;
};

static int parse_expr(struct parser *p);

int filemode(char *filename) {
  struct stat st;
  
  if (stat(filename, &st) < 0) return 0;
  return st.st_mode;
}

int filesize(char *filename) {
  struct stat st;
  
  if (stat(filename, &st) < 0) return -1;
  return st.st_size;
}

static next(struct parser *p) {
  if (p->current) p->current = p->argv[p->pos++];
}

static int match(struct parser *p, char *token) {
  return p->current && strcmp(p->current, token) == 0;
}

static int parse_term(struct parser *p) {
  int n;
  char *op;
  char *arg1;
  char *arg2;
  int cmp;

  if (!p->current) {
    // Empty expression evaluates to false
    return 0;
  } else if (match(p, "(")) {
    // Parse expression in parentheses
    next(p);
    n = parse_expr(p);
    if (!match(p, ")")) {
      fprintf(stderr, "Unmatched parentheses\n");
      return -1;
    }
    next(p);
    return n;
  } else if (match(p, "!")) {
    // Parse negation
    next(p);
    n = parse_expr(p);
    return n < 0 ? n : !n;
  } else if (*p->current == '-') {
    // Parse unary operator
    op = p->current;
    next(p);
    arg1 = p->current;
    if (!arg1) return -1;
    next(p);

    if (strlen(op) != 2) {
      fprintf(stderr, "%s: unary operator expected\n", op);
      return -1;
    }
    switch (op[1]) {
      case 'b': // File exists and is block device
        return S_ISBLK(filemode(arg1));
 
      case 'c': // File exists and is character device
        return S_ISCHR(filemode(arg1));

      case 'd': // File exists and is a directory
        return S_ISDIR(filemode(arg1));

      case 'e': // File exists
        return access(arg1, F_OK);

      case 'f': // File exists and is a regular file
        return S_ISREG(filemode(arg1));

      case 'g': // File exists and is has set-group-id
        return 0;

      case 'h': // File exists and is a symbolic link
        return 0;

      case 'n': // Length of string is non-zero
        return *arg1 != 0;

      case 'p': // File exists and is a FIFO
        return S_ISFIFO(filemode(arg1));

      case 'r': // File exists and is readable
        return access(arg1, R_OK) == 0;

      case 's': // File exists and has non-zero size
        return filesize(arg1) > 0;

      case 't': // File exists and is a terminal
        return 0;

      case 'u': // File exists and is has set-user-id
        return 0;

      case 'w': // File exists and is writable
        return access(arg1, W_OK) == 0;

      case 'x': // File exists and is executable
        return access(arg1, X_OK) == 0;

      case 'z': // Length of string is zero
        return *arg1 == 0;

      case 'L': // File exists and is a symbolic link
        return 0;

      case 'S': // File exists and is a socket
        return S_ISSOCK(filemode(arg1));

      default:
        fprintf(stderr, "%s: unary operator expected\n", op);
        return -1;
    }
  } else {
    // Parse binary operator
    arg1 = p->current;
    next(p);
    op = p->current;
    next(p);
    arg2 = p->current;
    next(p);
    if (!arg1 || !op || !arg2) return -1;
    cmp = strcmp(arg1, arg2);

    if (strcmp(op, "=") == 0) {
      return strcmp(arg1, arg2) == 0;
    } else if (strcmp(op, "!=") == 0) {
      return strcmp(arg1, arg2) != 0;
    } else {
      int n1 = atol(arg1);
      int n2 = atol(arg2);
      if (strcmp(op, "-eq") == 0) {
        return n1 == n2;
      } else if (strcmp(op, "-ne") == 0) {
        return n1 != n2;
      } else if (strcmp(op, "-gt") == 0) {
        return n1 > n2;
      } else if (strcmp(op, "-ge") == 0) {
        return n1 >= n2;
      } else if (strcmp(op, "-lt") == 0) {
        return n1 < n2;
      } else if (strcmp(op, "-le") == 0) {
        return n1 <= n2;
      } else {
        fprintf(stderr, "%s: binary operator expected\n", op);
        return -1;
      }
    }
  }

  return -1;
}

static int parse_or(struct parser *p) {
  int a, b;
  
  a = parse_term(p);
  if (a < 0) return a;
  while (match(p, "-o")) {
    next(p);
    b = parse_term(p);
    if (b < 0) return b;
    a = a || b;
  }
  return a;
}

static int parse_and(struct parser *p) {
  int a, b;

  a = parse_or(p);
  if (a < 0) return a;
  while (match(p, "-a")) {
    next(p);
    b = parse_or(p);
    if (b < 0) return b;
    a = a && b;
  }
  return a;
}

static int parse_expr(struct parser *p) {
  return parse_and(p);
}

shellcmd(test) {
  struct parser p;
  int rc;

  // Initialize parser
  p.argv = argv;
  p.pos = 1;
  p.current = argv[0];
  next(&p);

  // Parse expression
  rc = parse_expr(&p);
  if (match(&p, "]")) next(&p);
  if (p.current != NULL) {
    fprintf(stderr, "syntax error at '%s'\n", p.current);
    rc = -1;
  }

  // Return 0 if expression is true, 1 if false, and 2 on error
  if (rc < 0) return 2;
  if (rc > 0) return 0;
  return 1;
}

