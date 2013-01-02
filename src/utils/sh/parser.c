//
// parser.c
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

#include "sh.h"

static struct expr *parse_arith_expr(struct parser *p);
static void parse_heredocs(struct parser *p);
static int parse_bquoted(struct parser *p);
static int parse_squoted(struct parser *p);
static int parse_dquoted(struct parser *p);
static int parse_gettok(struct parser *p, int tempflags);
static int parse_word(struct parser *p);
static union node *parse_compound_list(struct parser *p);
static union node *parse_list(struct parser *p);

//
// Token table
//

static struct token tokens[] = {
  // Control operators
  { 1, "EOF"    }, // T_EOF     - end of file
  { 0, "NL"     }, // T_NL      - newline
  { 0, ";"      }, // T_SEMI    - semicolon
  { 1, ";;"     }, // T_ENDCASE - end of case
  { 0, "&"      }, // T_BACKGND - background operator
  { 0, "&&"     }, // T_AND     - boolean AND
  { 0, "|"      }, // T_PIPE    - pipe operator
  { 0, "||"     }, // T_OR      - boolean OR
  { 0, "("      }, // T_LP      - left parenthesis (
  { 1, ")"      }, // T_RP      - right parenthesis )
  { 1, "`"      }, // T_BQ      - backquote `
  
  // Special tokens
  { 0, "name"   }, // T_NAME    - name
  { 0, "word"   }, // T_WORD    - word
  { 0, "assign" }, // T_ASSIGN  - assignment
  { 0, "redir"  }, // T_REDIR   - redirection

  // Keyword tokens (sorted)
  { 0, "!"      }, // T_NOT - boolean NOT
  { 0, "case"   }, // T_CASE
  { 0, "do"     }, // T_DO
  { 1, "done"   }, // T_DONE
  { 1, "elif"   }, // T_ELIF
  { 1, "else"   }, // T_ELSE
  { 1, "esac"   }, // T_ESAC
  { 1, "fi"     }, // T_FI
  { 0, "for"    }, // T_FOR
  { 0, "if"     }, // T_IF
  { 0, "in"     }, // T_IN
  { 0, "then"   }, // T_THEN
  { 0, "until"  }, // T_UNTIL
  { 0, "while"  }, // T_WHILE
  { 0, "{"      }, // T_BEGIN
  { 1, "}"      }, // T_END
};

//
// Get next character from source
//

static int next(struct parser *p) {
  p->ch = pgetc(&p->source, 0);
  return p->ch;
}

//
// Peek next character from source
//

static int peek(struct parser *p) {
  return pgetc(&p->source, 1);
}

//
// Allocate new node
//

static union node *alloc_node(struct parser *p, int type) {
  union node *node;

  node = stalloc(p->mark, sizeof(union node));
  node->type = type;
  return node;
}

//
// Save parser state
//

static void push_parser(struct parser *p, struct parserstate *ps, int flags) {
  ps->flags = p->flags;
  ps->pushback = p->pushback;
  ps->quot = p->quot;
  ps->tok = p->tok;
  ps->node = p->node;
  ps->tree = p->tree;

  p->flags = flags;
  p->pushback = 0;
  p->quot = 0;
  p->tok = 0;
  p->node = NULL;
  p->tree = NULL;
}

//
// Restore parser state
//

static void pop_parser(struct parser *p, struct parserstate *ps) {
  p->flags = ps->flags;
  p->pushback = ps->pushback;
  p->quot = ps->quot;
  p->tok = ps->tok;
  p->node = ps->node;
  p->tree = ps->tree;
}

//
// Parser error message
//

static void parse_error(struct parser *p, char *msg) {
  fprintf(stderr, "syntax error: %s", msg);
  if (p->source) fprintf(stderr, ", line %d", p->source->lineno);
  fprintf(stderr, "\n");
  p->errors++;
}

//
// Expect a token, print error msg and return 0 if it wasn't that token
//

static int parse_expect(struct parser *p, int tempflags, int toks) {
  int n, i, f;
  
  if (parse_gettok(p, tempflags) & toks) return p->tok;

  if (p->tok) {
    for (i = 0, n = p->tok; n > 1; i++, n >>= 1);
    fprintf(stderr, "unexpected token '%s', expecting '", tokens[i].name);
  } else {
    fprintf(stderr, "unexpected token, expecting '");
  }
  
  f = 1;
  for (i = 0, n = toks; n > 0; i++, n >>=1) {
    if (n & 1) {
      if (!f) fprintf(stderr, ",");
      fprintf(stderr, "%s", tokens[i].name);
      f = 0;
    }
  }
  fprintf(stderr, "'\n");
  p->errors++;
  return 0;
}

//
// Make an argument node from the stuff parsed by parse_word
//

union node *parse_getarg(struct parser *p) {
  union node *n;
  
  n = alloc_node(p, N_ARG);
  n->narg.list = p->tree;
  p->tree = NULL;
  return n;
}

//
// Skip any unquoted whitespace preceeding a word
//

static int parse_skipspace(struct parser *p) {
  // Skip whitespace
  for (;;) {
    if (p->ch <= 0) return T_EOF;

    if (p->ch == '\n') {
      next(p);
      
      // In a here-doc skip the newline after the delimiter
      if (p->flags & P_HERE) break;

      // Skip leading newlines if requested so
      if (!(p->flags & P_SKIPNL)) return T_NL;
      
      continue;
    } else if (!is_space(p->ch)) {
      break;
    }

    next(p);
  }

  return -1;
}

//
// Parse simple tokens consisting of 1 or 2 chars
//

static int parse_simpletok(struct parser *p) {
  int nextch;

  while (1) {
    switch (p->ch) {
      // Check for end of file
      case -1:
        return T_EOF;

      // Skip whitespace
      case ' ': 
      case '\t':
        next(p);
        continue;

      // Skip comments
      case '#':
        next(p);
        while (p->ch != '\n') {
          if (p->ch < 0) return T_EOF;
          next(p);
        }
        next(p);
        continue;

      // Check for escaped newline (line continuation)
      case '\\':
        nextch = peek(p);
        if (nextch == '\r') {
          next(p);
          nextch = peek(p);
        }
        
        if (nextch == '\n') {
          next(p);
          next(p);
          continue;
        }

        return -1;

      case '\r':
        next(p);
        continue;

      case '\n':
        next(p);
        if (!(p->flags & P_SKIPNL)) return T_NL;
        continue;

      case '|':
        next(p);
        if (p->ch == '|') {
          next(p);
          return T_OR;
        } else {
          return T_PIPE;
        }

      case '&':
        next(p);
        if (p->ch == '&') {
          next(p);
          return T_AND;
        } else {
          return T_BGND;
        }

      case ';':
        next(p);
        if (p->ch == ';') {
          next(p);
          return T_ECASE;
        } else {
          return T_SEMI;
        }

      case '(':
        next(p);
        return T_LP;

      case ')':
        next(p);
        return T_RP;

      case '`':
        if (p->flags & P_BQUOTE) {
          next(p);
          return T_BQ;
        }
        return -1;

      default:
        return -1;
    }
  }
}

//
// Parse a string part of a word and add it to the tree in parse node
//

static void parse_string(struct parser *p, int flags) {
  char *text;
  union node *node;

  // Get text from string buffer
  text = ststr(p->mark);
  if (!text) return;
  
  // Add a new node
  node = alloc_node(p, N_ARGSTR);
  node->nargstr.flags = p->quot | flags;
  node->nargstr.text = text;

  if (p->node == NULL) {
    p->tree = p->node = node;
  } else {
    p->node->list.next = node;
    p->node = node;
  }
}

//
// Parse shell keywords
//

static int parse_keyword(struct parser *p) {
  int len;
  char *str;
  int ti, si;

  len = ststrlen(p->mark);
  str = ststrptr(p->mark);

  if (p->tok != T_NAME) {
    if (len != 1) return 0;
    
    switch (*str) {
      case '{': p->tok = T_BEGIN; return 1;
      case '}': p->tok = T_END; return 1;
      case '!': p->tok = T_NOT; return 1;
    }
    
    return 0;
  }

  // Longest keyword is until/while
  if (len > 5) return 0;
  
  for (ti = TI_CASE; ti <= TI_WHILE; ti++) {
    // Surely not found when char current is below token char
    if (str[0] < tokens[ti].name[0]) break;

    for (si = 0; si <= len; si++) {
      // Keyword match
      if (si == len) {
        if (tokens[ti].name[si] == '\0') {
          p->tok = (1 << ti);
          return 1;
        }
      } else if (str[si] != tokens[ti].name[si]) {
        break;
      }
    }
  }
  
  return 0;
}

//
// Parses a here-doc body, ending at <delim>
//

static int parse_here(struct parser *p, char *delim, int nosubst) {
  int rc = 0;
  int dlen;

  p->tree = NULL;
  p->node = NULL;
  dlen = strlen(delim);

  for (;;) {
    // If nosubst is set we treat it like single-quoted otherwise
    // like double-quoted, allowing parameter and command expansions
    if ((nosubst ? parse_squoted : parse_dquoted)(p)) {
      rc = -1;
      break;
    }
    
    if (p->quot == Q_UNQUOTED) {
      stputc(p->mark, (nosubst ? '\'' : '"'));
      continue;
    }
    
    // When the parser yields an argstr node 
    // we have to check for the delimiter
    parse_string(p, 0);
    if (p->node->type == N_ARGSTR) {
      char *str;
      int slen;

      str = p->node->nargstr.text;
      if (!str) continue;
      slen = strlen(str);
      while (slen > 0 && (str[slen - 1] == '\n' || str[slen - 1] == '\r')) slen--;
      if (slen != dlen) continue;
      if (memcmp(delim, str, dlen) != 0) continue;

      // Ignore last argument with delimiter
      p->node->nargstr.flags |= S_DELIM;
      break;
    }
  }

  return rc;
}

//
// Parse here-docs
//

static void parse_heredocs(struct parser *p) {
  struct parserstate ps;
  char *delim;
  union node *here;
  union node *next;
    
  push_parser(p, &ps, P_HERE);
  here = p->herelist;
  p->herelist = NULL;
  while (here) {
    // Expand the delimiter
    // TODO do real expansion, for now just take first argument
    delim = here->nredir.list->nargstr.text;

    // The next here document is stored in nredir.data
    next = here->nredir.data;

    // When any character of the delimiter has been escaped
    // then treat the whole here-doc as non-expanded word
    parse_here(p, delim, (here->nredir.list->nargstr.flags & S_ESCAPED));
    here->nredir.data = parse_getarg(p);

    // Process next here document
    here = next;
  }

  pop_parser(p, &ps);
}

//
// Parse a redirection word according to 3.7
//
// [n]<operator>word
// 
// input operators: <, <<, <<-, <&, <>
// output operators: >, >>, >|, >, <>
// 

static int parse_redir(struct parser *p, int rf, int fd) {
  // Initialize fd to 0 for input, 1 for output
  if (fd == -1) fd = rf;

  // Get next character
  if (next(p) < 0) return T_EOF;

  // Parse input redirection operator (3.7.1)
  if (rf & R_IN) {
    switch (p->ch) {
      case '<':
        // << is here-doc (3.7.4)
        rf |= R_HERE;
        next(p);

        // <<- means strip leading tabs and trailing whitespace
        if (p->ch == '-') {
          rf |= R_STRIP;
          next(p);
        }

        // Do not subst delimiter for here-docs because
        // here-docs are parsed before any expansion is done
        p->flags |= P_NOSUBST;
        break;

      case '&':
        // <& dups input file descriptor (3.7.5)
        rf |= R_DUP;
        next(p);
        break;

      case '>':
        // <> opens file r/w (3.7.7)
        rf |= R_OUT | R_OPEN;
        next(p);
        break;

      default:
        // < opens input file
        rf |= R_OPEN;
    }
  }

  // Parse output redirection operator (3.7.2)
  if (rf & R_OUT) {
    switch (p->ch) {
      case '&':
        // >& is dup2() (3.7.6)
        rf |= R_DUP;
        next(p);
        break;

      case '>':
        // >> is appending-mode (3.7.3)
        rf |= R_APPEND | R_OPEN;
        next(p);
        break;

      case '|':
        // >| is no-clobbering-mode
        rf |= R_CLOBBER | R_OPEN;
        next(p);
        break;

      default:
        // > opens output file
        rf |= R_OPEN;
    }
  }
  
  if (parse_gettok(p, P_DEFAULT) & (T_NAME | T_WORD)) {
    union node *node;

    node = alloc_node(p, N_REDIR);
    node->nredir.flags = rf;
    node->nredir.fd = fd;
    node->nredir.list = p->tree;
    p->tree = node;

    if (node->nredir.flags & R_HERE) {
      union node **nptr;

      nptr = &p->herelist;
      while (*nptr) nptr = &(*nptr)->nredir.data;
      *nptr = node;
    }

    p->tok = T_REDIR;
    return 1;
  }
  
  return 0;
}

//
// Parse arithmetic expressions
//

static int get_arith_token(struct parser *p) {
  int op;

  while (1) {
    switch (p->ch) {
      // Check for end of file
      case -1:
        return 0;

      // Skip whitespace
      case ' ': 
      case '\t':
        next(p);
        continue;

      case '(':
      case ')':
      case ',':
      case '?':
      case ':':
      case '~':
        op = p->ch;
        next(p);
        return op;

      case '+':
      case '-':
      case '*':
      case '/':
      case '%':
      case '^':
        op = p->ch;
        next(p);
        if (p->ch == '=') {
          next(p);
          return op | A_ASSIGN;
        } else {
          return op;
        }

      case '!':
        next(p);
        if (p->ch == '=') {
          next(p);
          return A_NE;
        } else {
          return '!';
        }

      case '<':
        next(p);
        if (p->ch == '<') {
          next(p);
          if (p->ch == '=') {
            next(p);
            return A_SHL | A_ASSIGN;
          } else {
            return A_SHL;
          }
        } else if (p->ch == '=') {
          next(p);
          return A_LE;
        } else {
          return '<';
        }

      case '>':
        next(p);
        if (p->ch == '>') {
          next(p);
          if (p->ch == '=') {
            next(p);
            return A_SHR | A_ASSIGN;
          } else {
            return A_SHR;
          }
        } else if (p->ch == '=') {
          next(p);
          return A_GE;
        } else {
          return '>';
        }

      case '=':
        next(p);
        if (p->ch == '=') {
          next(p);
          return '=' | A_ASSIGN;
        } else {
          return '=';
        }
        
      case '&':
        next(p);
        if (p->ch == '&') {
          next(p);
          if (p->ch == '=') {
            next(p);
            return A_AND | A_ASSIGN;
          } else {
            return A_AND;
          }
        } else if (p->ch == '=') {
          next(p);
          return '&' | A_ASSIGN;
        } else {
          return '&';
        }

      case '|':
        next(p);
        if (p->ch == '|') {
          next(p);
          if (p->ch == '=') {
            next(p);
            return A_OR | A_ASSIGN;
          } else {
            return A_OR;
          }
        } else if (p->ch == '=') {
          next(p);
          return '|' | A_ASSIGN;
        } else {
          return '|';
        }

      default:
        if (is_digit(p->ch)) return A_NUM;
        return A_VAR;
    }
  }
}

static void nexta(struct parser *p) {
  p->atok = get_arith_token(p);
}

static struct expr *alloc_arith(struct parser *p, int op, struct expr *left, struct expr *right) {
  struct expr *expr;

  expr = stalloc(p->mark, sizeof(struct expr));
  expr->op = op;
  expr->left = left;
  expr->right = right;
  return expr;
}

static struct expr *parse_binary(struct parser *p, int *ops, struct expr *(*term)(struct parser *p)) {
  struct expr *l;
  struct expr *r;

  l = term(p);
  if (!l) return NULL;
  for (;;) {
    int *op = ops;
    while (*op && p->atok != *op) op++;
    if (!*op) break;
    nexta(p);
    r = term(p);
    if (!r) return NULL;
    l = alloc_arith(p, *op, l, r);
  }
  return l;
}

static struct expr *parse_arith_term(struct parser *p) {
  struct expr *e;
  int op;

  switch (p->atok) {
    case '(':
      // Expression in parentheses
      nexta(p);
      e = parse_arith_expr(p);
      if (!e) return NULL;
      if (p->atok != ')') {
        parse_error(p, "unbalanced parentheses in arithmetic expression");
        return NULL;
      }
      nexta(p);
      return e;

    case '+':
      // Unary plus
      nexta(p);
      return parse_arith_expr(p);
      
    case '-':
    case '!':
    case '~':
      // Unary numeric/logical/bitwise negation
      op = p->atok;
      nexta(p);
      e = parse_arith_expr(p);
      if (!e) return NULL;
      return alloc_arith(p, op, NULL, e);
      
    case A_NUM:
      // Numeric constant
      e = alloc_arith(p, A_NUM, NULL, NULL);
      e->num = 0;
      while (p->ch != -1 && is_digit(p->ch)) {
        e->num = e->num * 10 + (p->ch - '0');
        next(p);
      }
      nexta(p);
      return e;

    case A_VAR:
      // Variable identifier
      e = alloc_arith(p, A_VAR, NULL, NULL);
      if (p->ch == '$') next(p);
      while (p->ch != -1 && is_name(p->ch)) {
        stputc(p->mark, p->ch);
        next(p);
      }
      e->var = ststr(p->mark);
      if (!e->var) {
        parse_error(p, "illegal variable name in arithmetic expression");
        return NULL;
      }
      nexta(p);
      return e;
  }

  parse_error(p, "syntax error in arithmetic expression");
  return NULL;
}

static struct expr *parse_arith_mul(struct parser *p) {
  static int ops[] = {'*', '/', '%', 0};
  return parse_binary(p, ops, parse_arith_term);
}

static struct expr *parse_arith_add(struct parser *p) {
  static int ops[] = {'+', '-', 0};
  return parse_binary(p, ops, parse_arith_mul);
}

static struct expr *parse_arith_shift(struct parser *p) {
  static int ops[] = {A_SHL, A_SHR, 0};
  return parse_binary(p, ops, parse_arith_add);
}

static struct expr *parse_arith_compare(struct parser *p) {
  static int ops[] = {'>', '<', A_GE, A_LE, 0};
  return parse_binary(p, ops, parse_arith_shift);
}

static struct expr *parse_arith_equal(struct parser *p) {
  static int ops[] = {A_EQ, A_NE, 0};
  return parse_binary(p, ops, parse_arith_compare);
}

static struct expr *parse_arith_and(struct parser *p) {
  static int ops[] = {'&', 0};
  return parse_binary(p, ops, parse_arith_equal);
}

static struct expr *parse_arith_xor(struct parser *p) {
  static int ops[] = {'^', 0};
  return parse_binary(p, ops, parse_arith_and);
}

static struct expr *parse_arith_or(struct parser *p) {
  static int ops[] = {'|', 0};
  return parse_binary(p, ops, parse_arith_xor);
}

static struct expr *parse_arith_land(struct parser *p) {
  static int ops[] = {A_AND, 0};
  return parse_binary(p, ops, parse_arith_or);
}

static struct expr *parse_arith_lor(struct parser *p) {
  static int ops[] = {A_OR, 0};
  return parse_binary(p, ops, parse_arith_land);
}

static struct expr *parse_arith_condexpr(struct parser *p) {
  struct expr *c;
  struct expr *t;
  struct expr *f;
  
  c = parse_arith_lor(p);
  if (!c) return NULL;
  if (p->atok != '?') return c;
  nexta(p);

  t = parse_arith_lor(p);
  if (p->atok != ':') {
    parse_error(p, "syntax error in arithmetic expression");
    return NULL;
  }
  nexta(p);
  
  f = parse_arith_lor(p);
  if (!f) return NULL;

  return alloc_arith(p, '?', c, alloc_arith(p, ':', t, f)); 
}

static struct expr *parse_arith_assign(struct parser *p) {
  struct expr *l;
  struct expr *r;
  
  l = parse_arith_condexpr(p);
  if (!l) return NULL;
  if (p->atok == '=' || (p->atok & A_ASSIGN)) {
    int op = p->atok;
    nexta(p);
    r = parse_arith_assign(p);
    if (!r) return NULL;
    if (!l->var) {
      parse_error(p, "illegal lvalue in arithmetic expression");
      return NULL;
    }
    l = alloc_arith(p, op, l, r);
  }
  return l;
}

static struct expr *parse_arith_expr(struct parser *p) {
  static int ops[] = {',', 0};
  return parse_binary(p, ops, parse_arith_assign);
}

static int parse_arith(struct parser *p) {
  union node *node;
  struct expr *expr;

  // Parse expression
  nexta(p);
  expr = parse_arith_expr(p);
  if (!expr) return -1;
  
  // Expression must be terminated with ))
  if (p->atok != ')' || get_arith_token(p) != ')') {
    parse_error(p, "unexpected end of arithmetic expression");
    return -1;
  }

  // Add a new arithmetics node
  node = alloc_node(p, N_ARGARITH);
  node->nargarith.expr = expr;

  if (p->node == NULL) {
    p->tree = p->node = node;
  } else {
    p->node->list.next = node;
    p->node = node;
  }

  return 0;
}

//
// Parse parameter substitutions 
//

static int parse_param(struct parser *p) {
  int braces = 0;
  struct parserstate ps;
  union node *node;
  
  if (p->ch == '{') {
    braces++;
    next(p);
  }

  // Link in a new node
  node = alloc_node(p, N_ARGPARAM);
  node->nargparam.flags = p->quot;
  node->nargparam.name = NULL;
  node->nargparam.word = NULL;
  node->nargparam.num = -1;

  if (p->node == NULL) {
    p->tree = p->node = node;
  } else {
    p->node->list.next = node;
    p->node = node;
  }
  
  // If we have # as first char in substitution and we're inside a ${}
  // then check if the next char is a valid parameter char. If so then
  // it's a string length subst.
  if (p->ch == '#' && braces) {
    int ch = peek(p);

    if (ch > 0 && is_param(ch)) {
      node->nargparam.flags |= S_STRLEN;
      next(p);
    }
  }

  // Check for special arguments
  switch (p->ch) {
    case '#': node->nargparam.flags |= S_ARGC; break;
    case '*': node->nargparam.flags |= S_ARGV; break;
    case '@': node->nargparam.flags |= S_ARGVS; break;
    case '?': node->nargparam.flags |= S_EXITCODE; break;
    case '-': node->nargparam.flags |= S_FLAGS; break;
    case '!': node->nargparam.flags |= S_BGEXCODE; break;
    case '$': node->nargparam.flags |= S_PID; break;
  }
  
  if (node->nargparam.flags & S_SPECIAL) {
    stputc(p->mark, p->ch);
    next(p);
  } else {
    // Check if it is numeric, if so assume S_ARG
    if (is_digit(p->ch)) {
      node->nargparam.flags |= S_ARG;
      stputc(p->mark, p->ch);
      next(p);
      
      // Now get the complete parameter number
      if (braces) {
        while (p->ch > 0 && is_digit(p->ch)) {
          stputc(p->mark, p->ch);
          next(p);
        }
      }
    } else {
      // Now get the complete variable name
      while (p->ch > 0 && is_name(p->ch)) {
        stputc(p->mark, p->ch);
        next(p);
      }
    }
  }

  // Get parameter name
  node->nargparam.name = ststr(p->mark);

  // Get parameter number on S_ARG
  if (node->nargparam.flags & S_ARG) node->nargparam.num = atoi(node->nargparam.name);

  // Skip whitespace if inside braces (unusual), otherwise return
  if (!braces) return 0;
  while (p->ch > 0 && is_space(p->ch)) next(p);

  // Done parsing?
  if (p->ch == '}') {
    next(p);
    return 0;
  }
  
  // Check for remove prefix/suffix pattern
  if (p->ch == '%') {
    next(p);
    if (p->ch == '%') {
      node->nargparam.flags |= S_RLSFX;
      next(p);
    } else {
      node->nargparam.flags |= S_RSSFX;
    }
  } else if (p->ch == '#') {
    next(p);
    if (p->ch == '#') {
      node->nargparam.flags |= S_RLPFX;
      next(p);
    } else {
      node->nargparam.flags |= S_RSPFX;
    }
  } else {
    // : before -, =, ?, + means take care of set but null and not only of unset
    if (p->ch == ':') {
      node->nargparam.flags |= S_NULL;
      next(p);
    }
    
    switch (p->ch) {
      case '-': node->nargparam.flags |= S_DEFAULT; next(p); break;
      case '=': node->nargparam.flags |= S_ASGNDEF; next(p); break;
      case '?': node->nargparam.flags |= S_ERRNULL; next(p); break;
      case '+': node->nargparam.flags |= S_ALTERNAT; next(p); break;
    }
  }

  // Parse word
  push_parser(p, &ps, P_SUBSTW);
  parse_word(p);
  node->nargparam.word = parse_getarg(p);
  pop_parser(p, &ps);

  return 0;
}

//
// Parse substitution
//

static int parse_subst(struct parser *p) {
  if (p->ch == '(') {
    return parse_bquoted(p);
  } else if (is_param(p->ch) || p->ch == '{') {
    return parse_param(p);
  }
  
  stputc(p->mark, '$');
  return 0;
}

//
// Parse backquoted commands
//

static int parse_bquoted(struct parser *p) {
  union node *cmdlist;
  union node *node;
  struct parserstate ps;
  int bq;

  if (p->tok == T_NAME) p->tok = T_WORD;
  
  if (p->ch == '(') {
    next(p);
    if (p->ch == '(') {
      next(p);
      return parse_arith(p);
    }
    push_parser(p, &ps, P_DEFAULT);
    bq = 0;
  } else {
    push_parser(p, &ps, P_BQUOTE);
    bq = 1;
    next(p);
  }

  if ((cmdlist = parse_compound_list(p)) == NULL) return -1;

  // MUST be terminated with right parenthesis or backquote
  if (!parse_expect(p, P_DEFAULT, (bq ? T_BQ : T_RP))) return -1;

  // Restore parser state
  pop_parser(p, &ps);

  // Add a new command node
  node = alloc_node(p, N_ARGCMD);
  node->nargcmd.flags = (bq ? S_BQUOTE : 0) | p->quot;
  node->nargcmd.list = cmdlist;

  if (p->node == NULL) {
    p->tree = p->node = node;
  } else {
    p->node->list.next = node;
    p->node = node;
  }
  
  return 0;
}

//
// Parse single-quoted text
//

static int parse_squoted(struct parser *p) {
  if (p->tok == T_NAME) p->tok = T_WORD;
  p->quot = Q_SQUOTED;
  
  for (;;) {
    if (p->ch < 0) return -1;
  
    if (p->ch == '\'') {
      parse_string(p, 0);
      next(p);
      p->quot = Q_UNQUOTED;
      break;
    }
    
    //if (is_esc(p->ch) && !(p->flags & P_HERE)) stputc(p->mark,  '\\');
    stputc(p->mark, p->ch);
    
    if (p->ch == '\n') {
      next(p);
      break;
    }

    next(p);
  }

  parse_string(p, 0);
  return 0;
}

//
// Parse double-quoted text
//

static int parse_dquoted(struct parser *p) {
  if (p->tok == T_NAME) p->tok = T_WORD;
  p->quot = Q_DQUOTED;
  
  for (;;) {
    if (p->ch < 0) return -1;

    // Only ", $ and ` must be escaped
    if (p->ch == '\\') {
      if (next(p) < 0) return -1;
      
      if (is_desc(p->ch)) {
        stputc(p->mark, p->ch);
        next(p);
      } else {
        stputc(p->mark, '\\');
      }
      continue;
    } else if (p->ch == '`') {
      parse_string(p, 0);
      if (parse_bquoted(p)) break;
      continue;
    } else if (p->ch == '$') {
      parse_string(p, 0);
      next(p);
      if (parse_subst(p)) break;
      continue;
    } else if (p->ch == '"') {
      parse_string(p, 0);
      next(p);
      p->quot = Q_UNQUOTED;
      break;
    }
    
    if (is_esc(p->ch) && !(p->flags & P_HERE)) stputc(p->mark, '\\');
    stputc(p->mark, p->ch);
    
    // Return on a newline for the here-doc delimiter check
    if (p->ch == '\n') {
      next(p);
      break;
    }

    next(p);
  }

  parse_string(p, 0);
  return 0;
}

//
// Parse unqouted text
//

static int parse_unquoted(struct parser *p) {
  int flags = 0;

  // Set the quotation mode
  p->quot = Q_UNQUOTED;
  
  for (;;) {
    if (p->ch == '\\') {
      // Everything can be escaped, escape next char
      if (next(p) < 0) return -1;
      p->tok = T_WORD;
      
      // Remember when escaped for here-delimiter
      flags |= S_ESCAPED;
    } else if (p->ch == '"') {
      // Enter double-quotation mode
      parse_string(p, 0);
      next(p);
      p->quot = Q_DQUOTED;
      break;
    } else if (p->ch == '\'') {
      // Enter single-quotation mode
      parse_string(p, 0);
      next(p);
      p->quot = Q_SQUOTED;
      break;
    } else if (p->ch == '`') {
      // Enter command substitution mode
      parse_string(p, 0);
      
      // If we're already parsing backquoted stuff then we should 
      // terminate the current subst instead of creating a new one 
      // inside.
      if  (p->flags & P_BQUOTE) return 1;

      if (parse_bquoted(p)) break;

      continue;
    } else if (p->ch == '$') {
      // Enter parameter substitution mode
      parse_string(p, 0);
      next(p);
      if (parse_subst(p)) break;
      continue;
    } else if (p->ch == '<' || p->ch == '>') {
      // Handle redirections
      int fd = (p->ch == '<' ? 0 : 1);
      if (ststrlen(p->mark) > 0) fd = atoi(ststr(p->mark));
      if (fd != -1) return parse_redir(p, (p->ch == '<' ? R_IN : R_OUT), fd);
    } else if (p->flags & P_SUBSTW) {
      // On a substition word in ${name:word} we parse until a right brace occurs
      if (p->ch == '}') {
        next(p);
        parse_string(p, flags);
        return 1;
      }
    } else if (is_ctrl(p->ch) || is_space(p->ch) || p->ch < 0) {
      // If we're looking for keywords, there is no word tree and 
      // there is a string in the parser we check for keywords
      if ((p->flags & P_NOKEYWD) || p->tree || ststrlen(p->mark) < 0 || !parse_keyword(p)) {
        parse_string(p, flags);
      }
      return 1;
    } else if (is_esc(p->ch)) {
      // If it is a character subject to globbing then set S_GLOB flag
      if (p->tok != T_ASSIGN && !(p->flags & P_NOGLOB)) flags |= S_GLOB;
    }

    if (p->ch < 0) return 0;

    if (p->tok == T_NAME && ststrlen(p->mark) > 0 && p->ch == '=') p->tok = T_ASSIGN;
    if (p->tok == T_NAME && !is_name(p->ch)) p->tok = T_WORD;

    stputc(p->mark, p->ch);
    next(p);
  }

  return 0;
}

//
// Parse a word token
//

static int parse_word(struct parser *p) {
  // Prepare for parsing word token
  p->tree = NULL;
  p->node = NULL;
  p->quot = Q_UNQUOTED;
  p->tok = T_NAME;
  
  for (;;) {
    if (p->ch < 0) break;
    
    switch (p->quot) {
      case Q_DQUOTED:
        if (!parse_dquoted(p)) continue; 
        break;

      case Q_SQUOTED:
        if (!parse_squoted(p)) continue; 
        break;

      default:
        if (!parse_unquoted(p)) continue; 
        break;
    }
    
    break;
  }
  
  parse_string(p, 0);    
  return p->tok;
}

//
// Get a token
//

static int parse_gettok(struct parser *p, int tempflags) {
  int oldflags = p->flags;
  p->flags |= tempflags;

  if (!p->pushback || ((p->flags & P_SKIPNL) && p->tok == T_NL)) {
    // Skip whitespace
    p->tok = parse_skipspace(p);

    // Check for simple tokens first...
    if (p->tok == -1) p->tok = parse_simpletok(p);

    // ...and finally for words
    if (p->tok == -1) p->tok = parse_word(p);
  }
  
  p->flags = oldflags;
  p->pushback = 0;

  if (p->tok == T_NL && p->herelist != NULL) parse_heredocs(p);

  return p->tok;
}

//
// Parse a compound list
//
// The term compound-list is derived from the grammar in 3.10; it is
// equivalent to a sequence of lists, separated by <newline>s, that 
// can be preceded or followed by an arbitrary number of <newline>s.
//

static union node *parse_compound_list(struct parser *p) {
  union node *list;
  union node **nptr;

  list = NULL;
  nptr = &list;

  // Skip arbitrary newlines
  while (parse_gettok(p, P_SKIPNL) & T_NL);
  p->pushback++;

  for (;;) {
    // Try to parse a list
    *nptr = parse_list(p);

    // Skip arbitrary newlines
    while (p->tok & T_NL) parse_gettok(p, P_SKIPNL);
    p->pushback++;

    // No more lists
    if (*nptr == NULL) break;

    // parse_list() already returns a list, so we must 
    // skip over it to get &lastnode->next
    while (*nptr) nptr = &(*nptr)->list.next;
  }

  return list;
}

//
// Parse grouping compound (3.9.4.1)
//
// The format for grouping commands is a s follows:
//
//  (compound-list)       Execute compound-list in a subshell environment;
//                        Variable assignments and built-in commands that
//                        affect the environment shall not remain in effect
//                        after the list finishes.
// 
//  { compound-list;}     Execute compound-list in the current process
//                        environment.
// 

static union node *parse_grouping(struct parser *p) {
  int tok;
  union node **rptr;
  union node *grouping;
  union node *compound_list;

  // Return NULL on empty compound
  grouping = NULL;
  if (!(tok = parse_expect(p, P_DEFAULT, T_BEGIN | T_LP))) return NULL;
 
  // Parse compound content and create a compound node if there are commands
  if ((compound_list = parse_compound_list(p))) {
    grouping = alloc_node(p, tok == T_BEGIN ? N_CMDLIST : N_SUBSHELL);
    grouping->ngrp.cmds = compound_list;
  }

  // Expect the appropriate ending token
  if  (!parse_expect(p, P_DEFAULT, tok == T_BEGIN ? T_END : T_RP)) return NULL;

  if (grouping) {
    rptr = &grouping->ngrp.rdir;

    // Now any redirections may follow
    while (parse_gettok(p, P_DEFAULT) & T_REDIR) {
      *rptr = p->tree;
      rptr = &p->tree->list.next;
    }

    p->pushback++;
  }

  return grouping;
}

//
// Parse function definition (3.9.5)
//

union node *parse_function(struct parser *p, char *name) {
  union node *node;

  // Create function node
  node = alloc_node(p, N_FUNCTION);
  node->nfunc.name = name;

  // Next tokens must be )
  if (!parse_expect(p, P_DEFAULT, T_RP)) return NULL;

  // Parse the function body
  if ((node->nfunc.cmds = parse_grouping(p)) == NULL) return NULL;

  return node;
}

//
// Parse simple command (3.9.1)
//

union node *parse_simple_command(struct parser *p) {
  union node **aptr, *args;
  union node **vptr, *vars;
  union node **rptr, *rdir;
  union node *cmd;
  char *funcname;
  int end;

  args = vars  = rdir = NULL;
  aptr = &args;
  vptr = &vars;
  rptr = &rdir;

  end = 0;
  funcname = NULL;
  while (!end) {
    // Look for assignments only when we have no args yet
    switch (parse_gettok(p, P_DEFAULT)) {
      case T_ASSIGN:
        // Handle variable assignments
        if (!(p->flags & P_NOASSIGN)) {
          *vptr = parse_getarg(p);
          vptr = &(*vptr)->list.next;
          break;
        }

      case T_NAME:
        // Remember name for first argument for function definition
        if (!(p->flags & P_NOKEYWD) && p->node && p->node->type == N_ARGSTR) {
          funcname = p->node->nargstr.text;
        }

      case T_WORD:
        // Handle arguments
        *aptr = parse_getarg(p);
        aptr = &(*aptr)->list.next;
        p->flags |= P_NOASSIGN;
        break;

      case T_LP:
        // Handle function defintion
        p->flags &= ~(P_NOKEYWD | P_NOASSIGN);
        if (funcname) return parse_function(p, funcname);

      case T_REDIR:
        // Handle redirections
        *rptr = p->tree;
        rptr = &(*rptr)->list.next;
        break;

      default:
        // End of command
        p->pushback++;
        end = 1;
    }
    
    // After the first word token we no longer 
    // scan for keywords because a simple command
    // ends with a control operator
    p->flags |= P_NOKEYWD;
  }

  p->flags &= ~(P_NOKEYWD | P_NOASSIGN);
  
  // Add a command node
  cmd = alloc_node(p, N_SIMPLECMD);
  cmd->ncmd.args = args;
  cmd->ncmd.vars = vars;
  cmd->ncmd.rdir = rdir;

  return cmd;
}

//
// Parse if conditional (3.9.4.4)
//

static union node *parse_if(struct parser *p) {
  union node *node;
  union node *list;
  union node **nptr;

  node = NULL;
  nptr = &node;

  // Parse if and elif statements in a loop
  do {
    // Create new N_IF node and parse the test expression
    *nptr = alloc_node(p, N_IF);
    if ((list = parse_compound_list(p)) == NULL) return NULL;
    (*nptr)->nif.test = list;

    // Next token must be a T_THEN
    if (!parse_expect(p, P_DEFAULT, T_THEN)) return NULL;

    // Parse the first command
    if ((list = parse_compound_list(p)) == NULL) return NULL;
    (*nptr)->nif.cmd0 = list;

    // Start a new branch on the else-case
    nptr = &(*nptr)->nif.cmd1;
  }
  while (parse_gettok(p, P_DEFAULT) == T_ELIF);

  // Check if we have an else-block, parse it if so
  if (p->tok == T_ELSE) {
    if ((list = parse_compound_list(p)) == NULL) return NULL;
    *nptr = list;
  } else {
    p->pushback++;
  }

  // Next token must be a T_FI 
  if (!parse_expect(p, P_DEFAULT, T_FI)) return NULL;

  return node;
}

//
// Parse while/until loops (3.9.4.5 and 3.9.4.6)
//

static union node *parse_loop(struct parser *p) {
  union node *node;
  union node *list;

  // Create list node and parse test expression
  node = alloc_node(p, (p->tok == T_WHILE) ? N_WHILE : N_UNTIL);

  // There must be newline or semicolon after the test expression
  if ((list = parse_compound_list(p)) == NULL) return NULL;
  node->nloop.test = list;

  // ... and then a "do" must follow
  if (!parse_expect(p, P_DEFAULT, T_DO)) return NULL;

  // Now parse the loop body
  if ((list = parse_compound_list(p)) == NULL) return NULL;
  node->nloop.cmds = list;

  // ... and then a "done" must follow
  if (!parse_expect(p, P_DEFAULT, T_DONE)) return NULL;

  return node;
}

//
// Parse for loop (3.9.4.2)
//

union node *parse_for(struct parser *p) {
  union node *node = NULL;
  union node **nptr;

  // Next token must be the variable name
  if (!parse_expect(p, P_DEFAULT, T_NAME)) return NULL;

  // Allocate and init N_FOR node
  node = alloc_node(p, N_FOR);
  node->nfor.varn = p->node->nargstr.text;
  
  nptr = &node->nfor.args;

  // Next token can be 'in'
  if (parse_gettok(p, P_DEFAULT) & T_IN) {
    // Now parse the arguments and build a list of them
    while (parse_gettok(p, P_DEFAULT) & (T_WORD | T_NAME | T_ASSIGN)) {
      *nptr = parse_getarg(p);
      nptr = &(*nptr)->list.next;
    }
  }
  p->pushback++;

  // There can be a semicolon after the argument list
  if (!(parse_gettok(p, P_DEFAULT) & T_SEMI)) p->pushback++;

  // ... and the next token must be the "do" keyword
  if (!parse_expect(p, P_SKIPNL, T_DO)) return NULL;

  // Parse the commands inside "do"<->"done"
  node->nfor.cmds = parse_compound_list(p);

  // Next token must be the "done" keyword
  if (!parse_expect(p, P_DEFAULT, T_DONE)) return NULL;

  return node;
}

//
// Parse case statement (3.9.4.3)
//
//  The format for the case construct is as follows.
// 
//        case word in
//             [(]pattern1)          compound-list;;
//             [(]pattern2|pattern3) compound-list;;
//             ...
//        esac
// 
//  The ;; is optional for the last compound-list.
//

static union node *parse_case(struct parser *p) {
  union node *node;
  union node **cptr;
  union node **pptr;
  union node *word;

  // Next tok must be a word
  if (!parse_expect(p, P_DEFAULT, T_WORD | T_NAME | T_ASSIGN)) return NULL;
  word = parse_getarg(p);
  
  // Then the keyword 'in' must follow
  if (!parse_expect(p, P_SKIPNL, T_IN)) return NULL;

  // Create new node and move the word to it
  node = alloc_node(p, N_CASE);
  node->ncase.word = word;

  // Parse the cases
  cptr = &node->ncase.list;
  while (!(parse_gettok(p, P_SKIPNL | P_NOGLOB) & T_ESAC)) {
    // Patterns may be introduced with '('
    if (!(p->tok & T_LP)) p->pushback++;

    *cptr = alloc_node(p, N_CASENODE);
    pptr = &(*cptr)->ncasenode.pats;

    // Parse the pattern list
    while (parse_gettok(p, P_SKIPNL | P_NOGLOB) & (T_WORD | T_NAME | T_ASSIGN)) {
      *pptr = parse_getarg(p);
      pptr = &(*pptr)->list.next;
      if (!(parse_gettok(p, P_DEFAULT) & T_PIPE)) break;
    }
    
    p->pushback++;
    if (!parse_expect(p, P_DEFAULT, T_RP | T_PIPE)) return NULL;

    // Parse the compound list
    (*cptr)->ncasenode.cmds = parse_compound_list(p);

    // Expect esac or ;;
    if (!parse_expect(p, P_DEFAULT, T_ESAC | T_ECASE)) return NULL;

    if (p->tok & T_ESAC) break;

    cptr = &(*cptr)->list.next;
  }
  
  return node;
}

//
// Parse a compound- or a simple-command
//

static union node *parse_command(struct parser *p, int tempflags) {
  int tok;
  union node *command;
  union node **rptr;

  tok = parse_gettok(p, tempflags);
  switch (tok) {
    case T_FOR:
      // T_FOR begins a for-loop statement
      command = parse_for(p);
      break;

    case T_CASE:
      // T_IF begins a case match statement
      command = parse_case(p);
      break;
      
    case T_IF:
      // T_IF begins a conditional statement
      command = parse_if(p);
      break;
      
    case T_WHILE:
    case T_UNTIL:
      // T_WHILE/T_UNTIL begin an iteration statement
      command = parse_loop(p);
      break;
      
    case T_LP:
    case T_BEGIN:
      // T_LP/T_BEGIN start a grouping compound
      p->pushback++;
      command = parse_grouping(p);
      break;

    case T_NAME:
    case T_WORD:
    case T_REDIR:
    case T_ASSIGN:
      // Handle simple commands
      p->pushback++;
      command = parse_simple_command(p);
      break;

    default:
      // It wasn't a compound command, return now
      p->pushback++;
      return NULL;
  }

  if (command && command->type != N_FUNCTION) {
    // They all can have redirections, so parse these now
    rptr = &command->ncmd.rdir;

    //
    // In the case of a simple command there are maybe already
    // redirections in the list (because in a simple command they
    // can appear between arguments), so skip to the end of the list.
    //
    while (*rptr) rptr = &(*rptr)->list.next;

    while (parse_gettok(p, P_DEFAULT) & T_REDIR) {
      *rptr = p->tree;
      rptr = &p->tree->list.next;
    }
    p->pushback++;
  }
  
  return command;
}

//
// Parse a pipeline (3.9.2)
//

static union node *parse_pipeline(struct parser *p) {
  int negate = 0;
  union node *node;
  union node *pipeline;
  union node **cmdptr;
  int tok;

  // On T_NOT toggle negate
  while ((tok = parse_gettok(p, P_DEFAULT)) == T_NOT) negate = !negate;
  p->pushback++;

  if ((node = parse_command(p, P_DEFAULT)) == NULL) return NULL;

  // On a T_PIPE, create a new pipeline
  if ((tok = parse_gettok(p, P_DEFAULT)) == T_PIPE) {
    // Create new pipeline node
    pipeline = alloc_node(p, N_PIPELINE);

    // Create a command list inside the pipeline
    pipeline->npipe.cmds = node;
    cmdptr = &node->ncmd.next;

    // Parse commands and add them to the pipeline 
    // as long as there are pipe tokens
    do {
      if ((node = parse_command(p, P_SKIPNL)) == NULL) {
        parse_error(p, "missing command in pipeline");
        return NULL;
      }
      *cmdptr = node;
      cmdptr = &node->ncmd.next;
    } while (parse_gettok(p, P_DEFAULT) == T_PIPE);

    // Set command to the pipeline
    node = pipeline;
  }

  p->pushback++;

  // Link in a N_NOT node if requested
  if (negate) {
    union node *neg;
    neg = alloc_node(p, N_NOT);
    neg->nandor.cmd0 = node;
    neg->nandor.cmd1 = NULL;
    node = neg;
  }

  return node;
}

//
// Parse a boolean AND-OR list
// 
// An AND-OR-list is a sequence of one or more pipelines separated by the
// operators
// 
//        &&    ||
// 
// Returns NULL if no commands
//

static union node *parse_and_or(struct parser *p) {
  union node *n0;
  union node *n1;
  union node *n2;
  int tok;

  // Parse a command or a pipeline first
  n0 = parse_pipeline(p);
  while (n0) {
    tok = parse_gettok(p, P_DEFAULT);

    // Whether && nor ||, it's not a list, return the pipeline
    if (!(tok & (T_AND | T_OR))) {
      p->pushback++;
      break;
    }

    // There can be a newline after the operator but this isn't 
    // mentioned in the draft text, only in the parser grammar
    while (parse_gettok(p, P_SKIPNL) & T_NL);
    p->pushback++;

    // Try to parse another pipeline
    if ((n1 = parse_pipeline(p)) == NULL) {
      p->pushback++;
      break;
    }

    // Set up a nandor node and continue
    n2 = alloc_node(p, tok == T_AND ? N_AND : N_OR);
    n2->nandor.cmd0 = n0;
    n2->nandor.cmd1 = n1;
    n0 = n2;
  }

  return n0;
}

//
// Parse list (3.9.3)
//
// A list is a sequence of one or more AND-OR-lists separated by the
// operators
//
//       ;    &
// 
// and optionally terminated by
// 
//       ;    &    <newline>
// 
//

static union node *parse_list(struct parser *p) {
  union node *list;
  union node **nptr;
  union node *group;
  int tok;

  // Keep looking for and-or lists
  list = NULL;
  nptr = &list;

  while ((*nptr = parse_and_or(p))) {
    tok = parse_gettok(p, P_DEFAULT);

    // <newline> terminates the list and eats the token
    if (tok & T_NL) break;

    // There must be & or ; after the and-or list, 
    // otherwise the list will be terminated
    if (!(tok & (T_SEMI | T_BGND))) {
      p->pushback++;
      break;
    }

    // & causes async exec of preceding and-or list
    if (tok & T_BGND) (*nptr)->list.flags |= S_BGND;

    // now check for another and-or list
    nptr = &(*nptr)->list.next;
  }

  // Add a command list node if there are multiple elements
  if (!list || !list->list.next) return list;
  group = alloc_node(p, N_CMDLIST);
  group->ngrp.cmds = list;
  return group;
}

//
// Parse next complete command in source
//

union node *parse(struct parser *p) {
  // Parse next command
  while (!(p->tok & T_EOF)) {
    union node *node = parse_list(p);
    if (node) return node;
    if (p->errors) return NULL;
    if (!(p->tok & (T_EOF | T_NL | T_SEMI | T_BGND))) {
      parse_error(p, "unexpected end");
      return NULL;
    }
    p->pushback = 0;
  }
  return NULL;
}

//
// Initialize parser
//

int parse_init(struct parser *p, int flags, struct inputfile *source, struct stkmark *mark) {
  memset(p, 0, sizeof(struct parser));
  p->flags = flags;
  p->source = source;
  p->mark = mark;
  next(p);
  return 0;
}
