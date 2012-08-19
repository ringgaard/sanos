//
//  compiler.c - Tiny C Compiler for Sanos
// 
//  Copyright (c) 2001-2004 Fabrice Bellard
//  Copyright (c) 2011-2012 Michael Ringgaard
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "cc.h"

// Global variables
int rsym, anon_sym, ind, loc;
SValue vstack[VSTACK_SIZE];
SValue *vtop;

int const_wanted;
int nocode_wanted;
int global_expr;

CType func_vt;
int func_vc;
char *func_name;

// Keywords
static const char tcc_keywords[] = 
#define DEF(id, str) str "\0"
#include "tokens.h"
#undef DEF
;

// Parse GNUC __attribute__ extension
void parse_attribute(AttributeDef *ad) {
  int t, n;
  
  while (tok == TOK_ATTRIBUTE1 || tok == TOK_ATTRIBUTE2) {
    next();
    skip('(');
    skip('(');
    while (tok != ')') {
      if (tok < TOK_IDENT) expect("attribute name");
      t = tok;
      next();
      switch (t) {
        case TOK_SECTION1:
        case TOK_SECTION2:
          skip('(');
          if (tok != TOK_STR) expect("section name");
          ad->section = find_section(tcc_state, (char *) tokc.cstr->data);
          next();
          skip(')');
          break;

        case TOK_ALIGNED1:
        case TOK_ALIGNED2:
          if (tok == '(') {
            next();
            n = expr_const();
            if (n <= 0 || (n & (n - 1)) != 0) error("alignment must be a positive power of two");
            skip(')');
          } else {
            n = MAX_ALIGN;
          }
          ad->aligned = n;
          break;

        case TOK_PACKED1:
        case TOK_PACKED2:
          ad->packed = 1;
          break;

        case TOK_UNUSED1:
        case TOK_UNUSED2:
          // Currently, no need to handle it because tcc does not track unused objects
          break;

        case TOK_NORETURN1:
        case TOK_NORETURN2:
          // Currently, no need to handle it because tcc does not track unused objects
          break;

        case TOK_CDECL1:
        case TOK_CDECL2:
        case TOK_CDECL3:
          FUNC_CALL(ad->func_attr) = FUNC_CDECL;
          break;

        case TOK_STDCALL1:
        case TOK_STDCALL2:
        case TOK_STDCALL3:
          FUNC_CALL(ad->func_attr) = FUNC_STDCALL;
          break;

        case TOK_REGPARM1:
        case TOK_REGPARM2:
          skip('(');
          n = expr_const();
          if (n > 3) {
            n = 3;
          } else if (n < 0) {
            n = 0;
          }
          if (n > 0) {
            FUNC_CALL(ad->func_attr) = FUNC_FASTCALL1 + n - 1;
          }
          skip(')');
          break;

        case TOK_FASTCALL1:
        case TOK_FASTCALL2:
        case TOK_FASTCALL3:
          FUNC_CALL(ad->func_attr) = FUNC_FASTCALLW;
          break;            

        case TOK_DLLEXPORT:
          FUNC_EXPORT(ad->func_attr) = 1;
          break;

        default:
          if (tcc_state->warn_unsupported) {
            warning("'%s' attribute ignored", get_tok_str(t, NULL));
          }

          // Skip parameters
          if (tok == '(') {
            int parenthesis = 0;
            do {
              if (tok == '(') {
                parenthesis++;
              } else if (tok == ')') {
                parenthesis--;
              }
              next();
            } while (parenthesis && tok != -1);
          }
      }
      if (tok != ',') break;
      next();
    }
    skip(')');
    skip(')');
  }
}

// Parse MSVC __declspec extension
void parse_declspec(AttributeDef *ad) {
  int t, n;
  
  while (tok == TOK_DECLSPEC) {
    next();
    skip('(');
    if (tok < TOK_IDENT) expect("declspec name");
    t = tok;
    next();
    switch (t) {
      case TOK_DLLEXPORT:
        FUNC_EXPORT(ad->func_attr) = 1;
        break;

      case TOK_DLLIMPORT:
        // Ignore
        break;

      case TOK_NAKED:
        FUNC_NAKED(ad->func_attr) = 1;
        break;

      default:
        if (tcc_state->warn_unsupported) {
          warning("'%s' declspec ignored", get_tok_str(t, NULL));
        }

        // Skip parameters
        if (tok == '(') {
          int parenthesis = 0;
          do {
            if (tok == '(') {
              parenthesis++;
            } else if (tok == ')') {
              parenthesis--;
            }
            next();
          } while (parenthesis && tok != -1);
        }
    }
    skip(')');
  }
}

// Parse type modifers
void parse_modifiers(AttributeDef *ad) {
  if (tok == TOK_ATTRIBUTE1 || tok == TOK_ATTRIBUTE2) parse_attribute(ad);
  if (tok == TOK_DECLSPEC) parse_declspec(ad);

  if (tok == TOK_CDECL1 || tok == TOK_CDECL2 || tok == TOK_CDECL3) {
    FUNC_CALL(ad->func_attr) = FUNC_CDECL;
    next();
  }
  if (tok == TOK_STDCALL1 || tok == TOK_STDCALL2 || tok == TOK_STDCALL3) {
    FUNC_CALL(ad->func_attr) = FUNC_STDCALL;
    next();
  }
  if (tok == TOK_FASTCALL1 || tok == TOK_FASTCALL2 || tok == TOK_FASTCALL3) {
    FUNC_CALL(ad->func_attr) = FUNC_FASTCALLW;
    next();
  }
}

// Structure lookup
Sym *struct_find(int v) {
  v -= TOK_IDENT;
  if ((unsigned) v >= (unsigned) (tok_ident - TOK_IDENT)) return NULL;
  return table_ident[v]->sym_struct;
}

// enum/struct/union declaration. u is either VT_ENUM or VT_STRUCT
void struct_decl(CType *type, int u) {
  int a, v, size, align, maxalign, c, offset;
  int bit_size, bit_pos, bsize, bt, lbit_pos;
  Sym *s, *ss, *ass, **ps;
  AttributeDef ad;
  CType type1, btype;

  a = tok; // Save decl type
  next();
  if (tok != '{') {
    v = tok;
    next();
    // Struct already defined? return it
    if (v < TOK_IDENT) expect("struct/union/enum name");
    s = struct_find(v);
    if (s) {
      if (s->type.t != a) error("invalid type");
      goto do_decl;
    }
  } else {
    v = anon_sym++;
  }
  type1.t = a;

  // We put an undefined size for struct/union
  s = sym_push(v | SYM_STRUCT, &type1, 0, -1);
  s->r = 0; // Default alignment is zero as GCC

  // Put struct/union/enum name in type
 do_decl:
  type->t = u;
  type->ref = s;
  
  if (tok == '{') {
    next();
    if (s->c != -1) error("struct/union/enum already defined");

    // Cannot be empty
    c = 0;

    // Non-empty enums are not allowed
    if (a == TOK_ENUM) {
      for (;;) {
        v = tok;
        if (v < TOK_UIDENT) expect("identifier");
        next();
        if (tok == '=') {
          next();
          c = expr_const();
        }

        // enum symbols have static storage
        ss = sym_push(v, &int_type, VT_CONST, c);
        ss->type.t |= VT_STATIC;
        if (tok != ',') break;
        next();
        c++;
        // NOTE: we accept a trailing comma
        if (tok == '}') break;
      }
      skip('}');
    } else {
      maxalign = 1;
      ps = &s->next;
      bit_pos = 0;
      offset = 0;
      while (tok != '}') {
        parse_btype(&btype, &ad);
        while (1) {
          bit_size = -1;
          v = 0;
          type1 = btype;
          if (tok != ':') {
            type_decl(&type1, &ad, &v, TYPE_DIRECT | TYPE_ABSTRACT);
            if (v == 0 && (type1.t & VT_BTYPE) != VT_STRUCT) expect("identifier");
            if ((type1.t & VT_BTYPE) == VT_FUNC ||
                (type1.t & (VT_TYPEDEF | VT_STATIC | VT_EXTERN | VT_INLINE))) {
              error("invalid type for '%s'", get_tok_str(v, NULL));
            }
          }
          if (tok == ':') {
            next();
            bit_size = expr_const();
            // TODO: handle v = 0 case for messages
            if (bit_size < 0) {
              error("negative width in bit-field '%s'", get_tok_str(v, NULL));
            }
            if (v && bit_size == 0) {
              error("zero width for bit-field '%s'", get_tok_str(v, NULL));
            }
          }

          size = type_size(&type1, &align);
          if (ad.aligned) {
            if (align < ad.aligned) align = ad.aligned;
          } else if (ad.packed) {
            align = 1;
          } else if (*tcc_state->pack_stack_ptr) {
            if (align > *tcc_state->pack_stack_ptr) align = *tcc_state->pack_stack_ptr;
          }

          lbit_pos = 0;
          if (bit_size >= 0) {
            bt = type1.t & VT_BTYPE;
            if (bt != VT_INT && bt != VT_BYTE && bt != VT_SHORT && bt != VT_BOOL && bt != VT_ENUM) {
              error("bitfields must have scalar type");
            }
            bsize = size * 8;
            if (bit_size > bsize) {
              error("width of '%s' exceeds its type", get_tok_str(v, NULL));
            } else if (bit_size == bsize) {
              // No need for bit fields
              bit_pos = 0;
            } else if (bit_size == 0) {
              // TODO: what to do if only padding in a structure?
              // Zero size: means to pad
              if (bit_pos > 0) bit_pos = bsize;
            } else {
              // We do not have enough room?
              if ((bit_pos + bit_size) > bsize) bit_pos = 0;
              lbit_pos = bit_pos;
              // TODO: handle LSB first
              type1.t |= VT_BITFIELD | (bit_pos << VT_STRUCT_SHIFT) | (bit_size << (VT_STRUCT_SHIFT + 6));
              bit_pos += bit_size;
            }
          } else {
            bit_pos = 0;
          }

          if (v != 0 || (type1.t & VT_BTYPE) == VT_STRUCT) {
            // Add new memory data only if starting bit field
            if (lbit_pos == 0) {
              if (a == TOK_STRUCT) {
                c = (c + align - 1) & -align;
                offset = c;
                if (size > 0) c += size;
              } else {
                offset = 0;
                if (size > c) c = size;
              }
              if (align > maxalign) maxalign = align;
            }
          }

          if (v == 0 && (type1.t & VT_BTYPE) == VT_STRUCT) {
            ass = type1.ref;
            while ((ass = ass->next) != NULL) {
              ss = sym_push(ass->v, &ass->type, 0, offset + ass->c);
              *ps = ss;
              ps = &ss->next;
            }
          } else if (v) {
            ss = sym_push(v | SYM_FIELD, &type1, 0, offset);
            *ps = ss;
            ps = &ss->next;
          }

          if (tok == ';' || tok == TOK_EOF) break;
          skip(',');
        }
        skip(';');
      }
      skip('}');
      // Store size and alignment
      s->c = (c + maxalign - 1) & -maxalign; 
      s->r = maxalign;
    }
  }
}

// Return 0 if no type declaration. otherwise, return the basic type and skip it. 
int parse_btype(CType *type, AttributeDef *ad) {
  int t, u, type_found, typespec_found, typedef_found;
  Sym *s;
  CType type1;

  memset(ad, 0, sizeof(AttributeDef));
  type_found = 0;
  typespec_found = 0;
  typedef_found = 0;
  t = 0;
  while (1) {
    switch (tok) {
      case TOK_EXTENSION:
        // Currently, we really ignore extension
        next();
        continue;

      // Basic types
      case TOK_CHAR:
        u = VT_BYTE;
      basic_type:
        next();
      basic_type1:
        if ((t & VT_BTYPE) != 0) error("too many basic types");
        t |= u;
        typespec_found = 1;
        break;

      case TOK_VOID:
        u = VT_VOID;
        goto basic_type;

      case TOK_SHORT:
        u = VT_SHORT;
        goto basic_type;

      case TOK_INT:
        next();
        typespec_found = 1;
        break;

      case TOK_INT64:
        u = VT_LLONG;
        goto basic_type;

      case TOK_LONG:
        next();
        if ((t & VT_BTYPE) == VT_DOUBLE) {
          t = (t & ~VT_BTYPE) | VT_LDOUBLE;
        } else if ((t & VT_BTYPE) == VT_LONG) {
          t = (t & ~VT_BTYPE) | VT_LLONG;
        } else {
          u = VT_LONG;
          goto basic_type1;
        }
        break;

      case TOK_BOOL:
        u = VT_BOOL;
        goto basic_type;

      case TOK_FLOAT:
        u = VT_FLOAT;
        goto basic_type;

      case TOK_DOUBLE:
        next();
        if ((t & VT_BTYPE) == VT_LONG) {
          t = (t & ~VT_BTYPE) | VT_LDOUBLE;
        } else {
          u = VT_DOUBLE;
          goto basic_type1;
        }
        break;

      case TOK_ENUM:
        struct_decl(&type1, VT_ENUM);
      basic_type2:
        u = type1.t;
        type->ref = type1.ref;
        goto basic_type1;

      case TOK_STRUCT:
      case TOK_UNION:
        struct_decl(&type1, VT_STRUCT);
        goto basic_type2;

      // Type modifiers
      case TOK_CONST1:
      case TOK_CONST2:
      case TOK_CONST3:
        t |= VT_CONSTANT;
        next();
        break;

      case TOK_VOLATILE1:
      case TOK_VOLATILE2:
      case TOK_VOLATILE3:
        t |= VT_VOLATILE;
        next();
        break;

      case TOK_SIGNED1:
      case TOK_SIGNED2:
      case TOK_SIGNED3:
        typespec_found = 1;
        t |= VT_SIGNED;
        next();
        break;

      case TOK_REGISTER:
      case TOK_AUTO:
      case TOK_RESTRICT1:
      case TOK_RESTRICT2:
      case TOK_RESTRICT3:
        next();
        break;

      case TOK_UNSIGNED:
        t |= VT_UNSIGNED;
        next();
        typespec_found = 1;
        break;

      // Storage
      case TOK_EXTERN:
        t |= VT_EXTERN;
        next();
        break;

      case TOK_STATIC:
        t |= VT_STATIC;
        next();
        break;

      case TOK_TYPEDEF:
        t |= VT_TYPEDEF;
        next();
        break;

      case TOK_INLINE1:
      case TOK_INLINE2:
      case TOK_INLINE3:
        t |= VT_INLINE | VT_STATIC;
        next();
        break;

      // GNUC attribute
      case TOK_ATTRIBUTE1:
      case TOK_ATTRIBUTE2:
        parse_attribute(ad);
        break;

      case TOK_DECLSPEC:
        parse_declspec(ad);
        break;

      // GNUC typeof
      case TOK_TYPEOF1:
      case TOK_TYPEOF2:
      case TOK_TYPEOF3:
        next();
        parse_expr_type(&type1);
        goto basic_type2;

      default:
        if (typespec_found || typedef_found) goto done;
        s = sym_find(tok);
        if (!s || !(s->type.t & VT_TYPEDEF)) goto done;
        typedef_found = 1;
        t |= (s->type.t & ~VT_TYPEDEF);
        type->ref = s->type.ref;
        next();
        typespec_found = 1;
    }
    type_found = 1;
  }

done:
  if ((t & (VT_SIGNED|VT_UNSIGNED)) == (VT_SIGNED|VT_UNSIGNED)) {
    error("signed and unsigned modifier");
  }
  if (tcc_state->char_is_unsigned) {
    if ((t & (VT_SIGNED|VT_UNSIGNED|VT_BTYPE)) == VT_BYTE) t |= VT_UNSIGNED;
  }
  t &= ~VT_SIGNED;

  // long is never used as type
  if ((t & VT_BTYPE) == VT_LONG) {
    t = (t & ~VT_BTYPE) | VT_INT;
  }
  
  type->t = t;
  return type_found;
}

// Convert a function parameter type (array to pointer and function to function pointer)
void convert_parameter_type(CType *pt) {
  // Remove const and volatile qualifiers (TODO: const could be used to indicate a const function parameter)
  pt->t &= ~(VT_CONSTANT | VT_VOLATILE);

  // Array must be transformed to pointer according to ANSI C
  pt->t &= ~VT_ARRAY;
  if ((pt->t & VT_BTYPE) == VT_FUNC) {
    mk_pointer(pt);
  }
}

void post_type(CType *type, AttributeDef *ad) {
  int n, l, t1, arg_size, align;
  Sym **plast, *s, *first;
  AttributeDef ad1;
  CType pt;

  if (tok == '(') {
    // Function declaration
    next();
    l = 0;
    first = NULL;
    plast = &first;
    arg_size = 0;
    if (tok != ')') {
      for (;;) {
        // Read param name and compute offset
        if (l != FUNC_OLD) {
          if (!parse_btype(&pt, &ad1)) {
            if (l) {
              error("invalid type");
            } else {
              l = FUNC_OLD;
              goto old_proto;
            }
          }
          l = FUNC_NEW;
          if ((pt.t & VT_BTYPE) == VT_VOID && tok == ')') break;
          type_decl(&pt, &ad1, &n, TYPE_DIRECT | TYPE_ABSTRACT);
          if ((pt.t & VT_BTYPE) == VT_VOID) {
            error("parameter declared as void");
          }
          arg_size += (type_size(&pt, &align) + 3) & ~3;
        } else {
        old_proto:
          n = tok;
          if (n < TOK_UIDENT) expect("identifier");
          pt.t = VT_INT;
          next();
        }
        convert_parameter_type(&pt);
        s = sym_push(n | SYM_FIELD, &pt, 0, 0);
        *plast = s;
        plast = &s->next;
        if (tok == ')') break;
        skip(',');
        if (l == FUNC_NEW && tok == TOK_DOTS) {
          l = FUNC_ELLIPSIS;
          next();
          break;
        }
      }
    }

    // If no parameters, then old type prototype
    if (l == 0) l = FUNC_OLD;
    skip(')');
    t1 = type->t & VT_STORAGE;
    // NOTE: const is ignored in returned type as it has a special meaning in gcc / C++
    type->t &= ~(VT_STORAGE | VT_CONSTANT); 
    post_type(type, ad);
    // We push a anonymous symbol which will contain the function prototype
    FUNC_ARGS(ad->func_attr) = arg_size;
    s = sym_push(SYM_FIELD, type, ad->func_attr, l);
    s->next = first;
    type->t = t1 | VT_FUNC;
    type->ref = s;
  } else if (tok == '[') {
    // Array definition
    next();
    n = -1;
    if (tok != ']') {
      n = expr_const();
      if (n < 0) error("invalid array size");    
    }
    skip(']');
    // Parse next post type
    t1 = type->t & VT_STORAGE;
    type->t &= ~VT_STORAGE;
    post_type(type, ad);
    
    // We push a anonymous symbol which will contain the array element type
    s = sym_push(SYM_FIELD, type, 0, n);
    type->t = t1 | VT_ARRAY | VT_PTR;
    type->ref = s;
  }
}

// Parse a type declaration (except basic type), and return the type
// in 'type'. 'td' is a bitmask indicating which kind of type decl is
// expected. 'type' should contain the basic type. 'ad' is the
// attribute definition of the basic type. It can be modified by
// type_decl().
void type_decl(CType *type, AttributeDef *ad, int *v, int td) {
  Sym *s;
  CType type1, *type2;
  int qualifiers;
  
  while (tok == '*') {
    qualifiers = 0;
  redo:
    next();
    switch (tok) {
      case TOK_CONST1:
      case TOK_CONST2:
      case TOK_CONST3:
        qualifiers |= VT_CONSTANT;
        goto redo;

      case TOK_VOLATILE1:
      case TOK_VOLATILE2:
      case TOK_VOLATILE3:
        qualifiers |= VT_VOLATILE;
        goto redo;

      case TOK_RESTRICT1:
      case TOK_RESTRICT2:
      case TOK_RESTRICT3:
        goto redo;
    }
    mk_pointer(type);
    type->t |= qualifiers;
  }

  // TODO: clarify attribute handling
  parse_modifiers(ad);

  // Recursive type
  // TODO: incorrect if abstract type for functions (e.g. 'int ()')
  type1.t = 0; // TODO: same as int
  if (tok == '(') {
    next();
    // TODO: this is not correct to modify 'ad' at this point, but the syntax is not clear
    parse_modifiers(ad);
    type_decl(&type1, ad, v, td);
    skip(')');
  } else {
    // Type identifier
    if (tok >= TOK_IDENT && (td & TYPE_DIRECT)) {
      *v = tok;
      next();
    } else {
      if (!(td & TYPE_ABSTRACT)) expect("identifier");
      *v = 0;
    }
  }
  post_type(type, ad);
  parse_modifiers(ad);
  if (!type1.t) return;

  // Append type at the end of type1
  type2 = &type1;
  for (;;) {
    s = type2->ref;
    type2 = &s->type;
    if (!type2->t) {
      *type2 = *type;
      break;
    }
  }
  *type = type1;
}

// Indirection with full error checking
void indir(void) {
  if ((vtop->type.t & VT_BTYPE) != VT_PTR) {
    if ((vtop->type.t & VT_BTYPE) == VT_FUNC) return;
    expect("pointer");
  }
  if ((vtop->r & VT_LVAL) && !nocode_wanted) gv(RC_INT);
  vtop->type = *pointed_type(&vtop->type);

  // Arrays and functions are never lvalues
  if (!(vtop->type.t & VT_ARRAY) && (vtop->type.t & VT_BTYPE) != VT_FUNC) {
    vtop->r |= lvalue_type(vtop->type.t);
  }
}

// Pass a parameter to a function and do type checking and casting
static void gfunc_param_typed(Sym *func, Sym *arg) {
  int func_type;
  CType type;

  func_type = func->c;
  if (func_type == FUNC_OLD || (func_type == FUNC_ELLIPSIS && arg == NULL)) {
    // Default casting: only need to convert float to double
    if ((vtop->type.t & VT_BTYPE) == VT_FLOAT) {
      type.t = VT_DOUBLE;
      gen_cast(&type);
    }
  } else if (arg == NULL) {
    error("too many arguments to function");
  } else {
    type = arg->type;
    type.t &= ~VT_CONSTANT; // Need to do that to avoid false warning
    gen_assign_cast(&type);
  }
}

// Parse an expression of the form '(type)' or '(expr)' and return its type
void parse_expr_type(CType *type) {
  int n;
  AttributeDef ad;

  skip('(');
  if (parse_btype(type, &ad)) {
    type_decl(type, &ad, &n, TYPE_ABSTRACT);
  } else {
    expr_type(type);
  }
  skip(')');
}

void parse_type(CType *type) {
  AttributeDef ad;
  int n;

  if (!parse_btype(type, &ad)) {
    expect("type");
  }
  type_decl(type, &ad, &n, TYPE_ABSTRACT);
}

// Post defines POST/PRE add. c is the token ++ or --
void inc(int post, int c) {
  test_lvalue();
  vdup(); // Save lvalue
  if (post) {
    gv_dup(); // Duplicate value
    vrotb(3);
    vrotb(3);
  }
  // Add constant
  vpushi(c - TOK_MID);
  gen_op('+');
  vstore(); // Store value
  if (post) vpop(); // If post op, return saved value
}

void unary(void) {
  int n, t, align, size, r;
  CType type;
  Sym *s;
  AttributeDef ad;

 tok_next:
  switch (tok) {
    case TOK_EXTENSION:
      next();
      goto tok_next;

    case TOK_CINT:
    case TOK_CCHAR: 
    case TOK_LCHAR:
      vpushi(tokc.i);
      next();
      break;

    case TOK_CUINT:
      vpush_tokc(VT_INT | VT_UNSIGNED);
      next();
      break;

    case TOK_CLLONG:
      vpush_tokc(VT_LLONG);
      next();
      break;

    case TOK_CULLONG:
      vpush_tokc(VT_LLONG | VT_UNSIGNED);
      next();
      break;

    case TOK_CFLOAT:
      vpush_tokc(VT_FLOAT);
      next();
      break;

    case TOK_CDOUBLE:
      vpush_tokc(VT_DOUBLE);
      next();
      break;

    case TOK_CLDOUBLE:
      vpush_tokc(VT_LDOUBLE);
      next();
      break;

    case TOK___FUNCTION__:
    case TOK___FUNC__: {
      void *ptr;
      int len;
      
      // Special function name identifier
      len = strlen(func_name) + 1;

      // Generate char[len] type
      type.t = VT_BYTE;
      mk_pointer(&type);
      type.t |= VT_ARRAY;
      type.ref->c = len;
      vpush_ref(&type, data_section, data_section->data_offset, len);
      ptr = section_ptr_add(data_section, len);
      memcpy(ptr, func_name, len);
      next();
      break;
    }

    case TOK_LSTR:
      t = VT_SHORT | VT_UNSIGNED;
      goto str_init;

    case TOK_STR:
      // String parsing
      t = VT_BYTE;
    str_init:
      if (tcc_state->warn_write_strings) t |= VT_CONSTANT;
      type.t = t;
      mk_pointer(&type);
      type.t |= VT_ARRAY;
      memset(&ad, 0, sizeof(AttributeDef));
      decl_initializer_alloc(&type, &ad, VT_CONST, 2, 0, 0);
      break;

    case '(':
      next();
      // cast?
      if (parse_btype(&type, &ad)) {
        type_decl(&type, &ad, &n, TYPE_ABSTRACT);
        skip(')');
        // Check ISOC99 compound literal
        if (tok == '{') {
          // Data is allocated locally by default
          if (global_expr) {
            r = VT_CONST;
          } else {
            r = VT_LOCAL;
          }

          // All except arrays are lvalues
          if (!(type.t & VT_ARRAY)) r |= lvalue_type(type.t);
          memset(&ad, 0, sizeof(AttributeDef));
          decl_initializer_alloc(&type, &ad, r, 1, 0, 0);
        } else {
          unary();
          gen_cast(&type);
        }
      } else if (tok == '{') {
        // Save all registers
        save_regs(0);

        // Statement expression: we do not accept break/continue inside as GCC does
        block(NULL, NULL, NULL, NULL, 0, 1);
        skip(')');
      } else {
        gexpr();
        skip(')');
      }
      break;

    case '*':
      next();
      unary();
      indir();
      break;

    case '&':
      next();
      unary();
      // Functions names must be treated as function pointers,
      // except for unary '&' and sizeof. Since we consider that
      // functions are not lvalues, we only have to handle it
      // there and in function calls.
      // Arrays can also be used although they are not lvalues.
      if ((vtop->type.t & VT_BTYPE) != VT_FUNC && !(vtop->type.t & VT_ARRAY) && !(vtop->type.t & VT_LLOCAL)) {
        test_lvalue();
      }
      mk_pointer(&vtop->type);
      gaddrof();
      break;

    case '!':
      next();
      unary();
      if ((vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST) {
        vtop->c.i = !vtop->c.i;
      } else if ((vtop->r & VT_VALMASK) == VT_CMP) {
        vtop->c.i = vtop->c.i ^ 1;
      } else {
        save_regs(1);
        vseti(VT_JMP, gtst(1, 0));
      }
      break;

    case '~':
      next();
      unary();
      vpushi(-1);
      gen_op('^');
      break;

    case '+':
      next();
      // In order to force cast, we add zero
      unary();
      if ((vtop->type.t & VT_BTYPE) == VT_PTR) {
        error("pointer not accepted for unary plus");
      }
      vpushi(0);
      gen_op('+');
      break;

    case TOK_SIZEOF:
    case TOK_ALIGNOF1:
    case TOK_ALIGNOF2:
      t = tok;
      next();
      if (tok == '(') {
        parse_expr_type(&type);
      } else {
        unary_type(&type);
      }
      size = type_size(&type, &align);
      if (t == TOK_SIZEOF) {
        if (size < 0) error("sizeof applied to an incomplete type");
        vpushi(size);
      } else {
        vpushi(align);
      }
      vtop->type.t |= VT_UNSIGNED;
      break;

    case TOK_builtin_types_compatible_p: {
      CType type1, type2;
      next();
      skip('(');
      parse_type(&type1);
      skip(',');
      parse_type(&type2);
      skip(')');
      type1.t &= ~(VT_CONSTANT | VT_VOLATILE);
      type2.t &= ~(VT_CONSTANT | VT_VOLATILE);
      vpushi(are_compatible_types(&type1, &type2));
      break;
    }

    case TOK_builtin_constant_p: {
      int saved_nocode_wanted, res;
      next();
      skip('(');
      saved_nocode_wanted = nocode_wanted;
      nocode_wanted = 1;
      gexpr();
      res = (vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;
      vpop();
      nocode_wanted = saved_nocode_wanted;
      skip(')');
      vpushi(res);
      break;
    }

    case TOK_INC:
    case TOK_DEC:
      t = tok;
      next();
      unary();
      inc(0, t);
      break;

    case '-':
      next();
      vpushi(0);
      unary();
      gen_op('-');
      break;

    case TOK_LAND:
      next();
      // Allow taking the address of a label
      if (tok < TOK_UIDENT) expect("label identifier");
      s = label_find(tok);
      if (!s) {
        s = label_push(&global_label_stack, tok, LABEL_FORWARD);
      } else {
        if (s->r == LABEL_DECLARED) s->r = LABEL_FORWARD;
      }
      if (!s->type.t) {
        s->type.t = VT_VOID;
        mk_pointer(&s->type);
        s->type.t |= VT_STATIC;
      }
      vset(&s->type, VT_CONST | VT_SYM, 0);
      vtop->sym = s;
      next();
      break;

    default:
    tok_identifier:
      t = tok;
      next();
      if (t < TOK_UIDENT) expect("identifier");
      s = sym_find(t);
      if (!s) {
        if (tok != '(') error("'%s' undeclared", get_tok_str(t, NULL));

        // For simple function calls, we tolerate undeclared
        // external reference to int() function
        if (tcc_state->warn_implicit_function_declaration) {
          warning("implicit declaration of function '%s'", get_tok_str(t, NULL));
        }
        s = external_global_sym(t, &func_old_type, 0); 
      }
      if ((s->type.t & (VT_STATIC | VT_INLINE | VT_BTYPE)) == (VT_STATIC | VT_INLINE | VT_FUNC)) {
        // If referencing an inline function, then we generate a
        // symbol to it if not already done. It will have the
        // effect to generate code for it at the end of the
        // compilation unit. Inline function as always
        // generated in the text section.
        if (!s->c) {
          put_extern_sym(s, text_section, 0, 0);
        }
        r = VT_SYM | VT_CONST;
      } else {
        r = s->r;
      }
      vset(&s->type, r, s->c);
      // If forward reference, we must point to s
      if (vtop->r & VT_SYM) {
        vtop->sym = s;
        vtop->c.ul = 0;
      }
  }
  
  // Post operations
  while (1) {
    if (tok == TOK_INC || tok == TOK_DEC) {
      inc(1, tok);
      next();
    } else if (tok == '.' || tok == TOK_ARROW) {
      // Field
      if (tok == TOK_ARROW) indir();
      test_lvalue();
      gaddrof();
      next();
      
      // Expect pointer on structure
      if ((vtop->type.t & VT_BTYPE) != VT_STRUCT) expect("struct or union");
      s = vtop->type.ref;
      
      // Find field
      tok |= SYM_FIELD;
      while ((s = s->next) != NULL) {
        if (s->v == tok) break;
      }
      if (!s) error("field not found: %s",  get_tok_str(tok & ~SYM_FIELD, NULL));
      
      // Add field offset to pointer
      vtop->type = char_pointer_type; // Change type to 'char *'
      vpushi(s->c);
      gen_op('+');

      // Change type to field type, and set to lvalue
      vtop->type = s->type;
      
      // An array is never an lvalue
      if (!(vtop->type.t & VT_ARRAY)) {
        vtop->r |= lvalue_type(vtop->type.t);
      }
      next();
    } else if (tok == '[') {
      next();
      gexpr();
      gen_op('+');
      indir();
      skip(']');
    } else if (tok == '(') {
      SValue ret;
      Sym *sa;
      int nb_args;

      // Function call
      if ((vtop->type.t & VT_BTYPE) != VT_FUNC) {
        // Pointer test (no array accepted)
        if ((vtop->type.t & (VT_BTYPE | VT_ARRAY)) == VT_PTR) {
          vtop->type = *pointed_type(&vtop->type);
          if ((vtop->type.t & VT_BTYPE) != VT_FUNC) goto error_func;
        } else {
        error_func:
          expect("function pointer");
        }
      } else {
        vtop->r &= ~VT_LVAL; // No lvalue
      }

      // Get return type
      s = vtop->type.ref;
      next();
      sa = s->next; // First parameter
      nb_args = 0;
      ret.r2 = VT_CONST;

      // Compute first implicit argument if a structure is returned
      if ((s->type.t & VT_BTYPE) == VT_STRUCT) {
        // Get some space for the returned structure
        size = type_size(&s->type, &align);
        loc = (loc - size) & -align;
        ret.type = s->type;
        ret.r = VT_LOCAL | VT_LVAL;
        // Pass it as 'int' to avoid structure arg passing problems
        vseti(VT_LOCAL, loc);
        ret.c = vtop->c;
        nb_args++;
      } else {
        ret.type = s->type; 
        // Return in register
        if (is_float(ret.type.t)) {
          ret.r = REG_FRET;
        } else {
          if ((ret.type.t & VT_BTYPE) == VT_LLONG) ret.r2 = REG_LRET;
          ret.r = REG_IRET;
        }
        ret.c.i = 0;
      }
      if (tok != ')') {
        for (;;) {
          expr_eq();
          gfunc_param_typed(s, sa);
          nb_args++;
          if (sa) sa = sa->next;
          if (tok == ')') break;
          skip(',');
        }
      }
      if (sa) error("too few arguments to function");
      skip(')');
      if (!nocode_wanted) {
        gfunc_call(nb_args);
      } else {
        vtop -= (nb_args + 1);
      }

      // Return value
      vsetc(&ret.type, ret.r, &ret.c);
      vtop->r2 = ret.r2;
    } else {
      break;
    }
  }
}

void uneq(void) {
  int t;
  
  unary();
  if (tok == '=' ||
      (tok >= TOK_A_MOD && tok <= TOK_A_DIV) ||
      tok == TOK_A_XOR || tok == TOK_A_OR ||
      tok == TOK_A_SHL || tok == TOK_A_SAR) {
    test_lvalue();
    t = tok;
    next();
    if (t == '=') {
      expr_eq();
    } else {
      vdup();
      expr_eq();
      gen_op(t & 0x7f);
    }
    vstore();
  }
}

void expr_prod(void) {
  int t;

  uneq();
  while (tok == '*' || tok == '/' || tok == '%') {
    t = tok;
    next();
    uneq();
    gen_op(t);
  }
}

void expr_sum(void) {
  int t;

  expr_prod();
  while (tok == '+' || tok == '-') {
    t = tok;
    next();
    expr_prod();
    gen_op(t);
  }
}

void expr_shift(void) {
  int t;

  expr_sum();
  while (tok == TOK_SHL || tok == TOK_SAR) {
    t = tok;
    next();
    expr_sum();
    gen_op(t);
  }
}

void expr_cmp(void) {
  int t;

  expr_shift();
  while ((tok >= TOK_ULE && tok <= TOK_GT) || tok == TOK_ULT || tok == TOK_UGE) {
    t = tok;
    next();
    expr_shift();
    gen_op(t);
  }
}

void expr_cmpeq(void) {
  int t;

  expr_cmp();
  while (tok == TOK_EQ || tok == TOK_NE) {
    t = tok;
    next();
    expr_cmp();
    gen_op(t);
  }
}

void expr_and(void) {
  expr_cmpeq();
  while (tok == '&') {
    next();
    expr_cmpeq();
    gen_op('&');
  }
}

void expr_xor(void) {
  expr_and();
  while (tok == '^') {
    next();
    expr_and();
    gen_op('^');
  }
}

void expr_or(void) {
  expr_xor();
  while (tok == '|') {
    next();
    expr_xor();
    gen_op('|');
  }
}

// TODO: fix this mess
void expr_land_const(void) {
  expr_or();
  while (tok == TOK_LAND) {
    next();
    expr_or();
    gen_op(TOK_LAND);
  }
}

// TODO: fix this mess
void expr_lor_const(void) {
  expr_land_const();
  while (tok == TOK_LOR) {
    next();
    expr_land_const();
    gen_op(TOK_LOR);
  }
}

// only used if non constant
void expr_land(void) {
  int t;

  expr_or();
  if (tok == TOK_LAND) {
    t = 0;
    save_regs(1);
    for (;;) {
      t = gtst(1, t);
      if (tok != TOK_LAND) {
        vseti(VT_JMPI, t);
        break;
      }
      next();
      expr_or();
    }
  }
}

void expr_lor(void) {
  int t;

  expr_land();
  if (tok == TOK_LOR) {
    t = 0;
    save_regs(1);
    for (;;) {
      t = gtst(0, t);
      if (tok != TOK_LOR) {
        vseti(VT_JMP, t);
        break;
      }
      next();
      expr_land();
    }
  }
}

// TODO: better constant handling
void expr_eq(void) {
  int tt, u, r1, r2, rc, t1, t2, bt1, bt2;
  SValue sv;
  CType type, type1, type2;

  if (const_wanted) {
    int c1, c;
    expr_lor_const();
    if (tok == '?') {
      c = vtop->c.i;
      vpop();
      next();
      if (tok == ':') {
        c1 = c;
      } else {
        gexpr();
        c1 = vtop->c.i;
        vpop();
      }
      skip(':');
      expr_eq();
      if (c) vtop->c.i = c1;
    }
  } else {
    expr_lor();
    if (tok == '?') {
      next();
      if (vtop != vstack) {
        // Needed to avoid having different registers saved in each branch
        if (is_float(vtop->type.t)) {
          rc = RC_FLOAT;
        } else {
          rc = RC_INT;
        }
        gv(rc);
        save_regs(1);
      }

      if (tok == ':') {
        gv_dup();
        tt = gtst(1, 0);
      } else {
        tt = gtst(1, 0);
        gexpr();
      }

      type1 = vtop->type;
      sv = *vtop; // Save value to handle it later
      vtop--; // No vpop so that FP stack is not flushed
      skip(':');
      u = gjmp(0, 0);
      gsym(tt);
      expr_eq();
      type2 = vtop->type;

      t1 = type1.t;
      bt1 = t1 & VT_BTYPE;
      t2 = type2.t;
      bt2 = t2 & VT_BTYPE;
      // Cast operands to correct type according to ISOC rules
      if (is_float(bt1) || is_float(bt2)) {
        if (bt1 == VT_LDOUBLE || bt2 == VT_LDOUBLE) {
          type.t = VT_LDOUBLE;
        } else if (bt1 == VT_DOUBLE || bt2 == VT_DOUBLE) {
          type.t = VT_DOUBLE;
        } else {
          type.t = VT_FLOAT;
        }
      } else if (bt1 == VT_LLONG || bt2 == VT_LLONG) {
        // Cast to biggest op
        type.t = VT_LLONG;
        // Convert to unsigned if it does not fit in a long long
        if ((t1 & (VT_BTYPE | VT_UNSIGNED)) == (VT_LLONG | VT_UNSIGNED) ||
            (t2 & (VT_BTYPE | VT_UNSIGNED)) == (VT_LLONG | VT_UNSIGNED)) {
          type.t |= VT_UNSIGNED;
        }
      } else if (bt1 == VT_PTR || bt2 == VT_PTR) {
        // TODO: test pointer compatibility
        type = type1;
      } else if (bt1 == VT_FUNC || bt2 == VT_FUNC) {
        // TODO: test function pointer compatibility
        type = type1;
      } else if (bt1 == VT_STRUCT || bt2 == VT_STRUCT) {
        // TODO: test structure compatibility
        type = type1;
      } else if (bt1 == VT_VOID || bt2 == VT_VOID) {
        // NOTE: as an extension, we accept void on only one side
        type.t = VT_VOID;
      } else {
        // Integer operations
        type.t = VT_INT;

        // Convert to unsigned if it does not fit in an integer
        if ((t1 & (VT_BTYPE | VT_UNSIGNED)) == (VT_INT | VT_UNSIGNED) ||
            (t2 & (VT_BTYPE | VT_UNSIGNED)) == (VT_INT | VT_UNSIGNED)) {
          type.t |= VT_UNSIGNED;
        }
      }
        
      // Now we convert second operand
      gen_cast(&type);
      if (VT_STRUCT == (vtop->type.t & VT_BTYPE)) gaddrof();
      rc = RC_INT;
      if (is_float(type.t)) {
        rc = RC_FLOAT;
      } else if ((type.t & VT_BTYPE) == VT_LLONG) {
        // For long longs, we use fixed registers to avoid having to handle a complicated move
        rc = RC_IRET; 
      }
      
      r2 = gv(rc);
      // This is horrible, but we must also convert first operand
      tt = gjmp(0, 0);
      gsym(u);
      // Put again first value and cast it
      *vtop = sv;
      gen_cast(&type);
      if (VT_STRUCT == (vtop->type.t & VT_BTYPE)) gaddrof();
      r1 = gv(rc);
      move_reg(r2, r1);
      vtop->r = r2;
      gsym(tt);
    }
  }
}

void gexpr(void) {
  while (1) {
    expr_eq();
    if (tok != ',') break;
    vpop();
    next();
  }
}

// Parse an expression and return its type without any side effect.
void expr_type(CType *type) {
  int saved_nocode_wanted;

  saved_nocode_wanted = nocode_wanted;
  nocode_wanted = 1;
  gexpr();
  *type = vtop->type;
  vpop();
  nocode_wanted = saved_nocode_wanted;
}

// Parse a unary expression and return its type without any side effect.
void unary_type(CType *type) {
  int a;

  a = nocode_wanted;
  nocode_wanted = 1;
  unary();
  *type = vtop->type;
  vpop();
  nocode_wanted = a;
}

// Parse a constant expression and return value in vtop.
void expr_const1(void) {
  int a;

  a = const_wanted;
  const_wanted = 1;
  expr_eq();
  const_wanted = a;
}

// Parse an integer constant and return its value.
int expr_const(void)
{
  int c;

  expr_const1();
  if ((vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) != VT_CONST) expect("constant expression");
  c = vtop->c.i;
  vpop();
  return c;
}

// Return the label token if current token is a label, otherwise return zero.
int is_label(void) {
  int last_tok;

  // Fast test first
  if (tok < TOK_UIDENT) return 0;

  // No need to save tokc because tok is an identifier
  last_tok = tok;
  next();
  if (tok == ':') {
    next();
    return last_tok;
  } else {
    unget_tok(last_tok);
    return 0;
  }
}

void block(int *bsym, int *csym, int *case_sym, int *def_sym, int case_reg, int is_expr) {
  int a, b, c, d;
  Sym *s;

  // Generate line number info
  if (do_debug && (last_line_num != file->line_num || last_ind != ind)) {
    gline(file->line_num);
    last_ind = ind;
    last_line_num = file->line_num;
  }

  if (is_expr) {
    // Default return value is (void)
    vpushi(0);
    vtop->type.t = VT_VOID;
  }

  if (tok == TOK_IF) {
    // if test
    next();
    skip('(');
    gexpr();
    skip(')');
    a = gtst(1, 0);
    block(bsym, csym, case_sym, def_sym, case_reg, 0);
    c = tok;
    if (c == TOK_ELSE) {
      next();
      d = gjmp(0, 0);
      gsym(a);
      block(bsym, csym, case_sym, def_sym, case_reg, 0);
      gsym(d); // Patch else jmp
    } else {
      gsym(a);
    }
  } else if (tok == TOK_WHILE) {
    next();
    d = glabel();
    skip('(');
    gexpr();
    skip(')');
    a = gtst(1, 0);
    b = 0;
    block(&a, &b, case_sym, def_sym, case_reg, 0);
    gjmp(d, 0);
    gsym(a);
    gsym_at(b, d);
  } else if (tok == '{') {
    Sym *llabel;

    next();
    // Record local declaration stack position
    s = local_stack;
    llabel = local_label_stack;
    // Handle local labels declarations
    if (tok == TOK_LABEL) {
      next();
      for (;;) {
        if (tok < TOK_UIDENT) expect("label identifier");
        label_push(&local_label_stack, tok, LABEL_DECLARED);
        next();
        if (tok == ',') {
          next();
        } else {
          skip(';');
          break;
        }
      }
    }
    while (tok != '}') {
      decl(VT_LOCAL);
      if (tok != '}') {
        if (is_expr) vpop();
        block(bsym, csym, case_sym, def_sym, case_reg, is_expr);
      }
    }

    // Pop locally defined labels
    label_pop(&local_label_stack, llabel);

    // Pop locally defined symbols
    sym_pop(&local_stack, s);
    next();
  } else if (tok == TOK_RETURN) {
    next();
    if (tok != ';') {
      gexpr();
      gen_assign_cast(&func_vt);
      if ((func_vt.t & VT_BTYPE) == VT_STRUCT) {
        CType type;
        // If returning structure, must copy it to implicit first pointer arg location
        type = func_vt;
        mk_pointer(&type);
        vset(&type, VT_LOCAL | VT_LVAL, func_vc);
        indir();
        vswap();
        // Copy structure value to pointer
        vstore();
      } else if (is_float(func_vt.t)) {
        gv(RC_FRET);
      } else {
        gv(RC_IRET);
      }
      vtop--; // NOT vpop() because on x86 it would flush the fp stack
    }
    skip(';');
    rsym = gjmp(rsym, 0); // jmp
  } else if (tok == TOK_BREAK) {
    // Compute jump
    if (!bsym) error("cannot break");
    *bsym = gjmp(*bsym, 0);
    next();
    skip(';');
  } else if (tok == TOK_CONTINUE) {
    // Compute jump
    if (!csym) error("cannot continue");
    *csym = gjmp(*csym, 0);
    next();
    skip(';');
  } else if (tok == TOK_FOR) {
    CodeBuffer cb;
    next();
    skip('(');
    if (tok != ';') {
      gexpr();
      vpop();
    }
    skip(';');
    a = glabel();
    b = 0;
    c = 0;
    if (tok != ';') {
      gexpr();
      b = gtst(1, 0);
    }
    skip(';');
    save_regs(0);
    mark_code_buffer(&cb);
    if (tok != ')') {
      gexpr();
      vpop();
    }
    save_regs(0);
    cut_code_buffer(&cb);
    skip(')');
    block(&b, &c, case_sym, def_sym, case_reg, 0);
    gsym(c);
    save_regs(0);
    paste_code_buffer(&cb);
    gjmp(a, 0);
    gsym(b);
  } else if (tok == TOK_DO) {
    next();
    a = 0;
    b = 0;
    d = glabel();
    block(&a, &b, case_sym, def_sym, case_reg, 0);
    skip(TOK_WHILE);
    skip('(');
    gsym(b);
    gexpr();
    c = gtst(0, 0);
    gsym_at(c, d);
    skip(')');
    gsym(a);
    skip(';');
  } else if (tok == TOK_SWITCH) {
    next();
    skip('(');
    gexpr();
    // TODO: other types than integer
    case_reg = gv(RC_INT);
    vpop();
    skip(')');
    a = 0;
    b = gjmp(0, 0); // Jump to first case
    c = 0;
    block(&a, csym, &b, &c, case_reg, 0);
    // If no default, jmp after switch
    if (c == 0) c = glabel();
    // Default label
    gsym_at(b, c);
    // Break label
    gsym(a);
  } else if (tok == TOK_CASE) {
    int v1, v2;
    if (!case_sym) expect("switch");
    next();
    v1 = expr_const();
    v2 = v1;
    if (tok == TOK_DOTS) {
      next();
      v2 = expr_const();
      if (v2 < v1) warning("empty case range");
    }
    
    // Since a case is like a label, we must skip it with a jmp
    b = gjmp(0, 0);
    gsym(*case_sym);
    vseti(case_reg, 0);
    vpushi(v1);
    if (v1 == v2) {
      gen_op(TOK_EQ);
      *case_sym = gtst(1, 0);
    } else {
      gen_op(TOK_GE);
      *case_sym = gtst(1, 0);
      vseti(case_reg, 0);
      vpushi(v2);
      gen_op(TOK_LE);
      *case_sym = gtst(1, *case_sym);
    }
    gsym(b);
    skip(':');
    is_expr = 0;
    goto block_after_label;
  } else if (tok == TOK_DEFAULT) {
    next();
    skip(':');
    if (!def_sym) expect("switch");
    if (*def_sym) error("too many 'default'");
    *def_sym = glabel();
    is_expr = 0;
    goto block_after_label;
  } else if (tok == TOK_GOTO) {
    next();
    if (tok == '*') {
      // Computed goto
      next();
      gexpr();
      if ((vtop->type.t & VT_BTYPE) != VT_PTR) expect("pointer");
      ggoto();
    } else if (tok >= TOK_UIDENT) {
      s = label_find(tok);
      // Put forward definition if needed
      if (!s) {
        s = label_push(&global_label_stack, tok, LABEL_FORWARD);
      } else {
        if (s->r == LABEL_DECLARED) s->r = LABEL_FORWARD;
      }

      // Label already defined
      if (s->r & LABEL_FORWARD) {
        s->next = (void *) gjmp((long) s->next, 0);
      } else {
        gjmp((long) s->next, 0);
      }
      next();
    } else {
      expect("label identifier");
    }
    skip(';');
  } else if (tok == TOK_ASM2) {
    // Inline assembler with masm syntax
    masm_instr(tcc_state);
  } else if (tok == TOK_ASM1 || tok == TOK_ASM3) {
    // Inline assembler with gas syntax
    asm_instr();
  } else {
    b = is_label();
    if (b) {
      // Label case
      s = label_find(b);
      if (s) {
        if (s->r == LABEL_DEFINED) {
          error("duplicate label '%s'", get_tok_str(s->v, NULL));
        }
        gsym((long) s->next);
        s->r = LABEL_DEFINED;
      } else {
        s = label_push(&global_label_stack, b, LABEL_DEFINED);
      }
      s->next = (void *) glabel();
      // We accept this, but it is a mistake
    block_after_label:
      if (tok == '}') {
        warning("deprecated use of label at end of compound statement");
      } else {
        if (is_expr) vpop();
        block(bsym, csym, case_sym, def_sym, case_reg, is_expr);
      }
    } else {
      // Expression case
      if (tok != ';') {
        if (is_expr) {
          vpop();
          gexpr();
        } else {
          gexpr();
          vpop();
        }
      }
      skip(';');
    }
  }
}

// t is the array or struct type. 
// c is the array or struct address. 
// cur_index/cur_field is the pointer to the current value. 
// 'size_only' is true if only size info is needed (only used in arrays)
void decl_designator(CType *type, Section *sec, unsigned long c, int *cur_index, Sym **cur_field, int size_only) {
  Sym *s, *f;
  int notfirst, index, index_last, align, l, nb_elems, elem_size;
  CType type1;

  notfirst = 0;
  elem_size = 0;
  nb_elems = 1;
  if ((l = is_label()) != 0) goto struct_field;
  while (tok == '[' || tok == '.') {
    if (tok == '[') {
      if (!(type->t & VT_ARRAY)) expect("array type");
      s = type->ref;
      next();
      index = expr_const();
      if (index < 0 || (s->c >= 0 && index >= s->c)) {
        expect("invalid index");
      }
      if (tok == TOK_DOTS) {
        next();
        index_last = expr_const();
        if (index_last < 0 || (s->c >= 0 && index_last >= s->c) ||index_last < index) {
          expect("invalid index");
        }
      } else {
        index_last = index;
      }
      skip(']');
      if (!notfirst) *cur_index = index_last;
      type = pointed_type(type);
      elem_size = type_size(type, &align);
      c += index * elem_size;
      // NOTE: we only support ranges for last designator
      nb_elems = index_last - index + 1;
      if (nb_elems != 1) {
        notfirst = 1;
        break;
      }
    } else {
      next();
      l = tok;
      next();
    struct_field:
      if ((type->t & VT_BTYPE) != VT_STRUCT) expect("struct/union type");
      s = type->ref;
      l |= SYM_FIELD;
      f = s->next;
      while (f) {
        if (f->v == l) break;
        f = f->next;
      }
      if (!f) expect("field");
      if (!notfirst) *cur_field = f;
      // TODO: fix this mess by using explicit storage field
      type1 = f->type;
      type1.t |= (type->t & ~VT_TYPE);
      type = &type1;
      c += f->c;
    }
    notfirst = 1;
  }

  if (notfirst) {
    if (tok == '=') next();
  } else {
    if (type->t & VT_ARRAY) {
      index = *cur_index;
      type = pointed_type(type);
      c += index * type_size(type, &align);
    } else {
      f = *cur_field;
      if (!f) error("too many field init");
      // TODO: fix this mess by using explicit storage field
      type1 = f->type;
      type1.t |= (type->t & ~VT_TYPE);
      type = &type1;
      c += f->c;
    }
  }
  decl_initializer(type, sec, c, 0, size_only);

  // TODO: make it more general
  if (!size_only && nb_elems > 1) {
    unsigned long c_end;
    uint8_t *src, *dst;
    int i;

    if (!sec) error("range init not supported yet for dynamic storage");
    c_end = c + nb_elems * elem_size;
    if (c_end > sec->data_allocated) section_realloc(sec, c_end);
    src = sec->data + c;
    dst = src;
    for (i = 1; i < nb_elems; i++) {
      dst += elem_size;
      memcpy(dst, src, elem_size);
    }
  }
}

#define EXPR_VAL   0
#define EXPR_CONST 1
#define EXPR_ANY   2

// Store a value or an expression directly in global data or in local array
void init_putv(CType *type, Section *sec, unsigned long c, int v, int expr_type) {
  int saved_global_expr, bt, bit_pos, bit_size;
  void *ptr;
  uint64_t bit_mask;
  CType dtype;

  switch (expr_type) {
    case EXPR_VAL:
      vpushi(v);
      break;

    case EXPR_CONST:
      // Compound literals must be allocated globally in this case
      saved_global_expr = global_expr;
      global_expr = 1;
      expr_const1();
      global_expr = saved_global_expr;
      // NOTE: symbols are accepted
      if ((vtop->r & (VT_VALMASK | VT_LVAL)) != VT_CONST) {
        error("initializer element is not constant");
      }
      break;

    case EXPR_ANY:
      expr_eq();
      break;
  }
  
  dtype = *type;
  dtype.t &= ~VT_CONSTANT; // Need to do that to avoid false warning

  if (sec) {
    // TODO: not portable
    // TODO: generate error if incorrect relocation
    gen_assign_cast(&dtype);
    bt = type->t & VT_BTYPE;
    ptr = sec->data + c;
    // TODO: make code faster?
    if (!(type->t & VT_BITFIELD)) {
      bit_pos = 0;
      bit_size = 32;
      bit_mask = INT64_C(-1);
    } else {
      bit_pos = (vtop->type.t >> VT_STRUCT_SHIFT) & 0x3f;
      bit_size = (vtop->type.t >> (VT_STRUCT_SHIFT + 6)) & 0x3f;
      bit_mask = (INT64_C(1) << bit_size) - 1;
    }

    if ((vtop->r & VT_SYM) &&
        (bt == VT_BYTE ||
         bt == VT_SHORT ||
         bt == VT_DOUBLE ||
         bt == VT_LDOUBLE ||
         bt == VT_LLONG ||
         (bt == VT_INT && bit_size != 32))) {
      error("initializer element is not computable at load time");
    }
    
    switch (bt) {
      case VT_BYTE:
        *(char *) ptr |= (vtop->c.i & bit_mask) << bit_pos;
        break;
      case VT_SHORT:
        *(short *) ptr |= (vtop->c.i & bit_mask) << bit_pos;
        break;
      case VT_DOUBLE:
        *(double *) ptr = vtop->c.d;
        break;
      case VT_LDOUBLE:
        *(long double *) ptr = vtop->c.ld;
        break;
      case VT_LLONG:
        *(int64_t *) ptr |= (vtop->c.ll & bit_mask) << bit_pos;
        break;
      default:
        if (vtop->r & VT_SYM) {
          put_reloc(sec, vtop->sym, c, R_386_32);
        }
        *(int *)ptr |= (vtop->c.i & bit_mask) << bit_pos;
        break;
    }
    vtop--;
  } else {
    vset(&dtype, VT_LOCAL|VT_LVAL, c);
    vswap();
    vstore();
    vpop();
  }
}

// Put zeros for variable based init
void init_putz(CType *t, Section *sec, unsigned long c, int size) {
  if (sec) {
    // Nothing to do because globals are already set to zero
  } else {
    vpush_global_sym(&func_old_type, TOK_memset);
    vseti(VT_LOCAL, c);
    vpushi(0);
    vpushi(size);
    gfunc_call(3);
  }
}

// 't' contains the type and storage info. 
// 'c' is the offset of the object in section 'sec'. 
// If 'sec' is NULL, it means stack based allocation. 
// 'first' is true if array '{' must be read (multi dimension implicit array init handling). 
// 'size_only' is true if size only evaluation is wanted (only for arrays).
void decl_initializer(CType *type, Section *sec, unsigned long c, int first, int size_only) {
  int index, array_length, n, no_oblock, nb, parlevel, i;
  int size1, align1, expr_type;
  Sym *s, *f;
  CType *t1;

  if (type->t & VT_ARRAY) {
    s = type->ref;
    n = s->c;
    array_length = 0;
    t1 = pointed_type(type);
    size1 = type_size(t1, &align1);

    no_oblock = 1;
    if ((first && tok != TOK_LSTR && tok != TOK_STR) || 
      tok == '{') {
      skip('{');
      no_oblock = 0;
    }

    // Only parse strings here if correct type (otherwise: handle them as ((w)char *) expressions
    if ((tok == TOK_LSTR && (t1->t & VT_BTYPE) == VT_SHORT && (t1->t & VT_UNSIGNED)) || 
        (tok == TOK_STR && (t1->t & VT_BTYPE) == VT_BYTE)) {
      while (tok == TOK_STR || tok == TOK_LSTR) {
        int cstr_len, ch;
        CString *cstr;

        cstr = tokc.cstr;
        // Compute maximum number of chars wanted
        if (tok == TOK_STR) {
          cstr_len = cstr->size;
        } else {
          cstr_len = cstr->size / sizeof(nwchar_t);
        }
        cstr_len--;
        nb = cstr_len;
        if (n >= 0 && nb > (n - array_length)) {
          nb = n - array_length;
        }
        if (!size_only) {
          if (cstr_len > nb) warning("initializer-string for array is too long");
          // In order to go faster for common case (char string in global variable, we handle it specifically
          if (sec && tok == TOK_STR && size1 == 1) {
            memcpy(sec->data + c + array_length, cstr->data, nb);
          } else {
            for (i = 0; i < nb; i++) {
              if (tok == TOK_STR) {
                ch = ((unsigned char *) cstr->data)[i];
              } else {
                ch = ((nwchar_t *)cstr->data)[i];
              }
              init_putv(t1, sec, c + (array_length + i) * size1, ch, EXPR_VAL);
            }
          }
        }
        array_length += nb;
        next();
      }

      // only add trailing zero if enough storage (no warning in this case since it is standard)
      if (n < 0 || array_length < n) {
        if (!size_only) {
          init_putv(t1, sec, c + (array_length * size1), 0, EXPR_VAL);
        }
        array_length++;
      }
    } else {
      index = 0;
      while (tok != '}') {
        decl_designator(type, sec, c, &index, NULL, size_only);
        if (n >= 0 && index >= n) error("index too large");
        // Must put zero in holes (note that doing it that way ensures that it even works with designators)
        if (!size_only && array_length < index) {
          init_putz(t1, sec, c + array_length * size1, (index - array_length) * size1);
        }
        index++;
        if (index > array_length) array_length = index;
        // Special test for multi dimensional arrays (may not be strictly correct 
        // if designators are used at the same time)
        if (index >= n && no_oblock) break;
        if (tok == '}') break;
        skip(',');
      }
    }

    if (!no_oblock) skip('}');
    // Put zeros at the end
    if (!size_only && n >= 0 && array_length < n) {
      init_putz(t1, sec, c + array_length * size1, (n - array_length) * size1);
    }
    // patch type size if needed
    if (n < 0) s->c = array_length;
  } else if ((type->t & VT_BTYPE) == VT_STRUCT && (sec || !first || tok == '{')) {
    int par_count;

    // NOTE: the previous test is a specific case for automatic struct/union init
    // TODO: union needs only one init

    // TODO: this test is incorrect for local initializers beginning 
    // with ( without {. It would be much more difficult to do it 
    // correctly (ideally, the expression parser should be used in all cases)
    par_count = 0;
    if (tok == '(') {
      AttributeDef ad1;
      CType type1;
      next();
      while (tok == '(') {
        par_count++;
        next();
      }
      if (!parse_btype(&type1, &ad1)) expect("cast");
      type_decl(&type1, &ad1, &n, TYPE_ABSTRACT);
      skip(')');
    }
    no_oblock = 1;
    if (first || tok == '{') {
      skip('{');
      no_oblock = 0;
    }
    s = type->ref;
    f = s->next;
    array_length = 0;
    index = 0;
    n = s->c;
    while (tok != '}') {
      decl_designator(type, sec, c, NULL, &f, size_only);
      index = f->c;
      if (!size_only && array_length < index) {
        init_putz(type, sec, c + array_length, index - array_length);
      }
      index = index + type_size(&f->type, &align1);
      if (index > array_length) array_length = index;
      f = f->next;
      if (no_oblock && f == NULL) break;
      if (tok == '}') break;
      skip(',');
    }
    // Put zeros at the end
    if (!size_only && array_length < n) {
      init_putz(type, sec, c + array_length, n - array_length);
    }
    if (!no_oblock) skip('}');
    while (par_count) {
      skip(')');
      par_count--;
    }
  } else if (tok == '{') {
    next();
    decl_initializer(type, sec, c, first, size_only);
    skip('}');
  } else if (size_only) {
    // Just skip expression
    parlevel = 0;
    while ((parlevel > 0 || (tok != '}' && tok != ',')) && tok != -1) {
      if (tok == '(') {
        parlevel++;
      } else if (tok == ')') {
        parlevel--;
      }
      next();
    }
  } else {
    // Currently, we always use constant expression for globals (may change for scripting case)
    expr_type = EXPR_CONST;
    if (!sec) expr_type = EXPR_ANY;
    init_putv(type, sec, c, 0, expr_type);
  }
}

// Parse an initializer for type 't' if 'has_init' is non zero, and
// allocate space in local or global data space ('r' is either
// VT_LOCAL or VT_CONST). If 'v' is non zero, then an associated
// variable 'v' of scope 'scope' is declared before initializers are
// parsed. If 'v' is zero, then a reference to the new object is put
// in the value stack. If 'has_init' is 2, a special parsing is done
// to handle string constants.
void decl_initializer_alloc(CType *type, AttributeDef *ad, int r, int has_init, int v, int scope) {
  int size, align, addr, data_offset;
  int level;
  ParseState saved_parse_state;
  TokenString init_str;
  Section *sec;

  size = type_size(type, &align);
  // If unknown size, we must evaluate it before evaluating initializers because
  // initializers can generate global data too (e.g. string pointers or ISOC99 
  // compound literals). It also simplifies local initializers handling.
  tok_str_new(&init_str);
  if (size < 0) {
    if (!has_init) error("unknown type size");
    // Get all init string
    if (has_init == 2) {
      // only get strings
      while (tok == TOK_STR || tok == TOK_LSTR) {
        tok_str_add_tok(&init_str);
        next();
      }
    } else {
      level = 0;
      while (level > 0 || (tok != ',' && tok != ';')) {
        if (tok < 0) error("unexpected end of file in initializer");
        tok_str_add_tok(&init_str);
        if (tok == '{') {
          level++;
        } else if (tok == '}') {
          if (level == 0) break;
          level--;
        }
        next();
      }
    }
    tok_str_add(&init_str, -1);
    tok_str_add(&init_str, 0);
    
    // Compute size
    save_parse_state(&saved_parse_state);

    macro_ptr = init_str.str;
    next();
    decl_initializer(type, NULL, 0, 1, 1);

    // Prepare second initializer parsing
    macro_ptr = init_str.str;
    next();
    
    // If still unknown size, error
    size = type_size(type, &align);
    if (size < 0) error("unknown type size");
  }

  // Take into account specified alignment if bigger
  if (ad->aligned) {
    if (ad->aligned > align)
      align = ad->aligned;
  } else if (ad->packed) {
    align = 1;
  }

  if ((r & VT_VALMASK) == VT_LOCAL) {
    sec = NULL;
    loc = (loc - size) & -align;
    addr = loc;
    if (v) {
      // Local variable
      sym_push(v, type, r, addr);
    } else {
      // Push local reference
      vset(type, r, addr);
    }
  } else {
    Sym *sym;

    sym = NULL;
    if (v && scope == VT_CONST) {
      // See if the symbol was already defined
      sym = sym_find(v);
      if (sym) {
        if (!are_compatible_types(&sym->type, type)) {
          error("incompatible types for redefinition of '%s'", get_tok_str(v, NULL));
        }
        if (sym->type.t & VT_EXTERN) {
          // If the variable is extern, it was not allocated
          sym->type.t &= ~VT_EXTERN;

          // Set array size if it was ommited in extern declaration
          if ((sym->type.t & VT_ARRAY) && sym->type.ref->c < 0 && type->ref->c >= 0) {
            sym->type.ref->c = type->ref->c;
          }
        } else {
          // We accept several definitions of the same global variable. 
          // This is tricky, because we must play with the SHN_COMMON 
          // type of the symbol.
          // TODO: should check if the variable was already initialized. 
          // It is incorrect to initialized it twice
          
          // No init data, we won't add more to the symbol
          if (!has_init) goto no_alloc;
        }
      }
    }

    // Allocate symbol in corresponding section
    sec = ad->section;
    if (!sec) {
      if (has_init) {
        sec = data_section;
      } else if (tcc_state->nocommon) {
        sec = bss_section;
      }
    }
    if (sec) {
      data_offset = sec->data_offset;
      data_offset = (data_offset + align - 1) & -align;
      addr = data_offset;
      // Very important to increment global pointer at this time
      // because initializers themselves can create new initializers
      data_offset += size;
      sec->data_offset = data_offset;
      // Allocate section space to put the data
      if (sec->sh_type != SHT_NOBITS && data_offset > sec->data_allocated) {
        section_realloc(sec, data_offset);
      }
      // Align section if needed
      if (align > sec->sh_addralign) {
        sec->sh_addralign = align;
      }
    } else {
      addr = 0;
    }

    if (v) {
      if (scope != VT_CONST || !sym) {
        sym = sym_push(v, type, r | VT_SYM, 0);
      }
      // Update symbol definition
      if (sec) {
        put_extern_sym(sym, sec, addr, size);
      } else {
        Elf32_Sym *esym;
        // Put a common area
        put_extern_sym(sym, NULL, align, size);
        // TODO: find a nicer way
        esym = &((Elf32_Sym *) symtab_section->data)[sym->c];
        esym->st_shndx = SHN_COMMON;
      }
    } else {
      CValue cval;

      // Push global reference
      sym = get_sym_ref(type, sec, addr, size);
      cval.ul = 0;
      vsetc(type, VT_CONST | VT_SYM, &cval);
      vtop->sym = sym;
    }
  }

  if (has_init) {
    decl_initializer(type, sec, addr, 1, 0);
    // Restore parse state if needed
    if (init_str.str) {
      tok_str_free(init_str.str);
      restore_parse_state(&saved_parse_state);
    }
  }
 no_alloc: ;
}

void put_func_debug(Sym *sym) {
  char buf[512];

  // Stabs info
  // TODO: we put a dummy type here
  snprintf(buf, sizeof(buf), "%s:%c1", func_name, sym->type.t & VT_STATIC ? 'f' : 'F');
  put_stabs_r(buf, N_FUN, 0, file->line_num, 0, cur_text_section, sym->c);
  last_ind = 0;
  last_line_num = 0;
}

// Parse an old style function declaration list
// TODO: check multiple parameter
void func_decl_list(Sym *func_sym) {
  AttributeDef ad;
  int v;
  Sym *s;
  CType btype, type;

  // Parse each declaration
  while (tok != '{' && tok != ';' && tok != ',' && tok != TOK_EOF) {
    if (!parse_btype(&btype, &ad)) expect("declaration list");
    if (((btype.t & VT_BTYPE) == VT_ENUM || (btype.t & VT_BTYPE) == VT_STRUCT) && tok == ';') {
      // We accept no variable after
    } else {
      for (;;) {
        type = btype;
        type_decl(&type, &ad, &v, TYPE_DIRECT);
        // Find parameter in function parameter list
        s = func_sym->next;
        while (s != NULL) {
          if ((s->v & ~SYM_FIELD) == v) goto found;
          s = s->next;
        }
        error("declaration for parameter '%s' but no such parameter", get_tok_str(v, NULL));

      found:
        // Check that no storage specifier except 'register' was given
        if (type.t & VT_STORAGE) {
          error("storage class specified for '%s'", get_tok_str(v, NULL));
        }
        convert_parameter_type(&type);
        
        // We can add the type (NOTE: it could be local to the function)
        s->type = type;

        // Accept other parameters
        if (tok == ',') {
          next();
        } else {
          break;
        }
      }
    }
    skip(';');
  }
}

// Parse a function defined by symbol 'sym' and generate its code in 'cur_text_section'
void gen_function(Sym *sym) {
  int func_start, func_size;
  int saved_nocode_wanted = nocode_wanted;
  nocode_wanted = 0;

  // Define function symbol. The function size is patched later.
  func_start = cur_text_section->data_offset;
  func_name = get_tok_str(sym->v, NULL);
  put_extern_sym(sym, cur_text_section, func_start, 0);

  // Put debug symbol
  if (do_debug) put_func_debug(sym);

  // Push a dummy symbol to enable local sym storage
  sym_push2(&local_stack, SYM_FIELD, 0, 0);
  gfunc_prolog(&sym->type);
  rsym = 0;
  block(NULL, NULL, NULL, NULL, 0, 0);
  gsym(rsym);
  gfunc_epilog();
  func_size = cur_text_section->data_offset - func_start;
  label_pop(&global_label_stack, NULL);
  sym_pop(&local_stack, NULL); // Reset local stack

  // Patch symbol size
  ((Elf32_Sym *) symtab_section->data)[sym->c].st_size = func_size;
  if (do_debug) put_stabn(N_FUN, 0, 0, cur_text_section->data_offset - func_start);
  func_name = ""; // For safety
  func_vt.t = VT_VOID; // For safety
  nocode_wanted = saved_nocode_wanted;
}

void gen_inline_functions(void) {
  Sym *sym;
  CType *type;
  int *str, inline_generated;

  // Iterate while inline function are referenced
  for (;;) {
    inline_generated = 0;
    for (sym = global_stack; sym != NULL; sym = sym->prev) {
      type = &sym->type;
      if (((type->t & VT_BTYPE) == VT_FUNC) &&
          (type->t & (VT_STATIC | VT_INLINE)) == (VT_STATIC | VT_INLINE) &&
          sym->c != 0) {
        // The function was used: generate its code and convert it to a normal function
        str = INLINE_DEF(sym->r);
        sym->r = VT_SYM | VT_CONST;
        sym->type.t &= ~VT_INLINE;

        macro_ptr = str;
        next();
        cur_text_section = text_section;
        gen_function(sym);
        macro_ptr = NULL; // Fail safe

        tok_str_free(str);
        inline_generated = 1;
      }
    }
    if (!inline_generated) break;
  }

  // Free all remaining inline function tokens
  for (sym = global_stack; sym != NULL; sym = sym->prev) {
    type = &sym->type;
    if (((type->t & VT_BTYPE) == VT_FUNC) && (type->t & (VT_STATIC | VT_INLINE)) == (VT_STATIC | VT_INLINE)) {
      if (sym->r == (VT_SYM | VT_CONST)) continue;
      str = INLINE_DEF(sym->r);
      tok_str_free(str);
      sym->r = 0; // Fail safe
    }
  }
}

// 'l' is VT_LOCAL or VT_CONST to define default storage type
void decl(int l) {
  int v, has_init, r;
  CType type, btype;
  Sym *sym;
  AttributeDef ad;
  
  while (1) {
    if (!parse_btype(&btype, &ad)) {
      // Skip redundant ';'
      // TODO: find more elegant solution
      if (tok == ';') {
        next();
        continue;
      }
      if (l == VT_CONST && (tok == TOK_ASM1 || tok == TOK_ASM2 || tok == TOK_ASM3)) {
        // Global asm block
        asm_global_instr();
        continue;
      }
      // Special test for old K&R protos without explicit int type. 
      // Only accepted when defining global data
      if (l == VT_LOCAL || tok < TOK_DEFINE) break;
      btype.t = VT_INT;
    }
    
    if (((btype.t & VT_BTYPE) == VT_ENUM || (btype.t & VT_BTYPE) == VT_STRUCT) && tok == ';') {
      // We accept no variable after
      next();
      continue;
    }

    // Iterate thru each declaration
    while (1) { 
      type = btype;
      type_decl(&type, &ad, &v, TYPE_DIRECT);

      if ((type.t & VT_BTYPE) == VT_FUNC) {
        // If old style function prototype, we accept a declaration list
        sym = type.ref;
        if (sym->c == FUNC_OLD) func_decl_list(sym);
      }

      if (tok == '{') {
        if (l == VT_LOCAL) error("cannot use local functions");
        if ((type.t & VT_BTYPE) != VT_FUNC) expect("function definition");

        // Reject abstract declarators in function definition
        sym = type.ref;
        while ((sym = sym->next) != NULL) {
          if (!(sym->v & ~SYM_FIELD)) expect("identifier");
        }

        // TODO: cannot do better now: convert extern line to static inline
        if ((type.t & (VT_EXTERN | VT_INLINE)) == (VT_EXTERN | VT_INLINE)) {
          type.t = (type.t & ~VT_EXTERN) | VT_STATIC;
        }
        
        sym = sym_find(v);
        if (sym) {
          if ((sym->type.t & VT_BTYPE) != VT_FUNC) goto func_error1;

          // Specific case: if not func_call defined, we put the one of the prototype
          // TODO: should have default value
          r = sym->type.ref->r;
          if (FUNC_CALL(r) != FUNC_CDECL && FUNC_CALL(type.ref->r) == FUNC_CDECL) {
            FUNC_CALL(type.ref->r) = FUNC_CALL(r);
          }
          if (FUNC_EXPORT(r)) {
            FUNC_EXPORT(type.ref->r) = 1;
          }

          if (!are_compatible_types(&sym->type, &type)) {
          func_error1:
            error("incompatible types for redefinition of '%s'", get_tok_str(v, NULL));
          }

          // If symbol is already defined, then put complete type
          sym->type = type;
        } else {
          // Put function symbol
          sym = global_identifier_push(v, type.t, 0);
          sym->type.ref = type.ref;
        }

        // Static inline functions are just recorded as a kind of macro. 
        // Their code will be emitted at the end of the compilation unit 
        // only if they are used
        if ((type.t & (VT_INLINE | VT_STATIC)) == (VT_INLINE | VT_STATIC)) {
          TokenString func_str;
          int block_level;

          tok_str_new(&func_str);
          
          block_level = 0;
          for (;;) {
            int t;
            if (tok == TOK_EOF) error("unexpected end of file");
            tok_str_add_tok(&func_str);
            t = tok;
            next();
            if (t == '{') {
              block_level++;
            } else if (t == '}') {
              block_level--;
              if (block_level == 0) break;
            } else if (t == TOK_ASM2 && tok == '{') {
              int saved_flags;

              saved_flags = parse_flags;
              parse_flags = PARSE_FLAG_MASM | PARSE_FLAG_PREPROCESS | PARSE_FLAG_LINEFEED;
              tok_str_add_tok(&func_str);
              next();
              while (tok != '}') {
                if (tok == TOK_EOF) error("unexpected end of file");
                tok_str_add_tok(&func_str);
                next();
              }
              tok_str_add_tok(&func_str);
              next();
              parse_flags = saved_flags;
            }
          }
          tok_str_add(&func_str, -1);
          tok_str_add(&func_str, 0);
          INLINE_DEF(sym->r) = func_str.str;
        } else {
          // Compute text section
          cur_text_section = ad.section;
          if (!cur_text_section) {
            if (!tcc_state->nofll) {
              // Create new text section for function
              CString section_name;
              cstr_new(&section_name);
              cstr_cat(&section_name, ".text_");
              cstr_cat(&section_name, get_tok_str(sym->v, NULL));
              cstr_ccat(&section_name, 0);
              cur_text_section = find_section(tcc_state, section_name.data);
            } else {
              cur_text_section = text_section;
            }
          }
          cur_text_section->sh_flags |= SHF_EXECINSTR;
          sym->r = VT_SYM | VT_CONST;
          gen_function(sym);
        }
        break;
      } else {
        if (btype.t & VT_TYPEDEF) {
          // Save typedefed type
          // TODO: test storage specifiers?
          sym = sym_push(v, &type, 0, 0);
          sym->type.t |= VT_TYPEDEF;
        } else if ((type.t & VT_BTYPE) == VT_FUNC) {
          // External function definition
          // Specific case for func_call attribute
          if (ad.func_attr) type.ref->r = ad.func_attr;
          external_sym(v, &type, 0);
        } else {
          // Not lvalue if array
          r = 0;
          if (!(type.t & VT_ARRAY)) r |= lvalue_type(type.t);
          has_init = (tok == '=');
          if ((btype.t & VT_EXTERN) || ((type.t & VT_ARRAY) && 
              (type.t & VT_STATIC) && !has_init && 
              l == VT_CONST && type.ref->c < 0)) {
            // External variable
            // NOTE: as GCC, uninitialized global static arrays of null size are considered as extern
            external_sym(v, &type, r);
          } else {
            type.t |= (btype.t & VT_STATIC); // Retain "static"
            if (type.t & VT_STATIC) {
              r |= VT_CONST;
            } else {
              r |= l;
            }
            if (has_init) next();
            decl_initializer_alloc(&type, &ad, r, has_init, v, l);
          }
        }
        if (tok != ',') {
          skip(';');
          break;
        }
        next();
      }
    }
  }
}

// Better than nothing, but needs extension to handle '-E' option correctly too
void preprocess_init(TCCState *s1) {
  s1->include_stack_ptr = s1->include_stack;
  // TODO: move that before to avoid having to initialize file->ifdef_stack_ptr?
  s1->ifdef_stack_ptr = s1->ifdef_stack;
  file->ifdef_stack_ptr = s1->ifdef_stack_ptr;

  // TODO: not ANSI compliant: bound checking says error
  vtop = vstack - 1;
  s1->pack_stack[0] = 0;
  s1->pack_stack_ptr = s1->pack_stack;
}

// Compile the C file opened in 'file'. Return non zero if errors.
int tcc_compile(TCCState *s1) {
  Sym *define_start;
  char buf[512];
  volatile int section_sym;

#ifdef INC_DEBUG
  printf("%s: **** new file\n", file->filename);
#endif
  preprocess_init(s1);

  func_name = "";
  anon_sym = SYM_FIRST_ANOM; 

  // File info: full path + filename
  section_sym = 0;
  if (do_debug) {
    section_sym = put_elf_sym(symtab_section, 0, 0, 
                              ELF32_ST_INFO(STB_LOCAL, STT_SECTION), 0, 
                              text_section->sh_num, NULL);
    getcwd(buf, sizeof(buf));
    pstrcat(buf, sizeof(buf), "/");
    put_stabs_r(buf, N_SO, 0, 0, text_section->data_offset, text_section, section_sym);
    put_stabs_r(file->filename, N_SO, 0, 0, text_section->data_offset, text_section, section_sym);
  }

  // An ELF symbol of type STT_FILE must be put so that STB_LOCAL
  // symbols can be safely used
  put_elf_sym(symtab_section, 0, 0, ELF32_ST_INFO(STB_LOCAL, STT_FILE), 0, SHN_ABS, file->filename);

  // Define some often used types
  int_type.t = VT_INT;
  char_pointer_type.t = VT_BYTE;
  mk_pointer(&char_pointer_type);

  func_old_type.t = VT_FUNC;
  func_old_type.ref = sym_push(SYM_FIELD, &int_type, FUNC_CDECL, FUNC_OLD);

  define_start = define_stack;
  nocode_wanted = 1;

  // Compile file
  if (setjmp(s1->error_jmp_buf) == 0) {
    s1->nb_errors = 0;
    s1->error_set_jmp_enabled = 1;

    ch = file->buf_ptr[0];
    tok_flags = TOK_FLAG_BOL | TOK_FLAG_BOF;
    parse_flags = PARSE_FLAG_PREPROCESS | PARSE_FLAG_TOK_NUM;
    next();
    decl(VT_CONST);
    if (tok != TOK_EOF) expect("declaration");

    // End of translation unit info
    if (do_debug) {
      put_stabs_r(NULL, N_SO, 0, 0, text_section->data_offset, text_section, section_sym);
    }
  }
  s1->error_set_jmp_enabled = 0;

  // Reset define stack, but leave -Dsymbols (may be incorrect if
  // they are undefined)
  free_defines(define_start); 

  // Generate inline functions
  gen_inline_functions();

  sym_pop(&global_stack, NULL);

  return s1->nb_errors != 0 ? -1 : 0;
}

// Preprocess the current file
// TODO: add line and file infos, add options to preserve spaces
int tcc_preprocess(TCCState *s1) {
  Sym *define_start;
  int last_is_space;
  
  preprocess_init(s1);

  define_start = define_stack;

  ch = file->buf_ptr[0];
  tok_flags = TOK_FLAG_BOL | TOK_FLAG_BOF;
  parse_flags = PARSE_FLAG_ASM_COMMENTS | PARSE_FLAG_PREPROCESS | PARSE_FLAG_LINEFEED;
  last_is_space = 1;
  next();
  for (;;) {
    if (tok == TOK_EOF) {
      break;
    } else if (tok == TOK_LINEFEED) {
      last_is_space = 1;
    } else {
      if (!last_is_space) fputc(' ', s1->outfile);
      last_is_space = 0;
    }
    fputs(get_tok_str(tok, &tokc), s1->outfile);
    next();
  }
  free_defines(define_start); 
  return 0;
}

int tcc_compile_string(TCCState *s, const char *str) {
  BufferedFile bf1, *bf = &bf1;
  int ret, len;
  char *buf;

  // Init file structure
  bf->fd = -1;
  // TODO: avoid copying
  len = strlen(str);
  buf = tcc_malloc(len + 1);
  if (!buf) return -1;
  memcpy(buf, str, len);
  buf[len] = CH_EOB;
  bf->buf_ptr = buf;
  bf->buf_end = buf + len;
  pstrcpy(bf->filename, sizeof(bf->filename), "<string>");
  bf->line_num = 1;
  file = bf;
  
  ret = tcc_compile(s);
  
  tcc_free(buf);

  // Currently, no need to close
  return ret;
}

// Define a preprocessor symbol. A value can also be provided with the '=' operator
void tcc_define_symbol(TCCState *s1, const char *sym, const char *value) {
  BufferedFile bf1, *bf = &bf1;

  pstrcpy(bf->buffer, IO_BUF_SIZE, sym);
  pstrcat(bf->buffer, IO_BUF_SIZE, " ");

  // Default value
  if (!value) value = "1";
  pstrcat(bf->buffer, IO_BUF_SIZE, value);
  
  // Init file structure
  bf->fd = -1;
  bf->buf_ptr = bf->buffer;
  bf->buf_end = bf->buffer + strlen(bf->buffer);
  *bf->buf_end = CH_EOB;
  bf->filename[0] = '\0';
  bf->line_num = 1;
  file = bf;

  s1->include_stack_ptr = s1->include_stack;

  // Parse with define parser
  ch = file->buf_ptr[0];
  next_nomacro();
  parse_define();
  file = NULL;
}

// Undefine a preprocessor symbol
void tcc_undefine_symbol(TCCState *s1, const char *sym) {
  TokenSym *ts;
  Sym *s;
  ts = tok_alloc(sym, strlen(sym));
  s = define_find(ts->tok);
  // Undefine symbol by putting an invalid name
  if (s) define_undef(s);
}

TCCState *tcc_new(void) {
  const char *p, *r;
  TCCState *s;
  TokenSym *ts;
  int i, c;

  s = tcc_mallocz(sizeof(TCCState));
  if (!s) return NULL;
  tcc_state = s;
  s->output_type = TCC_OUTPUT_EXE;

  // Add all tokens
  table_ident = NULL;
  memset(hash_ident, 0, TOK_HASH_SIZE * sizeof(TokenSym *));
  tok_ident = TOK_IDENT;
  p = tcc_keywords;
  while (*p) {
    r = p;
    for (;;) {
      c = *r++;
      if (c == '\0') break;
    }
    ts = tok_alloc(p, r - p - 1);
    p = r;
  }

  // Add dummy defines for some special macros to speed up tests
  // and to have working defined()
  define_push(TOK___LINE__, MACRO_OBJ, NULL, NULL);
  define_push(TOK___FILE__, MACRO_OBJ, NULL, NULL);
  define_push(TOK___DATE__, MACRO_OBJ, NULL, NULL);
  define_push(TOK___TIME__, MACRO_OBJ, NULL, NULL);

  // Standard defines
  tcc_define_symbol(s, "__STDC__", NULL);
  tcc_define_symbol(s, "__STDC_VERSION__", "199901L");
  tcc_define_symbol(s, "__i386__", NULL);
  tcc_define_symbol(s, "SANOS", NULL);

  // Tiny C specific defines
  tcc_define_symbol(s, "__TINYC__", NULL);
  tcc_define_symbol(s, "_TCC_VER", "\"" TCC_VERSION "\"");

  // Tiny C & gcc defines
  tcc_define_symbol(s, "__SIZE_TYPE__", "unsigned int");
  tcc_define_symbol(s, "__PTRDIFF_TYPE__", "int");
  tcc_define_symbol(s, "__WCHAR_TYPE__", "unsigned short");
  tcc_define_symbol(s, "_INTEGRAL_MAX_BITS", "64");
  tcc_define_symbol(s, "_TCC_PLATFORM", "\"" TCC_PLATFORM "\"");

  // No section zero
  dynarray_add((void ***)&s->sections, &s->nb_sections, NULL);

  // Create standard sections
  text_section = new_section(s, ".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR);
  data_section = new_section(s, ".data", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE);
  bss_section = new_section(s, ".bss", SHT_NOBITS, SHF_ALLOC | SHF_WRITE);

  // Symbols are always generated for linking stage
  symtab_section = new_symtab(s, ".symtab", SHT_SYMTAB, 0, ".strtab", ".hashtab", SHF_PRIVATE);
  strtab_section = symtab_section->link;
  
  // Private symbol table for dynamic symbols
  s->dynsymtab_section = new_symtab(s, ".dynsymtab", SHT_SYMTAB, SHF_PRIVATE, ".dynstrtab", ".dynhashtab", SHF_PRIVATE);
  s->alacarte_link = 1;
  s->imagebase = 0xFFFFFFFF;
  s->filealign = 512;

  return s;
}

void tcc_delete(TCCState *s1) {
  int i, n;

  // Free -D defines
  free_defines(NULL);

  // Free tokens
  n = tok_ident - TOK_IDENT;
  for (i = 0; i < n; i++) tcc_free(table_ident[i]);
  tcc_free(table_ident);

  // Free all sections
  free_section(symtab_section->hash);
  free_section(s1->dynsymtab_section->hash);
  free_section(s1->dynsymtab_section->link);
  free_section(s1->dynsymtab_section);
  for (i = 1; i < s1->nb_sections; i++) free_section(s1->sections[i]);
  tcc_free(s1->sections);
  
  // Free loaded DLLs array
  dynarray_reset(&s1->loaded_dlls, &s1->nb_loaded_dlls);

  // Free library paths
  dynarray_reset(&s1->library_paths, &s1->nb_library_paths);

  // Free include paths
  dynarray_reset(&s1->cached_includes, &s1->nb_cached_includes);
  dynarray_reset(&s1->include_paths, &s1->nb_include_paths);
  dynarray_reset(&s1->sysinclude_paths, &s1->nb_sysinclude_paths);

  tcc_free(s1);
}

int tcc_add_include_path(TCCState *s1, const char *pathname) {
  char *pathname1;
  
  pathname1 = tcc_strdup(pathname);
  dynarray_add((void ***)&s1->include_paths, &s1->nb_include_paths, pathname1);
  return 0;
}

int tcc_add_sysinclude_path(TCCState *s1, const char *pathname) {
  char *pathname1;
  
  pathname1 = tcc_strdup(pathname);
  dynarray_add((void ***)&s1->sysinclude_paths, &s1->nb_sysinclude_paths, pathname1);
  return 0;
}

int tcc_add_file_ex(TCCState *s1, const char *filename, int flags) {
  const char *ext;
  Elf32_Ehdr ehdr;
  int fd, ret;
  BufferedFile *saved_file;

  // Find source file type with extension
  ext = tcc_fileextension(filename);
  if (ext[0]) ext++;

  // Open the file
  saved_file = file;
  file = tcc_open(s1, filename);
  if (!file) {
    if (flags & AFF_PRINT_ERROR) {
      error_noabort("file '%s' not found", filename);
    }
    ret = -1;
    goto fail1;
  }

  if (flags & AFF_PREPROCESS) {
    ret = tcc_preprocess(s1);
  } else if (!ext[0] || !strcmp(ext, "c")) {
    // C file assumed
    ret = tcc_compile(s1);
  } else if (!strcmp(ext, "S")) {
    // Preprocessed assembler
    ret = tcc_assemble(s1, 1);
  } else if (!strcmp(ext, "s")) {
    // Non-preprocessed assembler
    ret = tcc_assemble(s1, 0);
  } else if (!strcmp(ext, "def")) {
    ret = pe_load_def_file(s1, file->fd);
  } else {
    fd = file->fd;
    // Assume executable format: auto guess file type
    ret = read(fd, &ehdr, sizeof(ehdr));
    lseek(fd, 0, SEEK_SET);
    if (ret <= 0) {
      error_noabort("could not read header");
      goto fail;
    } else if (ret != sizeof(ehdr)) {
      goto try_load_script;
    }

    if (ehdr.e_ident[0] == ELFMAG0 && ehdr.e_ident[1] == ELFMAG1 && ehdr.e_ident[2] == ELFMAG2 && ehdr.e_ident[3] == ELFMAG3) {
      file->line_num = 0; // Do not display line number if error
      if (ehdr.e_type == ET_REL) {
        ret = tcc_load_object_file(s1, fd, 0);
      } else if (ehdr.e_type == ET_DYN) {
        ret = tcc_load_dll(s1, fd, filename, (flags & AFF_REFERENCED_DLL) != 0);
      } else {
        error_noabort("unrecognized ELF file");
        goto fail;
      }
    } else if (memcmp((char *) &ehdr, ARMAG, 8) == 0) {
      file->line_num = 0; // Do not display line number if error
      ret = tcc_load_archive(s1, fd);
    } else if (pe_test_res_file(&ehdr, ret)) {
      ret = pe_load_res_file(s1, fd);
    } else {
      // As GNU ld, consider it is an ld script if not recognized
    try_load_script:
      ret = tcc_load_ldscript(s1);
      if (ret < 0) {
        error_noabort("unrecognized file type");
        goto fail;
      }
    }
  }
 cleanup:
  tcc_close(file);
 fail1:
  file = saved_file;
  return ret;
 fail:
  ret = -1;
  goto cleanup;
}

int tcc_add_file(TCCState *s, const char *filename) {
  return tcc_add_file_ex(s, filename, AFF_PRINT_ERROR);
}

int tcc_add_library_path(TCCState *s, const char *pathname) {
  char *pathname1;
  
  pathname1 = tcc_strdup(pathname);
  dynarray_add((void ***)&s->library_paths, &s->nb_library_paths, pathname1);
  return 0;
}

// Find and load a dll. Return non zero if not found 
// TODO: add '-rpath' option support?
int tcc_add_dll(TCCState *s, const char *filename, int flags) {
  char buf[1024];
  int i;

  for (i = 0; i < s->nb_library_paths; i++) {
    snprintf(buf, sizeof(buf), "%s/%s", s->library_paths[i], filename);
    if (tcc_add_file_ex(s, buf, flags) == 0) return 0;
  }
  return -1;
}

// The library name is the same as the argument of the '-l' option
int tcc_add_library(TCCState *s, const char *libraryname) {
  char buf[1024];
  int i;
  
  // First we look for the dynamic library if not static linking
  if (!s->static_link) {
    snprintf(buf, sizeof(buf), "%s.def", libraryname);
    if (tcc_add_dll(s, buf, 0) == 0) return 0;
  }

  // Then we look for the static library
  for (i = 0; i < s->nb_library_paths; i++) {
    snprintf(buf, sizeof(buf), "%s/lib%s.a", s->library_paths[i], libraryname);
    if (tcc_add_file_ex(s, buf, 0) == 0) return 0;
  }
  return -1;
}

int tcc_add_symbol(TCCState *s, const char *name, unsigned long val) {
  add_elf_sym(symtab_section, val, 0, 
              ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE), 0,
              SHN_ABS, name);
  return 0;
}

int tcc_set_output_type(TCCState *s, int output_type) {
  char buf[1024];

  s->output_type = output_type;

  if (!s->nostdinc) {
    // Default include paths
    // TODO: reverse order needed if -isystem support
    snprintf(buf, sizeof(buf), "%s/include", tcc_lib_path);
    tcc_add_sysinclude_path(s, buf);
  }

  if (s->char_is_unsigned) {
    tcc_define_symbol(s, "__CHAR_UNSIGNED__", NULL);
  }

  // Add debug sections
  if (do_debug) {
    // Stab symbols
    stab_section = new_section(s, ".stab", SHT_PROGBITS, 0);
    stab_section->sh_entsize = sizeof(Stab_Sym);
    stabstr_section = new_section(s, ".stabstr", SHT_STRTAB, 0);
    put_elf_str(stabstr_section, "");
    stab_section->link = stabstr_section;
    // Put first entry
    put_stabs("", 0, 0, 0, 0);
  }

  snprintf(buf, sizeof(buf), "%s/lib", tcc_lib_path);
  tcc_add_library_path(s, buf);

  return 0;
}

