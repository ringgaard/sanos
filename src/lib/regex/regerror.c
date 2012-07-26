//
// engine.c
//
// Error codes for regular expression library
//
// Ported to sanos by Michael Ringgaard.
//
// Copyright 1992, 1993, 1994, 1997 Henry Spencer.  All rights reserved.
// This software is not subject to any license of the American Telephone
// and Telegraph Company or of the Regents of the University of California.
// 
// Permission is granted to anyone to use this software for any purpose on
// any computer system, and to alter it and redistribute it, subject
// to the following restrictions:
// 
// 1. The author is not responsible for the consequences of use of this
//    software, no matter how awful, even if they arise from flaws in it.
// 
// 2. The origin of this software must not be misrepresented, either by
//    explicit claim or by omission.  Since few users ever read sources,
//    credits must appear in the documentation.
// 
// 3. Altered versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.  Since few users
//    ever read sources, credits must appear in the documentation.
// 
// 4. This notice may not be removed or altered.
// 

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <regex.h>

#include "regex2.h"

static struct rerr {
  int code;
  char *name;
  char *explain;
} rerrs[] = {
  {REG_OKAY,     "REG_OKAY",     "no errors detected"},
  {REG_NOMATCH,  "REG_NOMATCH",  "regexec() failed to match"},
  {REG_BADPAT,   "REG_BADPAT",   "invalid regular expression"},
  {REG_ECOLLATE, "REG_ECOLLATE", "invalid collating element"},
  {REG_ECTYPE,   "REG_ECTYPE",   "invalid character class"},
  {REG_EESCAPE,  "REG_EESCAPE",  "trailing backslash (\\)"},
  {REG_ESUBREG,  "REG_ESUBREG",  "invalid backreference number"},
  {REG_EBRACK,   "REG_EBRACK",   "brackets ([ ]) not balanced"},
  {REG_EPAREN,   "REG_EPAREN",   "parentheses not balanced"},
  {REG_EBRACE,   "REG_EBRACE",   "braces not balanced"},
  {REG_BADBR,    "REG_BADBR",    "invalid repetition count(s)"},
  {REG_ERANGE,   "REG_ERANGE",   "invalid character range"},
  {REG_ESPACE,   "REG_ESPACE",   "out of memory"},
  {REG_BADRPT,   "REG_BADRPT",   "repetition-operator operand invalid"},
  {REG_EMPTY,    "REG_EMPTY",    "empty (sub)expression"},
  {REG_ASSERT,   "REG_ASSERT",   "\"can't happen\" -- you found a bug"},
  {REG_INVARG,   "REG_INVARG",   "invalid argument to regex routine"},
  {-1,           "",             "*** unknown regexp error code ***"},
};

static char *regatoi(const regex_t *preg, char *localbuf);

//
// regerror - the interface to error numbers
//
size_t regerror(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size) {
  struct rerr *r;
  size_t len;
  int target = errcode & ~REG_ITOA;
  char *s;
  char convbuf[50];

  if (errcode == REG_ATOI) {
    s = regatoi(preg, convbuf);
  } else {
    for (r = rerrs; r->code >= 0; r++) {
      if (r->code == target) break;
    }
  
    if (errcode & REG_ITOA) {
      if (r->code >= 0) {
        strcpy(convbuf, r->name);
      } else {
        sprintf(convbuf, "REG_0x%x", target);
      }
      assert(strlen(convbuf) < sizeof(convbuf));
      s = convbuf;
    } else {
      s = r->explain;
    }
  }

  len = strlen(s) + 1;
  if (errbuf_size > 0) {
    if (errbuf_size > len) {
      strcpy(errbuf, s);
    } else {
      strncpy(errbuf, s, errbuf_size - 1);
      errbuf[errbuf_size-1] = '\0';
    }
  }

  return len;
}

//
// regatoi - internal routine to implement REG_ATOI
//
static char *regatoi(const regex_t *preg, char *localbuf) {
  struct rerr *r;

  for (r = rerrs; r->code >= 0; r++) {
    if (strcmp(r->name, preg->re_endp) == 0) break;
  }
  
  if (r->code < 0) return "0";

  sprintf(localbuf, "%d", r->code);
  return localbuf;
}
