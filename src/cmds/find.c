//
// find.c
//
// Find files in a directory hierarchy
//
// Copyright (C) 2013 Michael Ringgaard. All rights reserved.
// Original version by Erik Baalbergen.
// POSIX compliant version by Bert Laverman.
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

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <limits.h>
#include <fnmatch.h>
#include <stdio.h>
#include <shlib.h>
#include <os.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define SHELL          "/bin/sh"
#define MAXARG          256
#define BSIZE           512
#define SECS_PER_DAY    (24 * 60 * 60)

//
// Predicates
//

#define OP_NAME          1      // match name
#define OP_PERM          2      // check file permission bits
#define OP_TYPE          3      // check file type bits
#define OP_LINKS         4      // check link count
#define OP_USER          5      // check owner
#define OP_GROUP         6      // check group ownership
#define OP_SIZE          7      // check size, blocks
#define OP_SIZEC         8      // check size, bytes
#define OP_INUM          9      // compare inode number
#define OP_ATIME        10      // check last access time
#define OP_CTIME        11      // check creation time
#define OP_MTIME        12      // check last modification time
#define OP_EXEC         13      // execute command
#define OP_OK           14      // execute with confirmation
#define OP_PRINT        15      // print name
#define OP_PRINT0       16      // print name null terminated
#define OP_NEWER        17      // compare modification times
#define OP_AND          18      // logical and (short circuit)
#define OP_OR           19      // logical or (short circuit)
#define OP_XDEV         20      // do not cross file-system boundaries
#define OP_DEPTH        21      // descend directory before testing
#define OP_PRUNE        22      // don't descend into current directory
#define OP_NOUSER       23      // check validity of user id
#define OP_NOGROUP      24      // check validity of group id
#define LPAR            25      // left parenthesis
#define RPAR            26      // right parenthesis
#define NOT             27      // logical not

//
// Return values
//

#define EOI             -1      // end of expression
#define NONE             0      // not a valid predicate

//
// For -perm with symbolic modes
//

#define ISWHO(c)        (c == 'u' || c == 'g' || c == 'o' || c == 'a')
#define ISOPER(c)       (c == '-' || c == '=' || c == '+')
#define ISMODE(c)       (c == 'r' || c == 'w' || c == 'x')

#define MUSER           1
#define MGROUP          2
#define MOTHERS         4

//
// Expression node
//

struct exec {
  int argc;
  char *argv[MAXARG];
};

struct node {
  int type;
  union {
    char *str;
    struct {
      int val;
      int sign;
    } num;
    struct exec *exec;
    struct {
      struct node *left;
      struct node *right;
    } arg;
  };
};

//
// Operator names
//

struct oper {
  char *name;
  int id;
} ops[] = {
  {"name", OP_NAME},
  {"perm", OP_PERM},
  {"type", OP_TYPE},
  {"links", OP_LINKS},
  {"user", OP_USER},
  {"group", OP_GROUP},
  {"size", OP_SIZE},
  {"inum", OP_INUM},
  {"atime", OP_ATIME},
  {"ctime", OP_CTIME},
  {"mtime", OP_MTIME},
  {"exec", OP_EXEC},
  {"ok", OP_OK},
  {"print", OP_PRINT},
  {"print0", OP_PRINT0},
  {"newer", OP_NEWER},
  {"a", OP_AND},
  {"o", OP_OR},
  {"xdev", OP_XDEV},
  {"depth", OP_DEPTH},
  {"prune", OP_PRUNE},
  {"nouser", OP_NOUSER},
  {"nogroup", OP_NOGROUP},
  {0, 0}
};

char **ipp;                     // pointer to next argument during parsing
char *prog;                     // program name (== argv [0])
int current_time;               // for computing age
int xdev_flag = 0;              // cross device boundaries?
int devnr;                      // device number of first inode
int depth_flag = 0;             // descend before check?
int prune_here = 0;             // prune state
int umask_val;                  // current umask()
int need_print = 1;             // implicit -print needed?

static struct node *expr(int t);

static void nonfatal(char *s1, char *s2) {
  fprintf(stderr, "%s: %s%s\n", prog, s1, s2);
}

static void fatal(char *s1, char *s2) {
  fprintf(stderr, "%s: %s%s\n", prog, s1, s2);
  exit(1);
}

static struct node *newnode(int t) {
  struct node *n = (struct node *) malloc(sizeof(struct node));
  n->type = t;
  return n;
}

static int lex(char *str) {
  if (!str) return EOI;

  if (*str == '-') {
    struct oper *op;
    str++;
    for (op = ops; op->name; op++) {
      if (strcmp(str, op->name) == 0) break;
    }
    return op->id;
  }

  if (str[1] == 0) {
    switch (*str) {
      case '(': return LPAR;
      case ')': return RPAR;
      case '!': return NOT;
    }
  }

  return NONE;
}

static int isnumber(char *str, int base, int sign) {
  if (sign && (*str == '-' || (base == 8 && *str == '+'))) str++;
  while (*str >= '0' && *str < '0' + base) str++;
  return *str == '\0';
}

static void number(char *str, int base, int *pn, int *ps) {
  int up = '0' + base - 1;
  int val = 0;

  *ps = (*str == '-' || *str == '+') ? ((*str++ == '-') ? -1 : 1) : 0;
  while (*str >= '0' && *str <= up) val = base * val + *str++ - '0';
  if (*str) fatal("syntax error: illegal numeric value", "");
  *pn = val;
}

static void domode(int op, int *mode, int bits) {
  switch (op) {
    case '-':
      // Clear bits
      *mode &= ~bits;
      break;
    case '=':
      // Set bits
      *mode |= bits;
      break;
    case '+':
      // Set bit but take umask into account
      *mode |= (bits & ~umask_val);
      break;
  }
}

static void filemode(char *str, int *pl, int *ps) {
  int m = 0, w, op;
  char *p = str;

  if (*p == '-') {
    *ps = -1;
    p++;
  } else {
    *ps = 0;
  }

  while (*p) {
    w = 0;
    if (ISOPER(*p)) {
        w = MUSER | MGROUP | MOTHERS;
    } else if (!ISWHO(*p)) {
        fatal("u, g, o, or a expected: ", p);
    } else {
      while (ISWHO(*p)) {
        switch (*p) {
          case 'u':
             w |= MUSER;
             break;
          case 'g':
             w |= MGROUP;
             break;
          case 'o':
             w |= MOTHERS;
             break;
           case 'a':
             w = MUSER | MGROUP | MOTHERS;
          }
          p++;
        }
        if (!ISOPER(*p)) fatal("-, + or = expected: ", p);
    }
    op = *p++;
    while (ISMODE(*p)) {
      switch (*p) {
        case 'r':
          if (w & MUSER) domode(op, &m, S_IRUSR);
          if (w & MGROUP) domode(op, &m, S_IRGRP);
          if (w & MOTHERS) domode(op, &m, S_IROTH);
          break;
        case 'w':
          if (w & MUSER) domode(op, &m, S_IWUSR);
          if (w & MGROUP) domode(op, &m, S_IWGRP);
          if (w & MOTHERS) domode(op, &m, S_IWOTH);
          break;
        case 'x':
          if (w & MUSER) domode(op, &m, S_IXUSR);
          if (w & MGROUP) domode(op, &m, S_IXGRP);
          if (w & MOTHERS) domode(op, &m, S_IXOTH);
          break;
      }
      p++;
    }
    if (*p) {
      if (*p != ',') fatal("comma expected: ", p);
      p++;
    }
  }
  *pl = m;
}

static void checkarg(char *arg) {
  if (!arg) fatal("syntax error, argument expected", "");
}

static struct node *simple(int t) {
  struct node *p = newnode(t);
  struct exec *e;
  struct stat est;
  struct passwd *pw;
  struct group *gr;
  int i;

  switch (t) {
    case OP_TYPE:
      checkarg(*++ipp);
      switch (**ipp) {
        case 'b':
          p->num.val = S_IFBLK;
          break;
        case 'c':
          p->num.val = S_IFCHR;
          break;
        case 'd':
          p->num.val = S_IFDIR;
          break;
        case 'f':
          p->num.val = S_IFREG;
          break;
        case 'l':
          p->num.val = S_IFLNK;
          break;
        default:
          fatal("-type needs b, c, d, f or l", "");
      }
      break;

    case OP_USER:
      checkarg(*++ipp);
      if ((pw = getpwnam(*ipp)) == NULL && isnumber(*ipp, 10, 0)) {
        number(*ipp, 10, &p->num.val, &p->num.sign);
      } else {
        if (pw == NULL) fatal("unknown user: ", *ipp);
        p->num.val = pw->pw_uid;
        p->num.sign = 0;
      }
      break;

    case OP_GROUP:
      checkarg(*++ipp);
      if ((gr = getgrnam(*ipp)) == NULL && isnumber(*ipp, 10, 0)) {
        number(*ipp, 10, &p->num.val, &p->num.sign);
      } else {
        if (gr == NULL) fatal("unknown group: ", *ipp);
        p->num.val = gr->gr_gid;
        p->num.sign = 0;
      }
      break;

    case OP_SIZE:
      checkarg(*++ipp);
      i = strlen(*ipp) - 1;
      if ((*ipp)[i] == 'c') {
        p->type = OP_SIZEC;
        (*ipp)[i] = '\0';
      }
      number(*ipp, 10, &p->num.val, &p->num.sign);
      break;

    case OP_LINKS:
    case OP_INUM:
      checkarg(*++ipp);
      number(*ipp, 10, &p->num.val, &p->num.sign);
      break;

    case OP_PERM:
      checkarg(*++ipp);
      if (isnumber(*ipp, 8, 1)) {
        number(*ipp, 8, &p->num.val, &p->num.sign);
      } else {
        filemode(*ipp, &p->num.val, &p->num.sign);
      }
      break;

    case OP_ATIME:
    case OP_CTIME:
    case OP_MTIME:
      checkarg(*++ipp);
      number(*ipp, 10, &i, &p->num.sign);
      p->num.val = current_time - i * SECS_PER_DAY;
      // More than n days old means less than the absolute time
      p->num.sign *= -1;
      break;

    case OP_EXEC:
    case OP_OK:
      checkarg(*++ipp);
      e = (struct exec *) malloc(sizeof(struct exec));
      p->exec = e;
      e->argc = 1;
      e->argv[0] = SHELL;
      while (*ipp) {
        if (**ipp == ';' && (*ipp)[1] == '\0') {
          e->argv[e->argc] = 0;
          break;
        }
        e->argv[e->argc++] =
            ((*ipp)[0] == '{' && (*ipp)[1] == '}' && (*ipp)[2] == '\0') ? (char *) (-1) : *ipp;
        ipp++;
      }
      if (*ipp == 0) fatal("-exec/-ok: ; missing", "");
      break;

    case OP_NEWER:
      checkarg(*++ipp);
      if (lstat(*ipp, &est) == -1) fatal("-newer: can't get status of ", *ipp);
      p->num.val = est.st_mtime;
      break;

    case OP_NAME:
      checkarg(*++ipp);
      p->str = *ipp;
      break;

    case OP_XDEV:
      xdev_flag = 1;
      break;

    case OP_DEPTH:
      depth_flag = 1;    
      break;

    case OP_PRUNE:
    case OP_PRINT:
    case OP_PRINT0:
    case OP_NOUSER:    
    case OP_NOGROUP:    
      break;
    
    default:
      fatal("syntax error, operator expected", "");
  }

  if (t == OP_PRINT || t == OP_PRINT0 || t == OP_EXEC || t == OP_OK) {
    need_print = 0;
  }

  return p;
}

static struct node *secondary(int t) {
  struct node *n, *p;

  if (t == LPAR) {
    n = expr(lex(*++ipp));
    if (lex(*++ipp) != RPAR) fatal("syntax error, ) expected", "");
    return n;
  }
  if (t == NOT) {
    n = secondary(lex(*++ipp));
    p = newnode(NOT);
    p->arg.left = n;
    return p;
  }
  return simple(t);
}

static struct node *primary(int t) {
  struct node *nd, *p, *nd2;

  nd = secondary(t);
  if ((t = lex(*++ipp)) != OP_AND) {
    ipp--;
    if (t == EOI || t == RPAR || t == OP_OR) return nd;
  }
  nd2 = primary(lex(*++ipp));
  p = newnode(OP_AND);
  p->arg.left = nd;
  p->arg.right = nd2;
  return p;
}

static struct node *expr(int t) {
  struct node *nd, *p, *nd2;

  nd = primary(t);
  if ((t = lex(*++ipp)) == OP_OR) {
    nd2 = expr(lex(*++ipp));
    p = newnode(OP_OR);
    p->arg.left = nd;
    p->arg.right = nd2;
    return p;
  }
  ipp--;
  return nd;
}

static int execute(int op, struct exec *e, char *path) {
  char *argv[MAXARG];
  char **p, **q;

  // Replace {}s with path
  for (p = e->argv, q = argv; *p;) {
    if ((*q++ = *p++) == (char *) -1) q[-1] = path;
  }
  *q = '\0';

  // Optionally confirm before execute
  if (op == OP_OK) {
    struct term *term = gettib()->proc->term;
    char answer;
    for (p = &argv[1]; *p; p++) {
      write(term->ttyout, *p, strlen(*p));
      write(term->ttyout, " ", 1);
    }
    write(term->ttyout, "? ", 2);
    if (read(term->ttyin, &answer, 1) < 0) return 0;
    write(term->ttyout, &answer, 1);
    write(term->ttyout, "\n", 1);
    if (answer != 'y') return 0;
  }

  // Execute command
  return spawnv(0, NULL, argv);
}

static int ichk(int val, struct node *n) {
  switch (n->num.sign) {
    case  0: return val == n->num.val;
    case  1: return val > n->num.val;
    case -1: return val < n->num.val;
    default: fatal("internal: bad sign", "");
  }
  return 0;
}

static int check(char *path, struct stat *st, struct node *n, char *last) {
  if (!n) return 1;

  switch (n->type) {
    case OP_AND:
      return check(path, st, n->arg.left, last) &&
             check(path, st, n->arg.right, last);

    case OP_OR:
      return check(path, st, n->arg.left, last) ||
             check(path, st, n->arg.right, last);

    case NOT:
      return !check(path, st, n->arg.left, last);

    case OP_NAME:
      return fnmatch(n->str, last, 0) == 0;

    case OP_PERM:
      if (n->num.sign < 0) {
        return (st->st_mode & n->num.val) == n->num.val;
      } else {
        return(st->st_mode & 0777) == n->num.val;
      }

    case OP_NEWER:
      return st->st_mtime > n->num.val;

    case OP_TYPE:
      return (st->st_mode & S_IFMT) == n->num.val;

    case OP_LINKS:
      return ichk(st->st_nlink, n);

    case OP_USER:
      return st->st_uid == n->num.val;

    case OP_GROUP:
      return st->st_gid == n->num.val;

    case OP_SIZE:
      return ichk(st->st_size == 0 ? 0 : ((st->st_size - 1) / BSIZE + 1), n);

    case OP_SIZEC:
      return ichk(st->st_size, n);

    case OP_INUM:
      return ichk(st->st_ino, n);

    case OP_ATIME:
      return ichk(st->st_atime, n);

    case OP_CTIME:
      return ichk(st->st_ctime, n);

    case OP_MTIME:
      return ichk(st->st_mtime, n);

    case OP_EXEC:
    case OP_OK:
      return execute(n->type, n->exec, path);

    case OP_PRINT:
      printf("%s\n", path);
      return 1;

    case OP_PRINT0:
      printf("%s", path); 
      putchar(0);
      return 1;

    case OP_XDEV:
    case OP_DEPTH:
      return 1;

    case OP_PRUNE:
      prune_here = 1;
      return 1;

    case OP_NOUSER:
      return !getpwuid(st->st_uid);

    case OP_NOGROUP:
      return !getgrgid(st->st_gid);
    
    default:
      fatal("illegal node", "");
  }
  return 0;
}

static void find(char *path, struct node *pred, char *last) {
  char spath[PATH_MAX];
  char *send = spath;
  struct stat st;
  DIR *dp;
  struct dirent *de;

  if (path[0] == '/' && path[1] == '\0') {
    *send++ = '/';
    *send = '\0';
  } else {
    while (*send++ = *path++);
  }

  if (stat(spath, &st) == -1) {
    perror(spath);
  } else {
    switch (xdev_flag) {
      case 0:
        break;
      case 1:
        if (st.st_dev != devnr) return;
        break;
      case 2:
        xdev_flag = 1;
        devnr = st.st_dev;
        break;
    }

    prune_here = 0;
    if (!depth_flag && check(spath, &st, pred, last) && need_print) {
      printf("%s\n", spath);
    }

    if (!prune_here && (st.st_mode & S_IFMT) == S_IFDIR) {
      if ((dp = opendir(spath)) == NULL) {
        perror(spath);
        return;
      }
      send[-1] = '/';
      while ((de = readdir(dp)) != NULL) {
        char *name = de->d_name;
        if (name[0] != '.' || name[1] != '.' || name[2]) {
          strcpy(send, name);
          find(spath, pred, send);
        }
      }
      closedir(dp);
    }

    if (depth_flag) {
      send[-1] = '\0';
      if (check(spath, &st, pred, last) && need_print) {
        printf("%s\n", spath);
      }
    }
  }
}

shellcmd(find) {
  char **pathlist, *path, *last;
  int pathcnt = 0;
  int i;
  struct node *pred;

  // Set program name (for diagnostics)
  prog = *argv++;

  // Get umask and time
  umask(umask_val = umask(0));
  time(&current_time);

  // Find paths
  pathlist = argv;
  while (--argc > 0 && lex(*argv) == NONE) {
    pathcnt++;
    argv++;
  }
  
  // There must be at least one path
  if (pathcnt == 0) {
    fprintf(stderr, "usage: find PATH... [PREDICATE...]\n");
    exit(1);
  }

  // Parse predicate list
  ipp = argv;
  if (argc != 0) {
    pred = expr(lex(*ipp));
    if (lex(*++ipp) != EOI) fatal("syntax error", "");
  } else {
    // No predicate list
    pred = NULL;
  }

  // Find files
  for (i = 0; i < pathcnt; i++) {
    if (xdev_flag) xdev_flag = 2;
    path = pathlist[i];
    if ((last = strrchr(path, '/')) == NULL) {
      last = path;
    } else {
      last++;
    }
    find(path, pred, last);
  }

  return 0;
}

