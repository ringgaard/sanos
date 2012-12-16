#ifndef NODE_H
#define NODE_H

//
// Node types
//

#define N_SIMPLECMD   0
#define N_PIPELINE    1
#define N_AND         2
#define N_OR          3
#define N_NOT         4
#define N_SUBSHELL    5
#define N_CMDLIST     6
#define N_FOR         7
#define N_CASE        8
#define N_CASENODE    9
#define N_IF         10
#define N_WHILE      11
#define N_UNTIL      12
#define N_FUNCTION   13
#define N_ARG        14
#define N_ASSIGN     15
#define N_REDIR      16
#define N_ARGSTR     17
#define N_ARGCMD     18
#define N_ARGPARAM   19
#define N_ARGARITH   20

//
// Redirection types
//

#define R_IN      0x001 // Direction of redirection, either R_IN or R_OUT
#define R_OUT     0x002 

#define R_ACT     0x01c // Type of redirection, R_OPEN, R_DUP or R_HERE.
#define R_OPEN    0x004
#define R_DUP     0x008
#define R_HERE    0x010

#define R_STRIP   0x020 // Stripped here-document
#define R_APPEND  0x040 // Append mode for output redirections
#define R_CLOBBER 0x080 // Clobbering for output redirections
#define R_NOW     0x100 // Immediately open

#define IFS_DEFAULT " \t\n"

// Parameter quoting
#define S_TABLE      0x0003
#define S_UNQUOTED (0 << 0)
#define S_DQUOTED  (1 << 0)
#define S_SQUOTED  (2 << 0)
#define S_ARITH    (3 << 0)

#define S_BQUOTE   (1 << 2)

// Substitution types
#define S_SPECIAL    0x00f8
#define S_ARGC     (1 << 3)  // $#
#define S_ARGV     (2 << 3)  // $*
#define S_ARGVS    (3 << 3)  // $@
#define S_EXITCODE (4 << 3)  // $?
#define S_FLAGS    (5 << 3)  // $-
#define S_BGEXCODE (6 << 3)  // $!
#define S_ARG      (7 << 3)  // $[0-9]
#define S_PID      (8 << 3)  // $$

#define S_VAR        0x0f00
#define S_DEFAULT  (0 << 8)  // ${parameter:-word}
#define S_ASGNDEF  (1 << 8)  // ${parameter:=word}
#define S_ERRNULL  (2 << 8)  // ${parameter:?[word]}
#define S_ALTERNAT (3 << 8)  // ${parameter:+word}
#define S_RSSFX    (4 << 8)  // ${parameter%word}
#define S_RLSFX    (5 << 8)  // ${parameter%%word}
#define S_RSPFX    (6 << 8)  // ${parameter#word}
#define S_RLPFX    (7 << 8)  // ${parameter##word}

#define S_STRLEN    0x01000
#define S_NULL      0x02000  // Treat set but null as unset (:)
#define S_NOSPLIT   0x04000
#define S_ESCAPED   0x08000  // A char within here-doc delim is escaped
#define S_GLOB      0x10000
#define S_DELIM     0x20000  // Delimiter for here-dodument should be ignored
#define S_BGND      0x40000  // Background execution

//
// All node types starts with type, flags, and a next pointer for building lists
//

struct list {
  int type;
  int flags;
  union node *next;
};

//
// 3.9.1 - Simple command
//
// The simple command and the compound commands in 3.9.4 have the bgnd and
// rdir members in common, because they all can be redirected and put in
// background
//

struct ncmd {
  int type;
  int flags;
  union node *next;
  union node *rdir;
  union node *args;
  union node *vars;
};

//
// 3.9.2 - Pipeline
//

struct npipe {
  int type;
  int flags;
  union node *next;
  union node *cmds;
};

//
// 3.9.3 - Lists
//
// The compound list is simply done with node->next, because we don't
// need any further information
//

// AND-OR list

struct nandor {
  int type;
  int flags;
  union node *next;
  union node *cmd0;
  union node *cmd1;
};

// List

struct nlist {
  int type;
  int flags;
  union node *next;
  union node *cmds;
};

//
// 3.9.4.1 - Grouping compound
//

struct ngrp {
  int type;
  int flags;
  union node *next;
  union node *rdir;
  union node *cmds;
};

//
// 3.9.4.2 - for loop
//

struct nfor {
  int type;
  int flags;
  union node *next;
  union node *rdir;
  union node *cmds;
  union node *args;
  char *varn;
};

//
// 3.9.4.3 - case conditional 
//

struct ncase {
  int type;
  int flags;
  union node *next;
  union node *rdir;
  union node *list;
  union node *word;
};

struct ncasenode {
  int type;
  int flags;
  union node *next;
  union node *pats;
  union node *cmds;
};

//
// 3.9.4.4 - if conditional
//

struct nif {
  int type;
  int flags;
  union node *next;
  union node *rdir;
  union node *cmd0;
  union node *cmd1;
  union node *test;
};

//
// 3.9.4.5 - while loop
// 3.9.4.6 - until loop
//

struct nloop {
  int type;
  int flags;
  union node *next;
  union node *rdir;
  union node *cmds;
  union node *test;
};

//
// 3.9.5 - Function definition 
//

struct nfunc {
  int type;
  int flags;
  union node *next;
  union node *cmds;
  char *name;
};


//
// Subsets of T_WORD
//
// A word is either a redirection, an argument, or an assignment.
// The members nredir.file and nassign.args are themselves a narg.
// 

struct narg {
  int type;
  int flags;
  union node *next;
  union node *list;
  char *text;
};

// [fd]<operator><file>

struct nredir {
  int type;
  int flags;
  union node *next;
  union node *list; // can be file, fd, delim, here-doc-data
  union node *data; // next here-doc or expansion
  int fd;
};

struct nassign {
  int type;
  int flags;
  union node *next;
  union node *list;
  char *text;
};

//
// Argument (word) subnodes
//

struct nargstr {
  int type;
  int flags;
  union node *next;
  char *text;
};

struct nargparam {
  int type;
  int flags;
  union node *next;
  char *name;
  union node *word;
  int num;
};

struct nargcmd {
  int type;
  int flags;
  union node *next;
  union node *list;
};

struct nargarith {
  int type;
  int flags;
  union node *next;
  union node *list;
};

//
// Nodes
//

union node {
  int type;
  struct list list;
  struct ncmd ncmd;
  struct npipe npipe;
  struct nandor nandor;
  struct nlist nlist;
  struct ngrp ngrp;
  struct nfor nfor;
  struct ncase ncase;
  struct ncasenode ncasenode; 
  struct nif nif;
  struct nloop nloop;
  struct nfunc nfunc;
  struct narg narg;
  struct nredir nredir;
  struct nassign nassign;
  struct nargstr nargstr;
  struct nargcmd nargcmd;
  struct nargarith nargarith;
  struct nargparam nargparam;
};

#endif
