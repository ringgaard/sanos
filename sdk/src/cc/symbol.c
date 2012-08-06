//
//  symbol.c - Tiny C Compiler for Sanos
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

#define SYM_POOL_NB (8192 / sizeof(Sym))

Section *stab_section, *stabstr_section;
Section *symtab_section, *strtab_section;
Section *text_section, *data_section, *bss_section;
Section *cur_text_section;
Section *last_text_section;

static Sym *sym_free_first;

static Sym *__sym_malloc(void) {
  Sym *sym_pool, *sym, *last_sym;
  int i;

  sym_pool = tcc_malloc(SYM_POOL_NB * sizeof(Sym));

  last_sym = sym_free_first;
  sym = sym_pool;
  for (i = 0; i < SYM_POOL_NB; i++) {
    sym->next = last_sym;
    last_sym = sym;
    sym++;
  }
  sym_free_first = last_sym;
  return last_sym;
}

Sym *sym_malloc(void) {
  Sym *sym;
  sym = sym_free_first;
  if (!sym) sym = __sym_malloc();
  sym_free_first = sym->next;
  return sym;
}

void sym_free(Sym *sym) {
  sym->next = sym_free_first;
  sym_free_first = sym;
}

Section *new_section(TCCState *s1, const char *name, int sh_type, int sh_flags) {
  Section *sec;

  sec = tcc_mallocz(sizeof(Section) + strlen(name));
  strcpy(sec->name, name);
  sec->sh_type = sh_type;
  sec->sh_flags = sh_flags;
  switch (sh_type) {
    case SHT_HASH:
    case SHT_REL:
    case SHT_DYNSYM:
    case SHT_SYMTAB:
    case SHT_DYNAMIC:
    case SHT_PROGBITS:
      sec->sh_addralign = 4;
      break;
    case SHT_STRTAB:
      sec->sh_addralign = 1;
      break;
    default:
      sec->sh_addralign = 32; // default conservative alignment
  }

  // Only add section if not private
  if (!(sh_flags & SHF_PRIVATE)) {
    sec->sh_num = s1->nb_sections;
    dynarray_add((void ***)&s1->sections, &s1->nb_sections, sec);
  }
  return sec;
}

void free_section(Section *s) {
  tcc_free(s->data);
  tcc_free(s);
}

// Realloc section and set its content to zero
void section_realloc(Section *sec, unsigned long new_size) {
  unsigned long size;
  unsigned char *data;
  
  size = sec->data_allocated;
  if (size == 0) size = 1;
  while (size < new_size) size = size * 2;
  data = tcc_realloc(sec->data, size);
  if (!data) error("memory full");
  memset(data + sec->data_allocated, 0, size - sec->data_allocated);
  sec->data = data;
  sec->data_allocated = size;
}

// Reserve at least 'size' bytes in section 'sec' from sec->data_offset
void *section_ptr_add(Section *sec, unsigned long size) {
  unsigned long offset, new_offset;

  offset = sec->data_offset;
  new_offset = offset + size;
  if (new_offset > sec->data_allocated) section_realloc(sec, new_offset);
  sec->data_offset = new_offset;
  return sec->data + offset;
}

// Return a reference to a section, and create it if it does not exists
Section *find_section(TCCState *s1, const char *name) {
  Section *sec;
  int i;
  
  for (i = 1; i < s1->nb_sections; i++) {
    sec = s1->sections[i];
    if (!strcmp(name, sec->name)) return sec;
  }

  // Sections are created as PROGBITS
  return new_section(s1, name, SHT_PROGBITS, SHF_ALLOC);
}

// Update sym->c so that it points to an external symbol in section 
// 'section' with value 'value'
void put_extern_sym_ex(Sym *sym, Section *section, 
                       unsigned long value, unsigned long size,
                       int add_underscore) {
  int sym_type, sym_bind, sh_num, info, other, attr;
  Elf32_Sym *esym;
  const char *name;
  char buf1[256];

  if (section == NULL) {
    sh_num = SHN_UNDEF;
  } else if (section == SECTION_ABS) {
    sh_num = SHN_ABS;
  } else {
    sh_num = section->sh_num;
  }

  other = attr = 0;

  if ((sym->type.t & VT_BTYPE) == VT_FUNC) {
    sym_type = STT_FUNC;
    if (sym->type.ref) attr = sym->type.ref->r;
    if (FUNC_EXPORT(attr)) other |= 1;
    if (FUNC_CALL(attr) == FUNC_STDCALL) other |= 2;
  } else {
    sym_type = STT_OBJECT;
  }

  if (sym->type.t & VT_STATIC) {
    sym_bind = STB_LOCAL;
  } else {
    sym_bind = STB_GLOBAL;
  }

  if (!sym->c) {
    name = get_tok_str(sym->v, NULL);

    if ((other & 2) && add_underscore) {
      sprintf(buf1, "_%s@%d", name, FUNC_ARGS(attr));
      name = buf1;
    } else if (tcc_state->leading_underscore && add_underscore) {
      buf1[0] = '_';
      pstrcpy(buf1 + 1, sizeof(buf1) - 1, name);
      name = buf1;
    }
    info = ELF32_ST_INFO(sym_bind, sym_type);
    sym->c = add_elf_sym(symtab_section, value, size, info, other, sh_num, name);
  } else {
    esym = &((Elf32_Sym *) symtab_section->data)[sym->c];
    esym->st_value = value;
    esym->st_size = size;
    esym->st_shndx = sh_num;
    esym->st_other |= other;
  }
}

void put_extern_sym(Sym *sym, Section *section, unsigned long value, unsigned long size) {
  put_extern_sym_ex(sym, section, value, size, 1);
}

// Add a new relocation entry to symbol 'sym' in section 's'
void put_reloc(Section *s, Sym *sym, unsigned long offset, int type) {
  if (!sym->c) put_extern_sym(sym, NULL, 0, 0);
  // Now we can add ELF relocation info
  put_elf_reloc(symtab_section, s, offset, type, sym->c);
}

// Push, without hashing
Sym *sym_push2(Sym **ps, int v, int t, int c) {
  Sym *s;
  s = sym_malloc();
  s->v = v;
  s->type.t = t;
  s->c = c;
  s->next = NULL;
  // Add in stack
  s->prev = *ps;
  *ps = s;
  return s;
}

// Find a symbol and return its associated structure. 's' is the top of the symbol stack
Sym *sym_find2(Sym *s, int v) {
  while (s) {
    if (s->v == v) return s;
    s = s->prev;
  }
  return NULL;
}

// Push a given symbol on the symbol stack
Sym *sym_push(int v, CType *type, int r, int c) {
  Sym *s, **ps;
  TokenSym *ts;

  if (local_stack) {
    ps = &local_stack;
  } else {
    ps = &global_stack;
  }

  s = sym_push2(ps, v, type->t, c);
  s->type.ref = type->ref;
  s->r = r;
  // Don't record fields or anonymous symbols
  if (!(v & SYM_FIELD) && (v & ~SYM_STRUCT) < SYM_FIRST_ANOM) {
    // Record symbol in token array
    ts = table_ident[(v & ~SYM_STRUCT) - TOK_IDENT];
    if (v & SYM_STRUCT) {
      ps = &ts->sym_struct;
    } else {
      ps = &ts->sym_identifier;
    }
    s->prev_tok = *ps;
    *ps = s;
  }
  return s;
}

// Pop symbols until top reaches 'b'
void sym_pop(Sym **ptop, Sym *b) {
  Sym *s, *ss, **ps;
  TokenSym *ts;
  int v;

  s = *ptop;
  while (s != b) {
    ss = s->prev;
    v = s->v;
    // Remove symbol from token array
    if (!(v & SYM_FIELD) && (v & ~SYM_STRUCT) < SYM_FIRST_ANOM) {
      ts = table_ident[(v & ~SYM_STRUCT) - TOK_IDENT];
      if (v & SYM_STRUCT) {
        ps = &ts->sym_struct;
      } else {
        ps = &ts->sym_identifier;
      }
      *ps = s->prev_tok;
    }
    sym_free(s);
    s = ss;
  }
  *ptop = b;
}

// Find an identifier
Sym *sym_find(int v) {
  v -= TOK_IDENT;
  if ((unsigned)v >= (unsigned)(tok_ident - TOK_IDENT)) return NULL;
  return table_ident[v]->sym_identifier;
}

Sym *global_identifier_push(int v, int t, int c) {
  Sym *s, **ps;
  s = sym_push2(&global_stack, v, t, c);
  // Don't record anonymous symbol
  if (v < SYM_FIRST_ANOM) {
    ps = &table_ident[v - TOK_IDENT]->sym_identifier;
    // Modify the top most local identifier, so that sym_identifier will point to 's' when popped
    while (*ps != NULL) ps = &(*ps)->prev_tok;
    s->prev_tok = NULL;
    *ps = s;
  }
  return s;
}

// Define a new external reference to a symbol 'v' of type 'u'
Sym *external_global_sym(int v, CType *type, int r) {
  Sym *s;

  s = sym_find(v);
  if (!s) {
    // Push forward reference
    s = global_identifier_push(v, type->t | VT_EXTERN, 0);
    s->type.ref = type->ref;
    s->r = r | VT_CONST | VT_SYM;
  }
  return s;
}

// Return a static symbol pointing to a section
Sym *get_sym_ref(CType *type, Section *sec, unsigned long offset, unsigned long size) {
  int v;
  Sym *sym;

  v = anon_sym++;
  sym = global_identifier_push(v, type->t | VT_STATIC, 0);
  sym->type.ref = type->ref;
  sym->r = VT_CONST | VT_SYM;
  put_extern_sym(sym, sec, offset, size);
  return sym;
}

// Define a new external reference to a symbol 'v' of type 'u'
Sym *external_sym(int v, CType *type, int r) {
  Sym *s;

  s = sym_find(v);
  if (!s) {
    // Push forward reference
    s = sym_push(v, type, r | VT_CONST | VT_SYM, 0);
    s->type.t |= VT_EXTERN;
  } else {
    if (!are_compatible_types(&s->type, type)) {
      error("incompatible types for redefinition of '%s'", get_tok_str(v, NULL));
    }
  }
  return s;
}

// Label lookup
Sym *label_find(int v) {
  v -= TOK_IDENT;
  if ((unsigned) v >= (unsigned) (tok_ident - TOK_IDENT)) return NULL;
  return table_ident[v]->sym_label;
}

Sym *label_push(Sym **ptop, int v, int flags) {
  Sym *s, **ps;
  s = sym_push2(ptop, v, 0, 0);
  s->r = flags;
  ps = &table_ident[v - TOK_IDENT]->sym_label;
  if (ptop == &global_label_stack) {
    // Modify the top most local identifier, so that
    // sym_identifier will point to 's' when popped
    while (*ps != NULL) ps = &(*ps)->prev_tok;
  }
  s->prev_tok = *ps;
  *ps = s;
  return s;
}

// Pop labels until element last is reached. Look if any labels are
// undefined. Define symbols if '&&label' was used.
void label_pop(Sym **ptop, Sym *slast) {
  Sym *s, *s1;
  for (s = *ptop; s != slast; s = s1) {
    s1 = s->prev;
    if (s->r == LABEL_DECLARED) {
      warning("label '%s' declared but not used", get_tok_str(s->v, NULL));
    } else if (s->r == LABEL_FORWARD) {
      error("label '%s' used but not defined", get_tok_str(s->v, NULL));
    } else {
      if (s->c) {
        // Define corresponding symbol. A size of 1 is put.
        put_extern_sym(s, cur_text_section, (long)s->next, 1);
      }
    }
    // Remove label
    table_ident[s->v - TOK_IDENT]->sym_label = s->prev_tok;
    sym_free(s);
  }
  *ptop = slast;
}

