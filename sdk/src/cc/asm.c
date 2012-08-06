//
//  asm.c - Tiny C Compiler for Sanos
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

int asm_get_local_label_name(TCCState *s1, unsigned int n) {
  char buf[64];
  TokenSym *ts;

  snprintf(buf, sizeof(buf), "L..%u", n);
  ts = tok_alloc(buf, strlen(buf));
  return ts->tok;
}

// We do not use the C expression parser to handle symbols. Maybe the
// C expression parser could be tweaked to do so.
void asm_expr_unary(TCCState *s1, ExprValue *pe) {
  Sym *sym;
  int op, n, label;
  const char *p;

  switch (tok) {
    case TOK_PPNUM:
      p = tokc.cstr->data;
      n = strtoul(p, (char **) &p, 0);
      if (*p == 'b' || *p == 'f') {
        // Backward or forward label
        label = asm_get_local_label_name(s1, n);
        sym = label_find(label);
        if (*p == 'b') {
          // Backward: find the last corresponding defined label
          if (sym && sym->r == 0) sym = sym->prev_tok;
          if (!sym) error("local label '%d' not found backward", n);
        } else {
          // Forward
          if (!sym || sym->r) {
            // If the last label is defined, then define a new one
            sym = label_push(&s1->asm_labels, label, 0);
            sym->type.t = VT_STATIC | VT_LABEL;
          }
        }
        pe->v = 0;
        pe->sym = sym;
      } else if (*p == '\0') {
        pe->v = n;
        pe->sym = NULL;
      } else {
        error("invalid number syntax");
      }
      next();
      break;

    case TOK_CINT:
    case TOK_CUINT:
     pe->v = tokc.i;
     pe->sym = NULL;
     next();
     break;

    case '+':
      next();
      asm_expr_unary(s1, pe);
      break;

    case '-':
    case '~':
      op = tok;
      next();
      asm_expr_unary(s1, pe);
      if (pe->sym) error("invalid operation with label");
      if (op == '-') {
        pe->v = -pe->v;
      } else {
        pe->v = ~pe->v;
      }
      break;

    case TOK_CCHAR:
    case TOK_LCHAR:
	    pe->v = tokc.i;
	    pe->sym = NULL;
	    next();
	    break;

    case '(':
      next();
      asm_expr(s1, pe);
      skip(')');
      break;

    default:
      if (tok >= TOK_IDENT) {
        // Allow all symbols in masm mode
        if (parse_flags & PARSE_FLAG_MASM) {
         sym = sym_find(tok);
         if (!sym) sym = label_find(tok);
        } else {
         sym = label_find(tok);
        }

        // Label case: if the label was not found, add one
        if (!sym) {
          sym = label_push(&s1->asm_labels, tok, 0);
          // NOTE: by default, the symbol is global
          sym->type.t = VT_LABEL;
        }
        if (sym->r == SHN_ABS) {
          // If absolute symbol, no need to put a symbol value
          pe->v = (long) sym->next;
          pe->sym = NULL;
        } else {
          pe->v = 0;
          pe->sym = sym;
        }
        next();
      } else {
        error("bad expression syntax [%s]", get_tok_str(tok, &tokc));
      }
  }
}

void asm_expr_prod(TCCState *s1, ExprValue *pe) {
  int op;
  ExprValue e2;

  asm_expr_unary(s1, pe);
  for (;;) {
    op = tok;
    if (op != '*' && op != '/' && op != '%' && op != TOK_SHL && op != TOK_SAR) break;
    next();
    asm_expr_unary(s1, &e2);
    if (pe->sym || e2.sym) error("invalid operation with label");
    
    switch (op) {
      case '*':
        pe->v *= e2.v;
        break;

      case '/':  
        if (e2.v == 0) {
        div_error:
          error("division by zero");
        }
        pe->v /= e2.v;
        break;

      case '%':  
        if (e2.v == 0) goto div_error;
        pe->v %= e2.v;
        break;

      case TOK_SHL:
        pe->v <<= e2.v;
        break;

      case TOK_SAR:
      default:
        pe->v >>= e2.v;
        break;
    }
  }
}

void asm_expr_logic(TCCState *s1, ExprValue *pe) {
  int op;
  ExprValue e2;

  asm_expr_prod(s1, pe);
  for (;;) {
    op = tok;
    if (op != '&' && op != '|' && op != '^') break;
    next();
    asm_expr_prod(s1, &e2);
    if (pe->sym || e2.sym) error("invalid operation with label");
    switch (op) {
      case '&':
        pe->v &= e2.v;
        break;

      case '|':  
        pe->v |= e2.v;
        break;

      case '^':
      default:
        pe->v ^= e2.v;
        break;
      }
  }
}

void asm_expr_sum(TCCState *s1, ExprValue *pe) {
  int op;
  ExprValue e2;

  asm_expr_logic(s1, pe);
  for (;;) {
    op = tok;
    if (op != '+' && op != '-') break;
    next();
    asm_expr_logic(s1, &e2);
    if (op == '+') {
      if (pe->sym != NULL && e2.sym != NULL) goto cannot_relocate;
      pe->v += e2.v;
      if (pe->sym == NULL && e2.sym != NULL) pe->sym = e2.sym;
    } else {
      pe->v -= e2.v;
      // NOTE: we are less powerful than gas in that case
      // because we store only one symbol in the expression
      if (!pe->sym && !e2.sym) {
        // OK
      } else if (pe->sym && !e2.sym) {
        // OK
      } else if (pe->sym && e2.sym) {
        if (pe->sym == e2.sym) { 
          // OK
        } else if (pe->sym->r == e2.sym->r && pe->sym->r != 0) {
          // We also accept defined symbols in the same section (FIXME)
          pe->v += (long) pe->sym->next - (long) e2.sym->next;
        } else {
          goto cannot_relocate;
        }
        pe->sym = NULL; // same symbols can be subtracted from NULL
      } else {
      cannot_relocate:
        error("invalid operation with label");
      }
    }
  }
}

void asm_expr(TCCState *s1, ExprValue *pe) {
  asm_expr_sum(s1, pe);
}

int asm_int_expr(TCCState *s1) {
  ExprValue e;
  asm_expr(s1, &e);
  if (e.sym) expect("constant");
  return e.v;
}

// NOTE: the same name space as C labels is used to avoid using too
// much memory when storing labels in TokenStrings

void asm_new_abs_label(TCCState *s1, int label, int value) {
  Sym *sym;

  sym = label_find(label);
  if (sym) {
    if (sym->r) {
      // The label is already defined
      error("assembler label '%s' already defined", get_tok_str(label, NULL));
    }
  } else {
    sym = label_push(&s1->asm_labels, label, 0);
    sym->type.t = VT_STATIC;
  }
  sym->r = SHN_ABS;
  sym->next = (void *) value;
}

void asm_new_label(TCCState *s1, int label, int is_local) {
  Sym *sym;

  sym = label_find(label);
  if (sym) {
    if (sym->r) {
      // The label is already defined
      error("assembler label '%s' already defined", get_tok_str(label, NULL));
    } else {
      // Patch existing references to the label
      sym->r = cur_text_section->sh_num;
      sym->next = (void *) gsym((long) sym->next);
    }
  } else {
    sym = label_push(&s1->asm_labels, label, 0);
    sym->type.t = VT_LABEL;
    if (is_local) sym->type.t = VT_STATIC;
    sym->r = cur_text_section->sh_num;
    sym->next = (void *) glabel();
  }
}

void asm_free_labels(TCCState *st) {
  Sym *s, *s1;
  Section *sec;
  int defer;
  
  for (s = st->asm_labels; s != NULL; s = s1) {
    s1 = s->prev;

    // Define symbol value in object file
    defer = 0;
    if (s->r) {
      if (s->r == SHN_ABS) {
        sec = SECTION_ABS;
      } else {
        sec = st->sections[s->r];
        if (sec == cur_text_section) {
          assign_label_symbol((long) s->next, s);
          defer = 1;
        }
      }
      if (!defer) put_extern_sym_ex(s, sec, s->c, 0, 0);
    }

    // Remove label
    table_ident[s->v - TOK_IDENT]->sym_label = NULL;
    if (!defer) sym_free(s);
  }
  st->asm_labels = NULL;
}

void use_section_ex(TCCState *s1, Section *sec) {
  gend();
  gcode();
  clear_code_buf();
  cur_text_section = sec;
  gstart();
}

void use_section(TCCState *s1, const char *name) {
  Section *sec;
  sec = find_section(s1, name);
  use_section_ex(s1, sec);
}

void asm_parse_directive(TCCState *s1) {
  int n, offset, v, size, tok1;
  Section *sec;
  uint8_t *ptr;

  // Assembler directive
  next();
  sec = cur_text_section;
  switch (tok) {
    case TOK_ASM_align:
      next();
      n = asm_int_expr(s1);
      if (n < 0 || (n & (n - 1)) != 0) {
        error("alignment must be a positive power of two");
      }

      // The section must have a compatible alignment
      if (sec->sh_addralign < n) sec->sh_addralign = n;

      v = 0;
      if (tok == ',') {
        next();
        v = asm_int_expr(s1);
      }

      if (sec->sh_type == SHT_NOBITS) {
        sec->data_offset = (sec->data_offset + n - 1) & -n;
      } else {
        galign(n, v);
      }
      break;

    case TOK_ASM_skip:
    case TOK_ASM_space:
      next();
      size = asm_int_expr(s1);
      v = 0;
      if (tok == ',') {
        next();
        v = asm_int_expr(s1);
      }
      if (sec->sh_type == SHT_NOBITS) {
        sec->data_offset += size;
      } else if (sec == cur_text_section) {
        for (n = 0; n < size; ++n) g(v);
      } else {
        memset(section_ptr_add(sec, size), v, size);
      }
      break;

    case TOK_ASM_quad:
      next();
      for (;;) {
        uint64_t vl;
        const char *p;

        p = tokc.cstr->data;
        if (tok != TOK_PPNUM) {
        error_constant:
          error("64 bit constant");
        }
        vl = strtoll(p, (char **)&p, 0);
        if (*p != '\0') goto error_constant;
        next();
        if (sec->sh_type != SHT_NOBITS) {
          // TODO: endianness
          gen_le32(vl);
          gen_le32(vl >> 32);
        } else {
          sec->data_offset += 8;
        }
        if (tok != ',') break;
        next();
      }
      break;

    case TOK_ASM_byte:
      size = 1;
      goto asm_data;

    case TOK_ASM_word:
    case TOK_SHORT:
      size = 2;
      goto asm_data;

    case TOK_LONG:
    case TOK_INT:
      size = 4;
    asm_data:
      next();
      for (;;) {
        ExprValue e;
        asm_expr(s1, &e);
        if (sec->sh_type != SHT_NOBITS) {
          if (size == 4) {
            gen_expr32(&e);
          } else {
            if (e.sym) expect("constant");
            if (size == 1) {
              g(e.v);
            } else {
              gen_le16(e.v);
            }
          }
        } else {
          sec->data_offset += size;
        }
        if (tok != ',') break;
        next();
      }
      break;

    case TOK_ASM_fill: {
      int repeat, size, val, i, j;
      uint8_t repeat_buf[8];
      next();
      repeat = asm_int_expr(s1);
      if (repeat < 0) {
        error("repeat < 0; .fill ignored");
        break;
      }
      size = 1;
      val = 0;
      if (tok == ',') {
        next();
        size = asm_int_expr(s1);
        if (size < 0) {
          error("size < 0; .fill ignored");
          break;
        }
        if (size > 8) size = 8;
        if (tok == ',') {
          next();
          val = asm_int_expr(s1);
        }
      }
      // TODO: endianness
      repeat_buf[0] = val;
      repeat_buf[1] = val >> 8;
      repeat_buf[2] = val >> 16;
      repeat_buf[3] = val >> 24;
      repeat_buf[4] = 0;
      repeat_buf[5] = 0;
      repeat_buf[6] = 0;
      repeat_buf[7] = 0;
      for (i = 0; i < repeat; i++) {
        for (j = 0; j < size; j++) {
          g(repeat_buf[j]);
        }
      }
      break;
    }

    case TOK_ASM_globl:
    case TOK_ASM_global: {
      Sym *sym;

      next();
      sym = label_find(tok);
      if (!sym) {
        sym = label_push(&s1->asm_labels, tok, 0);
        sym->type.t = VT_LABEL;
      }
      sym->type.t &= ~VT_STATIC;
      next();
      break;
    }

    case TOK_ASM_string:
    case TOK_ASM_ascii:
    case TOK_ASM_asciz: {
      const uint8_t *p;
      int i, size, t;

      t = tok;
      next();
      for (;;) {
        if (tok != TOK_STR) expect("string constant");
        p = tokc.cstr->data;
        size = tokc.cstr->size;
        if (t == TOK_ASM_ascii && size > 0) size--;
        for (i = 0; i < size; i++) g(p[i]);
        next();
        if (tok == ',') {
          next();
        } else if (tok != TOK_STR) {
          break;
        }
      }
      break;
	  }

    case TOK_ASM_text:
    case TOK_ASM_data:
    case TOK_ASM_bss: {
      char sname[64];
      tok1 = tok;
      n = 0;
      next();
      if (tok != ';' && tok != TOK_LINEFEED) {
	      n = asm_int_expr(s1);
	      next();
      }
      sprintf(sname, (n?".%s%d":".%s"), get_tok_str(tok1, NULL), n);
      use_section(s1, sname);
      break;
	  }

    case TOK_SECTION1: {
      char sname[256];

      // TODO: support more options
      next();
      sname[0] = '\0';
      while (tok != ';' && tok != TOK_LINEFEED && tok != ',') {
        if (tok == TOK_STR) {
          pstrcat(sname, sizeof(sname), tokc.cstr->data);
        } else {
          pstrcat(sname, sizeof(sname), get_tok_str(tok, NULL));
        }
        next();
      }
      if (tok == ',') {
        // Skip section options
        next();
        if (tok != TOK_STR) expect("string constant");
        next();
      }
      last_text_section = cur_text_section;
      use_section(s1, sname);
      break;
    }

    case TOK_ASM_previous: { 
      Section *sec;
      next();
      if (!last_text_section) error("no previous section referenced");
      sec = cur_text_section;
      use_section_ex(s1, last_text_section);
      last_text_section = sec;
      break;
    }

    default:
      error("unknown assembler directive '.%s'", get_tok_str(tok, NULL));
      break;
  }
}


// Assemble a file
int tcc_assemble_internal(TCCState *s1, int do_preprocess) {
  int opcode;

  // TODO: undefine C labels

  ch = file->buf_ptr[0];
  tok_flags = TOK_FLAG_BOL | TOK_FLAG_BOF;
  parse_flags = PARSE_FLAG_ASM_COMMENTS;
  if (do_preprocess) parse_flags |= PARSE_FLAG_PREPROCESS;
  next();
  for (;;) {
    if (tok == TOK_EOF) break;
    parse_flags |= PARSE_FLAG_LINEFEED; // TODO: suppress that hack
  redo:
    if (tok == '#') {
      // Horrible gas comment
      while (tok != TOK_LINEFEED) next();
    } else if (tok == '.') {
      asm_parse_directive(s1);
    } else if (tok == TOK_PPNUM) {
      const char *p;
      int n;
      p = tokc.cstr->data;
      n = strtoul(p, (char **) &p, 10);
      if (*p != '\0') expect("':'");
      // New local label
      asm_new_label(s1, asm_get_local_label_name(s1, n), 1);
      next();
      skip(':');
      goto redo;
    } else if (tok >= TOK_IDENT) {
      // Instruction or label
      opcode = tok;
      next();
      if (tok == ':') {
        // New label
        asm_new_label(s1, opcode, 0);
        next();
        goto redo;
      } else if (tok == '=') {
        int n;
        next();
        n = asm_int_expr(s1);
        asm_new_abs_label(s1, opcode, n);
        goto redo;
      } else {
        asm_opcode(s1, opcode);
      }
    }

    // End of line
    if (tok != ';' && tok != TOK_LINEFEED){
      expect("end of line");
    }
    parse_flags &= ~PARSE_FLAG_LINEFEED; // TODO: suppress that hack
    next();
  }

  asm_free_labels(s1);

  return 0;
}

// Assemble the current file
int tcc_assemble(TCCState *s1, int do_preprocess) {
  Sym *define_start;
  int ret;

  preprocess_init(s1);

  // Default section is text
  cur_text_section = text_section;
  func_naked = 1;
  reset_code_buf();
  gstart();

  define_start = define_stack;
  ret = tcc_assemble_internal(s1, do_preprocess);
  free_defines(define_start); 

  gend();
  gcode();
  clear_code_buf();

  return ret;
}

// GCC inline asm support
//
// Assemble the string 'str' in the current C compilation unit without
// C preprocessing. NOTE: str is modified by modifying the '\0' at the
// end
void tcc_assemble_inline(TCCState *s1, char *str, int len) {
  BufferedFile *bf, *saved_file;
  int saved_parse_flags, *saved_macro_ptr;

  bf = tcc_malloc(sizeof(BufferedFile));
  memset(bf, 0, sizeof(BufferedFile));
  bf->fd = -1;
  bf->buf_ptr = str;
  bf->buf_end = str + len;
  str[len] = CH_EOB;
  // Same name as current file so that errors are correctly reported
  pstrcpy(bf->filename, sizeof(bf->filename), file->filename);
  bf->line_num = file->line_num;
  saved_file = file;
  file = bf;
  saved_parse_flags = parse_flags;
  saved_macro_ptr = macro_ptr;
  macro_ptr = NULL;
  
  tcc_assemble_internal(s1, 0);

  parse_flags = saved_parse_flags;
  macro_ptr = saved_macro_ptr;
  file = saved_file;
  tcc_free(bf);
}

// Find a constraint by its number or id (gcc 3 extended syntax). 
// Return -1 if not found. Return in *pp in char after the constraint.
int find_constraint(ASMOperand *operands, int nb_operands, const char *name, const char **pp) {
  int index;
  TokenSym *ts;
  const char *p;

  if (is_num(*name)) {
    index = 0;
    while (is_num(*name)) {
      index = (index * 10) + (*name) - '0';
      name++;
    }
    if ((unsigned) index >= nb_operands) index = -1;
  } else if (*name == '[') {
    name++;
    p = strchr(name, ']');
    if (p) {
      ts = tok_alloc(name, p - name);
      for (index = 0; index < nb_operands; index++) {
        if (operands[index].id == ts->tok) goto found;
      }
      index = -1;
    found:
      name = p + 1;
    } else {
      index = -1;
    }
  } else {
    index = -1;
  }

  if (pp) *pp = name;
  return index;
}

void subst_asm_operands(ASMOperand *operands, int nb_operands, int nb_outputs, CString *out_str, CString *in_str) {
  int c, index, modifier;
  const char *str;
  ASMOperand *op;
  SValue sv;

  cstr_new(out_str);
  str = in_str->data;
  for (;;) {
    c = *str++;
    if (c == '%') {
      if (*str == '%') {
        str++;
        goto add_char;
      }
      modifier = 0;
      if (*str == 'c' || *str == 'n' || *str == 'b' || *str == 'w' || *str == 'h') {
        modifier = *str++;
      }
      index = find_constraint(operands, nb_operands, str, &str);
      if (index < 0) error("invalid operand reference after %%");
      op = &operands[index];
      sv = *op->vt;
      if (op->reg >= 0) {
        sv.r = op->reg;
        if ((op->vt->r & VT_VALMASK) == VT_LLOCAL && op->is_memory) {
          sv.r |= VT_LVAL;
        }
      }
      subst_asm_operand(out_str, &sv, modifier);
    } else {
    add_char:
      cstr_ccat(out_str, c);
      if (c == '\0') break;
    }
  }
}

void parse_asm_operands(ASMOperand *operands, int *nb_operands_ptr, int is_output) {
  ASMOperand *op;
  int nb_operands;

  if (tok != ':') {
    nb_operands = *nb_operands_ptr;
    for (;;) {
      if (nb_operands >= MAX_ASM_OPERANDS) error("too many asm operands");
      op = &operands[nb_operands++];
      op->id = 0;
      if (tok == '[') {
        next();
        if (tok < TOK_IDENT) expect("identifier");
        op->id = tok;
        next();
        skip(']');
      }
      if (tok != TOK_STR) expect("string constant");
      op->constraint = tcc_malloc(tokc.cstr->size);
      strcpy(op->constraint, tokc.cstr->data);
      next();
      skip('(');
      gexpr();
      if (is_output) {
        test_lvalue();
      } else {
        // We want to avoid LLOCAL case, except when the 'm'
        // constraint is used. Note that it may come from
        // register storage, so we need to convert (reg)
        // case
        if ((vtop->r & VT_LVAL) &&
            ((vtop->r & VT_VALMASK) == VT_LLOCAL ||
            (vtop->r & VT_VALMASK) < VT_CONST) &&
            !strchr(op->constraint, 'm')) {
          gv(RC_INT);
        }
      }
      op->vt = vtop;
      skip(')');
      if (tok == ',') {
        next();
      } else {
        break;
      }
    }
    *nb_operands_ptr = nb_operands;
  }
}

void parse_asm_str(CString *astr) {
  skip('(');
  // Read the string
  if (tok != TOK_STR) expect("string constant");
  cstr_new(astr);
  while (tok == TOK_STR) {
    // TODO: add \0 handling too?
    cstr_cat(astr, tokc.cstr->data);
    next();
  }
  cstr_ccat(astr, '\0');
}

// Parse the GCC asm() instruction
void asm_instr(void) {
  CString astr, astr1;
  ASMOperand operands[MAX_ASM_OPERANDS];
  int nb_inputs, nb_outputs, nb_operands, i, must_subst, out_reg;
  uint8_t clobber_regs[NB_ASM_REGS];

  next();
  // Since we always generate the asm() instruction, we can ignore volatile
  if (tok == TOK_VOLATILE1 || tok == TOK_VOLATILE2 || tok == TOK_VOLATILE3) {
    next();
  }
  parse_asm_str(&astr);
  nb_operands = 0;
  nb_outputs = 0;
  must_subst = 0;
  memset(clobber_regs, 0, sizeof(clobber_regs));
  if (tok == ':') {
    next();
    must_subst = 1;
    // Output args
    parse_asm_operands(operands, &nb_operands, 1);
    nb_outputs = nb_operands;
    if (tok == ':') {
      next();
      if (tok != ')') {
        // Input args
        parse_asm_operands(operands, &nb_operands, 0);
        if (tok == ':') {
          // Clobber list
          // TODO: handle registers
          next();
          for (;;) {
            if (tok != TOK_STR) expect("string constant");
            asm_clobber(clobber_regs, tokc.cstr->data);
            next();
            if (tok == ',') {
              next();
            } else {
              break;
            }
          }
        }
      }
    }
  }
  skip(')');
  // NOTE: we do not eat the ';' so that we can restore the current
  // token after the assembler parsing.
  if (tok != ';') expect("';'");
  nb_inputs = nb_operands - nb_outputs;
  
  // Save all values in the memory
  save_regs(0);

  // Compute constraints
  asm_compute_constraints(operands, nb_operands, nb_outputs, clobber_regs, &out_reg);

  // Substitute the operands in the asm string. No substitution is
  // done if no operands (GCC behaviour)
#ifdef ASM_DEBUG
  printf("asm: \"%s\"\n", (char *)astr.data);
#endif
  if (must_subst) {
    subst_asm_operands(operands, nb_operands, nb_outputs, &astr1, &astr);
    cstr_free(&astr);
  } else {
    astr1 = astr;
  }
#ifdef ASM_DEBUG
  printf("subst_asm: \"%s\"\n", (char *)astr1.data);
#endif

  // Generate loads
  asm_gen_code(operands, nb_operands, nb_outputs, 0, clobber_regs, out_reg);    

  // Assemble the string with tcc internal assembler
  tcc_assemble_inline(tcc_state, astr1.data, astr1.size - 1);

  // Restore the current C token
  next();

  // Store the output values if needed
  asm_gen_code(operands, nb_operands, nb_outputs, 1, clobber_regs, out_reg);
  
  // Free everything
  for (i = 0;i < nb_operands; i++) {
    ASMOperand *op;
    op = &operands[i];
    tcc_free(op->constraint);
    vpop();
  }
  cstr_free(&astr1);
}

void parse_masm_instr(TCCState *s1) {
  if (tok == TOK_PPNUM) {
    const char *p;
    int n;
    p = tokc.cstr->data;
    n = strtoul(p, (char **) &p, 10);
    if (*p != '\0') expect("':'");
    // New local label
    asm_new_label(s1, asm_get_local_label_name(s1, n), 1);
    next();
    skip(':');
  } else if (tok == TOK_ASM__emit) {
    int opcode;
    next();
    opcode = asm_int_expr(s1);
    g(opcode);
  } else if (tok >= TOK_IDENT) {
    // Instruction or label
    int opcode = tok;
    next();
    if (tok == ':') {
      // New label
      asm_new_label(s1, opcode, 0);
      next();
    } else {
      asm_opcode(s1, opcode);
    }
  } else {
    expect("asm instruction");
  }
}

void masm_instr(TCCState *s1) {
  int opcode;
  int saved_parse_flags = parse_flags;

  parse_flags = PARSE_FLAG_MASM | PARSE_FLAG_PREPROCESS;
  next();
  if (tok == '{') {
    parse_flags |= PARSE_FLAG_LINEFEED;
    next();
    while (tok != '}') {
      if (tok == TOK_EOF) expect("end of block");
      if (tok == TOK_ASM2) {
        next();
        continue;
      }
      if (tok != TOK_LINEFEED) parse_masm_instr(s1);
      if (tok == TOK_LINEFEED || tok == ';') next();
    }
    skip('}');
  } else {
    while (tok == TOK_ASM2) next();
    parse_masm_instr(s1);
    if (tok == ';') next();
  }
  parse_flags = saved_parse_flags;
  if (tok == TOK_LINEFEED) next();
  asm_free_labels(s1);
}

void asm_global_instr(void) {
  CString astr;

  next();
  parse_asm_str(&astr);
  skip(')');
  // NOTE: we do not eat the ';' so that we can restore the current
  // token after the assembler parsing
  if (tok != ';') expect("';'");
  
#ifdef ASM_DEBUG
  printf("asm_global: \"%s\"\n", (char *) astr.data);
#endif

  cur_text_section = text_section;
  func_naked = 1;
  reset_code_buf();
  gstart();

  // Assemble the string with tcc internal assembler
  tcc_assemble_inline(tcc_state, astr.data, astr.size - 1);

  gend();
  gcode();
  clear_code_buf();

  // Restore the current C token
  next();

  cstr_free(&astr);
}
