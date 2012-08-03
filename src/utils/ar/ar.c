//
// ar.c
//
// Library archive utility
//
// Copyright (C) 2011 Michael Ringgaard. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <netinet/in.h>

#define AR_MAGIC "!<arch>\012"

struct ar_header {
  char ar_name[16];               // Name of this member
  char ar_date[12];               // File mtime
  char ar_uid[6];                 // Owner uid; printed as decimal
  char ar_gid[6];                 // Owner gid; printed as decimal
  char ar_mode[8];                // File mode, printed as octal
  char ar_size[10];               // File size, printed as decimal
  char ar_fmag[2];                // Should contain ARFMAG
};

//
// ELF file header
//

#define ELF_MAGIC "\177ELF"
#define EI_NIDENT (16)

#define ET_NONE   0               // No file type
#define ET_REL    1               // Relocatable file
#define ET_EXEC   2               // Executable file
#define ET_DYN    3               // Shared object file
#define ET_CORE   4               // Core file

struct elf_header {
  uint8_t  e_ident[EI_NIDENT];    // Magic number and other info
  uint16_t e_type;                // Object file type
  uint16_t e_machine;             // Architecture
  uint32_t e_version;             // Object file version
  uint32_t e_entry;               // Entry point virtual address
  uint32_t e_phoff;               // Program header table file offset
  uint32_t e_shoff;               // Section header table file offset
  uint32_t e_flags;               // Processor-specific flags
  uint16_t e_ehsize;              // ELF header size in bytes
  uint16_t e_phentsize;           // Program header table entry size
  uint16_t e_phnum;               // Program header table entry count
  uint16_t e_shentsize;           // Section header table entry size
  uint16_t e_shnum;               // Section header table entry count
  uint16_t e_shstrndx;            // Section header string table index
};

//
// ELF section header
//
//

#define SHT_NULL         0        // Section header table entry unused
#define SHT_PROGBITS     1        // Program data
#define SHT_SYMTAB       2        // Symbol table
#define SHT_STRTAB       3        // String table
#define SHT_RELA         4        // Relocation entries with addends
#define SHT_HASH         5        // Symbol hash table
#define SHT_DYNAMIC      6        // Dynamic linking information
#define SHT_NOTE         7        // Notes
#define SHT_NOBITS       8        // Program space with no data (bss)
#define SHT_REL          9        // Relocation entries, no addends
#define SHT_SHLIB        10       // Reserved
#define SHT_DYNSYM       11       // Dynamic linker symbol table

struct elf_section_header {
  uint32_t sh_name;               // Section name (string table index)
  uint32_t sh_type;               // Section type
  uint32_t sh_flags;              // Section flags
  uint32_t sh_addr;               // Section virtual addr at execution
  uint32_t sh_offset;             // Section file offset
  uint32_t sh_size;               // Section size in bytes
  uint32_t sh_link;               // Link to another section
  uint32_t sh_info;               // Additional section information
  uint32_t sh_addralign;          // Section alignment
  uint32_t sh_entsize;            // Entry size if section holds table
};

//
// ELF symbol table entry
//

struct elf_symbol {
  uint32_t st_name;               // Symbol name (string table index)
  uint32_t st_value;              // Symbol value
  uint32_t st_size;               // Symbol size
  uint8_t  st_info;               // Symbol type and binding
  uint8_t  st_other;              // No defined meaning, 0 
  uint16_t st_shndx;              // Section index
};

//
// Archive state information
//

struct entry {
  struct ar_header header;        // Header for archive entry
  char *contents;                 // File contents
  int size;                       // Size of input file
  int offset;                     // Position in archive
  struct entry *next;             // Next entry in archive
};

struct symbol {
  char *name;                     // Symbol name
  struct entry *entry;            // Entry in the archive the symbol belongs to
  struct symbol *next;            // Next symbol
};

struct archive {
  struct entry *first_entry;      // First archive entry
  struct entry *last_entry;       // Last archive entry
  struct symbol *first_symbol;    // First archive entry
  struct symbol *last_symbol;     // Last archive entry
  struct entry *symtab;           // Entry for symbol table 
};

int make_symtab = 0;
int merge_archives = 0;

void *load_data(int fd, int offset, int size) {
  void *data;

  data = malloc(size);
  if (!data) return NULL;
  
  if (lseek(fd, offset, SEEK_SET) != offset)  {
    free(data);
    return NULL;
  }

  if (read(fd, data, size) != size) {
    free(data);
    return NULL;
  }

  return data;
}

struct entry *add_new_entry(struct archive *ar) {
  struct entry *e = (struct entry *) malloc(sizeof(struct entry));
  memset(e, 0, sizeof(struct entry));
  memset(&e->header, ' ', sizeof(struct ar_header));

  if (ar->last_entry) ar->last_entry->next = e;
  if (!ar->first_entry) ar->first_entry = e;
  ar->last_entry = e;
  return e;
}

struct symbol *add_new_symbol(struct archive *ar, struct entry *e, char *name) {
  struct symbol *s = (struct symbol *) malloc(sizeof(struct symbol));
  s->name = strdup(name);
  s->entry = e;
  s->next = NULL;

  if (ar->last_symbol) ar->last_symbol->next = s;
  if (!ar->first_symbol) ar->first_symbol = s;
  ar->last_symbol = s;
  return s;
}

struct symbol *find_symbol(struct archive *ar, char *name)  {
  struct symbol *s = ar->first_symbol;
  while (s) {
    if (strcmp(s->name, name) == 0) return s;
    s = s->next;
  }
  return NULL;
}
  
void init_archive(struct archive *ar) {
  memset(ar, 0, sizeof(struct archive));
  if (make_symtab) ar->symtab = add_new_entry(ar);
}

void free_archive(struct archive *ar) {
  struct entry *e;
  struct symbol *s;

  e = ar->first_entry;
  while (e) {
    struct entry *next = e->next;
    if (e->contents) free(e->contents);
    free(e);
    e = next;
  }

  s = ar->first_symbol;
  while (s) {
    struct symbol *next = s->next;
    if (s->name) free(s->name);
    free(s);
    s = next;
  }
}

int add_symbols(struct archive  *ar, struct entry *e) {
  struct elf_header *hdr;
  struct elf_section_header *sections, *s;
  struct elf_symbol *symtab, *sym;
  char *strtab;
  char *name;
  int i, j, num_syms, type, bind;
  
  // The ELF header is at the begining of the file.
  hdr = (struct elf_header *) e->contents;

  // Check that this is an ELF object file.
  if (memcmp(hdr->e_ident, ELF_MAGIC, 4) != 0) return -1;
  if (hdr->e_type != ET_REL) return -1;

  // Get sections
  sections = (struct elf_section_header *) (e->contents + hdr->e_shoff);
    
  // Get symbols
  for (i = 1; i < hdr->e_shnum; i++) {
    s = &sections[i];
    if (s->sh_type == SHT_SYMTAB) {
      num_syms = s->sh_size / sizeof(struct elf_symbol);
      symtab = (struct elf_symbol *) (e->contents + s->sh_offset);
      
      s = &sections[s->sh_link];
      strtab = e->contents + s->sh_offset;
      
      for (j = 0; j < num_syms; j++) {
        sym = &symtab[j];
        name = strtab + sym->st_name;
        type = sym->st_info & 0x0f;
        bind = sym->st_info >> 4;
        
        //printf("symbol %s type=%d bind=%d section=%d\n", name, type, bind, sym->st_shndx);
        
        // Only keep defined symbols.
        if (sym->st_shndx == 0) continue;

        // Only keep global symbols.
        if (bind != 1) continue;
        
        add_new_symbol(ar, e, name);
      }
    }
  }

  return 0;
}

void fill_header(struct ar_header *hdr, char *filename, struct stat *st) {
  char *p, *name;
  int len;
  char buf[32];

  // Set the file name
  name = filename;
  p = filename;
  while (*p) {
    if (*p == '/' || *p == '\\') {
      name = ++p;
    } else {
      p++;
    }
  }
  len = strlen(name);
  if (len > 15) len = 15;
  memcpy(hdr->ar_name, name, len);
  hdr->ar_name[len] = '/';

  // Set date.
  sprintf(buf, "%d", st->st_mtime);
  memcpy(hdr->ar_date, buf, strlen(buf));

  // Set owner uid and gid.
  sprintf(buf, "%d", st->st_uid);
  memcpy(hdr->ar_uid, buf, strlen(buf));
  sprintf(buf, "%d", st->st_gid);
  memcpy(hdr->ar_gid, buf, strlen(buf));

  // Set file mode and size.
  sprintf(buf, "%o", st->st_mode);
  memcpy(hdr->ar_mode, buf, strlen(buf));
  sprintf(buf, "%d", st->st_size);
  memcpy(hdr->ar_size, buf, strlen(buf));

  // Set magic marker
  hdr->ar_fmag[0] = '`';
  hdr->ar_fmag[1] = '\n';
}

int read_file(struct archive  *ar, int fd, int offset, int size, char *filename) {
  struct entry *e;
  struct stat st;

  // Add new entry to archive.
  e = add_new_entry(ar);
  e->size = size;
  e->contents = load_data(fd, offset, size);
  if (e->contents == NULL) return -1;

  // Get file information
  if (fstat(fd, &st) < 0) return -1;
  
  // Fill out archive entry header
  fill_header(&e->header, filename, &st);

  // Add symbols from ELF object file
  if (make_symtab && size >= 4 && memcmp(e->contents, ELF_MAGIC, 4) == 0) {
    if (add_symbols(ar, e) < 0) return -1;
  }
  
  return 0;
}

int read_archive_file(struct archive  *ar, int fd) {
  struct ar_header hdr;
  char name[17];
  char sizebuf[11];
  int len, i, offset, size;
  struct entry *e;
  
  for (;;) {
    // Read next header from input archive
    len = read(fd, &hdr, sizeof(hdr));
    if (len == 0) break;
    if (len != sizeof(hdr)) {
      fprintf(stderr, "invalid archive");
      return -1;
    }
    
    // Get entry name and length from header
    memcpy(name, hdr.ar_name, sizeof(hdr.ar_name));
    for(i = sizeof(hdr.ar_name) - 1; i >= 0; i--) {
      if (name[i] != ' ') break;
    }
    name[i + 1] = '\0';
    memcpy(sizebuf, hdr.ar_size, sizeof(hdr.ar_size));
    sizebuf[sizeof(hdr.ar_size)] = '\0';
    size = strtol(sizebuf, NULL, 0);
    
    // Skip the symbol table
    offset = lseek(fd, 0, SEEK_CUR);
    if (strcmp(name, "/") != 0) {
      // Add new entry to archive.
      e = add_new_entry(ar);
      memcpy(&e->header, &hdr, sizeof(hdr));
      e->size = size;
      e->contents = load_data(fd, offset, size);
      if (e->contents == NULL) return -1;

     // Add symbols from ELF object file
     if (make_symtab && size >= 4 && memcmp(e->contents, ELF_MAGIC, 4) == 0) {
       if (add_symbols(ar, e) < 0) return -1;
     }
   }

    size = (size + 1) & ~1;  // align to even
    lseek(fd, offset + size, SEEK_SET);
  }
  return 0;
}

void build_symbol_table(struct archive *ar) {
  struct symbol *s;
  struct entry *e;
  int size, num_syms, offset;
  uint32_t *index;
  char *symtab;
  char *strtab;
  struct stat st;

  // Compute size of symbol table
  size = 4;
  num_syms = 0;
  s = ar->first_symbol;
  while (s) {
    size += strlen(s->name) + 1 + 4;
    num_syms++;
    s = s->next;
  }
  ar->symtab->size = size;
  
  // Compute offsets for all entries
  offset = 8;
  e = ar->first_entry;
  while (e) {
    e->offset = offset;
    offset += sizeof(struct ar_header) + e->size;
    if (offset & 1) offset++;
    e = e->next;
  }
  
  // Build symbol table
  symtab = ar->symtab->contents = malloc(ar->symtab->size);
  *((uint32_t *) symtab) = htonl(num_syms);
  index = (uint32_t *) (symtab + 4);
  strtab = symtab + 4 * num_syms + 4;
  s = ar->first_symbol;
  while (s) {
    *index++ = htonl(s->entry->offset);
    strcpy(strtab, s->name);
    strtab += strlen(s->name) + 1;
    s = s->next;
  }

  // Setup archive header for symbol table
  memset(&st, 0, sizeof(struct stat));
  st.st_mode = 0666;
  st.st_mtime = time(NULL);
  st.st_size = ar->symtab->size;
  fill_header(&ar->symtab->header, "", &st);
}

int write_archive(struct archive *ar, int fd) {
  struct entry *e;
  int size;
  int offset;

  // Write header
  size = write(fd, AR_MAGIC, 8);
  offset = 8;
  
  // Write entries.
  e = ar->first_entry;
  while (e) {
    size += write(fd, &e->header, sizeof(struct ar_header));
    size += write(fd, e->contents, e->size);
    offset += sizeof(struct ar_header) + e->size;
    if (offset & 1) {
      size += write(fd, "\0", 1);
      offset++;
    }
    if (size != offset) break;
    e = e->next;
  }

  return offset == size ? 0 : -1;
}

void usage() {
  fprintf(stderr, "usage: ar [options] <archive> <files>\n\n");
  fprintf(stderr, "  -s            Add symbol table to archive.\n");
  fprintf(stderr, "  -m            Add individual entries in input archives.\n");
}

int main(int argc, char *argv[]) {
  int c, fd;
  char *archive_filename;
  char magic[8];
  struct archive ar;

  // Parse command line options
  while ((c = getopt(argc, argv, "ms")) != EOF) {
    switch (c) {
      case 'm':
        merge_archives = 1;
        break;

      case 's':
        make_symtab = 1;
        break;

      default:
        usage();
        return 1;
    }
  }

  if (optind == argc) {
    usage();
    return 1;
  }
  archive_filename = argv[optind++];
  
  // Read all the input object files
  init_archive(&ar);
  while (optind < argc) {
    // Open next input file
    char *input_file = argv[optind++];
    fd = open(input_file, O_RDONLY | O_BINARY);
    if (fd < 0) {
      perror(archive_filename);
      free_archive(&ar);
      return 1;
    }
    
    // Determine file type
    if (read(fd, magic, 8) != 8) {
      fprintf(stderr, "%s: Invalid input file\n", input_file);
      close(fd);
      free_archive(&ar);
      return 1;
    }
    
    // Add input file to archive
    if (merge_archives && memcmp(magic, AR_MAGIC, 8) == 0) {
      if (read_archive_file(&ar, fd) < 0) {
        fprintf(stderr, "%s: Error reading archive file\n", input_file);
        close(fd);
        free_archive(&ar);
        return 1;
      }
    } else {
      if (read_file(&ar, fd, 0, lseek(fd, 0, SEEK_END), input_file) < 0) {
        fprintf(stderr, "%s: Error reading file\n", input_file);
        close(fd);
        free_archive(&ar);
        return 1;
      }
    }
    close(fd);
  }

  // Open output archive
  fd = open(archive_filename, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, 0666);
  if (fd < 0)  {
    perror(archive_filename);
    free_archive(&ar);
    return 1;
  }

  // Build symbol table.
  if (make_symtab) build_symbol_table(&ar);
  
  // Write archive.
  if (write_archive(&ar, fd) < 0) {
    perror(archive_filename);
    free_archive(&ar);
    close(fd);
    return 1;
  }

  close(fd);
  free_archive(&ar);
  
  return 0;
}
