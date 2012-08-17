//
//  codegen386.c - Tiny C Compiler for Sanos
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

//#define DEBUG_BRANCH

int reg_classes[8] = {
  RC_INT | RC_EAX,            // eax
  RC_INT | RC_ECX,            // ecx
  RC_INT | RC_EDX,            // edx
#ifdef USE_EBX
  RC_INT | RC_SAVE | RC_PTR,  // ebx
#else
  0,
#endif
  RC_FLOAT | RC_ST0,          // st0
  0,
  RC_INT | RC_SAVE | RC_PTR,  // esi
  RC_INT | RC_SAVE | RC_PTR,  // edi
};

static unsigned char *code;
static int code_size;
static Branch *branch;
static int branch_size;
static int br;

static int regs_used;
static int func_ret_sub;
static int func_noargs;
int func_naked;

void reset_code_buf(void) {
  code = NULL;
  code_size = 0;
  branch = NULL;
  branch_size = 0;
  ind = 0;
  br = 0;
}

void clear_code_buf(void) {
  tcc_free(code);
  tcc_free(branch);
  reset_code_buf();
}

void g(int c) {
  if (ind == code_size) {
    code_size *= 2;
    if (code_size == 0) code_size = 1024;
    code = tcc_realloc(code, code_size);
  }
  code[ind++] = c;
}

void o(unsigned int c) {
  while (c) {
    g(c);
    c = c >> 8;
  }
}

void gen_le16(int c) {
  g(c);
  g(c >> 8);
}

void gen_le32(int c) {
  g(c);
  g(c >> 8);
  g(c >> 16);
  g(c >> 24);
}

// Instruction + 4 bytes data
void oad(int c, int s) {
  int a;

  o(c);
  gen_le32(s);
}

int gbranch(int type) {
  Branch *b;
  
  if (br == branch_size) {
    branch_size *= 2;
    if (branch_size == 0) branch_size = 128;
    branch = tcc_realloc(branch, branch_size * sizeof(Branch));
  }

  b = branch + br;
  b->type = type;
  b->param = 0;
  b->ind = ind;
  b->addr = 0;
  b->target = 0;
  b->sym = NULL;

  return br++;
}

void gstart(void) {
  gbranch(CodeStart);
}

void gend(void) {
  gbranch(CodeEnd);
}

void gline(int linenum) {
  int b;
  
  b = gbranch(CodeLine);
  branch[b].target = linenum;
}

int glabel(void) {
  return gbranch(CodeLabel);
}

int gjmp(int l, int c) {
  int b;
  
  b = gbranch(CodeJump);
  branch[b].target = l;
  branch[b].param = c;
  return b;
}

void galign(int n, int v) {
  int b;
  
  b = gbranch(CodeAlign);
  branch[b].param = n;
  branch[b].target = v;
}

// Output a label and patch all calls to it
void gsym_at(int b, int l) {
  int n;
  while (b) {
    n = branch[b].target;
    branch[b].target = l;
    b = n;
  }
}

// Output label at current location
int gsym(int b) {
  int l;
  
  l = glabel();
  gsym_at(b, l);
  return l;
}

// Assign symbol to label
void assign_label_symbol(int l, Sym *sym) {
  if (branch[l].sym) error("internal error: symbol already assigned to label");
  branch[l].sym = sym;
}

// Output relocation
void greloc(Sym *sym, int c, int rel) {
  int b;
  
  if (sym && !sym->c) put_extern_sym(sym, NULL, 0, 0);

  b = gbranch(CodeReloc);
  if (sym) branch[b].target = sym->c;
  branch[b].param = rel;
  gen_le32(c);
}

// Output constant with relocation if 'r & VT_SYM' is true
void gen_addr32(int r, Sym *sym, int c) {
  if (r & VT_SYM) {
    greloc(sym, c, 0);
  } else {
    gen_le32(c);
  }
}

void mark_code_buffer(CodeBuffer *cb) {
  cb->code = NULL;
  cb->ind = ind;
  cb->branch = NULL;
  cb->br = br;
}

void cut_code_buffer(CodeBuffer *cb) {
  int i;
  int code_buffer_size = ind - cb->ind;
  int branch_buffer_size = br - cb->br;
  cb->code = (unsigned char *) tcc_malloc(code_buffer_size);
  memcpy(cb->code, code + cb->ind, code_buffer_size);
  cb->branch = (Branch *) tcc_malloc(branch_buffer_size * sizeof(Branch));
  memcpy(cb->branch, branch + cb->br, branch_buffer_size * sizeof(Branch));
  for (i = 0; i < branch_buffer_size; ++i) {
    Branch *b = cb->branch + i;
    b->ind -= cb->ind;
    if (b->type == CodeJump) b->target -= cb->br;
  }
  ind = cb->ind;
  br = cb->br;
  cb->ind = code_buffer_size;
  cb->br = branch_buffer_size;
}

void paste_code_buffer(CodeBuffer *cb) {
  int i;
  int ind_ofs = ind;
  int br_ofs = br;
  
  for (i = 0; i < cb->ind; ++i) g(cb->code[i]);
  for (i = 0; i < cb->br; ++i) {
    Branch *b = branch + gbranch(cb->branch[i].type);
    *b = cb->branch[i];
    b->ind += ind_ofs;
    if (b->type == CodeJump) b->target += br_ofs;
  }

  free(cb->code);
  free(cb->branch);
}

void gen(int c) {
  *(char *) section_ptr_add(cur_text_section, 1) = c;
}

void genword(int w) {
  *(int *) section_ptr_add(cur_text_section, 4) = w;
}

void genblk(unsigned char *data, int size) {
  memcpy(section_ptr_add(cur_text_section, size), data, size);
}

int skip_nops(int b, int skip_labels) {
  int org;

  org = branch[b].ind;
  while (branch[b + 1].ind == org) {
    switch (branch[b].type) {
      case CodeNop:
      case CodeLine:
        break;
      
      case CodeLabel:
        if (skip_labels) break;
        
      default:
        return b;
    }
    b++;
  }
  return b;
}

void gcode(void) {
  int i, n, t, r, stacksize, addr, size, pc, disp, rel, errs, more, func_start;
  Branch *b, *bn;

  // Generate function prolog
  func_start = cur_text_section->data_offset;
  if (!func_naked) {
    // Align local size to word
    stacksize = (-loc + 3) & -4;

    if (stacksize >= 4096) {
      // Generate stack guard since parameters can cross page boundary
      Sym *sym = external_global_sym(TOK___chkstk, &func_old_type, 0);
      gen(0xb8); // mov stacksize, %eax
      genword(stacksize);
      gen(0xe8);  // call __chkstk, (does the stackframe too)
      put_reloc(cur_text_section, sym, cur_text_section->data_offset, R_386_PC32);
      genword(-4);
    } else {
      if (do_debug || loc || !func_noargs) {
        gen(0x55); // push %ebp
        gen(0x89); // mov %esp, %ebp
        gen(0xe5);
      }
      if (stacksize > 0) {
        if (stacksize == (char) stacksize) {
          gen(0x83);  // sub esp, stacksize
          gen(0xec);
          gen(stacksize);
        } else {
          gen(0x81);  // sub esp, stacksize
          gen(0xec);
          genword(stacksize);
        }
      }
    }
    
    // Save callee-saved registers used by function.
    for (r = 0; r < NB_REGS; ++r) {
      if ((reg_classes[r] & RC_SAVE) && (regs_used & (1 << r))) {
        gen(0x50 + r); // push r
      }
    }
  }

  // Optimize jumps
  more = 1;
  while (more) {
    more = 0;

    for (i = 0; i < br; ++i) {
      b = branch + i;
      if (b->type == CodeLabel) b->target = 0;
      if (b->type != CodeJump) continue;

      t = skip_nops(b->target, 1);
      if (branch[t].type == CodeJump && !branch[t].param && b->target != branch[t].target) {
        // Eliminate jump to jump
        b->target = branch[t].target;
        more = 1;
        continue;
      }

      // Find next non-nop
      n = i + 1;
      while (branch[n].type == CodeNop || branch[n].type == CodeLine) n++;
      bn = branch + n;
      if (b->ind != bn->ind) continue;

      if (!b->param && bn->type == CodeJump) {
        // Eliminate dead jump instruction
        bn->type = CodeNop;
        more = 1;
        continue;
      }

      if (b->target > i && b->target <= n) {
        // Eliminate jump to next instruction
        b->type = CodeNop;
        more = 1;
        continue;
      }
      
      t = skip_nops(n + 1, 0);
      if (bn->type == CodeJump && !bn->param && b->target == t && bn->ind == branch[t].ind) {
        // Optimize inverted jump
        if (b->param) b->param ^= 1;
        b->target = bn->target;
        bn->type = CodeNop;
        more = 1;
        continue;
      }
    }

    // Eliminate unused labels
    for (i = 0; i < br; ++i) {
      b = branch + i;
      if (b->type == CodeJump) branch[b->target].target++;
    }
    for (i = 0; i < br; ++i) {
      b = branch + i;
      if (b->type == CodeLabel && !b->target && !b->sym) {
        // Remove label with no references
        b->type = CodeNop;
        more = 1;
      }
    }
  }

  // Assign addresses to branch points, assuming only long jumps
  addr = cur_text_section->data_offset;
  pc = 0;
  for (i = 0; i < br; ++i) {
    b = branch + i;
    addr += b->ind - pc;
    b->addr = addr;
    switch (b->type) {
      case CodeJump:
        addr += 5;
        if (b->param != 0) addr++;
        break;
        
      case CodeAlign:
        // Use convervative estimate for short/long jump estimation
        addr += b->param - 1;
        break;
    }
    pc = b->ind;
  }
  
  // Find jumps which can be encoded as short jumps
  for (i = 0; i < br; ++i) {
    b = branch + i;
    if (b->type == CodeJump) {
      disp = branch[b->target].addr - b->addr - 2;
      if (b->param) disp--;
      if (disp == (char) disp) b->type = CodeShortJump;
    }
  }

  // Assign final addresses to branch points
  addr = cur_text_section->data_offset;
  pc = 0;
  for (i = 0; i < br; ++i) {
    b = branch + i;
    addr += b->ind - pc;
    b->addr = addr;
    switch (b->type) {
      case CodeJump:
        addr += 5;
        if (b->param) addr++;
        break;

      case CodeShortJump:
        addr += 2;
        break;
      
      case CodeAlign:
        addr = (addr + b->param - 1) & -b->param;
        break;
    }
    pc = b->ind;
  }

  // Generate code blocks
  pc = 0;
  errs = 0;
  for (i = 0; i < br; ++i) {
    b = branch + i;
    
    // Output code block before branch point
    if (b->ind != pc) {
      genblk(code + pc, b->ind - pc);
      pc = b->ind;
    }

    switch (b->type) {
      case CodeLabel:
        // Export label if symbol defined
        if (b->sym) {
          put_extern_sym_ex(b->sym, cur_text_section, b->addr, 0, 0);
          sym_free(b->sym);
        }
        break;

      case CodeJump:
        // Generate long jump instruction
        if (branch[b->target].type != CodeLabel) {
          printf("internal error: jump %d to non-label %d\n", i, b->target);
          errs++;
        }
        if (b->param > 0xff) error("invalid displacement");
        if (b->param == 0) {
          gen(0xe9);
          genword(branch[b->target].addr - (b->addr + 5));
        } else {
          gen(0x0f);
          gen(b->param - 0x10);
          genword(branch[b->target].addr - (b->addr + 6));
        }
        break;

      case CodeShortJump:
        // Generate short jump instruction
        if (branch[b->target].type != CodeLabel) {
          printf("internal error: jump %d to non-label %d\n", i, b->target);
          errs++;
        }
        if (b->param == 0) {
          gen(0xeb);
        } else {
          gen(b->param - 0x20);
        }
        gen(branch[b->target].addr - (b->addr + 2));
        break;

      case CodeReloc:
        if (b->param) {
          rel = R_386_PC32;
        } else {
          rel = R_386_32;
        }
        put_elf_reloc(symtab_section, cur_text_section, b->addr, rel, b->target);
        break;
        
      case CodeAlign:
        i = addr;
        while (i & (b->param - 1)) {
          gen(b->target);
          i++;
        }
        break;

      case CodeLine:
        put_stabn(N_SLINE, 0, b->target, b->addr - func_start);
        break;
    }
  }

  // Generate function epilog
  if (!func_naked) {
    // Restore callee-saved registers used by function.
    for (r = NB_REGS; r >= 0; --r) {
      if ((reg_classes[r] & RC_SAVE) && (regs_used & (1 << r))) {
        gen(0x58 + r); // pop r
      }
    }

    if (do_debug || loc || !func_noargs) gen(0xc9); // leave

    // Generate return
    if (func_ret_sub == 0) {
      gen(0xc3); // ret
    } else {
      gen(0xc2); // ret n
      gen(func_ret_sub);
      gen(func_ret_sub >> 8);
    }
  }

#ifdef DEBUG_BRANCH
  printf("\nbranch table for %s\n", func_name);
  printf(" #   t targ parm    ind      addr\n");
  printf("---- - ---- ----- -------- --------\n");
  for (i = 0; i < br; ++i) {
    b = branch + i;
    printf("%04d %c %04d %04x %08x %08x", i, "SLJjRANlE"[b->type], b->target, b->param, b->ind, b->addr);
    if (branch[i].sym) {
      printf(" sym=%s", get_tok_str(b->sym->v, NULL));
    }
    printf("\n");
  }
  printf("\n");
#endif

  if (errs) error("internal error: code generation");
}

// Generate a modrm reference. 'op_reg' contains the additional 3
// opcode bits
void gen_modrm(int op_reg, int r, Sym *sym, int c) {
  op_reg = op_reg << 3;
  if ((r & VT_VALMASK) == VT_CONST) {
    // Constant memory reference
    o(0x05 | op_reg);
    gen_addr32(r, sym, c);
  } else if ((r & VT_VALMASK) == VT_LOCAL) {
    // Currently, we use only ebp as base
    if (c == (char) c) {
      // Short reference
      o(0x45 | op_reg);
      g(c);
    } else {
      oad(0x85 | op_reg, c);
    }
  } else {
    g(0x00 | op_reg | (r & VT_VALMASK));
  }
}

// Load 'r' from value 'sv'
void load(int r, SValue *sv) {
  int v, t, ft, fc, fr, a;
  SValue v1;

  fr = sv->r;
  ft = sv->type.t;
  fc = sv->c.ul;
  regs_used |= 1 << r;

  v = fr & VT_VALMASK;
  if (fr & VT_LVAL) {
    if (v == VT_LLOCAL) {
      v1.type.t = VT_INT;
      v1.r = VT_LOCAL | VT_LVAL;
      v1.c.ul = fc;
      load(r, &v1);
      fr = r;
    }
    if ((ft & VT_BTYPE) == VT_FLOAT) {
      o(0xd9); // flds
      r = 0;
    } else if ((ft & VT_BTYPE) == VT_DOUBLE) {
      o(0xdd); // fldl
      r = 0;
    } else if ((ft & VT_BTYPE) == VT_LDOUBLE) {
      o(0xdb); // fldt
      r = 5;
    } else if ((ft & VT_TYPE) == VT_BYTE) {
      o(0xbe0f);   // movsbl
    } else if ((ft & VT_TYPE) == (VT_BYTE | VT_UNSIGNED)) {
      o(0xb60f);   // movzbl
    } else if ((ft & VT_TYPE) == VT_SHORT) {
      o(0xbf0f);   // movswl
    } else if ((ft & VT_TYPE) == (VT_SHORT | VT_UNSIGNED)) {
      o(0xb70f);   // movzwl
    } else {
      o(0x8b);     // movl
    }
    gen_modrm(r, fr, sv->sym, fc);
  } else {
    if (v == VT_CONST) {
      if (fc == 0 && (fr & VT_SYM) == 0) {
        o(0x33); // xor r, r
        o(0xc0 + r + r * 8);
      } else {
        o(0xb8 + r); // mov $xx, r
        gen_addr32(fr, sv->sym, fc);
      }
    } else if (v == VT_LOCAL) {
      o(0x8d); // lea xxx(%ebp), r
      gen_modrm(r, VT_LOCAL, sv->sym, fc);
    } else if (v == VT_CMP) {
      o(0x0f); // setxx br
      o(fc);
      o(0xc0 + r);
      o(0x0f); // movzx r,br
      o(0xb6);
      o(0xc0 + r + r * 8);
    } else if (v == VT_JMP || v == VT_JMPI) {
      t = v & 1;
      oad(0xb8 + r, t); // mov $1, r
      a = gjmp(0, 0); // jmp after
      gsym(fc);
      oad(0xb8 + r, t ^ 1); // mov $0, r
      gsym(a);
    } else if (v != r) {
      o(0x89);
      o(0xc0 + r + v * 8); // mov v, r
    }
  }
}

// Store register 'r' in lvalue 'v'
void store(int r, SValue *v) {
  int fr, bt, ft, fc;

  ft = v->type.t;
  fc = v->c.ul;
  fr = v->r & VT_VALMASK;
  bt = ft & VT_BTYPE;
  regs_used |= 1 << r;

  // TODO: incorrect if float reg to reg
  if (bt == VT_FLOAT) {
    o(0xd9); // fsts
    r = 2;
  } else if (bt == VT_DOUBLE) {
    o(0xdd); // fstpl
    r = 2;
  } else if (bt == VT_LDOUBLE) {
    o(0xc0d9); // fld %st(0)
    o(0xdb); // fstpt
    r = 7;
  } else {
    if (bt == VT_SHORT) o(0x66);
    if (bt == VT_BYTE || bt == VT_BOOL) {
      o(0x88);
    } else {
      o(0x89);
    }
  }
  if (fr == VT_CONST || fr == VT_LOCAL || (v->r & VT_LVAL)) {
    gen_modrm(r, v->r, v->sym, fc);
  } else if (fr != r) {
    o(0xc0 + fr + r * 8); // mov r, fr
  }
}

void gadd_sp(int val) {
  if (val == (char) val) {
    o(0xc483);
    g(val);
  } else {
    oad(0xc481, val); // add $xxx, %esp
  }
}

// 'is_jmp' is '1' if it is a jump
void gcall_or_jmp(int is_jmp) {
  int r;
  Sym *sym;

  if ((vtop->r & (VT_VALMASK | VT_LVAL)) == VT_CONST) {
    // Constant case
    o(0xe8 + is_jmp); // call/jmp im
    sym = NULL;
    if (vtop->r & VT_SYM) sym = vtop->sym;
    greloc(sym, vtop->c.ul - 4, 1);
  } else {
    // Otherwise, indirect call
    r = gv(RC_INT);
    o(0xff); // call/jmp *r
    o(0xd0 + r + (is_jmp << 4));
  }
}

static uint8_t fastcall_regs[3] = { TREG_EAX, TREG_EDX, TREG_ECX };
static uint8_t fastcallw_regs[2] = { TREG_ECX, TREG_EDX };

// Generate function call. The function address is pushed first, then
// all the parameters in call order. This function pops all the
// parameters and the function address.
void gfunc_call(int nb_args) {
  int size, align, r, args_size, i, func_call, v;
  Sym *func_sym;
  
  args_size = 0;
  for (i = 0; i < nb_args; i++) {
    if ((vtop->type.t & VT_BTYPE) == VT_STRUCT) {
      size = type_size(&vtop->type, &align);
      // Align to stack align size
      size = (size + 3) & ~3;
      // Allocate the necessary size on stack
      oad(0xec81, size); // sub $xxx, %esp
      // Generate structure store
      r = get_reg(RC_INT);
      o(0x89); // mov %esp, r
      o(0xe0 + r);
      vset(&vtop->type, r | VT_LVAL, 0);
      vswap();
      vstore();
      args_size += size;
    } else if (is_float(vtop->type.t)) {
      gv(RC_FLOAT); // Only one float register
      if ((vtop->type.t & VT_BTYPE) == VT_FLOAT) {
        size = 4;
      } else if ((vtop->type.t & VT_BTYPE) == VT_DOUBLE) {
        size = 8;
      } else {
        size = 12;
      }
      oad(0xec81, size); // sub $xxx, %esp
      if (size == 12) {
        o(0x7cdb);
      } else {
        o(0x5cd9 + size - 4); // fstp[s|l] 0(%esp)
      }
      g(0x24);
      g(0x00);
      args_size += size;
    } else {
      // Simple type (currently always same size)
      // TODO: implicit cast?
      v = vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM);
      if (v == VT_CONST || v == (VT_CONST | VT_SYM)) {
        // Push constant
        if ((vtop->type.t & VT_BTYPE) == VT_LLONG) {
          size = 8;
          if (vtop->c.word[1] == (char) vtop->c.word[1]) {
            g(0x6a); // push imm8
            g(vtop->c.word[1]);
          } else {
            g(0x68); // push imm32
            gen_le32(vtop->c.word[1]);
          }
        } else {
          size = 4;
        }
        if ((v & VT_SYM) == 0 && vtop->c.i == (char) vtop->c.i) {
          g(0x6a); // push imm8
          g(vtop->c.i);
        } else {
          g(0x68); // push imm32
          gen_addr32(v, vtop->sym, vtop->c.i);
        }
      } else {
        r = gv(RC_INT);
        if ((vtop->type.t & VT_BTYPE) == VT_LLONG) {
          size = 8;
          o(0x50 + vtop->r2); // push r2
        } else {
          size = 4;
        }
        o(0x50 + r); // push r
      }
      args_size += size;
    }
    vtop--;
  }
  save_regs(0); // Save used temporary registers
  func_sym = vtop->type.ref;
  func_call = FUNC_CALL(func_sym->r);

  // fast call case
  if ((func_call >= FUNC_FASTCALL1 && func_call <= FUNC_FASTCALL3) ||
    func_call == FUNC_FASTCALLW) {
    int fastcall_nb_regs;
    uint8_t *fastcall_regs_ptr;
    if (func_call == FUNC_FASTCALLW) {
      fastcall_regs_ptr = fastcallw_regs;
      fastcall_nb_regs = 2;
    } else {
      fastcall_regs_ptr = fastcall_regs;
      fastcall_nb_regs = func_call - FUNC_FASTCALL1 + 1;
    }
    for (i = 0; i < fastcall_nb_regs; i++) {
      if (args_size <= 0) break;
      o(0x58 + fastcall_regs_ptr[i]); // pop r
      // TODO: incorrect for struct/floats
      args_size -= 4;
    }
  }
  gcall_or_jmp(0);
  if (args_size && func_call != FUNC_STDCALL) gadd_sp(args_size);
  vtop--;
}

// Generate function prolog of type 't'
void gfunc_prolog(CType *func_type) {
  int addr, align, size, func_call, fastcall_nb_regs, i;
  int param_index, param_addr;
  uint8_t *fastcall_regs_ptr;
  Sym *sym;
  CType *type;

#ifdef DEBUG_BRANCH
  printf("compile %s\n", func_name);
#endif
  reset_code_buf();
  gbranch(CodeStart);

  sym = func_type->ref;
  func_naked = FUNC_NAKED(sym->r);
  func_call = FUNC_CALL(sym->r);
  addr = 8;
  loc = 0;
  regs_used = 0;
  if (func_call >= FUNC_FASTCALL1 && func_call <= FUNC_FASTCALL3) {
    fastcall_nb_regs = func_call - FUNC_FASTCALL1 + 1;
    fastcall_regs_ptr = fastcall_regs;
  } else if (func_call == FUNC_FASTCALLW) {
    fastcall_nb_regs = 2;
    fastcall_regs_ptr = fastcallw_regs;
  } else {
    fastcall_nb_regs = 0;
    fastcall_regs_ptr = NULL;
  }
  param_index = 0;

  // If the function returns a structure, then add an implicit pointer parameter
  func_vt = sym->type;
  if ((func_vt.t & VT_BTYPE) == VT_STRUCT) {
    // TODO: fastcall case?
    func_vc = addr;
    addr += 4;
    param_index++;
  }

  // Define parameters
  while ((sym = sym->next) != NULL) {
    type = &sym->type;
    size = type_size(type, &align);
    size = (size + 3) & ~3;
    if (param_index < fastcall_nb_regs) {
      // Save FASTCALL register
      if (!func_naked) {
        loc -= 4;
        o(0x89);     // movl
        gen_modrm(fastcall_regs_ptr[param_index], VT_LOCAL, NULL, loc);
        param_addr = loc;
      }
    } else {
      param_addr = addr;
      addr += size;
    }
    sym_push(sym->v & ~SYM_FIELD, type, VT_LOCAL | VT_LVAL, param_addr);
    param_index++;
  }

  // pascal type call?
  func_ret_sub = 0;
  if (func_call == FUNC_STDCALL) func_ret_sub = addr - 8;
  
  func_noargs = (addr == 8);
}

// Generate function epilog
void gfunc_epilog(void) {
  // Mark end of code
  gbranch(CodeEnd);
  
  // Output code for function
  gcode();

  // Clear code buffer
  clear_code_buf();
}

// Generate a test. set 'inv' to invert test. Stack entry is popped.
int gtst(int inv, int t) {
  int v, r, *p;

  v = vtop->r & VT_VALMASK;
  if (v == VT_CMP) {
    // Fast case: can jump directly since flags are set
    t = gjmp(t, vtop->c.i ^ inv); // jcc t
  } else if (v == VT_JMP || v == VT_JMPI) {
    // && or || optimization
    if ((v & 1) == inv) {
      // Insert vtop->c jump list in t
      p = &vtop->c.i;
      while (*p != 0) {
        p = &branch[*p].target;
      }
      *p = t;
      t = vtop->c.i;
    } else {
      t = gjmp(t, 0);
      gsym(vtop->c.i);
    }
  } else {
    if (is_float(vtop->type.t) || (vtop->type.t & VT_BTYPE) == VT_LLONG) {
      vpushi(0);
      gen_op(TOK_NE);
    }
    if ((vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST) {
      // Constant jmp optimization
      if ((vtop->c.i != 0) != inv) t = gjmp(t, 0);
    } else {
      r = gv(RC_INT);
      o(0x85);  // test r,r
      o(0xc0 + r * 9);
      t = gjmp(t, TOK_NE ^ inv); // jz/jnz t
    }
  }
  vtop--;
  return t;
}

// Generate an integer binary operation
void gen_opi(int op) {
  int r, fr, opc, c;

  switch (op) {
    case '+':
    case TOK_ADDC1: // Add with carry generation
      opc = 0;
    gen_op8:
      if ((vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST) {
        // Constant case
        vswap();
        r = gv(RC_INT);
        vswap();
        c = vtop->c.i;
        if (c == (char) c) {
          // Optimize +/- 1 case with inc and dec
          if (op == '+' && c == 1 || op == '-' && c == -1) {
            o(0x40 | r);  // inc r
          } else if (op == '-' && c == 1 || op == '+' && c == -1) {
            o(0x48 | r);  // dec r
          } else {
            o(0x83);
            o(0xc0 | (opc << 3) | r);
            g(c);
          }
        } else {
          o(0x81);
          oad(0xc0 | (opc << 3) | r, c);
        }
      } else {
        gv2(RC_INT, RC_INT);
        r = vtop[-1].r;
        fr = vtop[0].r;
        o((opc << 3) | 0x01);
        o(0xc0 + r + fr * 8); 
      }
      vtop--;
      if (op >= TOK_ULT && op <= TOK_GT) {
        vtop->r = VT_CMP;
        vtop->c.i = op;
      }
      break;
    case '-':
    case TOK_SUBC1: // Subtract with carry generation
      opc = 5;
      goto gen_op8;
    case TOK_ADDC2: // Add with carry use
      opc = 2;
      goto gen_op8;
    case TOK_SUBC2: // Subtract with carry use
      opc = 3;
      goto gen_op8;
    case '&':
      opc = 4;
      goto gen_op8;
    case '^':
      opc = 6;
      goto gen_op8;
    case '|':
      opc = 1;
      goto gen_op8;
    case '*':
      gv2(RC_INT, RC_INT);
      r = vtop[-1].r;
      fr = vtop[0].r;
      vtop--;
      o(0xaf0f); // imul fr, r
      o(0xc0 + fr + r * 8);
      break;
    case TOK_SHL:
      opc = 4;
      goto gen_shift;
    case TOK_SHR:
      opc = 5;
      goto gen_shift;
    case TOK_SAR:
      opc = 7;
    gen_shift:
      opc = 0xc0 | (opc << 3);
      if ((vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST) {
        // Constant case
        vswap();
        r = gv(RC_INT);
        vswap();
        c = vtop->c.i & 0x1f;
        o(0xc1); // shl/shr/sar $xxx, r
        o(opc | r);
        g(c);
      } else {
        // Generate the shift in ecx
        gv2(RC_INT, RC_ECX);
        r = vtop[-1].r;
        o(0xd3); // shl/shr/sar %cl, r
        o(opc | r);
      }
      vtop--;
      break;
    case '/':
    case TOK_UDIV:
    case TOK_PDIV:
    case '%':
    case TOK_UMOD:
    case TOK_UMULL:
      // First operand must be in eax
      // TODO: need better constraint for second operand
      gv2(RC_EAX, RC_ECX);
      r = vtop[-1].r;
      fr = vtop[0].r;
      vtop--;
      save_reg(TREG_EDX);
      if (op == TOK_UMULL) {
        o(0xf7); // mul fr
        o(0xe0 + fr);
        vtop->r2 = TREG_EDX;
        r = TREG_EAX;
      } else {
        if (op == TOK_UDIV || op == TOK_UMOD) {
          o(0xf7d231); // xor %edx, %edx, div fr, %eax
          o(0xf0 + fr);
        } else {
          o(0xf799); // cltd, idiv fr, %eax
          o(0xf8 + fr);
        }
        if (op == '%' || op == TOK_UMOD) {
          r = TREG_EDX;
        } else {
          r = TREG_EAX;
        }
      }
      vtop->r = r;
      break;
    default:
      opc = 7;
      goto gen_op8;
  }
}

// Generate a floating point operation 'v = t1 op t2' instruction. The
// two operands are guaranted to have the same floating point type
// TODO: need to use ST1 too
void gen_opf(int op) {
  int a, ft, fc, swapped, r;

  // Convert constants to memory references
  if ((vtop[-1].r & (VT_VALMASK | VT_LVAL)) == VT_CONST) {
    vswap();
    gv(RC_FLOAT);
    vswap();
  }
  if ((vtop[0].r & (VT_VALMASK | VT_LVAL)) == VT_CONST) gv(RC_FLOAT);

  // Must put at least one value in the floating point register
  if ((vtop[-1].r & VT_LVAL) && (vtop[0].r & VT_LVAL)) {
    vswap();
    gv(RC_FLOAT);
    vswap();
  }
  swapped = 0;
  // Swap the stack if needed so that t1 is the register and t2 is the memory reference
  if (vtop[-1].r & VT_LVAL) {
    vswap();
    swapped = 1;
  }
  if (op >= TOK_ULT && op <= TOK_GT) {
    // Load on stack second operand
    load(TREG_ST0, vtop);
    save_reg(TREG_EAX); // eax is used by FP comparison code
    if (op == TOK_GE || op == TOK_GT) {
      swapped = !swapped;
    } else if (op == TOK_EQ || op == TOK_NE) {
      swapped = 0;
    }
    if (swapped) o(0xc9d9); // fxch %st(1)
    o(0xe9da); // fucompp
    o(0xe0df); // fnstsw %ax
    if (op == TOK_EQ) {
      o(0x45e480); // and $0x45, %ah
      o(0x40fC80); // cmp $0x40, %ah
    } else if (op == TOK_NE) {
      o(0x45e480); // and $0x45, %ah
      o(0x40f480); // xor $0x40, %ah
      op = TOK_NE;
    } else if (op == TOK_GE || op == TOK_LE) {
      o(0x05c4f6); // test $0x05, %ah
      op = TOK_EQ;
    } else {
      o(0x45c4f6); // test $0x45, %ah
      op = TOK_EQ;
    }
    vtop--;
    vtop->r = VT_CMP;
    vtop->c.i = op;
  } else {
    // No memory reference possible for long double operations
    if ((vtop->type.t & VT_BTYPE) == VT_LDOUBLE) {
      load(TREG_ST0, vtop);
      swapped = !swapped;
    }

    switch (op) {
      case '+':
        a = 0;
        break;
      case '-':
        a = 4;
        if (swapped) a++;
        break;
      case '*':
        a = 1;
        break;
      case '/':
        a = 6;
        if (swapped) a++;
        break;
      default:
        a = 0;
    }
    ft = vtop->type.t;
    fc = vtop->c.ul;
    if ((ft & VT_BTYPE) == VT_LDOUBLE) {
      o(0xde); // fxxxp %st, %st(1)
      o(0xc1 + (a << 3));
    } else {
      // If saved lvalue, then we must reload it
      r = vtop->r;
      if ((r & VT_VALMASK) == VT_LLOCAL) {
        SValue v1;
        r = get_reg(RC_INT);
        v1.type.t = VT_INT;
        v1.r = VT_LOCAL | VT_LVAL;
        v1.c.ul = fc;
        load(r, &v1);
        fc = 0;
      }

      if ((ft & VT_BTYPE) == VT_DOUBLE) {
        o(0xdc);
      } else {
        o(0xd8);
      }
      gen_modrm(a, r, vtop->sym, fc);
    }
    vtop--;
  }
}

// Convert integers to fp 't' type. Must handle 'int', 'unsigned int'
// and 'long long' cases.
void gen_cvt_itof(int t) {
  save_reg(TREG_ST0);
  gv(RC_INT);
  if ((vtop->type.t & VT_BTYPE) == VT_LLONG) {
    // signed long long to float/double/long double (unsigned case is handled generically)
    o(0x50 + vtop->r2); // push r2
    o(0x50 + (vtop->r & VT_VALMASK)); // push r
    o(0x242cdf); // fildll (%esp)
    o(0x08c483); // add $8, %esp
  } else if ((vtop->type.t & (VT_BTYPE | VT_UNSIGNED)) == (VT_INT | VT_UNSIGNED)) {
    // unsigned int to float/double/long double
    o(0x6a); // push $0
    g(0x00);
    o(0x50 + (vtop->r & VT_VALMASK)); // push r
    o(0x242cdf); // fildll (%esp)
    o(0x08c483); // add $8, %esp
  } else {
    // int to float/double/long double
    o(0x50 + (vtop->r & VT_VALMASK)); // push r
    o(0x2404db); // fildl (%esp)
    o(0x04c483); // add $4, %esp
  }
  vtop->r = TREG_ST0;
}

// Convert fp to int 't' type
// TODO: handle long long case
void gen_cvt_ftoi(int t) {
  int r, r2, size;
  Sym *sym;
  CType ushort_type;

  ushort_type.t = VT_SHORT | VT_UNSIGNED;

  gv(RC_FLOAT);
  if (t != VT_INT) {
    size = 8;
  } else {
    size = 4;
  }

  o(0x2dd9); // ldcw xxx
  greloc(external_global_sym(TOK___tcc_int_fpu_control, &ushort_type, VT_LVAL), 0, 0);
  
  oad(0xec81, size); // sub $xxx, %esp
  if (size == 4)  {
    o(0x1cdb); // fistpl
  } else {
    o(0x3cdf); // fistpll
  }
  o(0x24);
  o(0x2dd9); // ldcw xxx
  greloc(external_global_sym(TOK___tcc_fpu_control, &ushort_type, VT_LVAL), 0, 0);

  r = get_reg(RC_INT);
  o(0x58 + r); // pop r
  if (size == 8) {
    if (t == VT_LLONG) {
      vtop->r = r; // Mark reg as used
      r2 = get_reg(RC_INT);
      o(0x58 + r2); // pop r2
      vtop->r2 = r2;
    } else {
      o(0x04c483); // add $4, %esp
    }
  }
  vtop->r = r;
}

// Convert from one floating point type to another
void gen_cvt_ftof(int t) {
  // All we have to do on i386 is to put the float in a register
  gv(RC_FLOAT);
}

// Computed goto support
void ggoto(void) {
  gcall_or_jmp(1);
  vtop--;
}

