//
//  cc.c - Tiny C Compiler for Sanos
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

// State for current compilation.
TCCState *tcc_state;

// display some information during compilation
int verbose = 0;

// Compile with debug symbol (and use them if error during execution)
int do_debug = 0;

// Number of token types, including identifiers and strings
int tok_ident;

// Path to the C runtime libraries
const char *tcc_lib_path = CONFIG_TCCDIR;

// Display benchmark infos
static int do_bench = 0;

#define WD_ALL            0x0001  // Warning is activated when using -Wall
#define FD_INVERT         0x0002  // Invert value before storing

typedef struct FlagDef {
  uint16_t offset;
  uint16_t flags;
  const char *name;
} FlagDef;

static const FlagDef warning_defs[] = {
  { offsetof(TCCState, warn_unsupported), 0, "unsupported" },
  { offsetof(TCCState, warn_write_strings), 0, "write-strings" },
  { offsetof(TCCState, warn_error), 0, "error" },
  { offsetof(TCCState, warn_implicit_function_declaration), WD_ALL, "implicit-function-declaration" },
};

static const FlagDef flag_defs[] = {
  { offsetof(TCCState, char_is_unsigned), 0, "unsigned-char" },
  { offsetof(TCCState, char_is_unsigned), FD_INVERT, "signed-char" },
  { offsetof(TCCState, nocommon), FD_INVERT, "common" },
  { offsetof(TCCState, leading_underscore), 0, "leading-underscore" },
};

#define TCC_OPTION_HAS_ARG 0x0001
#define TCC_OPTION_NOSEP   0x0002 // Cannot have space before option and arg

typedef struct TCCOption {
  const char *name;
  uint16_t index;
  uint16_t flags;
} TCCOption;

enum {
  TCC_OPTION_HELP,
  TCC_OPTION_I,
  TCC_OPTION_D,
  TCC_OPTION_U,
  TCC_OPTION_L,
  TCC_OPTION_B,
  TCC_OPTION_l,
  TCC_OPTION_bench,
  TCC_OPTION_g,
  TCC_OPTION_c,
  TCC_OPTION_static,
  TCC_OPTION_shared,
  TCC_OPTION_soname,
  TCC_OPTION_entry,
  TCC_OPTION_fixed,
  TCC_OPTION_filealign,
  TCC_OPTION_stub,
  TCC_OPTION_def,
  TCC_OPTION_o,
  TCC_OPTION_r,
  TCC_OPTION_Wl,
  TCC_OPTION_W,
  TCC_OPTION_m,
  TCC_OPTION_f,
  TCC_OPTION_nofll,
  TCC_OPTION_nostdinc,
  TCC_OPTION_nostdlib,
  TCC_OPTION_print_search_dirs,
  TCC_OPTION_rdynamic,
  TCC_OPTION_v,
  TCC_OPTION_w,
  TCC_OPTION_E,
};

static const TCCOption tcc_options[] = {
  { "h", TCC_OPTION_HELP, 0 },
  { "?", TCC_OPTION_HELP, 0 },
  { "I", TCC_OPTION_I, TCC_OPTION_HAS_ARG },
  { "D", TCC_OPTION_D, TCC_OPTION_HAS_ARG },
  { "U", TCC_OPTION_U, TCC_OPTION_HAS_ARG },
  { "L", TCC_OPTION_L, TCC_OPTION_HAS_ARG },
  { "B", TCC_OPTION_B, TCC_OPTION_HAS_ARG },
  { "l", TCC_OPTION_l, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
  { "bench", TCC_OPTION_bench, 0 },
  { "g", TCC_OPTION_g, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
  { "c", TCC_OPTION_c, 0 },
  { "static", TCC_OPTION_static, 0 },
  { "shared", TCC_OPTION_shared, 0 },
  { "soname", TCC_OPTION_soname, TCC_OPTION_HAS_ARG },
  { "entry", TCC_OPTION_entry, TCC_OPTION_HAS_ARG },
  { "fixed", TCC_OPTION_fixed, TCC_OPTION_HAS_ARG },
  { "filealign", TCC_OPTION_filealign, TCC_OPTION_HAS_ARG },
  { "stub", TCC_OPTION_stub, TCC_OPTION_HAS_ARG },
  { "def", TCC_OPTION_def, TCC_OPTION_HAS_ARG },
  { "o", TCC_OPTION_o, TCC_OPTION_HAS_ARG },
  { "rdynamic", TCC_OPTION_rdynamic, 0 },
  { "r", TCC_OPTION_r, 0 },
  { "Wl,", TCC_OPTION_Wl, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
  { "W", TCC_OPTION_W, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
  { "m", TCC_OPTION_m, TCC_OPTION_HAS_ARG },
  { "f", TCC_OPTION_f, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
  { "nofll", TCC_OPTION_nofll, 0 },
  { "nostdinc", TCC_OPTION_nostdinc, 0 },
  { "nostdlib", TCC_OPTION_nostdlib, 0 },
  { "print-search-dirs", TCC_OPTION_print_search_dirs, 0 }, 
  { "v", TCC_OPTION_v, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
  { "w", TCC_OPTION_w, 0 },
  { "E", TCC_OPTION_E, 0},
  { NULL },
};

static char **files;
static int nb_files, nb_libraries;
static int multiple_files;
static int print_search_dirs;
static int output_type;
static int reloc_output;
static const char *outfile;

static int set_flag(TCCState *s, const FlagDef *flags, int nb_flags, const char *name, int value) {
  int i;
  const FlagDef *p;
  const char *r;

  r = name;
  if (r[0] == 'n' && r[1] == 'o' && r[2] == '-') {
    r += 3;
    value = !value;
  }
  for (i = 0, p = flags; i < nb_flags; i++, p++) {
    if (!strcmp(r, p->name)) goto found;
  }
  return -1;
 found:
  if (p->flags & FD_INVERT) value = !value;
  *(int *)((uint8_t *) s + p->offset) = value;
  return 0;
}

// Set/reset a warning
int tcc_set_warning(TCCState *s, const char *warning_name, int value)
{
  int i;
  const FlagDef *p;

  if (!strcmp(warning_name, "all")) {
    for (i = 0, p = warning_defs; i < countof(warning_defs); i++, p++) {
      if (p->flags & WD_ALL) *(int *)((uint8_t *)s + p->offset) = 1;
    }
    return 0;
  } else {
    return set_flag(s, warning_defs, countof(warning_defs), warning_name, value);
  }
}

// Set/reset a flag
int tcc_set_flag(TCCState *s, const char *flag_name, int value) {
  return set_flag(s, flag_defs, countof(flag_defs), flag_name, value);
}

static int64_t getclock_us(void) {
#ifdef _WIN32
  struct _timeb tb;
  _ftime(&tb);
  return (tb.time * 1000LL + tb.millitm) * 1000LL;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * INT64_C(1000000) + tv.tv_usec;
#endif
}

void help(void) {
  printf("tcc version " TCC_VERSION " - Tiny C Compiler - Copyright (C) 2001-2006 Fabrice Bellard\n"
      "usage: cc [-v] [-c] [-o outfile] [-Bdir] [-bench] [-Idir] [-Dsym[=val]] [-Usym]\n"
      "          [-Wwarn] [-g] [-Ldir] [-llib] [-shared] [-soname name]\n"
      "          [-static] [infile1 infile2...]\n"
      "\n"
      "General options:\n"
      "  -v           display current version, increase verbosity\n"
      "  -c           compile only - generate an object file\n"
      "  -o outfile   set output filename\n"
      "  -B dir       set tcc internal library path\n"
      "  -bench       output compilation statistics\n"
      "  -fflag       set or reset (with 'no-' prefix) 'flag' (see man page)\n"
      "  -Wwarning    set or reset (with 'no-' prefix) 'warning' (see man page)\n"
      "  -w           disable all warnings\n"
      "  -g           generate runtime debug info\n"
      "Preprocessor options:\n"
      "  -E           preprocess only\n"
      "  -Idir        add include path 'dir'\n"
      "  -Dsym[=val]  define 'sym' with value 'val'\n"
      "  -Usym        undefine 'sym'\n"
      "Linker options:\n"
      "  -Ldir        add library path 'dir'\n"
      "  -llib        link with dynamic or static library 'lib'\n"
      "  -shared      generate a shared library\n"
      "  -soname      set name for shared library to be used at runtime\n"
      "  -entry sym   set start symbol name\n"
      "  -fixed addr  set base address (and do not generate relocation info)\n"
      "  -filealign n alignment for sections in PE file\n"
      "  -stub file   set DOS stub for PE file\n"
      "  -def file    generate import definition file for shared library\n"
      "  -static      static linking\n"
      "  -rdynamic    export all global symbols to dynamic linker\n"
      "  -r           generate (relocatable) object file\n"
      "  -m mapfile   generate linker map file\n"
      );
}

int parse_arguments(TCCState *s, int argc, char **argv) {
  int oind;
  const TCCOption *popt;
  const char *oarg, *p1, *r1;
  char *r;

  oind = 0;
  while (1) {
    if (oind >= argc) {
      if (nb_files == 0 && !print_search_dirs) {
        if (verbose) exit(0);
        goto show_help;
      }
      break;
    }
    r = argv[oind++];
    if (r[0] != '-' || r[1] == '\0') {
      // Add a new file
      dynarray_add((void ***)&files, &nb_files, r);
      if (!multiple_files) {
        oind--;
        // argv[0] will be this file
        break;
      }
    } else {
      // Find option in table (match only the first chars)
      popt = tcc_options;
      for (;;) {
        p1 = popt->name;
        if (p1 == NULL) error("invalid option -- '%s'", r);
        r1 = r + 1;
        for (;;) {
          if (*p1 == '\0') goto option_found;
          if (*r1 != *p1) break;
          p1++;
          r1++;
        }
        popt++;
      }
    option_found:
      if (popt->flags & TCC_OPTION_HAS_ARG) {
        if (*r1 != '\0' || (popt->flags & TCC_OPTION_NOSEP)) {
          oarg = r1;
        } else {
          if (oind >= argc) error("argument to '%s' is missing", r);
          oarg = argv[oind++];
        }
      } else {
        if (*r1 != '\0') goto show_help;
        oarg = NULL;
      }

      switch (popt->index) {
        case TCC_OPTION_HELP:
          show_help:
          help();
          exit(1);
        case TCC_OPTION_I:
          if (tcc_add_include_path(s, oarg) < 0) error("too many include paths");
          break;
        case TCC_OPTION_D: {
          char *sym, *value;
          sym = (char *) oarg;
          value = strchr(sym, '=');
          if (value) {
            *value = '\0';
            value++;
          }
          tcc_define_symbol(s, sym, value);
          break;
        }
        case TCC_OPTION_U:
          tcc_undefine_symbol(s, oarg);
          break;
        case TCC_OPTION_L:
          tcc_add_library_path(s, oarg);
          break;
        case TCC_OPTION_B:
          // Set tcc utilities path (mainly for tcc development)
          tcc_lib_path = oarg;
          break;
        case TCC_OPTION_l:
          dynarray_add((void ***) &files, &nb_files, r);
          nb_libraries++;
          break;
        case TCC_OPTION_bench:
          do_bench = 1;
          break;
        case TCC_OPTION_g:
          do_debug = 1;
          break;
        case TCC_OPTION_c:
          multiple_files = 1;
          output_type = TCC_OUTPUT_OBJ;
          break;
        case TCC_OPTION_nofll:
          s->nofll = 1;
          break;
        case TCC_OPTION_static:
          s->static_link = 1;
          break;
        case TCC_OPTION_shared:
          output_type = TCC_OUTPUT_DLL;
          break;
        case TCC_OPTION_soname:
          s->soname = oarg; 
          break;
        case TCC_OPTION_entry:
          s->start_symbol = oarg; 
          break;
        case TCC_OPTION_fixed:
          s->imagebase = strtoul(oarg, NULL, 0);
          break;
        case TCC_OPTION_filealign:
          s->filealign = strtoul(oarg, NULL, 0);
          break;
        case TCC_OPTION_stub:
          s->stub = oarg; 
          break;
        case TCC_OPTION_def:
          s->def_file = oarg; 
          break;
        case TCC_OPTION_o:
          multiple_files = 1;
          outfile = oarg;
          break;
        case TCC_OPTION_m:
          s->mapfile = oarg;
          break;
        case TCC_OPTION_r:
          // Generate a .o merging several output files
          reloc_output = 1;
          output_type = TCC_OUTPUT_OBJ;
          break;
        case TCC_OPTION_nostdinc:
          s->nostdinc = 1;
          break;
        case TCC_OPTION_nostdlib:
          s->nostdlib = 1;
          break;
        case TCC_OPTION_print_search_dirs:
          print_search_dirs = 1;
          break;
        case TCC_OPTION_v:
          do {
            if (verbose++ == 0)  printf("tcc version %s\n", TCC_VERSION);
          } while (*oarg++ == 'v');
          break;
        case TCC_OPTION_f:
          if (tcc_set_flag(s, oarg, 1) < 0 && s->warn_unsupported) goto unsupported_option;
          break;
        case TCC_OPTION_W:
          if (tcc_set_warning(s, oarg, 1) < 0 && s->warn_unsupported) goto unsupported_option;
          break;
        case TCC_OPTION_w:
          s->warn_none = 1;
          break;
        case TCC_OPTION_rdynamic:
          s->rdynamic = 1;
          break;
        case TCC_OPTION_Wl: {
          const char *p;
          if (strstart(oarg, "-Ttext,", &p)) {
            s->text_addr = strtoul(p, NULL, 16);
            s->has_text_addr = 1;
          } else if (strstart(oarg, "--oformat,", &p)) {
            if (strstart(p, "elf32-", NULL)) {
              s->output_format = TCC_OUTPUT_FORMAT_ELF;
            } else if (!strcmp(p, "binary")) {
              s->output_format = TCC_OUTPUT_FORMAT_BINARY;
            } else {
              error("target %s not found", p);
            }
          } else {
            error("unsupported linker option '%s'", oarg);
          }
          break;
        }
        case TCC_OPTION_E:
          output_type = TCC_OUTPUT_PREPROCESS;
          break;
        default:
          if (s->warn_unsupported) {
          unsupported_option:
            warning("unsupported option '%s'", r);
          }
      }
    }
  }
  return oind;
}

int main(int argc, char **argv) {
  int i;
  TCCState *s;
  int nb_objfiles, ret, oind;
  char objfilename[1024];
  int64_t start_time = 0;

  init_util();
  s = tcc_new();
  output_type = TCC_OUTPUT_EXE;
  outfile = NULL;
  multiple_files = 1;
  files = NULL;
  nb_files = 0;
  nb_libraries = 0;
  reloc_output = 0;
  print_search_dirs = 0;
  ret = 0;

  oind = parse_arguments(s, argc - 1, argv + 1) + 1;

  if (print_search_dirs) {
    printf("install: %s/\n", tcc_lib_path);
    return 0;
  }
  
  nb_objfiles = nb_files - nb_libraries;

  // Check -c consistency: only single file handled. 
  // TODO: check file type
  if (output_type == TCC_OUTPUT_OBJ && !reloc_output) {
    // Accepts only a single input file
    if (nb_objfiles != 1) error("cannot specify multiple files with -c");
    if (nb_libraries != 0) error("cannot specify libraries with -c");
  }

  if (output_type == TCC_OUTPUT_PREPROCESS) {
    if (!outfile) {
      s->outfile = stdout;
    } else {
      s->outfile = fopen(outfile, "w");
      if (!s->outfile) error("could not open '%s", outfile);
    }
  } else {
    if (!outfile) {
      // Compute default outfile name
      char *ext;
      const char *name = strcmp(files[0], "-") == 0 ? "a" : tcc_basename(files[0]);
      pstrcpy(objfilename, sizeof(objfilename), name);
      ext = tcc_fileextension(objfilename);

      if (output_type == TCC_OUTPUT_DLL) {
        strcpy(ext, ".dll");
      } else if (output_type == TCC_OUTPUT_EXE) {
        strcpy(ext, ".exe");
      } else if (output_type == TCC_OUTPUT_OBJ && !reloc_output && *ext) {
        strcpy(ext, ".o");
      } else {
        pstrcpy(objfilename, sizeof(objfilename), "a.out");
      }
      outfile = objfilename;
    }
  }

  if (do_bench) {
    start_time = getclock_us();
  }

  tcc_set_output_type(s, output_type);

  // Compile or add each files or library
  for (i = 0; i < nb_files && ret == 0; i++) {
    const char *filename;

    filename = files[i];
    if (output_type == TCC_OUTPUT_PREPROCESS) {
      if (tcc_add_file_ex(s, filename, AFF_PRINT_ERROR | AFF_PREPROCESS) < 0) ret = 1;
    } else if (filename[0] == '-' && filename[1]) {
      if (tcc_add_library(s, filename + 2) < 0) error("cannot find %s", filename);
    } else {
      if (verbose == 1) printf("-> %s\n", filename);
      if (tcc_add_file(s, filename) < 0) ret = 1;
    }
  }

  // Free all files
  tcc_free(files);
  if (ret) goto cleanup;

  if (do_bench) {
    double total_time;
    total_time = (double) (getclock_us() - start_time) / 1000000.0;
    if (total_time < 0.001) total_time = 0.001;
    if (total_bytes < 1) total_bytes = 1;
    printf("%d idents, %d lines, %d bytes, %0.3f s, %d lines/s, %0.1f MB/s\n", 
           tok_ident - TOK_IDENT, total_lines, total_bytes,
           total_time, (int)(total_lines / total_time), 
           total_bytes / total_time / 1000000.0); 
  }

  if (s->output_type == TCC_OUTPUT_PREPROCESS) {
    if (outfile) fclose(s->outfile);
  } else if (s->output_type != TCC_OUTPUT_OBJ) {
    ret = pe_output_file(s, outfile);
  } else {
    ret = tcc_output_file(s, outfile) ? 1 : 0;
  }

cleanup:
  tcc_delete(s);

#ifdef MEM_DEBUG
  if (do_bench) {
    printf("memory: %d bytes, max = %d bytes\n", mem_cur_size, mem_max_size);
  }
#endif
  return ret;
}

