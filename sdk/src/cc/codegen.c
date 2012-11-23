//
//  codegen.c - Tiny C Compiler for Sanos
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

// TODO: endianness dependent
int ieee_finite(double d) {
  int *p = (int *) &d;
  return ((unsigned)((p[1] | 0x800fffff) + 1)) >> 31;
}

// Swap integers
void swap(int *p, int *q) {
  int t;
  t = *p;
  *p = *q;
  *q = t;
}

void vsetc(CType *type, int r, CValue *vc) {
  int v;

  if (vtop >= vstack + (VSTACK_SIZE - 1)) error("memory full");

  // Cannot let CPU flags if other instruction are generated. Also
  // avoid leaving VT_JMP anywhere except on the top of the stack
  // because it would complicate the code generator.
  if (vtop >= vstack) {
    v = vtop->r & VT_VALMASK;
    if (v == VT_CMP || (v & ~1) == VT_JMP) gv(RC_INT);
  }
  vtop++;
  vtop->type = *type;
  vtop->r = r;
  vtop->r2 = VT_CONST;
  vtop->c = *vc;
}

// Push integer constant
void vpushi(int v) {
  CValue cval;
  cval.i = v;
  vsetc(&int_type, VT_CONST, &cval);
}

// Push a reference to a section offset by adding a dummy symbol
void vpush_ref(CType *type, Section *sec, unsigned long offset, unsigned long size) {
  CValue cval;

  cval.ul = 0;
  vsetc(type, VT_CONST | VT_SYM, &cval);
  vtop->sym = get_sym_ref(type, sec, offset, size);
}

// Push a reference to global symbol v
void vpush_global_sym(CType *type, int v) {
  Sym *sym;
  CValue cval;

  sym = external_global_sym(v, type, 0);
  cval.ul = 0;
  vsetc(type, VT_CONST | VT_SYM, &cval);
  vtop->sym = sym;
}

void vpush_tokc(int t) {
  CType type;
  type.t = t;
  vsetc(&type, VT_CONST, &tokc);
}

void vset(CType *type, int r, int v) {
  CValue cval;

  cval.i = v;
  vsetc(type, r, &cval);
}

void vseti(int r, int v) {
  CType type;

  type.t = VT_INT;
  vset(&type, r, v);
}

void vswap(void) {
  SValue tmp;

  tmp = vtop[0];
  vtop[0] = vtop[-1];
  vtop[-1] = tmp;
}

void vpushv(SValue *v) {
  if (vtop >= vstack + (VSTACK_SIZE - 1)) error("memory full");
  vtop++;
  *vtop = *v;
}

void vdup(void) {
  vpushv(vtop);
}

// Save r to the memory stack, and mark it as being free
void save_reg(int r) {
  int l, saved, size, align;
  SValue *p, sv;
  CType *type;

  // Modify all stack values
  saved = 0;
  l = 0;
  for (p = vstack; p <= vtop; p++) {
    if ((p->r & VT_VALMASK) == r || ((p->type.t & VT_BTYPE) == VT_LLONG && (p->r2 & VT_VALMASK) == r)) {
      // Must save value on stack if not already done
      if (!saved) {
        // Must reload 'r' because r might be equal to r2
        r = p->r & VT_VALMASK;

        // Store register in the stack
        type = &p->type;
        if ((p->r & VT_LVAL) || (!is_float(type->t) && (type->t & VT_BTYPE) != VT_LLONG)) {
          type = &int_type;
        }
        size = type_size(type, &align);
        loc = (loc - size) & -align;
        sv.type.t = type->t;
        sv.r = VT_LOCAL | VT_LVAL;
        sv.c.ul = loc;
        store(r, &sv);

        // x86 specific: need to pop fp register ST0 if saved
        if (r == TREG_ST0) {
          o(0xd9dd); // fstp %st(1)
        }

        // Special long long case
        if ((type->t & VT_BTYPE) == VT_LLONG) {
          sv.c.ul += 4;
          store(p->r2, &sv);
        }
        l = loc;
        saved = 1;
      }

      // Mark that stack entry as being saved on the stack
      if (p->r & VT_LVAL) {
        p->r = (p->r & ~VT_VALMASK) | VT_LLOCAL;
      } else {
        p->r = lvalue_type(p->type.t) | VT_LOCAL;
      }
      p->r2 = VT_CONST;
      p->c.ul = l;
    }
  }
}

// Find a free register of class 'rc'. If none, save one register
int get_reg(int rc) {
  int r;
  SValue *p;

  // Find a free register
  for (r = 0; r < NB_REGS; r++) {
    if (reg_classes[r] & rc) {
      for (p = vstack; p <= vtop; p++) {
        if ((p->r & VT_VALMASK) == r || (p->r2 & VT_VALMASK) == r) goto notfound;
      }
      return r;
    }
  notfound: ;
  }

  // No register left: free the first one on the stack (VERY
  // IMPORTANT to start from the bottom to ensure that we don't
  // spill registers used in gen_opi())
  for (p = vstack; p <= vtop; p++) {
    r = p->r & VT_VALMASK;
    if (r < VT_CONST && (reg_classes[r] & rc)) goto save_found;

    // Also look at second register (if long long)
    r = p->r2 & VT_VALMASK;
    if (r < VT_CONST && (reg_classes[r] & rc)) {
    save_found:
      save_reg(r);
      return r;
    }
  }
  // Should never come here
  return -1;
}

// Find a free pointer-type register
int get_ptr_reg() {
  int r;
  SValue *p;

  // Find a free pointer register
  for (r = 0; r < NB_REGS; r++) {
    if (reg_classes[r] & RC_PTR) {
      for (p = vstack; p <= vtop; p++) {
        if ((p->r & VT_VALMASK) == r || (p->r2 & VT_VALMASK) == r) goto notfound;
      }
      return r;
    }
  notfound: ;
  }

  // No pointer register found
  return get_reg(RC_INT);
}

// Save registers up to (vtop - n) stack entry
void save_regs(int n) {
  int r;
  SValue *p, *p1;
  p1 = vtop - n;
  for (p = vstack; p <= p1; p++) {
    r = p->r & VT_VALMASK;
    if (r < VT_CONST) {
      save_reg(r);
    }
  }
}

// Move register 's' to 'r', and flush previous value of r to memory if needed
void move_reg(int r, int s) {
  SValue sv;

  if (r != s) {
    save_reg(r);
    sv.type.t = VT_INT;
    sv.r = s;
    sv.c.ul = 0;
    load(r, &sv);
  }
}

// Get address of vtop (vtop MUST BE an lvalue)
void gaddrof(void) {
  vtop->r &= ~VT_LVAL;
  // Tricky: if saved lvalue, then we can go back to lvalue
  if ((vtop->r & VT_VALMASK) == VT_LLOCAL) {
    vtop->r = (vtop->r & ~(VT_VALMASK | VT_LVAL_TYPE)) | VT_LOCAL | VT_LVAL;
  }
}

// Store vtop in a register belonging to class 'rc'. lvalues are
// converted to values. Cannot be used if vtop cannot be converted 
// to register value (such as structures).
int gv(int rc) {
  int r, r2, rc2, bit_pos, bit_size, size, align, i;
  uint64_t ll;

  // NOTE: get_reg can modify vstack[]
  if (vtop->type.t & VT_BITFIELD) {
    bit_pos = (vtop->type.t >> VT_STRUCT_SHIFT) & 0x3f;
    bit_size = (vtop->type.t >> (VT_STRUCT_SHIFT + 6)) & 0x3f;
    // Remove bit field info to avoid loops
    vtop->type.t &= ~(VT_BITFIELD | (-1 << VT_STRUCT_SHIFT));
    // Generate shifts
    vpushi(32 - (bit_pos + bit_size));
    gen_op(TOK_SHL);
    vpushi(32 - bit_size);
    // NOTE: transformed to SHR if unsigned
    gen_op(TOK_SAR);
    r = gv(rc);
  } else {
    if (is_float(vtop->type.t) && (vtop->r & (VT_VALMASK | VT_LVAL)) == VT_CONST) {
      Sym *sym;
      int *ptr;
      unsigned long offset;

      // TODO: unify with initializers handling?
      // CPUs usually cannot use float constants, so we store them
      // generically in data segment
      size = type_size(&vtop->type, &align);
      offset = (data_section->data_offset + align - 1) & -align;
      data_section->data_offset = offset;
      // TODO: not portable yet
      // Zero pad x87 tenbyte long doubles
      if (size == 12) vtop->c.tab[2] &= 0xffff;
      ptr = section_ptr_add(data_section, size);
      size = size >> 2;
      for (i = 0; i < size; i++) ptr[i] = vtop->c.tab[i];
      sym = get_sym_ref(&vtop->type, data_section, offset, size << 2);
      vtop->r |= VT_LVAL | VT_SYM;
      vtop->sym = sym;
      vtop->c.ul = 0;
    }

    r = vtop->r & VT_VALMASK;
    // Need to reload if:
    //   - constant
    //   - lvalue (need to dereference pointer)
    //   - already a register, but not in the right class
    if (r >= VT_CONST || 
      (vtop->r & VT_LVAL) ||
      !(reg_classes[r] & rc) ||
      ((vtop->type.t & VT_BTYPE) == VT_LLONG && 
       !(reg_classes[vtop->r2] & rc))) {
      if (rc == RC_INT && (vtop->type.t & VT_BTYPE) == VT_PTR) {
        r = get_ptr_reg();
      } else {
        r = get_reg(rc);
      }
      if ((vtop->type.t & VT_BTYPE) == VT_LLONG) {
        // Two register type load: expand to two words temporarily
        if ((vtop->r & (VT_VALMASK | VT_LVAL)) == VT_CONST) {
          // Load constant
          ll = vtop->c.ull;
          vtop->c.ui = ll; // First word
          load(r, vtop);
          vtop->r = r; // Save register value
          vpushi(ll >> 32); // Second word
        } else if (r >= VT_CONST || (vtop->r & VT_LVAL)) {
          // We do not want to modify the long long pointer here, so the safest (and less
          // efficient) is to save all the other registers in the stack. TODO: totally inefficient.
          save_regs(1);

          // Load from memory
          load(r, vtop);
          vdup();
          vtop[-1].r = r; // Save register value

          // Increment pointer to get second word
          vtop->type.t = VT_INT;
          gaddrof();
          vpushi(4);
          gen_op('+');
          vtop->r |= VT_LVAL;
        } else {
          // Move registers
          load(r, vtop);
          vdup();
          vtop[-1].r = r; // Save register value
          vtop->r = vtop[-1].r2;
        }

        // Allocate second register
        rc2 = RC_INT;
        if (rc == RC_IRET) rc2 = RC_LRET;
        r2 = get_reg(rc2);
        load(r2, vtop);
        vpop();

        // Write second register
        vtop->r2 = r2;
      } else if ((vtop->r & VT_LVAL) && !is_float(vtop->type.t)) {
        int t1, t;
        // lvalue of scalar type: need to use lvalue type because of possible cast
        t = vtop->type.t;
        t1 = t;

        // Compute memory access type
        if (vtop->r & VT_LVAL_BYTE) {
          t = VT_BYTE;
        } else if (vtop->r & VT_LVAL_SHORT) {
          t = VT_SHORT;
        }
        if (vtop->r & VT_LVAL_UNSIGNED) t |= VT_UNSIGNED;
        vtop->type.t = t;
        load(r, vtop);

        // Restore wanted type
        vtop->type.t = t1;
      } else {
        // One register type load
        load(r, vtop);
      }
    }
    vtop->r = r;
  }
  return r;
}

// Generate vtop[-1] and vtop[0] in resp. classes rc1 and rc2
void gv2(int rc1, int rc2) {
  int v;

  // Generate more generic register first. But VT_JMP or VT_CMP
  // values must be generated first in all cases to avoid possible
  // reload errors.
  v = vtop[0].r & VT_VALMASK;
  if (v != VT_CMP && (v & ~1) != VT_JMP && rc1 <= rc2) {
    vswap();
    gv(rc1);
    vswap();
    gv(rc2);
    // Test if reload is needed for first register
    if ((vtop[-1].r & VT_VALMASK) >= VT_CONST) {
      vswap();
      gv(rc1);
      vswap();
    }
  } else {
    gv(rc2);
    vswap();
    gv(rc1);
    vswap();
    // Test if reload is needed for first register
    if ((vtop[0].r & VT_VALMASK) >= VT_CONST) {
      gv(rc2);
    }
  }
}

// Expand long long on stack in two int registers
void lexpand(void) {
  int u;

  u = vtop->type.t & VT_UNSIGNED;
  gv(RC_INT);
  vdup();
  vtop[0].r = vtop[-1].r2;
  vtop[0].r2 = VT_CONST;
  vtop[-1].r2 = VT_CONST;
  vtop[0].type.t = VT_INT | u;
  vtop[-1].type.t = VT_INT | u;
}

// Build a long long from two ints
void lbuild(int t) {
  gv2(RC_INT, RC_INT);
  vtop[-1].r2 = vtop[0].r;
  vtop[-1].type.t = t;
  vpop();
}

// Rotate n first stack elements to the bottom I1 ... In -> I2 ... In I1 [top is right]
void vrotb(int n) {
  int i;
  SValue tmp;

  tmp = vtop[-n + 1];
  for (i = -n + 1; i != 0; i++) vtop[i] = vtop[i + 1];
  vtop[0] = tmp;
}

// Rotate n first stack elements to the top I1 ... In -> In I1 ... I(n-1)  [top is right]
void vrott(int n) {
  int i;
  SValue tmp;

  tmp = vtop[0];
  for (i = 0; i < n - 1; i++) vtop[-i] = vtop[-i - 1];
  vtop[-n + 1] = tmp;
}

// Pop stack value
void vpop(void) {
  int v;
  v = vtop->r & VT_VALMASK;

  // For x86, we need to pop the FP stack
  if (v == TREG_ST0 && !nocode_wanted) {
    o(0xd9dd); // fstp %st(1)
  } else if (v == VT_JMP || v == VT_JMPI) {
    // Need to put correct jump if && or || without test
    gsym(vtop->c.ul);
  }
  vtop--;
}

// Convert stack entry to register and duplicate its value in another register
void gv_dup(void) {
  int rc, t, r, r1;
  SValue sv;

  t = vtop->type.t;
  if ((t & VT_BTYPE) == VT_LLONG) {
    lexpand();
    gv_dup();
    vswap();
    vrotb(3);
    gv_dup();
    vrotb(4);
    // Stack: H L L1 H1
    lbuild(t);
    vrotb(3);
    vrotb(3);
    vswap();
    lbuild(t);
    vswap();
  } else {
    // Duplicate value
    rc = RC_INT;
    sv.type.t = VT_INT;
    if (is_float(t)) {
      rc = RC_FLOAT;
      sv.type.t = t;
    }
    r = gv(rc);
    r1 = get_reg(rc);
    sv.r = r;
    sv.c.ul = 0;
    load(r1, &sv); // Move r to r1
    vdup();
    // Duplicates value
    vtop->r = r1;
  }
}

// Generate CPU independent (unsigned) long long operations
void gen_opl(int op) {
  int t, a, b, op1, c, i;
  int func;
  SValue tmp;

  switch (op) {
    case '/':
    case TOK_PDIV:
      func = TOK___divdi3;
      goto gen_func;
    case TOK_UDIV:
      func = TOK___udivdi3;
      goto gen_func;
    case '%':
      func = TOK___moddi3;
      goto gen_func;
    case TOK_UMOD:
      func = TOK___umoddi3;
    gen_func:
      // Call generic long long function
      vpush_global_sym(&func_old_type, func);
      vrott(3);
      gfunc_call(2);
      vpushi(0);
      vtop->r = REG_IRET;
      vtop->r2 = REG_LRET;
      break;

    case '^':
    case '&':
    case '|':
    case '*':
    case '+':
    case '-':
      t = vtop->type.t;
      vswap();
      lexpand();
      vrotb(3);
      lexpand();
      // Stack: L1 H1 L2 H2
      tmp = vtop[0];
      vtop[0] = vtop[-3];
      vtop[-3] = tmp;
      tmp = vtop[-2];
      vtop[-2] = vtop[-3];
      vtop[-3] = tmp;
      vswap();
      // Stack: H1 H2 L1 L2
      if (op == '*') {
        vpushv(vtop - 1);
        vpushv(vtop - 1);
        gen_op(TOK_UMULL);
        lexpand();
        // Stack: H1 H2 L1 L2 ML MH
        for (i = 0; i < 4; i++) vrotb(6);
        // Stack: ML MH H1 H2 L1 L2
        tmp = vtop[0];
        vtop[0] = vtop[-2];
        vtop[-2] = tmp;
        // Stack: ML MH H1 L2 H2 L1
        gen_op('*');
        vrotb(3);
        vrotb(3);
        gen_op('*');
        // Stack: ML MH M1 M2
        gen_op('+');
        gen_op('+');
      } else if (op == '+' || op == '-') {
        if (op == '+') {
          op1 = TOK_ADDC1;
        } else {
          op1 = TOK_SUBC1;
        }
        gen_op(op1);
        // Stack: H1 H2 (L1 op L2)
        vrotb(3);
        vrotb(3);
        gen_op(op1 + 1); // TOK_xxxC2
      } else {
        gen_op(op);
        // Stack: H1 H2 (L1 op L2)
        vrotb(3);
        vrotb(3);
        // Stack: (L1 op L2) H1 H2
        gen_op(op);
        // Stack: (L1 op L2) (H1 op H2)
      }
      // Stack: L H
      lbuild(t);
      break;

    case TOK_SAR:
    case TOK_SHR:
    case TOK_SHL:
      if ((vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST) {
        t = vtop[-1].type.t;
        vswap();
        lexpand();
        vrotb(3);
        // Stack: L H shift
        c = (int) vtop->c.i;
        // Constant: simpler
        // NOTE: All comments are for SHL. The other cases are done by swaping words
        vpop();
        if (op != TOK_SHL) vswap();
        if (c >= 32) {
          // Stack: L H
          vpop();
          if (c > 32) {
            vpushi(c - 32);
            gen_op(op);
          }
          if (op != TOK_SAR) {
            vpushi(0);
          } else {
            gv_dup();
            vpushi(31);
            gen_op(TOK_SAR);
          }
          vswap();
        } else {
          vswap();
          gv_dup();
          // Stack: H L L
          vpushi(c);
          gen_op(op);
          vswap();
          vpushi(32 - c);
          if (op == TOK_SHL) {
            gen_op(TOK_SHR);
          } else {
            gen_op(TOK_SHL);
          }
          vrotb(3);
          // Stack: L L H
          vpushi(c);
          if (op == TOK_SHL) {
            gen_op(TOK_SHL);
          } else {
            gen_op(TOK_SHR);
          }
          gen_op('|');
        }
        if (op != TOK_SHL) vswap();
        lbuild(t);
      } else {
        // TODO: should provide a faster fallback on x86?
        switch (op) {
          case TOK_SAR:
            func = TOK___sardi3;
            goto gen_func;
          case TOK_SHR:
            func = TOK___shrdi3;
            goto gen_func;
          case TOK_SHL:
            func = TOK___shldi3;
            goto gen_func;
        }
      }
      break;

    default:
      // Compare operations
      vswap();
      lexpand();
      vrotb(3);
      lexpand();
      // Stack: L1 H1 L2 H2
      tmp = vtop[-1];
      vtop[-1] = vtop[-2];
      vtop[-2] = tmp;
      // Stack: L1 L2 H1 H2
      // Compare high
      op1 = op;
      // When values are equal, we need to compare low words. since the jump is inverted, we invert the test too.
      if (op1 == TOK_LT) {
        op1 = TOK_LE;
      } else if (op1 == TOK_GT) {
        op1 = TOK_GE;
      } else if (op1 == TOK_ULT) {
        op1 = TOK_ULE;
      } else if (op1 == TOK_UGT) {
        op1 = TOK_UGE;
      }
      a = 0;
      b = 0;
      gen_op(op1);
      if (op1 != TOK_NE) {
        a = gtst(1, 0);
      }
      if (op != TOK_EQ) {
        // Generate non equal test
        if (a == 0) {
          b = gtst(0, 0);
        } else {
          b = gjmp(0, TOK_NE);
        }
      }
      // Compare low. Always unsigned
      op1 = op;
      if (op1 == TOK_LT) {
        op1 = TOK_ULT;
      } else if (op1 == TOK_LE) {
        op1 = TOK_ULE;
      } else if (op1 == TOK_GT) {
        op1 = TOK_UGT;
      } else if (op1 == TOK_GE) {
        op1 = TOK_UGE;
      }
      gen_op(op1);
      a = gtst(1, a);
      gsym(b);
      vseti(VT_JMPI, a);
  }
}

// Handle integer constant optimizations and various machine independent optimizations.
void gen_opic(int op) {
  int c1, c2, t1, t2, n;
  SValue *v1, *v2;
  int64_t l1, l2;
  typedef uint64_t U;

  v1 = vtop - 1;
  v2 = vtop;
  t1 = v1->type.t & VT_BTYPE;
  t2 = v2->type.t & VT_BTYPE;
  l1 = (t1 == VT_LLONG) ? v1->c.ll : (v1->type.t & VT_UNSIGNED) ? v1->c.ui : v1->c.i;
  l2 = (t2 == VT_LLONG) ? v2->c.ll : (v2->type.t & VT_UNSIGNED) ? v2->c.ui : v2->c.i;

  // Currently, we cannot do computations with forward symbols
  c1 = (v1->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;
  c2 = (v2->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;
  if (c1 && c2) {
    switch (op) {
      case '+': l1 += l2; break;
      case '-': l1 -= l2; break;
      case '&': l1 &= l2; break;
      case '^': l1 ^= l2; break;
      case '|': l1 |= l2; break;
      case '*': l1 *= l2; break;

      case TOK_PDIV:
      case '/':
      case '%':
      case TOK_UDIV:
      case TOK_UMOD:
        // If division by zero, generate explicit division
        if (l2 == 0) {
          if (const_wanted) error("division by zero in constant");
          goto general_case;
        }
        switch (op) {
          case '%': l1 %= l2; break;
          case TOK_UDIV: l1 = (U)l1 / l2; break;
          case TOK_UMOD: l1 = (U)l1 % l2; break;
          default: l1 /= l2; break;
        }
        break;

      case TOK_SHL: l1 <<= l2; break;
      case TOK_SHR: l1 = (U)l1 >> l2; break;
      case TOK_SAR: l1 >>= l2; break;
 
      // Tests
      case TOK_ULT: l1 = (U)l1 < (U)l2; break;
      case TOK_UGE: l1 = (U)l1 >= (U)l2; break;
      case TOK_EQ: l1 = l1 == l2; break;
      case TOK_NE: l1 = l1 != l2; break;
      case TOK_ULE: l1 = (U)l1 <= (U)l2; break;
      case TOK_UGT: l1 = (U)l1 > (U)l2; break;
      case TOK_LT: l1 = l1 < l2; break;
      case TOK_GE: l1 = l1 >= l2; break;
      case TOK_LE: l1 = l1 <= l2; break;
      case TOK_GT: l1 = l1 > l2; break;

      // Logical
      case TOK_LAND: l1 = l1 && l2; break;
      case TOK_LOR: l1 = l1 || l2; break;
      default: goto general_case;
    }
    v1->c.ll = l1;
    vtop--;
  } else {
    // If commutative ops, put c2 as constant
    if (c1 && (op == '+' || op == '&' || op == '^' || op == '|' || op == '*')) {
      vswap();
      c2 = c1; //c = c1, c1 = c2, c2 = c;
      l2 = l1; //l = l1, l1 = l2, l2 = l;
    }

    // Filter out NOP operations like x*1, x-0, x&-1...
    if (c2 && (((op == '*' || op == '/' || op == TOK_UDIV || op == TOK_PDIV) && l2 == 1) ||
        ((op == '+' || op == '-' || op == '|' || op == '^' ||  op == TOK_SHL || op == TOK_SHR || op == TOK_SAR) && l2 == 0) ||
        (op == '&' && l2 == -1))) {
      // Nothing to do
      vtop--;
    } else if (c2 && (op == '*' || op == TOK_PDIV || op == TOK_UDIV)) {
      // Try to use shifts instead of muls or divs
      if (l2 > 0 && (l2 & (l2 - 1)) == 0) {
        n = -1;
        while (l2) {
          l2 >>= 1;
          n++;
        }
        vtop->c.ll = n;
        if (op == '*') {
          op = TOK_SHL;
        } else if (op == TOK_PDIV) {
          op = TOK_SAR;
        } else {
          op = TOK_SHR;
        }
      }
      goto general_case;
    } else if (c2 && (op == '+' || op == '-') && (vtop[-1].r & (VT_VALMASK | VT_LVAL | VT_SYM)) == (VT_CONST | VT_SYM)) {
      // Symbol + constant case
      if (op == '-') l2 = -l2;
      vtop--;
      vtop->c.ll += l2;
    } else {
    general_case:
      if (!nocode_wanted) {
        // Call low level op generator
        if (t1 == VT_LLONG || t2 == VT_LLONG) {
          gen_opl(op);
        } else {
          gen_opi(op);
        }
      } else {
        vtop--;
      }
    }
  }
}

// Generate a floating point operation with constant propagation
void gen_opif(int op) {
  int c1, c2;
  SValue *v1, *v2;
  long double f1, f2;

  v1 = vtop - 1;
  v2 = vtop;
  // Currently, we cannot do computations with forward symbols
  c1 = (v1->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;
  c2 = (v2->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;
  if (c1 && c2) {
    if (v1->type.t == VT_FLOAT) {
      f1 = v1->c.f;
      f2 = v2->c.f;
    } else if (v1->type.t == VT_DOUBLE) {
      f1 = v1->c.d;
      f2 = v2->c.d;
    } else {
      f1 = v1->c.ld;
      f2 = v2->c.ld;
    }

    // NOTE: we only do constant propagation if finite number (not NaN or infinity) (ANSI spec)
    if (!ieee_finite(f1) || !ieee_finite(f2)) goto general_case;

    switch (op) {
      case '+': f1 += f2; break;
      case '-': f1 -= f2; break;
      case '*': f1 *= f2; break;
      case '/': 
        if (f2 == 0.0) {
          if (const_wanted) error("division by zero in constant");
          goto general_case;
        }
        f1 /= f2; 
        break;
        // TODO: also handles tests?
      default: goto general_case;
    }

    // TODO: overflow test?
    if (v1->type.t == VT_FLOAT) {
      v1->c.f = f1;
    } else if (v1->type.t == VT_DOUBLE) {
      v1->c.d = f1;
    } else {
      v1->c.ld = f1;
    }
    vtop--;
  } else {
  general_case:
    if (!nocode_wanted) {
      gen_opf(op);
    } else {
      vtop--;
    }
  }
}

// Generic gen_op: handles types problems
void gen_op(int op) {
  int u, t1, t2, bt1, bt2, t;
  CType type1;

  t1 = vtop[-1].type.t;
  t2 = vtop[0].type.t;
  bt1 = t1 & VT_BTYPE;
  bt2 = t2 & VT_BTYPE;

  if (bt1 == VT_PTR || bt2 == VT_PTR) {
    // At least one operand is a pointer
    // For relational op both must be pointers
    if (op >= TOK_ULT && op <= TOK_LOR) {
      check_comparison_pointer_types(vtop - 1, vtop, op);
      // Pointers are handled are unsigned
      t = VT_INT | VT_UNSIGNED;
      goto std_op;
    }

    // If both pointers, then it must be the '-' op
    if (bt1 == VT_PTR && bt2 == VT_PTR) {
      if (op != '-') error("cannot use pointers here");
      check_comparison_pointer_types(vtop - 1, vtop, op);
      // TODO: check that types are compatible
      u = pointed_size(&vtop[-1].type);
      gen_opic(op);

      // Set to integer type
      vtop->type.t = VT_INT; 
      vpushi(u);
      gen_op(TOK_PDIV);
    } else {
      // Exactly one pointer: must be '+' or '-'
      if (op != '-' && op != '+') error("cannot use pointers here");

      // Put pointer as first operand
      if (bt2 == VT_PTR) {
        vswap();
        swap(&t1, &t2);
      }
      type1 = vtop[-1].type;
      // TODO: cast to int? (long long case)
      vpushi(pointed_size(&vtop[-1].type));
      gen_op('*');
      gen_opic(op);
      // Put again type if gen_opic() swaped operands
      vtop->type = type1;
    }
  } else if (is_float(bt1) || is_float(bt2)) {
    // Compute bigger type and do implicit casts
    if (bt1 == VT_LDOUBLE || bt2 == VT_LDOUBLE) {
      t = VT_LDOUBLE;
    } else if (bt1 == VT_DOUBLE || bt2 == VT_DOUBLE) {
      t = VT_DOUBLE;
    } else {
      t = VT_FLOAT;
    }
    // Floats can only be used for a few operations
    if (op != '+' && op != '-' && op != '*' && op != '/' && (op < TOK_ULT || op > TOK_GT)) {
      error("invalid operands for binary operation");
    }
    goto std_op;
  } else if (bt1 == VT_LLONG || bt2 == VT_LLONG) {
    // Cast to biggest op
    t = VT_LLONG;
    // Convert to unsigned if it does not fit in a long long
    if ((t1 & (VT_BTYPE | VT_UNSIGNED)) == (VT_LLONG | VT_UNSIGNED) ||
        (t2 & (VT_BTYPE | VT_UNSIGNED)) == (VT_LLONG | VT_UNSIGNED)) {
      t |= VT_UNSIGNED;
    }
    goto std_op;
  } else {
    // Integer operations
    t = VT_INT;

    // Convert to unsigned if it does not fit in an integer
    if ((t1 & (VT_BTYPE | VT_UNSIGNED)) == (VT_INT | VT_UNSIGNED) ||
        (t2 & (VT_BTYPE | VT_UNSIGNED)) == (VT_INT | VT_UNSIGNED)) {
      t |= VT_UNSIGNED;
    }
  std_op:
    // TODO: currently, some unsigned operations are explicit, so we modify them here
    if (t & VT_UNSIGNED) {
      if (op == TOK_SAR) {
        op = TOK_SHR;
      } else if (op == '/') {
        op = TOK_UDIV;
      } else if (op == '%') {
        op = TOK_UMOD;
      } else if (op == TOK_LT) {
        op = TOK_ULT;
      } else if (op == TOK_GT) {
        op = TOK_UGT;
      } else if (op == TOK_LE) {
        op = TOK_ULE;
      } else if (op == TOK_GE) {
        op = TOK_UGE;
      }
    }

    vswap();
    type1.t = t;
    gen_cast(&type1);
    vswap();

    // Special case for shifts and long long: we keep the shift as an integer
    if (op == TOK_SHR || op == TOK_SAR || op == TOK_SHL) type1.t = VT_INT;
    
    gen_cast(&type1);
    if (is_float(t)) {
      gen_opif(op);
    } else {
      gen_opic(op);
    }

    if (op >= TOK_ULT && op <= TOK_GT) {
      // Relational op: the result is an int
      vtop->type.t = VT_INT;
    } else {
      vtop->type.t = t;
    }
  }
}

// Generic itof for unsigned long long case
void gen_cvt_itof1(int t) {
  if ((vtop->type.t & (VT_BTYPE | VT_UNSIGNED)) == (VT_LLONG | VT_UNSIGNED)) {

    if (t == VT_FLOAT) {
      vpush_global_sym(&func_old_type, TOK___ulltof);
    } else if (t == VT_DOUBLE) {
      vpush_global_sym(&func_old_type, TOK___ulltod);
    } else {
      vpush_global_sym(&func_old_type, TOK___ulltold);
    }
    vrott(2);
    gfunc_call(1);
    vpushi(0);
    vtop->r = REG_FRET;
  } else {
    gen_cvt_itof(t);
  }
}

// Generic ftoi for unsigned long long case
void gen_cvt_ftoi1(int t) {
  int st;

  if (t == (VT_LLONG | VT_UNSIGNED)) {
    // Not handled natively
    st = vtop->type.t & VT_BTYPE;
    if (st == VT_FLOAT) {
      vpush_global_sym(&func_old_type, TOK___fixunssfdi);
    } else if (st == VT_DOUBLE) {
      vpush_global_sym(&func_old_type, TOK___fixunsdfdi);
    } else {
      vpush_global_sym(&func_old_type, TOK___fixunsxfdi);
    }
    vrott(2);
    gfunc_call(1);
    vpushi(0);
    vtop->r = REG_IRET;
    vtop->r2 = REG_LRET;
  } else {
    gen_cvt_ftoi(t);
  }
}

// Force char or short cast
void force_charshort_cast(int t) {
  int bits, dbt;
  dbt = t & VT_BTYPE;
  // TODO: add optimization if lvalue: just change type and offset
  if (dbt == VT_BYTE) {
    bits = 8;
  } else {
    bits = 16;
  }

  if (t & VT_UNSIGNED) {
    vpushi((1 << bits) - 1);
    gen_op('&');
  } else {
    bits = 32 - bits;
    vpushi(bits);
    gen_op(TOK_SHL);
    // Result must be signed or the SAR is converted to an SHL
    // This was not the case when "t" was a signed short
    // and the last value on the stack was an unsigned int.
    vtop->type.t &= ~VT_UNSIGNED;
    vpushi(bits);
    gen_op(TOK_SAR);
  }
}

// Cast 'vtop' to 'type'. Casting to bitfields is forbidden.
void gen_cast(CType *type) {
  int sbt, dbt, sf, df, c;

  // Special delayed cast for char/short
  // TODO: in some cases (multiple cascaded casts), it may still be incorrect
  if (vtop->r & VT_MUSTCAST) {
    vtop->r &= ~VT_MUSTCAST;
    force_charshort_cast(vtop->type.t);
  }

  // Bitfields first get cast to ints
  if (vtop->type.t & VT_BITFIELD) {
    gv(RC_INT);
  }

  dbt = type->t & (VT_BTYPE | VT_UNSIGNED);
  sbt = vtop->type.t & (VT_BTYPE | VT_UNSIGNED);
  if (sbt != dbt && !nocode_wanted) {
    sf = is_float(sbt);
    df = is_float(dbt);
    c = (vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;
    if (sf && df) {
      // Convert from fp to fp
      if (c) {
        // Constant case: we can do it now 
        // TODO: in ISOC, cannot do it if error in convert
        if (dbt == VT_FLOAT && sbt == VT_DOUBLE) {
          vtop->c.f = (float) vtop->c.d;
        } else if (dbt == VT_FLOAT && sbt == VT_LDOUBLE) {
          vtop->c.f = (float) vtop->c.ld;
        } else if (dbt == VT_DOUBLE && sbt == VT_FLOAT) {
          vtop->c.d = (double) vtop->c.f;
        } else if (dbt == VT_DOUBLE && sbt == VT_LDOUBLE) {
          vtop->c.d = (double) vtop->c.ld;
        } else if (dbt == VT_LDOUBLE && sbt == VT_FLOAT) {
          vtop->c.ld = (long double) vtop->c.f;
        } else if (dbt == VT_LDOUBLE && sbt == VT_DOUBLE) {
          vtop->c.ld = (long double) vtop->c.d;
        }
      } else {
        // Non-constant case: generate code
        gen_cvt_ftof(dbt);
      }
    } else if (df) {
      // Convert int to fp
      if (c) {
        switch (sbt) {
          case VT_LLONG | VT_UNSIGNED:
          case VT_LLONG:
            // TODO: add const cases for long long
            goto do_itof;
          case VT_INT | VT_UNSIGNED:
            switch (dbt) {
              case VT_FLOAT: vtop->c.f = (float) vtop->c.ui; break;
              case VT_DOUBLE: vtop->c.d = (double) vtop->c.ui; break;
              case VT_LDOUBLE: vtop->c.ld = (long double) vtop->c.ui; break;
            }
           break;
          default:
            switch (dbt) {
              case VT_FLOAT: vtop->c.f = (float)vtop->c.i; break;
              case VT_DOUBLE: vtop->c.d = (double)vtop->c.i; break;
              case VT_LDOUBLE: vtop->c.ld = (long double)vtop->c.i; break;
            }
        }
      } else {
      do_itof:
        gen_cvt_itof(dbt);
      }
    } else if (sf) {
      // Convert fp to int
      if (dbt == VT_BOOL) {
         vpushi(0);
         gen_op(TOK_NE);
      } else {
        // We handle char/short/etc... with generic code
        if (dbt != (VT_INT | VT_UNSIGNED) && dbt != (VT_LLONG | VT_UNSIGNED) && dbt != VT_LLONG) dbt = VT_INT;
        if (c) {
          switch (dbt) {
            case VT_LLONG | VT_UNSIGNED:
            case VT_LLONG:
              // TODO: add const cases for long long
              goto do_ftoi;
            case VT_INT | VT_UNSIGNED:
              switch (sbt) {
                case VT_FLOAT: vtop->c.ui = (unsigned int) vtop->c.d; break;
                case VT_DOUBLE: vtop->c.ui = (unsigned int) vtop->c.d; break;
                case VT_LDOUBLE: vtop->c.ui = (unsigned int) vtop->c.d; break;
              }
              break;
            default:
              // int case
              switch (sbt) {
                case VT_FLOAT: vtop->c.i = (int) vtop->c.d; break;
                case VT_DOUBLE: vtop->c.i = (int) vtop->c.d; break;
                case VT_LDOUBLE: vtop->c.i = (int) vtop->c.d; break;
              }
          }
        } else {
        do_ftoi:
          gen_cvt_ftoi1(dbt);
        }
        if (dbt == VT_INT && (type->t & (VT_BTYPE | VT_UNSIGNED)) != dbt) {
          // Additional cast for char/short...
          vtop->type.t = dbt;
          gen_cast(type);
        }
      }
    } else if ((dbt & VT_BTYPE) == VT_LLONG) {
      if ((sbt & VT_BTYPE) != VT_LLONG) {
        // Scalar to long long
        if (c) {
          if (sbt == (VT_INT | VT_UNSIGNED)) {
            vtop->c.ll = vtop->c.ui;
          } else {
            vtop->c.ll = vtop->c.i;
          }
        } else {
          // Machine independent conversion
          gv(RC_INT);
          // Generate high word
          if (sbt == (VT_INT | VT_UNSIGNED)) {
            vpushi(0);
            gv(RC_INT);
          } else {
            gv_dup();
            vpushi(31);
            gen_op(TOK_SAR);
          }
          // Patch second register
          vtop[-1].r2 = vtop->r;
          vpop();
        }
      }
    } else if (dbt == VT_BOOL) {
      // Scalar to bool
      vpushi(0);
      gen_op(TOK_NE);
    } else if ((dbt & VT_BTYPE) == VT_BYTE || (dbt & VT_BTYPE) == VT_SHORT) {
      if (sbt == VT_PTR) {
        vtop->type.t = VT_INT;
        warning("nonportable conversion from pointer to char/short");
      }
      force_charshort_cast(dbt);
    } else if ((dbt & VT_BTYPE) == VT_INT) {
      // Scalar to int
      if (sbt == VT_LLONG) {
        // From long long: just take low order word
        lexpand();
        vpop();
      }
      // If lvalue and single word type, nothing to do because
      // the lvalue already contains the real type size (see
      // VT_LVAL_xxx constants)
    }
  } else if ((dbt & VT_BTYPE) == VT_PTR && !(vtop->r & VT_LVAL)) {
    // If we are casting between pointer types, we must update the VT_LVAL_xxx size
    vtop->r = (vtop->r & ~VT_LVAL_TYPE) | (lvalue_type(type->ref->type.t) & VT_LVAL_TYPE);
  }
  vtop->type = *type;
}

// Verify type compatibility to store vtop in 'dt' type, and generate casts if needed.
void gen_assign_cast(CType *dt) {
  CType *st, *type1, *type2;
  CType btype1, btype2;
  char buf1[256], buf2[256];
  int dbt, sbt;

  st = &vtop->type; // Source type
  dbt = dt->t & VT_BTYPE;
  sbt = st->t & VT_BTYPE;
  if (dt->t & VT_CONSTANT) warning("assignment of read-only location");
  switch (dbt) {
    case VT_PTR:
      // Special cases for pointers, '0' can also be a pointer
      if (is_null_pointer(vtop)) goto type_ok;
      
      // Accept implicit pointer to integer cast with warning
      if (is_integer_btype(sbt)) {
        warning("assignment makes pointer from integer without a cast");
        goto type_ok;
      }
      type1 = pointed_type(dt);

      // A function is implicitely a function pointer
      if (sbt == VT_FUNC) {
        if ((type1->t & VT_BTYPE) != VT_VOID && !are_compatible_types(pointed_type(dt), st)) {
          goto error;
        } else {
          goto type_ok;
        }
      }
      if (sbt != VT_PTR) goto error;
      type2 = pointed_type(st);
      if ((type1->t & VT_BTYPE) == VT_VOID || (type2->t & VT_BTYPE) == VT_VOID) {
        // void * can match anything
      } else {
        // Exact type match, except for unsigned
        btype1 = *type1;
        btype2 = *type2;
        btype1.t &= ~(VT_UNSIGNED | VT_CONSTANT | VT_VOLATILE);
        btype2.t &= ~(VT_UNSIGNED | VT_CONSTANT | VT_VOLATILE);
        if (!are_compatible_types(&btype1, &btype2)) {
          warning("assignment from incompatible pointer type");
        }
      }

      // Check const and volatile
      if ((!(type1->t & VT_CONSTANT) && (type2->t & VT_CONSTANT)) ||
          (!(type1->t & VT_VOLATILE) && (type2->t & VT_VOLATILE))) {
        warning("assignment discards qualifiers from pointer target type");
      }
      break;

    case VT_BYTE:
    case VT_SHORT:
    case VT_INT:
    case VT_LLONG:
      if (sbt == VT_PTR || sbt == VT_FUNC) {
        warning("assignment makes integer from pointer without a cast");
      }
      // TODO: more tests
      break;

    case VT_STRUCT:
      btype1 = *dt;
      btype2 = *st;
      btype1.t &= ~(VT_CONSTANT | VT_VOLATILE);
      btype2.t &= ~(VT_CONSTANT | VT_VOLATILE);
      if (!are_compatible_types(&btype1, &btype2)) {
      error:
        type_to_str(buf1, sizeof(buf1), st, NULL);
        type_to_str(buf2, sizeof(buf2), dt, NULL);
        error("cannot cast '%s' to '%s'", buf1, buf2);
      }
      break;
  }

 type_ok:
  gen_cast(dt);
}

// Store vtop in lvalue pushed on stack
void vstore(void) {
  int sbt, dbt, ft, r, t, size, align, bit_size, bit_pos, rc, delayed_cast;

  ft = vtop[-1].type.t;
  sbt = vtop->type.t & VT_BTYPE;
  dbt = ft & VT_BTYPE;
  if (((sbt == VT_INT || sbt == VT_SHORT) && dbt == VT_BYTE) || (sbt == VT_INT && dbt == VT_SHORT)) {
    // Optimize char/short casts
    delayed_cast = VT_MUSTCAST;
    vtop->type.t = ft & VT_TYPE;
    // TODO: factorize
    if (ft & VT_CONSTANT) warning("assignment of read-only location");
  } else {
    delayed_cast = 0;
    if (!(ft & VT_BITFIELD)) gen_assign_cast(&vtop[-1].type);
  }

  if (sbt == VT_STRUCT) {
    // If structure, only generate pointer structure assignment: generate memcpy
    // TODO: optimize if small size
    if (!nocode_wanted) {
      size = type_size(&vtop->type, &align);
      vpush_global_sym(&func_old_type, TOK_memcpy);

      // Destination
      vpushv(vtop - 2);
      vtop->type.t = VT_INT;
      gaddrof();

      // Source
      vpushv(vtop - 2);
      vtop->type.t = VT_INT;
      gaddrof();

      // Type size
      vpushi(size);
      gfunc_call(3);
      
      vswap();
      vpop();
    } else {
      vswap();
      vpop();
    }
    // Leave source on stack
  } else if (ft & VT_BITFIELD) {
    // Bitfield store handling
    bit_pos = (ft >> VT_STRUCT_SHIFT) & 0x3f;
    bit_size = (ft >> (VT_STRUCT_SHIFT + 6)) & 0x3f;
    // Remove bit field info to avoid loops
    vtop[-1].type.t = ft & ~(VT_BITFIELD | (-1 << VT_STRUCT_SHIFT));

    // Duplicate source into other register
    gv_dup();
    vswap();
    vrott(3);

    // Duplicate destination
    vdup();
    vtop[-1] = vtop[-2];

    // Mask and shift source
    vpushi((1 << bit_size) - 1);
    gen_op('&');
    vpushi(bit_pos);
    gen_op(TOK_SHL);

    // Load destination, mask and or with source
    vswap();
    vpushi(~(((1 << bit_size) - 1) << bit_pos));
    gen_op('&');
    gen_op('|');

    // Store result
    vstore();

    // Pop off shifted source from "duplicate source..." above
    vpop();

  } else {
    if (!nocode_wanted) {
      rc = RC_INT;
      if (is_float(ft)) rc = RC_FLOAT;
      r = gv(rc);  // Generate value
      // If lvalue was saved on stack, must read it
      if ((vtop[-1].r & VT_VALMASK) == VT_LLOCAL) {
        SValue sv;
        t = get_reg(RC_INT);
        sv.type.t = VT_INT;
        sv.r = VT_LOCAL | VT_LVAL;
        sv.c.ul = vtop[-1].c.ul;
        load(t, &sv);
        vtop[-1].r = t | VT_LVAL;
      }
      store(r, vtop - 1);

      // Two word case handling: store second register at word + 4
      if ((ft & VT_BTYPE) == VT_LLONG) {
        vswap();
        // Convert to int to increment easily
        vtop->type.t = VT_INT;
        gaddrof();
        vpushi(4);
        gen_op('+');
        vtop->r |= VT_LVAL;
        vswap();
        // This works because r2 is spilled last!
        store(vtop->r2, vtop - 1);
      }
    }
    vswap();
    vtop--; // NOT vpop() because on x86 it would flush the fp stack
    vtop->r |= delayed_cast;
  }
}

