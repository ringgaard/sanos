//
// parser.h
//
// Shell command parser
//
// Copyright (C) 2011 Michael Ringgaard. All rights reserved.
//
// This code is derived from software contributed to Berkeley by
// Kenneth Almquist.
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

#ifndef PARSER_H
#define PARSER_H

//
// Token table
//

struct token {
  int eol;            // End of list flag
  const char *name;   // Token name
};

extern struct token tokens[];

//
// Token numbers
//

#define TI_EOF     0 // End of file
#define TI_NL      1 // Newline
#define TI_SEMI    2 // Semicolon
#define TI_ECASE   3 // End of case (double-semicolon)
#define TI_BGND    4 // Background operator (&)
#define TI_AND     5 // Boolean AND (&&)
#define TI_PIPE    6 // Pipe operator (|)
#define TI_OR      7 // Boolean OR (||)
#define TI_LP      8 // Left parenthesis (
#define TI_RP      9 // Right parenthesis )
#define TI_BQ     10 // Backquote `
#define TI_NAME   11 // Word token is a valid name
#define TI_WORD   12 // Word (to be expanded)
#define TI_ASSIGN 13 // Word + token is a valid assignment in the form name=[word]
#define TI_REDIR  14 // Redirection
#define TI_NOT    15 // BANG !
#define TI_CASE   16
#define TI_DO     17
#define TI_DONE   18
#define TI_ELIF   19
#define TI_ELSE   20
#define TI_ESAC   21
#define TI_FI     22
#define TI_FOR    23
#define TI_IF     24
#define TI_IN     25
#define TI_THEN   26
#define TI_UNTIL  27
#define TI_WHILE  28
#define TI_BEGIN  29 // Left brace {
#define TI_END    30 // Right brace }

//
// Token  flags
//

#define T_EOF    (1 << TI_EOF)
#define T_NL     (1 << TI_NL)
#define T_SEMI   (1 << TI_SEMI)
#define T_ECASE  (1 << TI_ECASE)
#define T_BGND   (1 << TI_BGND)
#define T_AND    (1 << TI_AND)
#define T_PIPE   (1 << TI_PIPE)
#define T_OR     (1 << TI_OR)
#define T_LP     (1 << TI_LP)
#define T_RP     (1 << TI_RP)
#define T_BQ     (1 << TI_BQ)
#define T_REDIR  (1 << TI_REDIR)
#define T_NAME   (1 << TI_NAME)
#define T_WORD   (1 << TI_WORD)
#define T_ASSIGN (1 << TI_ASSIGN)
#define T_NOT    (1 << TI_NOT)
#define T_CASE   (1 << TI_CASE)
#define T_DO     (1 << TI_DO)
#define T_DONE   (1 << TI_DONE)
#define T_ELIF   (1 << TI_ELIF)
#define T_ELSE   (1 << TI_ELSE)
#define T_ESAC   (1 << TI_ESAC)
#define T_FI     (1 << TI_FI)
#define T_FOR    (1 << TI_FOR)
#define T_IF     (1 << TI_IF)
#define T_IN     (1 << TI_IN)
#define T_THEN   (1 << TI_THEN)
#define T_UNTIL  (1 << TI_UNTIL)
#define T_WHILE  (1 << TI_WHILE)
#define T_BEGIN  (1 << TI_BEGIN)
#define T_END    (1 << TI_END)

//
// Quoting
//

#define Q_UNQUOTED 0
#define Q_DQUOTED  1
#define Q_SQUOTED  2
#define Q_BQUOTED  3

//
// Parser flags
//

#define P_DEFAULT  0x0000
#define P_IACTIVE  0x0001
#define P_NOKEYWD  0x0002 // Do not recognize keyowrds
#define P_NOASSIGN 0x0004 // Don't parse assignments
#define P_NOREDIR  0x0008 // Don't parse redirections
#define P_SKIPNL   0x0040 // Skip newlines
#define P_SUBSTW   0x0100 // Word is inside a var, so its terminated with }
#define P_BQUOTE   0x0800 // Backquoted mode, delimit words on unesc'd bquotes
#define P_NOSUBST  0x1000 // Do not create substitution nodes
#define P_HERE     0x2000 // Parse here-doc
#define P_NOGLOB   0x4000 // No glob pattern expansion

//
// Parser context
//

struct parserstate {
  int flags;
  int pushback;
  int quot;
  int tok;
  union node *node;
  union node *tree;
};

struct parser {
  int flags;
  int pushback;
  int quot;
  int tok;
  int atok;
  union node *node;
  union node *tree;
  union node *herelist;

  struct stkmark *mark;
  struct inputfile *source;
  int ch;
  int errors;
};

int parse_init(struct parser *p, int flags, struct inputfile *source, struct stkmark *mark);
union node *parse(struct parser *p);

#endif
