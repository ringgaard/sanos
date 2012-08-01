//
//  type.c - Tiny C Compiler for Sanos
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

// Some predefined types
CType char_pointer_type, func_old_type, int_type;

// Returns true if float/double/long double type
int is_float(int t) {
  int bt;
  bt = t & VT_BTYPE;
  return bt == VT_LDOUBLE || bt == VT_DOUBLE || bt == VT_FLOAT;
}

void test_lvalue(void) {
  if (!(vtop->r & VT_LVAL)) expect("lvalue");
}

// Compute the lvalue VT_LVAL_xxx needed to match type t.
int lvalue_type(int t) {
  int bt, r;
  r = VT_LVAL;
  bt = t & VT_BTYPE;
  if (bt == VT_BYTE || bt == VT_BOOL) {
    r |= VT_LVAL_BYTE;
  } else if (bt == VT_SHORT) {
    r |= VT_LVAL_SHORT;
  } else {
    return r;
  }
  if (t & VT_UNSIGNED) r |= VT_LVAL_UNSIGNED;
  return r;
}

// Modify type so that its it is a pointer to type
void mk_pointer(CType *type) {
  Sym *s;
  s = sym_push(SYM_FIELD, type, 0, -1);
  type->t = VT_PTR | (type->t & ~VT_TYPE);
  type->ref = s;
}

// Return the pointed type of t
CType *pointed_type(CType *type) {
  return &type->ref->type;
}

int is_null_pointer(SValue *p) {
  if ((p->r & (VT_VALMASK | VT_LVAL | VT_SYM)) != VT_CONST) return 0;
  return ((p->type.t & VT_BTYPE) == VT_INT && p->c.i == 0) || ((p->type.t & VT_BTYPE) == VT_LLONG && p->c.ll == 0);
}

int is_integer_btype(int bt) {
  return (bt == VT_BYTE || bt == VT_SHORT || bt == VT_INT || bt == VT_LLONG);
}

// Check types for comparison or subtraction of pointers
void check_comparison_pointer_types(SValue *p1, SValue *p2, int op) {
  CType *type1, *type2;
  CType ptype1, ptype2;
  int bt1, bt2;

  // NULL pointers are accepted for all comparisons
  if (is_null_pointer(p1) || is_null_pointer(p2)) return;
  type1 = &p1->type;
  type2 = &p2->type;
  bt1 = type1->t & VT_BTYPE;
  bt2 = type2->t & VT_BTYPE;

  // Accept comparison between pointer and integer with a warning
  if ((is_integer_btype(bt1) || is_integer_btype(bt2)) && op != '-') {
    if (op != TOK_LOR && op != TOK_LAND ) {
      warning("comparison between pointer and integer");
    }
    return;
  }

  // Both must be pointers or implicit function pointers
  if (bt1 == VT_PTR) {
    type1 = pointed_type(type1);
  } else if (bt1 != VT_FUNC) { 
    goto invalid_operands;
  }

  if (bt2 == VT_PTR) {
    type2 = pointed_type(type2);
  } else if (bt2 != VT_FUNC) { 
  invalid_operands:
    error("invalid operands to binary %s", get_tok_str(op, NULL));
  }
  if ((type1->t & VT_BTYPE) == VT_VOID || (type2->t & VT_BTYPE) == VT_VOID) return;

  ptype1 = *type1;
  ptype2 = *type2;
  ptype1.t &= ~(VT_UNSIGNED | VT_CONSTANT | VT_VOLATILE);
  ptype2.t &= ~(VT_UNSIGNED | VT_CONSTANT | VT_VOLATILE);
  if (!are_compatible_types(&ptype1, &ptype2)) {
    // GCC-like error if '-' is used
    if (op == '-') {
      goto invalid_operands;
    } else {
      warning("comparison of distinct pointer types lacks a cast");
    }
  }
}

// Compare function types. Old functions match any new functions
int are_compatible_funcs(CType *type1, CType *type2) {
  Sym *s1, *s2;

  s1 = type1->ref;
  s2 = type2->ref;
  if (!are_compatible_types(&s1->type, &s2->type)) return 0;
  // Check func_call
  if (FUNC_CALL(s1->r) != FUNC_CALL(s2->r)) return 0;
  // TODO: not complete
  if (s1->c == FUNC_OLD || s2->c == FUNC_OLD) return 1;
  if (s1->c != s2->c) return 0;
  while (s1 != NULL) {
    if (s2 == NULL) return 0;
    if (!are_compatible_parameter_types(&s1->type, &s2->type)) return 0;
    s1 = s1->next;
    s2 = s2->next;
  }
  if (s2) return 0;
  return 1;
}

// Return true if type1 and type2 are the same.  If unqualified is
// true, qualifiers on the types are ignored.
// (enums are not checked as gcc __builtin_types_compatible_p)
int compare_types(CType *type1, CType *type2, int unqualified) {
  int bt1, t1, t2;

  t1 = type1->t & VT_TYPE;
  t2 = type2->t & VT_TYPE;

  if (unqualified) {
    // Strip qualifiers before comparing
    t1 &= ~(VT_CONSTANT | VT_VOLATILE);
    t2 &= ~(VT_CONSTANT | VT_VOLATILE);
  }

  // TODO: bitfields?
  if (t1 != t2) return 0;

  // Test more complicated cases
  bt1 = t1 & VT_BTYPE;
  if (bt1 == VT_PTR) {
    type1 = pointed_type(type1);
    type2 = pointed_type(type2);
    return are_compatible_types(type1, type2);
  } else if (bt1 == VT_STRUCT) {
    return (type1->ref == type2->ref);
  } else if (bt1 == VT_FUNC) {
    return are_compatible_funcs(type1, type2);
  } else {
    return 1;
  }
}

// Return true if type1 and type2 are exactly the same (including qualifiers).
int are_compatible_types(CType *type1, CType *type2) {
  return compare_types(type1,type2,0);
}

// Return true if type1 and type2 are the same (ignoring qualifiers).
int are_compatible_parameter_types(CType *type1, CType *type2) {
  return compare_types(type1,type2,1);
}

// Return type size. Put alignment at 'a'
int type_size(CType *type, int *a) {
  Sym *s;
  int bt;

  bt = type->t & VT_BTYPE;
  if (bt == VT_STRUCT) {
    // struct/union
    s = type->ref;
    *a = s->r;
    return s->c;
  } else if (bt == VT_PTR) {
    if (type->t & VT_ARRAY) {
      s = type->ref;
      return type_size(&s->type, a) * s->c;
    } else {
      *a = PTR_SIZE;
      return PTR_SIZE;
    }
  } else if (bt == VT_LDOUBLE) {
    *a = LDOUBLE_ALIGN;
    return LDOUBLE_SIZE;
  } else if (bt == VT_DOUBLE || bt == VT_LLONG) {
    *a = 8; 
    return 8;
  } else if (bt == VT_INT || bt == VT_ENUM || bt == VT_FLOAT) {
    *a = 4;
    return 4;
  } else if (bt == VT_SHORT) {
    *a = 2;
    return 2;
  } else {
    // char, void, function, _Bool
    *a = 1;
    return 1;
  }
}

int pointed_size(CType *type) {
  int align;
  return type_size(pointed_type(type), &align);
}

// Print a type. If 'varstr' is not NULL, then the variable is also printed in the type
// TODO: union
// TODO: add array and function pointers
void type_to_str(char *buf, int buf_size, CType *type, const char *varstr) {
  int bt, v, t;
  Sym *s, *sa;
  char buf1[256];
  const char *tstr;

  t = type->t & VT_TYPE;
  bt = t & VT_BTYPE;
  buf[0] = '\0';
  if (t & VT_CONSTANT) pstrcat(buf, buf_size, "const ");
  if (t & VT_VOLATILE) pstrcat(buf, buf_size, "volatile ");
  if (t & VT_UNSIGNED) pstrcat(buf, buf_size, "unsigned ");
  switch (bt) {
    case VT_VOID:
      tstr = "void";
      goto add_tstr;
    case VT_BOOL:
      tstr = "_Bool";
      goto add_tstr;
    case VT_BYTE:
      tstr = "char";
      goto add_tstr;
    case VT_SHORT:
      tstr = "short";
      goto add_tstr;
    case VT_INT:
      tstr = "int";
      goto add_tstr;
    case VT_LONG:
      tstr = "long";
      goto add_tstr;
    case VT_LLONG:
      tstr = "long long";
      goto add_tstr;
    case VT_FLOAT:
      tstr = "float";
      goto add_tstr;
    case VT_DOUBLE:
      tstr = "double";
      goto add_tstr;
    case VT_LDOUBLE:
      tstr = "long double";
    add_tstr:
      pstrcat(buf, buf_size, tstr);
      break;
    case VT_ENUM:
    case VT_STRUCT:
      if (bt == VT_STRUCT) {
        tstr = "struct ";
      } else {
        tstr = "enum ";
      }
      pstrcat(buf, buf_size, tstr);
      v = type->ref->v & ~SYM_STRUCT;
      if (v >= SYM_FIRST_ANOM) {
        pstrcat(buf, buf_size, "<anonymous>");
      } else {
        pstrcat(buf, buf_size, get_tok_str(v, NULL));
      }
      break;
    case VT_FUNC:
      s = type->ref;
      type_to_str(buf, buf_size, &s->type, varstr);
      pstrcat(buf, buf_size, "(");
      sa = s->next;
      while (sa != NULL) {
        type_to_str(buf1, sizeof(buf1), &sa->type, NULL);
        pstrcat(buf, buf_size, buf1);
        sa = sa->next;
        if (sa) pstrcat(buf, buf_size, ", ");
      }
      pstrcat(buf, buf_size, ")");
      goto no_var;
    case VT_PTR:
      s = type->ref;
      pstrcpy(buf1, sizeof(buf1), "*");
      if (varstr) pstrcat(buf1, sizeof(buf1), varstr);
      type_to_str(buf, buf_size, &s->type, buf1);
      goto no_var;
  }

  if (varstr) {
    pstrcat(buf, buf_size, " ");
    pstrcat(buf, buf_size, varstr);
  }
 no_var: ;
}

