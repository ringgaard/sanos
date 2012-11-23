//
//  elf.c - Tiny C Compiler for Sanos
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

#define ELF_START_ADDR 0x08048000
#define ELF_PAGE_SIZE  0x1000

int put_elf_str(Section *s, const char *sym) {
  int offset, len;
  char *ptr;

  len = strlen(sym) + 1;
  offset = s->data_offset;
  ptr = section_ptr_add(s, len);
  memcpy(ptr, sym, len);
  return offset;
}

// ELF symbol hashing function
unsigned long elf_hash(const unsigned char *name) {
  unsigned long h = 0, g;
  
  while (*name) {
    h = (h << 4) + *name++;
    g = h & 0xf0000000;
    if (g) h ^= g >> 24;
    h &= ~g;
  }
  return h;
}

// Rebuild hash table of section s
// NOTE: we do factorize the hash table code to go faster
void rebuild_hash(Section *s, unsigned int nb_buckets) {
  Elf32_Sym *sym;
  int *ptr, *hash, nb_syms, sym_index, h;
  char *strtab;

  strtab = s->link->data;
  nb_syms = s->data_offset / sizeof(Elf32_Sym);

  s->hash->data_offset = 0;
  ptr = section_ptr_add(s->hash, (2 + nb_buckets + nb_syms) * sizeof(int));
  ptr[0] = nb_buckets;
  ptr[1] = nb_syms;
  ptr += 2;
  hash = ptr;
  memset(hash, 0, (nb_buckets + 1) * sizeof(int));
  ptr += nb_buckets + 1;

  sym = (Elf32_Sym *) s->data + 1;
  for (sym_index = 1; sym_index < nb_syms; sym_index++) {
    if (ELF32_ST_BIND(sym->st_info) != STB_LOCAL) {
      h = elf_hash(strtab + sym->st_name) % nb_buckets;
      *ptr = hash[h];
      hash[h] = sym_index;
    } else {
      *ptr = 0;
    }
    ptr++;
    sym++;
  }
}

// Return the symbol number
int put_elf_sym(Section *s, 
                unsigned long value, unsigned long size,
                int info, int other, int shndx, const char *name) {
  int name_offset, sym_index;
  int nbuckets, h;
  Elf32_Sym *sym;
  Section *hs;
  
  sym = section_ptr_add(s, sizeof(Elf32_Sym));
  if (name) {
    name_offset = put_elf_str(s->link, name);
  } else {
    name_offset = 0;
  }
  // TODO: endianness
  sym->st_name = name_offset;
  sym->st_value = value;
  sym->st_size = size;
  sym->st_info = info;
  sym->st_other = other;
  sym->st_shndx = shndx;
  sym_index = sym - (Elf32_Sym *) s->data;
  hs = s->hash;
  if (hs) {
    int *ptr, *base;
    ptr = section_ptr_add(hs, sizeof(int));
    base = (int *) hs->data;
    // Only add global or weak symbols
    if (ELF32_ST_BIND(info) != STB_LOCAL) {
      // Add another hashing entry
      nbuckets = base[0];
      h = elf_hash(name) % nbuckets;
      *ptr = base[h + 2];
      base[h + 2] = sym_index;
      base[1]++;
      // Resize the hash table
      hs->nb_hashed_syms++;
      if (hs->nb_hashed_syms > 2 * nbuckets) {
        rebuild_hash(s, 2 * nbuckets);
      }
    } else {
      *ptr = 0;
      base[1]++;
    }
  }
  return sym_index;
}

// Find global ELF symbol 'name' and return its index. Return 0 if not found
int find_elf_sym(Section *s, const char *name) {
  Elf32_Sym *sym;
  Section *hs;
  int nbuckets, sym_index, h;
  const char *n;
  
  hs = s->hash;
  if (!hs) return 0;
  nbuckets = ((int *) hs->data)[0];
  h = elf_hash(name) % nbuckets;
  sym_index = ((int *) hs->data)[h + 2];
  while (sym_index != 0) {
    sym = &((Elf32_Sym *) s->data)[sym_index];
    n = s->link->data + sym->st_name;
    if (!strcmp(name, n)) return sym_index;
    sym_index = ((int *) hs->data)[nbuckets + sym_index + 2];
  }
  return 0;
}

// Return ELF symbol value or error
int tcc_get_symbol(TCCState *s, unsigned long *pval, const char *name) {
  int sym_index;
  Elf32_Sym *sym;
  
  sym_index = find_elf_sym(symtab_section, name);
  if (!sym_index) return -1;
  sym = &((Elf32_Sym *) symtab_section->data)[sym_index];
  *pval = sym->st_value;
  return 0;
}

void *tcc_get_symbol_err(TCCState *s, const char *name) {
  unsigned long val;
  if (tcc_get_symbol(s, &val, name) < 0) error("%s not defined", name);
  return (void *) val;
}

// Add an ELF symbol. Check if it is already defined and patch
// it. Return symbol index. NOTE that sh_num can be SHN_UNDEF.
int add_elf_sym(Section *s, unsigned long value, unsigned long size,
                int info, int other, int sh_num, const char *name) {
  Elf32_Sym *esym;
  int sym_bind, sym_index, sym_type, esym_bind;
  unsigned char sym_vis, esym_vis, new_vis;

  sym_bind = ELF32_ST_BIND(info);
  sym_type = ELF32_ST_TYPE(info);
  sym_vis = ELF32_ST_VISIBILITY(other);
    
  if (sym_bind != STB_LOCAL) {
    // Search global or weak symbols
    sym_index = find_elf_sym(s, name);
    if (!sym_index) goto do_def;
    esym = &((Elf32_Sym *) s->data)[sym_index];
    if (esym->st_shndx != SHN_UNDEF) {
      esym_bind = ELF32_ST_BIND(esym->st_info);
      // Propagate the most constraining visibility
      // STV_DEFAULT(0) < STV_PROTECTED(3) < STV_HIDDEN(2) < STV_INTERNAL(1)
      esym_vis = ELF32_ST_VISIBILITY(esym->st_other);
      if (esym_vis == STV_DEFAULT) {
        new_vis = sym_vis;
      } else if (sym_vis == STV_DEFAULT) {
        new_vis = esym_vis;
      } else {
        new_vis = (esym_vis < sym_vis) ? esym_vis : sym_vis;
      }
      esym->st_other = (esym->st_other & ~ELF32_ST_VISIBILITY(-1)) | new_vis;
      other = esym->st_other; // in case we have to patch esym
      if (sh_num == SHN_UNDEF) {
        // Ignore adding of undefined symbol if the
        // corresponding symbol is already defined
      } else if (sym_bind == STB_GLOBAL && esym_bind == STB_WEAK) {
        // Global overrides weak, so patch
        goto do_patch;
      } else if (sym_bind == STB_WEAK && esym_bind == STB_GLOBAL) {
        // Weak is ignored if already global
      } else if (sym_vis == STV_HIDDEN || sym_vis == STV_INTERNAL) {
        // Ignore hidden symbols after
      } else if (esym->st_shndx == SHN_COMMON && sh_num < SHN_LORESERVE) {
        // Happens with 'tcc ... -static tcctest.c' on e.g. Ubuntu 6.01
        // No idea if this is the correct solution ...
        goto do_patch;
      } else if (s == tcc_state->dynsymtab_section) {
        // We accept that two DLLs define the same symbol
      } else {
        error_noabort("'%s' defined twice", name);
      }
    } else {
    do_patch:
      esym->st_info = ELF32_ST_INFO(sym_bind, sym_type);
      esym->st_shndx = sh_num;
      esym->st_value = value;
      esym->st_size = size;
      esym->st_other = other;
    }
  } else {
  do_def:
    sym_index = put_elf_sym(s, value, size, ELF32_ST_INFO(sym_bind, sym_type), other, sh_num, name);
  }
  return sym_index;
}

// Put relocation
void put_elf_reloc(Section *symtab, Section *s, unsigned long offset, int type, int symbol) {
  char buf[256];
  Section *sr;
  Elf32_Rel *rel;

  sr = s->reloc;
  if (!sr) {
    // If no relocation section, create it
    snprintf(buf, sizeof(buf), ".rel%s", s->name);
    // If the symtab is allocated, then we consider the relocation are also
    sr = new_section(tcc_state, buf, SHT_REL, symtab->sh_flags);
    sr->sh_entsize = sizeof(Elf32_Rel);
    sr->link = symtab;
    sr->sh_info = s->sh_num;
    s->reloc = sr;
  }
  rel = section_ptr_add(sr, sizeof(Elf32_Rel));
  rel->r_offset = offset;
  rel->r_info = ELF32_R_INFO(symbol, type);
}

// Put stab debug information
void put_stabs(const char *str, int type, int other, int desc, unsigned long value) {
  Stab_Sym *sym;

  sym = section_ptr_add(stab_section, sizeof(Stab_Sym));
  if (str) {
    sym->n_strx = put_elf_str(stabstr_section, str);
  } else {
    sym->n_strx = 0;
  }
  sym->n_type = type;
  sym->n_other = other;
  sym->n_desc = desc;
  sym->n_value = value;
}

void put_stabs_r(const char *str, int type, int other, int desc, unsigned long value, Section *sec, int sym_index) {
  put_stabs(str, type, other, desc, value);
  put_elf_reloc(symtab_section, stab_section, stab_section->data_offset - sizeof(unsigned long), R_386_32, sym_index);
}

void put_stabn(int type, int other, int desc, int value) {
  put_stabs(NULL, type, other, desc, value);
}

void put_stabd(int type, int other, int desc) {
  put_stabs(NULL, type, other, desc, 0);
}

// In an ELF file symbol table, the local symbols must appear below
// the global and weak ones. Since TCC cannot sort it while generating
// the code, we must do it after. All the relocation tables are also
// modified to take into account the symbol table sorting

void sort_syms(TCCState *s1, Section *s) {
  int *old_to_new_syms;
  Elf32_Sym *new_syms;
  int nb_syms, i;
  Elf32_Sym *p, *q;
  Elf32_Rel *rel, *rel_end;
  Section *sr;
  int type, sym_index;

  nb_syms = s->data_offset / sizeof(Elf32_Sym);
  new_syms = tcc_malloc(nb_syms * sizeof(Elf32_Sym));
  old_to_new_syms = tcc_malloc(nb_syms * sizeof(int));

  // First pass for local symbols
  p = (Elf32_Sym *)s->data;
  q = new_syms;
  for (i = 0; i < nb_syms; i++) {
    if (ELF32_ST_BIND(p->st_info) == STB_LOCAL) {
      old_to_new_syms[i] = q - new_syms;
      *q++ = *p;
    }
    p++;
  }
  // Save the number of local symbols in section header
  s->sh_info = q - new_syms;

  // Then second pass for non local symbols
  p = (Elf32_Sym *)s->data;
  for (i = 0; i < nb_syms; i++) {
    if (ELF32_ST_BIND(p->st_info) != STB_LOCAL) {
      old_to_new_syms[i] = q - new_syms;
      *q++ = *p;
    }
    p++;
  }
  
  // Copy the new symbols to the old
  memcpy(s->data, new_syms, nb_syms * sizeof(Elf32_Sym));
  tcc_free(new_syms);

  // Now modify all the relocations
  for (i = 1; i < s1->nb_sections; i++) {
    sr = s1->sections[i];
    if (sr->sh_type == SHT_REL && sr->link == s) {
      rel_end = (Elf32_Rel *)(sr->data + sr->data_offset);
      for (rel = (Elf32_Rel *)sr->data; rel < rel_end; rel++) {
        sym_index = ELF32_R_SYM(rel->r_info);
        type = ELF32_R_TYPE(rel->r_info);
        sym_index = old_to_new_syms[sym_index];
        rel->r_info = ELF32_R_INFO(sym_index, type);
      }
    }
  }
  
  tcc_free(old_to_new_syms);
}

// Relocate common symbols in the .bss section
void relocate_common_syms(void) {
  Elf32_Sym *sym, *sym_end;
  unsigned long offset, align;
  
  sym_end = (Elf32_Sym *) (symtab_section->data + symtab_section->data_offset);
  for (sym = (Elf32_Sym *) symtab_section->data + 1; sym < sym_end; sym++) {
    if (sym->st_shndx == SHN_COMMON) {
      // Align symbol
      align = sym->st_value;
      offset = bss_section->data_offset;
      offset = (offset + align - 1) & -align;
      sym->st_value = offset;
      sym->st_shndx = bss_section->sh_num;
      offset += sym->st_size;
      bss_section->data_offset = offset;
    }
  }
}

// Relocate symbol table, resolve undefined symbols if do_resolve is
// true and output error if undefined symbol
void relocate_syms(TCCState *s1, int do_resolve) {
  Elf32_Sym *sym, *esym, *sym_end;
  int sym_bind, sh_num, sym_index;
  const char *name;
  unsigned long addr;

  sym_end = (Elf32_Sym *) (symtab_section->data + symtab_section->data_offset);
  for (sym = (Elf32_Sym *) symtab_section->data + 1; sym < sym_end; sym++) {
    sh_num = sym->st_shndx;
    if (sh_num == SHN_UNDEF) {
      name = strtab_section->data + sym->st_name;
      if (do_resolve) {
        name = symtab_section->link->data + sym->st_name;
        addr = (unsigned long) resolve_sym(s1, name, ELF32_ST_TYPE(sym->st_info));
        if (addr) {
          sym->st_value = addr;
          goto found;
        }
      } else if (s1->dynsym) {
        // If dynamic symbol exist, then use it
        sym_index = find_elf_sym(s1->dynsym, name);
        if (sym_index) {
          esym = &((Elf32_Sym *)s1->dynsym->data)[sym_index];
          sym->st_value = esym->st_value;
          goto found;
        }
      }
      // Only weak symbols are accepted to be undefined. Their value is zero
      sym_bind = ELF32_ST_BIND(sym->st_info);
      if (sym_bind == STB_WEAK) {
        sym->st_value = 0;
      } else {
        error_noabort("undefined symbol '%s'", name);
      }
    } else if (sh_num < SHN_LORESERVE) {
      // Add section base
      sym->st_value += s1->sections[sym->st_shndx]->sh_addr;
    }
  found: ;
  }
}

// Relocate a given section
void relocate_section(TCCState *s1, Section *s) {
  Section *sr;
  Elf32_Rel *rel, *rel_end, *qrel;
  Elf32_Sym *sym;
  int type, sym_index;
  unsigned char *ptr;
  unsigned long val, addr;
  int esym_index;

  sr = s->reloc;
  rel_end = (Elf32_Rel *) (sr->data + sr->data_offset);
  qrel = (Elf32_Rel *) sr->data;
  for (rel = qrel; rel < rel_end; rel++) {
    ptr = s->data + rel->r_offset;

    sym_index = ELF32_R_SYM(rel->r_info);
    sym = &((Elf32_Sym *) symtab_section->data)[sym_index];
    val = sym->st_value;
    type = ELF32_R_TYPE(rel->r_info);
    addr = s->sh_addr + rel->r_offset;
    switch (type) {
      case R_386_32:
        if (s1->output_type == TCC_OUTPUT_DLL) {
          esym_index = s1->symtab_to_dynsym[sym_index];
          qrel->r_offset = rel->r_offset;
          if (esym_index) {
            qrel->r_info = ELF32_R_INFO(esym_index, R_386_32);
            qrel++;
            break;
          } else {
            qrel->r_info = ELF32_R_INFO(0, R_386_RELATIVE);
            qrel++;
          }
        }
        *(int *) ptr += val;
        break;
      case R_386_PC32:
        if (s1->output_type == TCC_OUTPUT_DLL) {
          // DLL relocation
          esym_index = s1->symtab_to_dynsym[sym_index];
          if (esym_index) {
            qrel->r_offset = rel->r_offset;
            qrel->r_info = ELF32_R_INFO(esym_index, R_386_PC32);
            qrel++;
            break;
          }
        }
        *(int *) ptr += val - addr;
        break;
      case R_386_PLT32:
        *(int *) ptr += val - addr;
        break;
      case R_386_GLOB_DAT:
      case R_386_JMP_SLOT:
        *(int *) ptr = val;
        break;
      case R_386_GOTPC:
        *(int *) ptr += s1->got->sh_addr - addr;
        break;
      case R_386_GOTOFF:
        *(int *) ptr += val - s1->got->sh_addr;
        break;
      case R_386_GOT32:
        // Load the got offset
        *(int *) ptr += s1->got_offsets[sym_index];
        break;
    }
  }

  // If the relocation is allocated, we change its symbol table
  if (sr->sh_flags & SHF_ALLOC) sr->link = s1->dynsym;
}

// Relocate relocation table in 'sr'
void relocate_rel(TCCState *s1, Section *sr) {
  Section *s;
  Elf32_Rel *rel, *rel_end;
  
  s = s1->sections[sr->sh_info];
  rel_end = (Elf32_Rel *) (sr->data + sr->data_offset);
  for (rel = (Elf32_Rel *) sr->data; rel < rel_end; rel++) {
    rel->r_offset += s->sh_addr;
  }
}

// Count the number of dynamic relocations so that we can reserve their space
int prepare_dynamic_rel(TCCState *s1, Section *sr) {
  Elf32_Rel *rel, *rel_end;
  int sym_index, esym_index, type, count;

  count = 0;
  rel_end = (Elf32_Rel *)(sr->data + sr->data_offset);
  for (rel = (Elf32_Rel *) sr->data; rel < rel_end; rel++) {
    sym_index = ELF32_R_SYM(rel->r_info);
    type = ELF32_R_TYPE(rel->r_info);
    switch (type) {
      case R_386_32:
        count++;
        break;
      case R_386_PC32:
        esym_index = s1->symtab_to_dynsym[sym_index];
        if (esym_index) count++;
        break;
      default:
        break;
    }
  }
  if (count) {
    // Allocate the section
    sr->sh_flags |= SHF_ALLOC;
    sr->sh_size = count * sizeof(Elf32_Rel);
  }
  return count;
}

void put_got_offset(TCCState *s1, int index, unsigned long val) {
  int n;
  unsigned long *tab;

  if (index >= s1->nb_got_offsets) {
    // Find immediately bigger power of 2 and reallocate array
    n = 1;
    while (index >= n) n *= 2;
    tab = tcc_realloc(s1->got_offsets, n * sizeof(unsigned long));
    if (!tab) error("memory full");
    s1->got_offsets = tab;
    memset(s1->got_offsets + s1->nb_got_offsets, 0, (n - s1->nb_got_offsets) * sizeof(unsigned long));
    s1->nb_got_offsets = n;
  }
  s1->got_offsets[index] = val;
}

static void put32(unsigned char *p, uint32_t val) {
  p[0] = val;
  p[1] = val >> 8;
  p[2] = val >> 16;
  p[3] = val >> 24;
}

static uint32_t get32(unsigned char *p)
{
  return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

void build_got(TCCState *s1) {
  unsigned char *ptr;

  // If no got, then create it
  s1->got = new_section(s1, ".got", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE);
  s1->got->sh_entsize = 4;
  add_elf_sym(symtab_section, 0, 4, ELF32_ST_INFO(STB_GLOBAL, STT_OBJECT), 0, s1->got->sh_num, "_GLOBAL_OFFSET_TABLE_");
  ptr = section_ptr_add(s1->got, 3 * sizeof(int));
  // Keep space for _DYNAMIC pointer, if present
  put32(ptr, 0);
  // Two dummy got entries
  put32(ptr + 4, 0);
  put32(ptr + 8, 0);
}

// Put a got entry corresponding to a symbol in symtab_section. 'size'
// and 'info' can be modifed if more precise info comes from the DLL.
void put_got_entry(TCCState *s1, int reloc_type, unsigned long size, int info, int sym_index) {
  int index;
  const char *name;
  Elf32_Sym *sym;
  unsigned long offset;
  int *ptr;

  if (!s1->got) build_got(s1);

  // If a got entry already exists for that symbol, no need to add one
  if (sym_index < s1->nb_got_offsets && s1->got_offsets[sym_index] != 0) return;
  
  put_got_offset(s1, sym_index, s1->got->data_offset);

  if (s1->dynsym) {
    sym = &((Elf32_Sym *) symtab_section->data)[sym_index];
    name = symtab_section->link->data + sym->st_name;
    offset = sym->st_value;
    if (reloc_type == R_386_JMP_SLOT) {
      Section *plt;
      uint8_t *p;
      int modrm;

      // If we build a DLL, we add a %ebx offset
      if (s1->output_type == TCC_OUTPUT_DLL) {
        modrm = 0xa3;
      } else {
        modrm = 0x25;
      }

      // Add a PLT entry
      plt = s1->plt;
      if (plt->data_offset == 0) {
        // First PLT entry
        p = section_ptr_add(plt, 16);
        p[0] = 0xff; // pushl got + 4
        p[1] = modrm + 0x10;
        put32(p + 2, 4);
        p[6] = 0xff; // jmp *(got + 8)
        p[7] = modrm;
        put32(p + 8, 8);
      }

      p = section_ptr_add(plt, 16);
      p[0] = 0xff; // jmp *(got + x)
      p[1] = modrm;
      put32(p + 2, s1->got->data_offset);
      p[6] = 0x68; // push $xxx
      put32(p + 7, (plt->data_offset - 32) >> 1);
      p[11] = 0xe9; // jmp plt_start
      put32(p + 12, -(plt->data_offset));

      // The symbol is modified so that it will be relocated to the PLT
      if (s1->output_type == TCC_OUTPUT_EXE) offset = plt->data_offset - 16;
    }
    index = put_elf_sym(s1->dynsym, offset, size, info, 0, sym->st_shndx, name);
    // Put a got entry
    put_elf_reloc(s1->dynsym, s1->got, s1->got->data_offset, reloc_type, index);
  }
  ptr = section_ptr_add(s1->got, sizeof(int));
  *ptr = 0;
}

// Build GOT and PLT entries
void build_got_entries(TCCState *s1) {
  Section *s, *symtab;
  Elf32_Rel *rel, *rel_end;
  Elf32_Sym *sym;
  int i, type, reloc_type, sym_index;

  for (i = 1; i < s1->nb_sections; i++) {
    s = s1->sections[i];
    if (s->sh_type != SHT_REL) continue;
    // No need to handle got relocations
    if (s->link != symtab_section) continue;
    symtab = s->link;
    rel_end = (Elf32_Rel *)(s->data + s->data_offset);
    for (rel = (Elf32_Rel *) s->data; rel < rel_end; rel++) {
      type = ELF32_R_TYPE(rel->r_info);
      if (type == R_386_GOT32 || type == R_386_GOTOFF || type == R_386_GOTPC || type == R_386_PLT32) {
        if (!s1->got) build_got(s1);
        if (type == R_386_GOT32 || type == R_386_PLT32) {
          sym_index = ELF32_R_SYM(rel->r_info);
          sym = &((Elf32_Sym *) symtab_section->data)[sym_index];
          // Look at the symbol got offset. If none, then add one
          if (type == R_386_GOT32) {
            reloc_type = R_386_GLOB_DAT;
          } else {
            reloc_type = R_386_JMP_SLOT;
          }
          put_got_entry(s1, reloc_type, sym->st_size, sym->st_info, sym_index);
        }
      }
    }
  }
}

Section *new_symtab(TCCState *s1,
                    const char *symtab_name, int sh_type, int sh_flags,
                    const char *strtab_name, 
                    const char *hash_name, int hash_sh_flags) {
  Section *symtab, *strtab, *hash;
  int *ptr, nb_buckets;

  symtab = new_section(s1, symtab_name, sh_type, sh_flags);
  symtab->sh_entsize = sizeof(Elf32_Sym);
  strtab = new_section(s1, strtab_name, SHT_STRTAB, sh_flags);
  put_elf_str(strtab, "");
  symtab->link = strtab;
  put_elf_sym(symtab, 0, 0, 0, 0, 0, NULL);
  
  nb_buckets = 1;

  hash = new_section(s1, hash_name, SHT_HASH, hash_sh_flags);
  hash->sh_entsize = sizeof(int);
  symtab->hash = hash;
  hash->link = symtab;

  ptr = section_ptr_add(hash, (2 + nb_buckets + 1) * sizeof(int));
  ptr[0] = nb_buckets;
  ptr[1] = 1;
  memset(ptr + 2, 0, (nb_buckets + 1) * sizeof(int));
  return symtab;
}

// Put dynamic tag
void put_dt(Section *dynamic, int dt, unsigned long val) {
  Elf32_Dyn *dyn;
  dyn = section_ptr_add(dynamic, sizeof(Elf32_Dyn));
  dyn->d_tag = dt;
  dyn->d_un.d_val = val;
}

void add_init_array_defines(TCCState *s1, const char *section_name) {
  Section *s;
  long end_offset;
  char sym_start[1024];
  char sym_end[1024];
  
  snprintf(sym_start, sizeof(sym_start), "__%s_start", section_name + 1);
  snprintf(sym_end, sizeof(sym_end), "__%s_end", section_name + 1);

  s = find_section(s1, section_name);
  if (!s) {
    end_offset = 0;
    s = data_section;
  } else {
    end_offset = s->data_offset;
  }

  add_elf_sym(symtab_section, 0, 0, ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE), 0, s->sh_num, sym_start);
  add_elf_sym(symtab_section, end_offset, 0, ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE), 0, s->sh_num, sym_end);
}

// Add TCC runtime libraries
void tcc_add_runtime(TCCState *s1) {
  char buf[1024];

  // Add libc
  if (!s1->nostdlib) {
    tcc_add_library(s1, "c");
    snprintf(buf, sizeof(buf), "%s/%s", tcc_lib_path, "libtcc1.a");
    tcc_add_file(s1, buf);
  }

  // Add crt end
  if (!s1->nostdlib) {
    tcc_add_file(s1, CONFIG_TCC_CRT_PREFIX "/crtn.o");
  }
}

// Add various standard linker symbols (must be done after the
// sections are filled (for example after allocating common
// symbols))
void tcc_add_linker_symbols(TCCState *s1) {
  char buf[1024];
  int i;
  Section *s;

  add_elf_sym(symtab_section, 
              text_section->data_offset, 0,
              ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE), 0,
              text_section->sh_num, "_etext");
  add_elf_sym(symtab_section, 
              data_section->data_offset, 0,
              ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE), 0,
              data_section->sh_num, "_edata");
  add_elf_sym(symtab_section, 
              bss_section->data_offset, 0,
              ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE), 0,
              bss_section->sh_num, "_end");

  // Horrible new standard ldscript defines
  add_init_array_defines(s1, ".preinit_array");
  add_init_array_defines(s1, ".init_array");
  add_init_array_defines(s1, ".fini_array");
  
  // Add start and stop symbols for sections whose name can be expressed in C
  for (i = 1; i < s1->nb_sections; i++) {
    s = s1->sections[i];
    if (s->sh_type == SHT_PROGBITS && (s->sh_flags & SHF_ALLOC)) {
      const char *p;
      int ch;

      // Check if section name can be expressed in C
      p = s->name;
      for (;;) {
        ch = *p;
        if (!ch) break;
        if (!is_id(ch) && !is_num(ch)) goto next_sec;
        p++;
      }

      snprintf(buf, sizeof(buf), "__start_%s", s->name);
      add_elf_sym(symtab_section, 
                  0, 0,
                  ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE), 0,
                  s->sh_num, buf);

      snprintf(buf, sizeof(buf), "__stop_%s", s->name);
      add_elf_sym(symtab_section,
                  s->data_offset, 0,
                  ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE), 0,
                  s->sh_num, buf);
    }
  next_sec: ;
  }
}

static char elf_interp[] = "/lib/ld-linux.so.2";

void tcc_output_binary(TCCState *s1, FILE *f, const int *section_order) {
  Section *s;
  int i, offset, size;

  offset = 0;
  for (i = 1; i< s1->nb_sections; i++) {
    s = s1->sections[section_order[i]];
    if (s->sh_type != SHT_NOBITS && (s->sh_flags & SHF_ALLOC)) {
      while (offset < s->sh_offset) {
        fputc(0, f);
        offset++;
      }
      size = s->sh_size;
      fwrite(s->data, 1, size, f);
      offset += size;
    }
  }
}

// Output an ELF file
// TODO: suppress unneeded sections
int tcc_output_file(TCCState *s1, const char *filename) {
  Elf32_Ehdr ehdr;
  FILE *f;
  int fd, mode, ret;
  int *section_order;
  int shnum, i, phnum, file_offset, offset, size, j, tmp, sh_order_index, k;
  unsigned long addr;
  Section *strsec, *s;
  Elf32_Shdr shdr, *sh;
  Elf32_Phdr *phdr, *ph;
  Section *interp, *dynamic, *dynstr;
  unsigned long saved_dynamic_data_offset;
  Elf32_Sym *sym;
  int type, file_type;
  unsigned long rel_addr, rel_size;
  
  file_type = s1->output_type;
  s1->nb_errors = 0;

  if (file_type != TCC_OUTPUT_OBJ) {
    tcc_add_runtime(s1);
  }

  phdr = NULL;
  section_order = NULL;
  interp = NULL;
  dynamic = NULL;
  dynstr = NULL; // avoid warning
  saved_dynamic_data_offset = 0; // avoid warning
  
  if (file_type != TCC_OUTPUT_OBJ) {
    relocate_common_syms();

    tcc_add_linker_symbols(s1);

    if (!s1->static_link) {
      const char *name;
      int sym_index, index;
      Elf32_Sym *esym, *sym_end;
      
      if (file_type == TCC_OUTPUT_EXE) {
        char *ptr;
        // Add interpreter section only if executable
        interp = new_section(s1, ".interp", SHT_PROGBITS, SHF_ALLOC);
        interp->sh_addralign = 1;
        ptr = section_ptr_add(interp, sizeof(elf_interp));
        strcpy(ptr, elf_interp);
      }
    
      // Add dynamic symbol table
      s1->dynsym = new_symtab(s1, ".dynsym", SHT_DYNSYM, SHF_ALLOC, ".dynstr", ".hash", SHF_ALLOC);
      dynstr = s1->dynsym->link;
      
      // Add dynamic section
      dynamic = new_section(s1, ".dynamic", SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE);
      dynamic->link = dynstr;
      dynamic->sh_entsize = sizeof(Elf32_Dyn);
    
      // Add PLT
      s1->plt = new_section(s1, ".plt", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR);
      s1->plt->sh_entsize = 4;

      build_got(s1);

      // Scan for undefined symbols and see if they are in the
      // dynamic symbols. If a symbol STT_FUNC is found, then we
      // add it in the PLT. If a symbol STT_OBJECT is found, we
      // add it in the .bss section with a suitable relocation.
      sym_end = (Elf32_Sym *)(symtab_section->data + symtab_section->data_offset);
      if (file_type == TCC_OUTPUT_EXE) {
        for (sym = (Elf32_Sym *) symtab_section->data + 1; sym < sym_end; sym++) {
          if (sym->st_shndx == SHN_UNDEF) {
            name = symtab_section->link->data + sym->st_name;
            sym_index = find_elf_sym(s1->dynsymtab_section, name);
            if (sym_index) {
              esym = &((Elf32_Sym *) s1->dynsymtab_section->data)[sym_index];
              type = ELF32_ST_TYPE(esym->st_info);
              if (type == STT_FUNC) {
                put_got_entry(s1, R_386_JMP_SLOT, esym->st_size, esym->st_info, sym - (Elf32_Sym *)symtab_section->data);
              } else if (type == STT_OBJECT) {
                unsigned long offset;
                offset = bss_section->data_offset;
                // TODO: which alignment ?
                offset = (offset + 16 - 1) & -16;
                index = put_elf_sym(s1->dynsym, offset, esym->st_size, esym->st_info, 0, bss_section->sh_num, name);
                put_elf_reloc(s1->dynsym, bss_section, offset, R_386_COPY, index);
                offset += esym->st_size;
                bss_section->data_offset = offset;
              }
            } else {
              // STB_WEAK undefined symbols are accepted
              if (ELF32_ST_BIND(sym->st_info) != STB_WEAK) {
                error_noabort("undefined symbol '%s'", name);
              }
            }
          } else if (s1->rdynamic && ELF32_ST_BIND(sym->st_info) != STB_LOCAL) {
            // If -rdynamic option, then export all non-local symbols
            name = symtab_section->link->data + sym->st_name;
            put_elf_sym(s1->dynsym, sym->st_value, sym->st_size, sym->st_info, 0, sym->st_shndx, name);
          }
        }
      
        if (s1->nb_errors) goto fail;

        // Now look at unresolved dynamic symbols and export corresponding symbol
        sym_end = (Elf32_Sym *)(s1->dynsymtab_section->data + s1->dynsymtab_section->data_offset);
        for (esym = (Elf32_Sym *)s1->dynsymtab_section->data + 1; esym < sym_end; esym++) {
          if (esym->st_shndx == SHN_UNDEF) {
            name = s1->dynsymtab_section->link->data + esym->st_name;
            sym_index = find_elf_sym(symtab_section, name);
            if (sym_index) {
              // TODO: avoid adding a symbol if already present because of -rdynamic ?
              sym = &((Elf32_Sym *)symtab_section->data)[sym_index];
              put_elf_sym(s1->dynsym, sym->st_value, sym->st_size, sym->st_info, 0, sym->st_shndx, name);
            } else {
              // Weak symbols can stay undefined
              if (ELF32_ST_BIND(esym->st_info) != STB_WEAK) {
                warning("undefined dynamic symbol '%s'", name);
              }
            }
          }
        }
      } else {
        // Shared library case: simply export all the global symbols
        int nb_syms = symtab_section->data_offset / sizeof(Elf32_Sym);
        s1->symtab_to_dynsym = tcc_mallocz(sizeof(int) * nb_syms);
        for (sym = (Elf32_Sym *) symtab_section->data + 1; sym < sym_end; sym++) {
          if (ELF32_ST_BIND(sym->st_info) != STB_LOCAL) {
            name = symtab_section->link->data + sym->st_name;
            index = put_elf_sym(s1->dynsym, sym->st_value, sym->st_size, sym->st_info, 0, sym->st_shndx, name);
            s1->symtab_to_dynsym[sym - (Elf32_Sym *)symtab_section->data] = index;
          }
        }
      }

      build_got_entries(s1);
    
      // Add a list of needed DLLs
      for (i = 0; i < s1->nb_loaded_dlls; i++) {
        DLLReference *dllref = s1->loaded_dlls[i];
        if (dllref->level == 0) put_dt(dynamic, DT_NEEDED, put_elf_str(dynstr, dllref->name));
      }

      // TODO: currently, since we do not handle PIC code, we must relocate the readonly segments
      if (file_type == TCC_OUTPUT_DLL) {
        if (s1->soname) put_dt(dynamic, DT_SONAME, put_elf_str(dynstr, s1->soname));
        put_dt(dynamic, DT_TEXTREL, 0);
      }

      // Add necessary space for other entries
      saved_dynamic_data_offset = dynamic->data_offset;
      dynamic->data_offset += 8 * 9;
    } else {
      // Still need to build got entries in case of static link
      build_got_entries(s1);
    }
  }

  memset(&ehdr, 0, sizeof(ehdr));

  // Add a section for symbols
  strsec = new_section(s1, ".shstrtab", SHT_STRTAB, 0);
  put_elf_str(strsec, "");
  
  // Compute number of sections
  shnum = s1->nb_sections;

  // This array is used to reorder sections in the output file
  section_order = tcc_malloc(sizeof(int) * shnum);
  section_order[0] = 0;
  sh_order_index = 1;
  
  // Compute number of program headers
  switch (file_type) {
    case TCC_OUTPUT_OBJ:
      phnum = 0;
      break;
    case TCC_OUTPUT_EXE:
      if (!s1->static_link) {
        phnum = 4;
      } else {
        phnum = 2;
      }
      break;
    case TCC_OUTPUT_DLL:
      phnum = 3;
      break;
    default:
      phnum = 0;
  }

  // Allocate strings for section names and decide if an unallocated
  // section should be output
  // NOTE: the strsec section comes last, so its size is also correct!
  for (i = 1; i < s1->nb_sections; i++) {
    s = s1->sections[i];
    s->sh_name = put_elf_str(strsec, s->name);
    // When generating a DLL, we include relocations but we may patch them
    if (file_type == TCC_OUTPUT_DLL && s->sh_type == SHT_REL && !(s->sh_flags & SHF_ALLOC)) {
      prepare_dynamic_rel(s1, s);
    } else if (do_debug || file_type == TCC_OUTPUT_OBJ || (s->sh_flags & SHF_ALLOC) ||
      i == (s1->nb_sections - 1)) {
      // Output all sections if debug or object file
      s->sh_size = s->data_offset;
    }
  }

  // Allocate program segment headers
  phdr = tcc_mallocz(phnum * sizeof(Elf32_Phdr));
    
  if (s1->output_format == TCC_OUTPUT_FORMAT_ELF) {
    file_offset = sizeof(Elf32_Ehdr) + phnum * sizeof(Elf32_Phdr);
  } else {
    file_offset = 0;
  }
  if (phnum > 0) {
    // Compute section to program header mapping
    if (s1->has_text_addr) {
      int a_offset, p_offset;
      addr = s1->text_addr;
      // Ensure that (addr % ELF_PAGE_SIZE) == file_offset % ELF_PAGE_SIZE
      a_offset = addr & (ELF_PAGE_SIZE - 1);
      p_offset = file_offset & (ELF_PAGE_SIZE - 1);
      if (a_offset < p_offset) a_offset += ELF_PAGE_SIZE;
      file_offset += (a_offset - p_offset);
    } else {
      if (file_type == TCC_OUTPUT_DLL) {
        addr = 0;
      } else {
        addr = ELF_START_ADDR;
      }
      // Compute address after headers
      addr += (file_offset & (ELF_PAGE_SIZE - 1));
    }
    
    // Dynamic relocation table information, for .dynamic section
    rel_size = 0;
    rel_addr = 0;

    // Leave one program header for the program interpreter
    ph = &phdr[0];
    if (interp) ph++;

    for (j = 0; j < 2; j++) {
      ph->p_type = PT_LOAD;
      if (j == 0) {
        ph->p_flags = PF_R | PF_X;
      } else {
        ph->p_flags = PF_R | PF_W;
      }
      ph->p_align = ELF_PAGE_SIZE;
      
      // We do the following ordering: interp, symbol tables, relocations, progbits, nobits
      // TODO: do faster and simpler sorting
      for (k = 0; k < 5; k++) {
        for (i = 1; i < s1->nb_sections; i++) {
          s = s1->sections[i];
          // compute if section should be included
          if (j == 0) {
            if ((s->sh_flags & (SHF_ALLOC | SHF_WRITE)) != SHF_ALLOC) continue;
          } else {
            if ((s->sh_flags & (SHF_ALLOC | SHF_WRITE)) !=  (SHF_ALLOC | SHF_WRITE)) continue;
          }

          if (s == interp) {
            if (k != 0) continue;
          } else if (s->sh_type == SHT_DYNSYM || s->sh_type == SHT_STRTAB || s->sh_type == SHT_HASH) {
            if (k != 1) continue;
          } else if (s->sh_type == SHT_REL) {
            if (k != 2) continue;
          } else if (s->sh_type == SHT_NOBITS) {
            if (k != 4) continue;
          } else {
            if (k != 3) continue;
          }
          section_order[sh_order_index++] = i;

          // Section matches: we align it and add its size
          tmp = addr;
          addr = (addr + s->sh_addralign - 1) & ~(s->sh_addralign - 1);
          file_offset += addr - tmp;
          s->sh_offset = file_offset;
          s->sh_addr = addr;
          
          // Update program header infos
          if (ph->p_offset == 0) {
            ph->p_offset = file_offset;
            ph->p_vaddr = addr;
            ph->p_paddr = ph->p_vaddr;
          }

          // Update dynamic relocation infos
          if (s->sh_type == SHT_REL) {
            if (rel_size == 0) rel_addr = addr;
            rel_size += s->sh_size;
          }
          addr += s->sh_size;
          if (s->sh_type != SHT_NOBITS) file_offset += s->sh_size;
        }
      }
      ph->p_filesz = file_offset - ph->p_offset;
      ph->p_memsz = addr - ph->p_vaddr;
      ph++;
      if (j == 0) {
        if (s1->output_format == TCC_OUTPUT_FORMAT_ELF) {
          // If in the middle of a page, we duplicate the page in
          // memory so that one copy is RX and the other is RW
          if ((addr & (ELF_PAGE_SIZE - 1)) != 0) addr += ELF_PAGE_SIZE;
        } else {
          addr = (addr + ELF_PAGE_SIZE - 1) & ~(ELF_PAGE_SIZE - 1);
          file_offset = (file_offset + ELF_PAGE_SIZE - 1) & ~(ELF_PAGE_SIZE - 1);
        }
      }
    }

    // If interpreter, then add corresponing program header
    if (interp) {
      ph = &phdr[0];
      
      ph->p_type = PT_INTERP;
      ph->p_offset = interp->sh_offset;
      ph->p_vaddr = interp->sh_addr;
      ph->p_paddr = ph->p_vaddr;
      ph->p_filesz = interp->sh_size;
      ph->p_memsz = interp->sh_size;
      ph->p_flags = PF_R;
      ph->p_align = interp->sh_addralign;
    }
    
    // If dynamic section, then add corresponing program header
    if (dynamic) {
      Elf32_Sym *sym_end;

      ph = &phdr[phnum - 1];
      
      ph->p_type = PT_DYNAMIC;
      ph->p_offset = dynamic->sh_offset;
      ph->p_vaddr = dynamic->sh_addr;
      ph->p_paddr = ph->p_vaddr;
      ph->p_filesz = dynamic->sh_size;
      ph->p_memsz = dynamic->sh_size;
      ph->p_flags = PF_R | PF_W;
      ph->p_align = dynamic->sh_addralign;

      // Put GOT dynamic section address
      put32(s1->got->data, dynamic->sh_addr);

      // Relocate the PLT
      if (file_type == TCC_OUTPUT_EXE) {
        uint8_t *p, *p_end;

        p = s1->plt->data;
        p_end = p + s1->plt->data_offset;
        if (p < p_end) {
          put32(p + 2, get32(p + 2) + s1->got->sh_addr);
          put32(p + 8, get32(p + 8) + s1->got->sh_addr);
          p += 16;
          while (p < p_end) {
            put32(p + 2, get32(p + 2) + s1->got->sh_addr);
            p += 16;
          }
        }
      }

      // Relocate symbols in .dynsym
      sym_end = (Elf32_Sym *)(s1->dynsym->data + s1->dynsym->data_offset);
      for (sym = (Elf32_Sym *) s1->dynsym->data + 1; sym < sym_end; sym++) {
        if (sym->st_shndx == SHN_UNDEF) {
          // Relocate to the PLT if the symbol corresponds to a PLT entry
          if (sym->st_value) sym->st_value += s1->plt->sh_addr;
        } else if (sym->st_shndx < SHN_LORESERVE) {
          // Do symbol relocation
          sym->st_value += s1->sections[sym->st_shndx]->sh_addr;
        }
      }

      // Put dynamic section entries
      dynamic->data_offset = saved_dynamic_data_offset;
      put_dt(dynamic, DT_HASH, s1->dynsym->hash->sh_addr);
      put_dt(dynamic, DT_STRTAB, dynstr->sh_addr);
      put_dt(dynamic, DT_SYMTAB, s1->dynsym->sh_addr);
      put_dt(dynamic, DT_STRSZ, dynstr->data_offset);
      put_dt(dynamic, DT_SYMENT, sizeof(Elf32_Sym));
      put_dt(dynamic, DT_REL, rel_addr);
      put_dt(dynamic, DT_RELSZ, rel_size);
      put_dt(dynamic, DT_RELENT, sizeof(Elf32_Rel));
      put_dt(dynamic, DT_NULL, 0);
    }

    ehdr.e_phentsize = sizeof(Elf32_Phdr);
    ehdr.e_phnum = phnum;
    ehdr.e_phoff = sizeof(Elf32_Ehdr);
  }

  // All other sections come after
  for (i = 1; i < s1->nb_sections; i++) {
    s = s1->sections[i];
    if (phnum > 0 && (s->sh_flags & SHF_ALLOC)) continue;
    section_order[sh_order_index++] = i;
    
    file_offset = (file_offset + s->sh_addralign - 1) & ~(s->sh_addralign - 1);
    s->sh_offset = file_offset;
    if (s->sh_type != SHT_NOBITS) file_offset += s->sh_size;
  }

  // If building executable or DLL, then relocate each section
  // except the GOT which is already relocated
  if (file_type != TCC_OUTPUT_OBJ) {
    relocate_syms(s1, 0);

    if (s1->nb_errors != 0) {
    fail:
      ret = -1;
      goto cleanup;
    }

    // Relocate sections
    // TODO: ignore sections with allocated relocations?
    for (i = 1; i < s1->nb_sections; i++) {
      s = s1->sections[i];
      if (s->reloc && s != s1->got) relocate_section(s1, s);
    }

    // Relocate relocation entries if the relocation tables are
    // allocated in the executable
    for (i = 1; i < s1->nb_sections; i++) {
      s = s1->sections[i];
      if ((s->sh_flags & SHF_ALLOC) && s->sh_type == SHT_REL) {
        relocate_rel(s1, s);
      }
    }

    // Get entry point address
    if (file_type == TCC_OUTPUT_EXE) {
      ehdr.e_entry = (unsigned long) tcc_get_symbol_err(s1, "_start");
    } else {
      ehdr.e_entry = text_section->sh_addr; // TODO: is it correct?
    }
  }

  // Write ELF file
  if (file_type == TCC_OUTPUT_OBJ) {
    mode = 0666;
  } else {
    mode = 0777;
  }
  fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, mode); 
  if (fd < 0) {
    error_noabort("could not write '%s'", filename);
    goto fail;
  }
  f = fdopen(fd, "wb");
  if (verbose) printf("<- %s\n", filename);

  if (s1->output_format == TCC_OUTPUT_FORMAT_ELF) {
    sort_syms(s1, symtab_section);
    
    // Align to 4
    file_offset = (file_offset + 3) & -4;
  
    // Fill header
    ehdr.e_ident[0] = ELFMAG0;
    ehdr.e_ident[1] = ELFMAG1;
    ehdr.e_ident[2] = ELFMAG2;
    ehdr.e_ident[3] = ELFMAG3;
    ehdr.e_ident[4] = ELFCLASS32;
    ehdr.e_ident[5] = ELFDATA2LSB;
    ehdr.e_ident[6] = EV_CURRENT;

    switch (file_type) {
      case TCC_OUTPUT_EXE:
        ehdr.e_type = ET_EXEC;
        break;
      case TCC_OUTPUT_DLL:
        ehdr.e_type = ET_DYN;
        break;
      case TCC_OUTPUT_OBJ:
        ehdr.e_type = ET_REL;
        break;
      default:
        ehdr.e_type = ET_EXEC;
    }

    ehdr.e_machine = EM_386;
    ehdr.e_version = EV_CURRENT;
    ehdr.e_shoff = file_offset;
    ehdr.e_ehsize = sizeof(Elf32_Ehdr);
    ehdr.e_shentsize = sizeof(Elf32_Shdr);
    ehdr.e_shnum = shnum;
    ehdr.e_shstrndx = shnum - 1;
    
    fwrite(&ehdr, 1, sizeof(Elf32_Ehdr), f);
    fwrite(phdr, 1, phnum * sizeof(Elf32_Phdr), f);
    offset = sizeof(Elf32_Ehdr) + phnum * sizeof(Elf32_Phdr);

    for (i = 1;i < s1->nb_sections; i++) {
      s = s1->sections[section_order[i]];
      if (s->sh_type != SHT_NOBITS) {
        while (offset < s->sh_offset) {
          fputc(0, f);
          offset++;
        }
        size = s->sh_size;
        fwrite(s->data, 1, size, f);
        offset += size;
      }
    }

    // Output section headers
    while (offset < ehdr.e_shoff) {
      fputc(0, f);
      offset++;
    }
  
    for (i = 0; i < s1->nb_sections; i++) {
      sh = &shdr;
      memset(sh, 0, sizeof(Elf32_Shdr));
      s = s1->sections[i];
      if (s) {
        sh->sh_name = s->sh_name;
        sh->sh_type = s->sh_type;
        sh->sh_flags = s->sh_flags;
        sh->sh_entsize = s->sh_entsize;
        sh->sh_info = s->sh_info;
        if (s->link) sh->sh_link = s->link->sh_num;
        sh->sh_addralign = s->sh_addralign;
        sh->sh_addr = s->sh_addr;
        sh->sh_offset = s->sh_offset;
        sh->sh_size = s->sh_size;
      }
      fwrite(sh, 1, sizeof(Elf32_Shdr), f);
    }
  } else {
    tcc_output_binary(s1, f, section_order);
  }
  fclose(f);

  ret = 0;
 cleanup:
  tcc_free(s1->symtab_to_dynsym);
  tcc_free(section_order);
  tcc_free(phdr);
  tcc_free(s1->got_offsets);
  return ret;
}

typedef struct SectionMergeInfo {
  Section *s;                     // Corresponding existing section
  unsigned long offset;           // Offset of the new section in the existing section
  uint8_t new_section;            // True if section 's' was added
  uint8_t link_once;              // True if link once section
} SectionMergeInfo;

// Load an object file and merge it with current files
int tcc_load_object_file(TCCState *s1, int fd, unsigned long file_offset) {
  Elf32_Ehdr ehdr;
  Elf32_Shdr *shdr, *sh;
  int size, i, j, offset, offseti, nb_syms, sym_index, ret;
  unsigned char *strsec, *strtab;
  int *old_to_new_syms;
  char *sh_name, *name;
  SectionMergeInfo *sm_table, *sm;
  SectionMergeInfo *stab, *stabstr;
  Elf32_Sym *sym, *symtab;
  Elf32_Rel *rel, *rel_end;
  Section *s;

  if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) goto fail1;
  if (ehdr.e_ident[0] != ELFMAG0 ||
      ehdr.e_ident[1] != ELFMAG1 ||
      ehdr.e_ident[2] != ELFMAG2 ||
      ehdr.e_ident[3] != ELFMAG3) {
    goto fail1;
  }

  // Test if object file
  if (ehdr.e_type != ET_REL) goto fail1;

  // Test CPU specific stuff
  if (ehdr.e_ident[5] != ELFDATA2LSB || ehdr.e_machine != EM_386) {
  fail1:
    error_noabort("invalid object file");
    return -1;
  }
  
  // Read sections
  shdr = load_data(fd, file_offset + ehdr.e_shoff, sizeof(Elf32_Shdr) * ehdr.e_shnum);
  sm_table = tcc_mallocz(sizeof(SectionMergeInfo) * ehdr.e_shnum);
  
  // Load section names
  sh = &shdr[ehdr.e_shstrndx];
  strsec = load_data(fd, file_offset + sh->sh_offset, sh->sh_size);

  // Load symtab and strtab
  old_to_new_syms = NULL;
  symtab = NULL;
  strtab = NULL;
  nb_syms = 0;
  for (i = 1; i < ehdr.e_shnum; i++) {
    sh = &shdr[i];
    if (sh->sh_type == SHT_SYMTAB) {
      if (symtab) {
        error_noabort("object must contain only one symtab");
      fail:
        ret = -1;
        goto cleanup;
      }
      nb_syms = sh->sh_size / sizeof(Elf32_Sym);
      symtab = load_data(fd, file_offset + sh->sh_offset, sh->sh_size);
      sm_table[i].s = symtab_section;

      // Now load strtab
      sh = &shdr[sh->sh_link];
      strtab = load_data(fd, file_offset + sh->sh_offset, sh->sh_size);
    }
  }
    
  // Now examine each section and try to merge its content with the
  // ones in memory
  stab = NULL;
  stabstr = NULL;
  for (i = 1; i < ehdr.e_shnum; i++) {
    // No need to examine section name strtab
    if (i == ehdr.e_shstrndx) continue;
    sh = &shdr[i];
    sh_name = strsec + sh->sh_name;

    if (strcmp(sh_name, ".stab") == 0 || strcmp(sh_name, ".stabstr") == 0 || strcmp(sh_name, ".rel.stab") == 0) {
      // Only include stabs if we generate debug info
      if (!do_debug) continue;
    } else {
      // Ignore sections types we do not handle
      if (sh->sh_type != SHT_PROGBITS && sh->sh_type != SHT_REL && sh->sh_type != SHT_NOBITS) continue;
    }
    if (sh->sh_addralign < 1) sh->sh_addralign = 1;

    // Find corresponding section, if any
    for (j = 1; j < s1->nb_sections;j++) {
      s = s1->sections[j];
      if (!strcmp(s->name, sh_name)) {
        if (!strncmp(sh_name, ".gnu.linkonce", sizeof(".gnu.linkonce") - 1)) {
          // If a 'linkonce' section is already present, we do not add it again. 
          // It is a little tricky as symbols can still be defined in it.
          sm_table[i].link_once = 1;
          goto next;
        } else {
          goto found;
        }
      }
    }

    // Not found: create new section
    s = new_section(s1, sh_name, sh->sh_type, sh->sh_flags);

    // Take as much info as possible from the section. sh_link and
    // sh_info will be updated later
    s->sh_addralign = sh->sh_addralign;
    s->sh_entsize = sh->sh_entsize;
    sm_table[i].new_section = 1;
  found:
    if (sh->sh_type != s->sh_type) {
      error_noabort("invalid section type");
      goto fail;
    }

    // Align start of section
    offset = s->data_offset;
    size = sh->sh_addralign - 1;
    offset = (offset + size) & ~size;
    if (sh->sh_addralign > s->sh_addralign) s->sh_addralign = sh->sh_addralign;
    s->data_offset = offset;
    sm_table[i].offset = offset;
    sm_table[i].s = s;

    // Concatenate sections
    size = sh->sh_size;
    if (sh->sh_type != SHT_NOBITS) {
      unsigned char *ptr;
      lseek(fd, file_offset + sh->sh_offset, SEEK_SET);
      ptr = section_ptr_add(s, size);
      read(fd, ptr, size);
    } else {
      s->data_offset += size;
    }
    
    // We need to fixup string indices for stabs later
    if (s == stab_section) stab = &sm_table[i];
    if (s == stabstr_section) stabstr = &sm_table[i];
  next: ;
  }

  // Second short pass to update sh_link and sh_info fields of new sections
  for (i = 1; i < ehdr.e_shnum; i++) {
    s = sm_table[i].s;
    if (!s || !sm_table[i].new_section) continue;
    sh = &shdr[i];
    if (sh->sh_link > 0) s->link = sm_table[sh->sh_link].s;
    if (sh->sh_type == SHT_REL) {
      s->sh_info = sm_table[sh->sh_info].s->sh_num;
      // Update backward link
      s1->sections[s->sh_info]->reloc = s;
    }
  }
  sm = sm_table;

  // Resolve symbols
  old_to_new_syms = tcc_mallocz(nb_syms * sizeof(int));
  sym = symtab + 1;
  for (i = 1; i < nb_syms; i++, sym++) {
    if (sym->st_shndx != SHN_UNDEF && sym->st_shndx < SHN_LORESERVE) {
      sm = &sm_table[sym->st_shndx];
      if (sm->link_once) {
        // If a symbol is in a link once section, we use the
        // already defined symbol. It is very important to get
        // correct relocations
        if (ELF32_ST_BIND(sym->st_info) != STB_LOCAL) {
          name = strtab + sym->st_name;
          sym_index = find_elf_sym(symtab_section, name);
          if (sym_index) old_to_new_syms[i] = sym_index;
        }
        continue;
      }
      // If no corresponding section added, no need to add symbol
      if (!sm->s) continue;

      // Convert section number
      sym->st_shndx = sm->s->sh_num;

      // Offset value
      sym->st_value += sm->offset;
    }
    // Add symbol
    name = strtab + sym->st_name;
    sym_index = add_elf_sym(symtab_section, sym->st_value, sym->st_size, sym->st_info, sym->st_other, sym->st_shndx, name);
    old_to_new_syms[i] = sym_index;
  }

  // Third pass to patch relocation entries
  for (i = 1; i < ehdr.e_shnum; i++) {
    s = sm_table[i].s;
    if (!s) continue;
    sh = &shdr[i];
    offset = sm_table[i].offset;
    switch (s->sh_type) {
      case SHT_REL:
        // Take relocation offset information
        offseti = sm_table[sh->sh_info].offset;
        rel_end = (Elf32_Rel *) (s->data + s->data_offset);
        for (rel = (Elf32_Rel *) (s->data + offset); rel < rel_end; rel++) {
          int type;
          unsigned sym_index;
          // Convert symbol index
          type = ELF32_R_TYPE(rel->r_info);
          sym_index = ELF32_R_SYM(rel->r_info);
          // NOTE: only one symtab assumed
          if (sym_index >= nb_syms) goto invalid_reloc;
          sym_index = old_to_new_syms[sym_index];
          // Ignore link_once in rel section
          if (!sym_index && !sm->link_once) {
          invalid_reloc:
            error_noabort("Invalid relocation entry [%2d] '%s' @ %.8x", i, strsec + sh->sh_name, rel->r_offset);
            goto fail;
          }
          rel->r_info = ELF32_R_INFO(sym_index, type);
          // Offset the relocation offset
          rel->r_offset += offseti;
        }
        break;
      default:
        break;
    }
  }

  // Fixup string indices for stabs
  if (stab && stabstr) {
    Stab_Sym *ssym = (Stab_Sym *) (stab->s->data + stab->offset);
    Stab_Sym *ssym_end = (Stab_Sym *) (stab->s->data + stab->s->data_offset);
    while (ssym < ssym_end) {
      if (ssym->n_strx) ssym->n_strx += stabstr->offset;
      ssym++;
    }
  }

  ret = 0;
 cleanup:
  tcc_free(symtab);
  tcc_free(strtab);
  tcc_free(old_to_new_syms);
  tcc_free(sm_table);
  tcc_free(strsec);
  tcc_free(shdr);
  return ret;
}

typedef struct ArchiveHeader {
  char ar_name[16];               // Name of this member
  char ar_date[12];               // File mtime
  char ar_uid[6];                 // Owner uid; printed as decimal
  char ar_gid[6];                 // Owner gid; printed as decimal
  char ar_mode[8];                // File mode, printed as octal
  char ar_size[10];               // File size, printed as decimal
  char ar_fmag[2];                // Should contain ARFMAG
} ArchiveHeader;

static int get_be32(const uint8_t *b) {
  return b[3] | (b[2] << 8) | (b[1] << 16) | (b[0] << 24);
}

// Load only the objects which resolve undefined symbols
int tcc_load_alacarte(TCCState *s1, int fd, int size) {
  int i, bound, nsyms, sym_index, off, ret;
  uint8_t *data;
  const char *ar_names, *p;
  const uint8_t *ar_index;
  Elf32_Sym *sym;

  data = tcc_malloc(size);
  if (read(fd, data, size) != size) goto fail;
  nsyms = get_be32(data);
  ar_index = data + 4;
  ar_names = ar_index + nsyms * 4;

  do {
    bound = 0;
    for (p = ar_names, i = 0; i < nsyms; i++, p += strlen(p) + 1) {
      sym_index = find_elf_sym(symtab_section, p);
      if (sym_index) {
        sym = &((Elf32_Sym *) symtab_section->data)[sym_index];
        if (sym->st_shndx == SHN_UNDEF) {
          off = get_be32(ar_index + i * 4) + sizeof(ArchiveHeader);
          ++bound;
          lseek(fd, off, SEEK_SET);
          if (tcc_load_object_file(s1, fd, off) < 0) {
          fail:
            ret = -1;
            goto cleanup;
          }
        }
      }
    }
  } while (bound);
  ret = 0;
 cleanup:
  tcc_free(data);
  return ret;
}

// Load a '.a' file
int tcc_load_archive(TCCState *s1, int fd) {
  ArchiveHeader hdr;
  char ar_size[11];
  char ar_name[17];
  char magic[8];
  int size, len, i;
  unsigned long file_offset;

  // Skip magic which was already checked
  read(fd, magic, sizeof(magic));
  
  for (;;) {
    len = read(fd, &hdr, sizeof(hdr));
    if (len == 0) break;
    if (len != sizeof(hdr)) {
      error_noabort("invalid archive");
      return -1;
    }
    memcpy(ar_size, hdr.ar_size, sizeof(hdr.ar_size));
    ar_size[sizeof(hdr.ar_size)] = '\0';
    size = strtol(ar_size, NULL, 0);
    memcpy(ar_name, hdr.ar_name, sizeof(hdr.ar_name));
    for (i = sizeof(hdr.ar_name) - 1; i >= 0; i--) {
      if (ar_name[i] != ' ') break;
    }
    ar_name[i + 1] = '\0';
    file_offset = lseek(fd, 0, SEEK_CUR);
    // Align to even
    size = (size + 1) & ~1;
    if (!strcmp(ar_name, "/")) {
      // COFF symbol table
      if (s1->alacarte_link) return tcc_load_alacarte(s1, fd, size);
    } else if (!strcmp(ar_name, "//") ||
               !strcmp(ar_name, "__.SYMDEF") ||
               !strcmp(ar_name, "__.SYMDEF/") ||
               !strcmp(ar_name, "ARFILENAMES/")) {
      // Skip symbol table or archive names
    } else {
      if (tcc_load_object_file(s1, fd, file_offset) < 0) return -1;
    }
    lseek(fd, file_offset + size, SEEK_SET);
  }
  return 0;
}

// Load a DLL and all referenced DLLs. 'level = 0' means that the DLL
//is referenced by the user (so it should be added as DT_NEEDED in
// the generated ELF file)
int tcc_load_dll(TCCState *s1, int fd, const char *filename, int level) {
  Elf32_Ehdr ehdr;
  Elf32_Shdr *shdr, *sh, *sh1;
  int i, j, nb_syms, nb_dts, sym_bind, ret;
  Elf32_Sym *sym, *dynsym;
  Elf32_Dyn *dt, *dynamic;
  unsigned char *dynstr;
  const char *name, *soname;
  DLLReference *dllref;
  
  read(fd, &ehdr, sizeof(ehdr));

  // Test CPU specific stuff
  if (ehdr.e_ident[5] != ELFDATA2LSB || ehdr.e_machine != EM_386) {
    error_noabort("bad architecture");
    return -1;
  }

  // Read sections
  shdr = load_data(fd, ehdr.e_shoff, sizeof(Elf32_Shdr) * ehdr.e_shnum);

  // Load dynamic section and dynamic symbols
  nb_syms = 0;
  nb_dts = 0;
  dynamic = NULL;
  dynsym = NULL;
  dynstr = NULL;
  for (i = 0, sh = shdr; i < ehdr.e_shnum; i++, sh++) {
    switch (sh->sh_type) {
      case SHT_DYNAMIC:
        nb_dts = sh->sh_size / sizeof(Elf32_Dyn);
        dynamic = load_data(fd, sh->sh_offset, sh->sh_size);
        break;
      case SHT_DYNSYM:
        nb_syms = sh->sh_size / sizeof(Elf32_Sym);
        dynsym = load_data(fd, sh->sh_offset, sh->sh_size);
        sh1 = &shdr[sh->sh_link];
        dynstr = load_data(fd, sh1->sh_offset, sh1->sh_size);
        break;
      default:
        break;
    }
  }
  
  // Compute the real library name
  soname = tcc_basename(filename);
  for (i = 0, dt = dynamic; i < nb_dts; i++, dt++) {
    if (dt->d_tag == DT_SONAME) {
      soname = dynstr + dt->d_un.d_val;
    }
  }

  // If the DLL is already loaded, do not load it
  for (i = 0; i < s1->nb_loaded_dlls; i++) {
    dllref = s1->loaded_dlls[i];
    if (!strcmp(soname, dllref->name)) {
      // But update level if needed
      if (level < dllref->level) dllref->level = level;
      ret = 0;
      goto cleanup;
    }
  }

  // add the dll and its level
  dllref = tcc_malloc(sizeof(DLLReference) + strlen(soname));
  dllref->level = level;
  strcpy(dllref->name, soname);
  dynarray_add((void ***)&s1->loaded_dlls, &s1->nb_loaded_dlls, dllref);

  // add dynamic symbols in dynsym_section
  for (i = 1, sym = dynsym + 1; i < nb_syms; i++, sym++) {
    sym_bind = ELF32_ST_BIND(sym->st_info);
    if (sym_bind == STB_LOCAL) continue;
    name = dynstr + sym->st_name;
    add_elf_sym(s1->dynsymtab_section, sym->st_value, sym->st_size,
                sym->st_info, sym->st_other, sym->st_shndx, name);
  }

  // load all referenced DLLs
  for (i = 0, dt = dynamic; i < nb_dts; i++, dt++) {
    switch (dt->d_tag) {
      case DT_NEEDED:
        name = dynstr + dt->d_un.d_val;
        for (j = 0; j < s1->nb_loaded_dlls; j++) {
          dllref = s1->loaded_dlls[j];
          if (!strcmp(name, dllref->name)) goto already_loaded;
        }
        if (tcc_add_dll(s1, name, AFF_REFERENCED_DLL) < 0) {
          error_noabort("referenced dll '%s' not found", name);
          ret = -1;
          goto cleanup;
        }
      already_loaded:
        break;
    }
  }
  ret = 0;
 cleanup:
  tcc_free(dynstr);
  tcc_free(dynsym);
  tcc_free(dynamic);
  tcc_free(shdr);
  return ret;
}

#define LD_TOK_NAME 256
#define LD_TOK_EOF  (-1)

// Return next ld script token
static int ld_next(TCCState *s1, char *name, int name_size) {
  int c;
  char *q;

 redo:
  switch (ch) {
    case ' ': case '\t': case '\f': case '\v': case '\r': case '\n':
      finp();
      goto redo;
    case '/':
      minp();
      if (ch == '*') {
        file->buf_ptr = parse_comment(file->buf_ptr);
        ch = file->buf_ptr[0];
        goto redo;
      } else {
        q = name;
        *q++ = '/';
        goto parse_name;
      }
      break;
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': 
    case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': 
    case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I':
    case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '_': case '\\': case '.': case '$': case '~':
      q = name;
      parse_name:
      for (;;) {
        if (!((ch >= 'a' && ch <= 'z') || 
            (ch >= 'A' && ch <= 'Z') || 
            (ch >= '0' && ch <= '9') ||
            strchr("/.-_+=$:\\,~", ch))) {
          break;
        }
        if ((q - name) < name_size - 1) *q++ = ch;
        minp();
      }
      *q = '\0';
      c = LD_TOK_NAME;
      break;
    case CH_EOF:
      c = LD_TOK_EOF;
      break;
    default:
      c = ch;
      finp();
      break;
  }
  return c;
}

int ld_add_file_list(TCCState *s1, int as_needed) {
  char filename[1024];
  int t, ret;

  t = ld_next(s1, filename, sizeof(filename));
  if (t != '(') expect("(");
  t = ld_next(s1, filename, sizeof(filename));
  for (;;) {
    if (t == LD_TOK_EOF) {
      error_noabort("unexpected end of file");
      return -1;
    } else if (t == ')') {
      break;
    } else if (t != LD_TOK_NAME) {
      error_noabort("filename expected");
      return -1;
    } 
    if (!strcmp(filename, "AS_NEEDED")) {
      ret = ld_add_file_list(s1, 1);
      if (ret) return ret;
    } else {
      // TODO: Implement AS_NEEDED support. Ignore it for now
      if (!as_needed) tcc_add_file(s1, filename);
    }
    t = ld_next(s1, filename, sizeof(filename));
    if (t == ',') {
      t = ld_next(s1, filename, sizeof(filename));
    }
  }
  return 0;
}

// Interpret a subset of GNU ldscripts to handle the dummy libc.so files
int tcc_load_ldscript(TCCState *s1) {
  char cmd[64];
  char filename[1024];
  int t, ret;
  
  ch = file->buf_ptr[0];
  ch = handle_eob();
  for (;;) {
    t = ld_next(s1, cmd, sizeof(cmd));
    if (t == LD_TOK_EOF) {
      return 0;
    } else if (t != LD_TOK_NAME) {
      return -1;
    }
    if (!strcmp(cmd, "INPUT") || !strcmp(cmd, "GROUP")) {
      ret = ld_add_file_list(s1, 0);
      if (ret) return ret;
    } else if (!strcmp(cmd, "OUTPUT_FORMAT") || !strcmp(cmd, "TARGET")) {
      // Ignore some commands
      t = ld_next(s1, cmd, sizeof(cmd));
      if (t != '(') expect("(");
      for (;;) {
        t = ld_next(s1, filename, sizeof(filename));
        if (t == LD_TOK_EOF) {
          error_noabort("unexpected end of file");
          return -1;
        } else if (t == ')') {
          break;
        }
      }
    } else {
      return -1;
    }
  }
  return 0;
}
