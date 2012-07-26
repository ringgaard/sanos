//
// regex.h
//
// Regular expression library
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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef REGEX_H
#define REGEX_H

#include <sys/types.h>

typedef off_t regoff_t;

typedef struct {
  int re_magic;
  size_t re_nsub;       // number of parenthesized subexpressions
  const char *re_endp;  // end pointer for REG_PEND
  struct re_guts *re_g; // none of your business :-)
} regex_t;

typedef struct {
  regoff_t rm_so;       // start of match
  regoff_t rm_eo;       // end of match
} regmatch_t;

#define REG_BASIC       0000
#define REG_EXTENDED    0001
#define REG_ICASE       0002
#define REG_NOSUB       0004
#define REG_NEWLINE     0010
#define REG_NOSPEC      0020
#define REG_PEND        0040
#define REG_DUMP        0200

#define REG_NOTBOL      00001
#define REG_NOTEOL      00002
#define REG_STARTEND    00004
#define REG_TRACE       00400   // tracing of execution
#define REG_LARGE       01000   // force large representation
#define REG_BACKR       02000   // force use of backref code

#define REG_OKAY         0
#define REG_NOMATCH      1
#define REG_BADPAT       2
#define REG_ECOLLATE     3
#define REG_ECTYPE       4
#define REG_EESCAPE      5
#define REG_ESUBREG      6
#define REG_EBRACK       7
#define REG_EPAREN       8
#define REG_EBRACE       9
#define REG_BADBR       10
#define REG_ERANGE      11
#define REG_ESPACE      12
#define REG_BADRPT      13
#define REG_EMPTY       14
#define REG_ASSERT      15
#define REG_INVARG      16

#define REG_ATOI        255     // convert name to number (!)
#define REG_ITOA        0400    // convert number to name (!)

//
// regexp routines
//

#ifdef  __cplusplus
extern "C" {
#endif

int regcomp(regex_t *preg, const char *pattern, int cflags);
int regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags);
size_t regerror(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size);
void regfree(regex_t *preg);

#ifdef  __cplusplus
}
#endif

#endif
