//
//  cc.h - Tiny C Compiler for Sanos
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

#ifndef CC_H
#define CC_H

#include "config.h"

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "elf.h"

#ifndef offsetof
#define offsetof(type, field) ((size_t) &((type *)0)->field)
#endif

#ifndef countof
#define countof(tab) (sizeof(tab) / sizeof((tab)[0]))
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

// Wide-character type
typedef unsigned short nwchar_t;

// Pointer size, in bytes
#define PTR_SIZE 4

// long double size and alignment, in bytes
#define LDOUBLE_SIZE  12
#define LDOUBLE_ALIGN 4

// Maximum alignment (for aligned attribute support)
#define MAX_ALIGN     8

// Capacity limits
#define INCLUDE_STACK_SIZE          32
#define IFDEF_STACK_SIZE            64
#define VSTACK_SIZE                256
#define STRING_MAX_SIZE           1024
#define PACK_STACK_SIZE              8
#define CACHED_INCLUDES_HASH_SIZE  512
#define TOK_HASH_SIZE             8192  // must be a power of two
#define TOK_ALLOC_INCR             512  // must be a power of two
#define TOK_MAX_SIZE                 4  // Token max size in int unit when stored in string
#define IO_BUF_SIZE               8192

// Token values

#define TOK_EOF       (-1)         // End of file
#define TOK_LINEFEED  10           // Line feed

// All identifiers and strings have token above that
#define TOK_IDENT 256

// NB: The following compare tokens depend on i386 asm code
#define TOK_ULT       0x92
#define TOK_UGE       0x93
#define TOK_EQ        0x94
#define TOK_NE        0x95
#define TOK_ULE       0x96
#define TOK_UGT       0x97
#define TOK_Nset      0x98
#define TOK_Nclear    0x99
#define TOK_LT        0x9c
#define TOK_GE        0x9d
#define TOK_LE        0x9e
#define TOK_GT        0x9f

#define TOK_LAND      0xa0
#define TOK_LOR       0xa1

#define TOK_DEC       0xa2
#define TOK_MID       0xa3        // inc/dec, to void constant
#define TOK_INC       0xa4
#define TOK_UDIV      0xb0        // unsigned division
#define TOK_UMOD      0xb1        // unsigned modulo
#define TOK_PDIV      0xb2        // fast division with undefined rounding for pointers
#define TOK_CINT      0xb3        // number in tokc
#define TOK_CCHAR     0xb4        // char constant in tokc
#define TOK_STR       0xb5        // pointer to string in tokc
#define TOK_TWOSHARPS 0xb6        // ## preprocessing token
#define TOK_LCHAR     0xb7
#define TOK_LSTR      0xb8
#define TOK_CFLOAT    0xb9        // float constant
#define TOK_LINENUM   0xba        // line number info
#define TOK_CDOUBLE   0xc0        // double constant
#define TOK_CLDOUBLE  0xc1        // long double constant
#define TOK_UMULL     0xc2        // unsigned 32x32 -> 64 mul
#define TOK_ADDC1     0xc3        // add with carry generation
#define TOK_ADDC2     0xc4        // add with carry use
#define TOK_SUBC1     0xc5        // subtract with carry generation
#define TOK_SUBC2     0xc6        // subtract with carry use
#define TOK_CUINT     0xc8        // unsigned int constant
#define TOK_CLLONG    0xc9        // long long constant
#define TOK_CULLONG   0xca        // unsigned long long constant
#define TOK_ARROW     0xcb
#define TOK_DOTS      0xcc        // three dots
#define TOK_SHR       0xcd        // unsigned shift right
#define TOK_PPNUM     0xce        // preprocessor number

#define TOK_SHL       0x01        // shift left
#define TOK_SAR       0x02        // signed shift right

// Assignment operators: normal operator or 0x80
#define TOK_A_MOD     0xa5
#define TOK_A_AND     0xa6
#define TOK_A_MUL     0xaa
#define TOK_A_ADD     0xab
#define TOK_A_SUB     0xad
#define TOK_A_DIV     0xaf
#define TOK_A_XOR     0xde
#define TOK_A_OR      0xfc
#define TOK_A_SHL     0x81
#define TOK_A_SAR     0x82

#define TOK_ASM_int TOK_INT

enum tcc_token {
  TOK_LAST = TOK_IDENT - 1,
#define DEF(id, str) id,
#include "tokens.h"
#undef DEF
};

#define TOK_UIDENT TOK_DEFINE

// Header magic value for a.out archives
#define ARMAG  "!<arch>\012"

// Dynamic string buffer
typedef struct CString {
  int size;                       // Size in bytes
  void *data;                     // Either 'char *' or 'nwchar_t *'
  int size_allocated;
  void *data_allocated;           // If non NULL, data has been malloced
} CString;

struct Sym;

// Type definition
typedef struct CType {
  int t;
  struct Sym *ref;
} CType;

// Symbol definition
typedef struct Sym {
  int v;                          // Symbol token
  int r;                          // Associated register
  int c;                          // Associated number
  CType type;                     // Associated type
  struct Sym *next;               // Next related symbol
  struct Sym *prev;               // Prev symbol in stack
  struct Sym *prev_tok;           // Previous symbol for this token
} Sym;

// Section definition
typedef struct Section {
  unsigned long data_offset;      // Current data offset
  unsigned char *data;            // Section data
  unsigned long data_allocated;   // Used for realloc() handling
  int sh_name;                    // ELF section name (only used during output)
  int sh_num;                     // ELF section number
  int sh_type;                    // ELF section type
  int sh_flags;                   // ELF section flags
  int sh_info;                    // ELF section info
  int sh_addralign;               // ELF section alignment
  int sh_entsize;                 // ELF entry size
  unsigned long sh_size;          // Section size (only used during output)
  unsigned long sh_addr;          // Address at which the section is relocated
  unsigned long sh_offset;        // File offset
  int nb_hashed_syms;             // Used to resize the hash table
  struct Section *link;           // Link to another section
  struct Section *reloc;          // Corresponding section for relocation, if any
  struct Section *hash;           // Hash table for symbols
  struct Section *next;           // Used for linking merged sections
  int unused;                     // Used for unused section elimination
  char name[1];                   // Section name
} Section;

// GNU stab debug codes

typedef struct {
  unsigned long n_strx;           // Index into string table of name
  unsigned char n_type;           // Type of symbol
  unsigned char n_other;          // Misc info (usually empty)
  unsigned short n_desc;          // description field
  unsigned long n_value;          // value of symbol
} Stab_Sym;

#define STAB(name, code, str) name=code,

enum stab_debug_code {
  STAB(N_SO, 0x64, "SO")          // Name of main source file
  STAB(N_BINCL, 0x82, "BINCL")    // Beginning of an include file
  STAB(N_EINCL, 0xa2, "EINCL")    // End of an include file
  STAB(N_SLINE, 0x44, "SLINE")    // Line number in text segment
  STAB(N_FUN, 0x24, "FUN")        // Function name or text-segment variable for C
};

#undef STAB

// Special flag to indicate that the section should not be linked to the other ones
#define SHF_PRIVATE 0x80000000

#define SECTION_ABS ((void *) 1)

typedef struct DLLReference {
  int level;
  char name[1];
} DLLReference;

// Token symbol
typedef struct TokenSym {
  struct TokenSym *hash_next;
  struct Sym *sym_define;         // Direct pointer to define
  struct Sym *sym_label;          // Direct pointer to label
  struct Sym *sym_struct;         // Direct pointer to structure
  struct Sym *sym_identifier;     // Direct pointer to identifier
  int tok;                        // Token number
  int len;
  char str[1];
} TokenSym;

// Constant value
typedef union CValue {
  long double ld;
  double d;
  float f;
  int i;
  unsigned int ui;
  unsigned int ul;
  int64_t ll;
  uint64_t ull;
  unsigned int word[2];
  struct CString *cstr;
  void *ptr;
  int tab[1];
} CValue;

// Stack value
typedef struct SValue {
  CType type;                     // Type
  unsigned short r;               // Register + flags
  unsigned short r2;              // Second register, used for 'long long' type. If not used, set to VT_CONST
  CValue c;                       // constant, if VT_CONST
  struct Sym *sym;                // Symbol, if (VT_SYM | VT_CONST)
} SValue;

#define SYM_STRUCT     0x40000000 // struct/union/enum symbol space
#define SYM_FIELD      0x20000000 // struct/union field symbol space
#define SYM_FIRST_ANOM 0x10000000 // First anonymous sym

// Stored in 'Sym.c' field
#define FUNC_NEW       1          // ANSI function prototype
#define FUNC_OLD       2          // Old function prototype
#define FUNC_ELLIPSIS  3          // ANSI function prototype with ...

// Stored in 'Sym.r' field
#define FUNC_CDECL     0          // Standard c call
#define FUNC_STDCALL   1          // Pascal c call
#define FUNC_FASTCALL1 2          // First param in %eax
#define FUNC_FASTCALL2 3          // First parameters in %eax, %edx
#define FUNC_FASTCALL3 4          // First parameter in %eax, %edx, %ecx
#define FUNC_FASTCALLW 5          // First parameter in %ecx, %edx

// Field 'Sym.type.t' for macros
#define MACRO_OBJ      0          // Object-like macro
#define MACRO_FUNC     1          // Function-like macro

// Field 'Sym.r' for C labels
#define LABEL_DEFINED  0          // Label is defined
#define LABEL_FORWARD  1          // Label is forward defined
#define LABEL_DECLARED 2          // label is declared but never used

#define VT_VALMASK       0x00ff
#define VT_CONST         0x00f0   // constant in vc (must be first non register value)
#define VT_LLOCAL        0x00f1   // lvalue, offset on stack
#define VT_LOCAL         0x00f2   // offset on stack
#define VT_CMP           0x00f3   // the value is stored in processor flags (in vc)
#define VT_JMP           0x00f4   // value is the consequence of jmp true (even)
#define VT_JMPI          0x00f5   // value is the consequence of jmp false (odd)

#define VT_LVAL          0x0100   // var is an lvalue
#define VT_SYM           0x0200   // a symbol value is added
#define VT_MUSTCAST      0x0400   // value must be casted to be correct (used for char/short stored in integer registers)

#define VT_LVAL_BYTE     0x1000   // lvalue is a byte
#define VT_LVAL_SHORT    0x2000   // lvalue is a short
#define VT_LVAL_UNSIGNED 0x4000   // lvalue is unsigned
#define VT_LVAL_TYPE     (VT_LVAL_BYTE | VT_LVAL_SHORT | VT_LVAL_UNSIGNED)

// Types
#define VT_INT        0           // integer type
#define VT_BYTE       1           // signed byte type
#define VT_SHORT      2           // short type
#define VT_VOID       3           // void type
#define VT_PTR        4           // pointer
#define VT_ENUM       5           // enum definition
#define VT_FUNC       6           // function type
#define VT_STRUCT     7           // struct/union definition
#define VT_FLOAT      8           // IEEE float
#define VT_DOUBLE     9           // IEEE double
#define VT_LDOUBLE   10           // IEEE long double
#define VT_BOOL      11           // ISOC99 boolean type
#define VT_LLONG     12           // 64 bit integer
#define VT_LONG      13           // long integer (NEVER USED as type, only during parsing)
#define VT_LABEL     14           // asm label

#define VT_BTYPE      0x000f      // mask for basic type
#define VT_UNSIGNED   0x0010      // unsigned type
#define VT_ARRAY      0x0020      // array type (also has VT_PTR)
#define VT_BITFIELD   0x0040      // bitfield modifier
#define VT_CONSTANT   0x0800      // const modifier
#define VT_VOLATILE   0x1000      // volatile modifier
#define VT_SIGNED     0x2000      // signed type

// Storage
#define VT_EXTERN  0x00000080     // extern definition
#define VT_STATIC  0x00000100     // static variable
#define VT_TYPEDEF 0x00000200     // typedef definition
#define VT_INLINE  0x00000400     // inline definition

#define VT_STRUCT_SHIFT 16        // shift for bitfield shift values

// Type mask
#define VT_STORAGE (VT_EXTERN | VT_STATIC | VT_TYPEDEF | VT_INLINE)
#define VT_TYPE    (~(VT_STORAGE))

// Wrappers for casting sym->r for other purposes
typedef struct {
  unsigned
    func_call : 8,
    func_args : 8,
    func_export : 1,
    func_naked : 1;
} func_attr_t;

#define FUNC_CALL(r) (((func_attr_t*)&(r))->func_call)
#define FUNC_EXPORT(r) (((func_attr_t*)&(r))->func_export)
#define FUNC_NAKED(r) (((func_attr_t*)&(r))->func_naked)
#define FUNC_ARGS(r) (((func_attr_t*)&(r))->func_args)
#define INLINE_DEF(r) (*(int **)&(r))

// GNUC attribute definition
typedef struct AttributeDef {
  int aligned;
  int packed; 
  Section *section;
  int func_attr;                  // Calling convention, exports, ...
} AttributeDef;

// type_decl() types
#define TYPE_ABSTRACT  1          // Type without variable
#define TYPE_DIRECT    2          // Type with variable

// Number of available registers
#define NB_REGS        5          // Number of register used (between 5 and 8)
#define NB_SAVED_REGS  3
#define NB_ASM_REGS    8
//#define USE_EBX                   // Use ebx register for pointers

// A register can belong to several classes. The classes must be
// sorted from more general to more precise (see gv2() code which does
// assumptions on it).
#define RC_INT     0x0001         // Generic integer register
#define RC_FLOAT   0x0002         // Generic float register
#define RC_EAX     0x0004
#define RC_ST0     0x0008 
#define RC_ECX     0x0010
#define RC_EDX     0x0020
#define RC_SAVE    0x0040         // Register is callee-saved
#define RC_PTR     0x0080         // Prefered register for pointers
#define RC_IRET    RC_EAX         // Function return: integer register
#define RC_LRET    RC_EDX         // Function return: second integer register
#define RC_FRET    RC_ST0         // Function return: float register

// Pretty names for the registers
enum {
  TREG_EAX = 0,
  TREG_ECX,
  TREG_EDX,
  TREG_EBX,
  TREG_ST0,
};

// Return registers for function
#define REG_IRET TREG_EAX // Single word int return register
#define REG_LRET TREG_EDX // Second word return register (for long long)
#define REG_FRET TREG_ST0 // Float return register

extern int reg_classes[];

// Code generation buffer
enum {
  CodeStart,
  CodeLabel,
  CodeJump,
  CodeShortJump,
  CodeReloc,
  CodeAlign,
  CodeNop,
  CodeLine,
  CodeEnd,
};

typedef struct {
  int type;
  int param;
  int ind;
  int addr;
  int target;
  Sym *sym;
} Branch;

typedef struct {
  unsigned char *code;
  int ind;
  Branch *branch;
  int br;
} CodeBuffer;

// Parsing state (used to save parser state to reparse part of the source several times)
typedef struct ParseState {
  int *macro_ptr;
  int line_num;
  int tok;
  CValue tokc;
} ParseState;

// Used to record tokens
typedef struct TokenString {
  int *str;
  int len;
  int allocated_len;
  int last_line_num;
} TokenString;

struct macro_level {
  struct macro_level *prev;
  int *p;
};

// Additional informations about token (tok_flags)
#define TOK_FLAG_BOL   0x0001     /* beginning of line before */
#define TOK_FLAG_BOF   0x0002     /* beginning of file before */
#define TOK_FLAG_ENDIF 0x0004     /* a endif was found matching starting #ifdef */
#define TOK_FLAG_EOF   0x0008     /* end of file */

typedef struct ExprValue {
  uint32_t v;
  Sym *sym;
} ExprValue;

#define MAX_ASM_OPERANDS 30

typedef struct ASMOperand {
  int id;                         // GCC 3 optional identifier (0 if number only supported
  char *constraint;
  char asm_str[16];               // Computed asm string for operand
  SValue *vt;                     // C value of the expression
  int ref_index;                  // If >= 0, gives reference to a output constraint
  int input_index;                // If >= 0, gives reference to an input constraint
  int priority;                   // Priority, used to assign registers
  int reg;                        // If >= 0, register number used for this operand
  int is_llong;                   // True if double register value
  int is_memory;                  // True if memory operand
  int is_rw;                      // For '+' modifier
} ASMOperand;

typedef struct BufferedFile {
  uint8_t *buf_ptr;
  uint8_t *buf_end;
  int fd;
  int line_num;                   // Current line number
  int ifndef_macro;               // #ifndef macro / #endif search
  int ifndef_macro_saved;         // Saved ifndef_macro
  int *ifdef_stack_ptr;           // ifdef_stack value at the start of the file
  char inc_type;                  // Type of include
  char inc_filename[512];         // Filename specified by the user
  char filename[1024];            // Current filename
  unsigned char buffer[IO_BUF_SIZE + 1]; // Extra size for CH_EOB char
} BufferedFile;

#define CH_EOB   '\\'             // End of buffer or '\0' char in file
#define CH_EOF   (-1)             // End of file

// Include file cache, used to find files faster and also to eliminate
// inclusion if the include file is protected by #ifndef ... #endif
typedef struct CachedInclude {
  int ifndef_macro;
  int hash_next;                  // -1 if none
  char type;                      // '"' or '>' to give include type
  char filename[1];               // Path specified in #include
} CachedInclude;

typedef struct TCCState {
  int output_type;
 
  BufferedFile **include_stack_ptr;
  int *ifdef_stack_ptr;

  // Include file handling
  char **include_paths;
  int nb_include_paths;
  char **sysinclude_paths;
  int nb_sysinclude_paths;
  CachedInclude **cached_includes;
  int nb_cached_includes;

  char **library_paths;
  int nb_library_paths;

  // Array of all loaded dlls (including those referenced by loaded dlls)
  DLLReference **loaded_dlls;
  int nb_loaded_dlls;

  // Sections
  Section **sections;
  int nb_sections; // number of sections, including first dummy section

  // got handling
  Section *got;
  Section *plt;
  unsigned long *got_offsets;
  int nb_got_offsets;

  // Give the correspondance from symtab indexes to dynsym indexes
  int *symtab_to_dynsym;

  // Temporary dynamic symbol sections (for dll loading)
  Section *dynsymtab_section;

  // Exported dynamic symbol section
  Section *dynsym;

  // If true, disable funtion-level linking
  int nofll;

  // If true, no standard headers are added
  int nostdinc;

  // If true, no standard libraries are added
  int nostdlib;

  // If true, do not use common symbols for .bss data
  int nocommon; 

  // If true, static linking is performed
  int static_link;

  // soname as specified on the command line (-soname)
  const char *soname;
    
  // Start symbol
  const char *start_symbol;
    
  // DOS stub
  const char *stub;

  // DEF file
  const char *def_file;

  // Image base address for non-relocatable PE files
  unsigned long imagebase;

  // File alignment for sections in PE files
  unsigned long filealign;

  // If true, all symbols are exported
  int rdynamic;

  // If true, only link in referenced objects from archive
  int alacarte_link;

  // Address of text section
  unsigned long text_addr;
  int has_text_addr;

  // Output format, see TCC_OUTPUT_FORMAT_xxx
  int output_format;

  // C language options
  int char_is_unsigned;
  int leading_underscore;
    
  // Warning switches
  int warn_write_strings;
  int warn_unsupported;
  int warn_error;
  int warn_none;
  int warn_implicit_function_declaration;

  // Error handling
  void *error_opaque;
  void (*error_func)(void *opaque, const char *msg);
  int error_set_jmp_enabled;
  jmp_buf error_jmp_buf;
  int nb_errors;

  // Assembler state
  Sym *asm_labels;

  // See include_stack_ptr
  BufferedFile *include_stack[INCLUDE_STACK_SIZE];

  // See ifdef_stack_ptr
  int ifdef_stack[IFDEF_STACK_SIZE];

  // See cached_includes
  int cached_includes_hash[CACHED_INCLUDES_HASH_SIZE];

  // Pack stack
  int pack_stack[PACK_STACK_SIZE];
  int *pack_stack_ptr;

  // Output file for preprocessing
  FILE *outfile;
    
  // Linker map file
  const char *mapfile;
} TCCState;

// Output type
#define TCC_OUTPUT_EXE        1   // Executable file (default)
#define TCC_OUTPUT_DLL        2   // Dynamic library
#define TCC_OUTPUT_OBJ        3   // Object file
#define TCC_OUTPUT_PREPROCESS 4   // Preprocessed file (used internally)

// Output format
#define TCC_OUTPUT_FORMAT_ELF    0 // Default output format: ELF
#define TCC_OUTPUT_FORMAT_BINARY 1 // Binary image output

// Flags for tcc_add_file_internal()
#define AFF_PRINT_ERROR     0x0001 // Print error if file not found
#define AFF_REFERENCED_DLL  0x0002 // Load a referenced DLL from another DLL
#define AFF_PREPROCESS      0x0004 // Preprocess file

// Parse flags
#define PARSE_FLAG_PREPROCESS   0x0001 // Activate preprocessing
#define PARSE_FLAG_TOK_NUM      0x0002 // Return numbers instead of TOK_PPNUM
#define PARSE_FLAG_LINEFEED     0x0004 // Line feed is returned as a token (and at at eof)
#define PARSE_FLAG_ASM_COMMENTS 0x0008 // '#' can be used for line comment
#define PARSE_FLAG_MASM         0x0010 // Use masm syntax for inline assembler

//
// Globals
//

extern CType char_pointer_type, func_old_type, int_type;

extern int rsym;                  // return symbol
extern int anon_sym;              // anonymous symbol index
extern int ind;                   // output code index
extern int loc;                   // local variable index
extern int func_naked;            // no generation of function prolog

// Expression generation modifiers
extern int const_wanted;          // true if constant wanted
extern int nocode_wanted;         // true if no code generation wanted for an expression
extern int global_expr;           // true if compound literals must be allocated globally (used during initializers parsing)
extern CType func_vt;             // current function return type (used by return instruction)

extern int func_vc;
extern int last_line_num, last_ind, func_ind; // debug last line number and pc
extern TokenSym **table_ident;
extern TokenSym *hash_ident[TOK_HASH_SIZE];
extern char token_buf[STRING_MAX_SIZE + 1];
extern char *func_name;
extern CType func_old_type;
extern Sym *global_stack, *local_stack;
extern Sym *define_stack;
extern Sym *global_label_stack, *local_label_stack;

extern SValue vstack[VSTACK_SIZE];
extern SValue *vtop;

extern TCCState *tcc_state;
extern int verbose;
extern int do_debug;
extern int tok_ident;

// Parser state
extern BufferedFile *file;
extern int ch;
extern int tok;
extern CValue tokc;
extern CString tokcstr;           // Current parsed string, if any
extern int tok_flags;
extern int parse_flags;
extern int *macro_ptr;

extern int total_lines;
extern int total_bytes;

extern const char *tcc_lib_path;

// Symbol sections
extern Section *symtab_section, *strtab_section;

// Text section
extern Section *text_section, *data_section, *bss_section; // Predefined sections
extern Section *cur_text_section; // Current section where function code is generated
extern Section *last_text_section; // to handle .previous asm directive

// Debug sections
extern Section *stab_section, *stabstr_section;

//
// Global functions
//

// symbol.c
Sym *sym_malloc(void);
void sym_free(Sym *sym);
Section *new_section(TCCState *s1, const char *name, int sh_type, int sh_flags);
void free_section(Section *s);
void section_realloc(Section *sec, unsigned long new_size);
void *section_ptr_add(Section *sec, unsigned long size);
Section *find_section(TCCState *s1, const char *name);
void put_extern_sym_ex(Sym *sym, Section *section, unsigned long value, unsigned long size, int add_underscore);
void put_extern_sym(Sym *sym, Section *section, unsigned long value, unsigned long size);
void put_reloc(Section *s, Sym *sym, unsigned long addr, int type);
Sym *sym_push2(Sym **ps, int v, int t, int c);
Sym *sym_find2(Sym *s, int v);
Sym *sym_push(int v, CType *type, int r, int c);
void sym_pop(Sym **ptop, Sym *b);
Sym *sym_find(int v);
Sym *global_identifier_push(int v, int t, int c);
Sym *external_global_sym(int v, CType *type, int r);
Sym *get_sym_ref(CType *type, Section *sec, unsigned long offset, unsigned long size);
Sym *external_sym(int v, CType *type, int r);
void label_pop(Sym **ptop, Sym *slast);
Sym *label_push(Sym **ptop, int v, int flags);
Sym *label_find(int v);

// type.c
int is_float(int t);
int type_size(CType *type, int *a);
void test_lvalue(void);
int lvalue_type(int t);
int are_compatible_types(CType *type1, CType *type2);
int are_compatible_parameter_types(CType *type1, CType *type2);
void check_comparison_pointer_types(SValue *p1, SValue *p2, int op);
int pointed_size(CType *type);
int is_null_pointer(SValue *p);
int is_integer_btype(int bt);
CType *pointed_type(CType *type);
void mk_pointer(CType *type);
void type_to_str(char *buf, int buf_size, CType *type, const char *varstr);

// preproc.c
TokenSym *tok_alloc(const char *str, int len);
char *get_tok_str(int v, CValue *cv);
void tok_str_new(TokenString *s);
void tok_str_free(int *str);
void tok_str_add(TokenString *s, int t);
void tok_str_add_tok(TokenString *s);
BufferedFile *tcc_open(TCCState *s1, const char *filename);
void tcc_close(BufferedFile *bf);
int handle_eob(void);
void finp(void);
void minp(void);
uint8_t *parse_comment(uint8_t *p);
void skip(int c);
void next(void);
void next_nomacro(void);
void macro_subst(TokenString *tok_str, Sym **nested_list, const int *macro_str, struct macro_level **can_read_stream);
void parse_define(void);
void free_defines(Sym *b);
Sym *define_find(int v);
void define_push(int v, int macro_type, int *str, Sym *first_arg);
void define_undef(Sym *s);
void unget_tok(int last_tok);
void save_parse_state(ParseState *s);
void restore_parse_state(ParseState *s);

// compiler.c
void type_decl(CType *type, AttributeDef *ad, int *v, int td);
int expr_const(void);
void expr_eq(void);
void gexpr(void);
int parse_btype(CType *type, AttributeDef *ad);
void parse_expr_type(CType *type);
void inc(int post, int c);
void expr_type(CType *type);
void unary_type(CType *type);
void block(int *bsym, int *csym, int *case_sym, int *def_sym, int case_reg, int is_expr);
void decl_initializer(CType *type, Section *sec, unsigned long c, int first, int size_only);
void decl_initializer_alloc(CType *type, AttributeDef *ad, int r, int has_init, int v, int scope);
void decl(int l);

void preprocess_init(TCCState *s1);

int tcc_compile(TCCState *s1);
int tcc_preprocess(TCCState *s1);
int tcc_compile_string(TCCState *s, const char *str);
void tcc_define_symbol(TCCState *s1, const char *sym, const char *value);
void tcc_undefine_symbol(TCCState *s1, const char *sym);

TCCState *tcc_new(void);
void tcc_delete(TCCState *s1);

int tcc_add_include_path(TCCState *s1, const char *pathname);
int tcc_add_sysinclude_path(TCCState *s1, const char *pathname);
int tcc_add_library_path(TCCState *s, const char *pathname);
int tcc_add_file_ex(TCCState *s1, const char *filename, int flags);
int tcc_add_file(TCCState *s, const char *filename);
int tcc_add_dll(TCCState *s, const char *filename, int flags);
int tcc_add_library(TCCState *s, const char *libraryname);
int tcc_add_symbol(TCCState *s, const char *name, unsigned long val);
int tcc_set_output_type(TCCState *s, int output_type);

// codegen.c
void vstore(void);
void save_reg(int r);
void save_regs(int n);
void move_reg(int r, int s);
void vpushi(int v);
void vpush_tokc(int t);
void vpush_ref(CType *type, Section *sec, unsigned long offset, unsigned long size);
void vpush_global_sym(CType *type, int v);
void vdup(void);
void vsetc(CType *type, int r, CValue *vc);
void vset(CType *type, int r, int v);
void vseti(int r, int v);
void vswap(void);
int get_reg(int rc);
int gv(int rc);
void gv2(int rc1, int rc2);
void vrotb(int n);
void vpop(void);
void gv_dup(void);
void gen_cast(CType *type);
void gen_op(int op);
void gen_assign_cast(CType *dt);
void gaddrof(void);

// codegen386.c
void reset_code_buf(void);
void clear_code_buf(void);
void gcode(void);
void gstart(void);
void gend(void);
void gline(int linenum);

void mark_code_buffer(CodeBuffer *cb);
void cut_code_buffer(CodeBuffer *cb);
void paste_code_buffer(CodeBuffer *cb);

void g(int c);
void o(unsigned int c);
void gen_le16(int c);
void gen_le32(int c);
void gsym_at(int b, int l);
int gsym(int b);
void assign_label_symbol(int l, Sym *sym);
int gjmp(int l, int c);
void gjmp_addr(int a);
int glabel(void);
void galign(int n, int v);
void greloc(Sym *sym, int c, int rel);

void store(int r, SValue *v);
void load(int r, SValue *sv);
int gtst(int inv, int t);
void gen_opi(int op);
void gen_opf(int op);
void gfunc_call(int nb_args);
void gfunc_prolog(CType *func_type);
void gfunc_epilog(void);
void gen_cvt_itof(int t);
void gen_cvt_ftoi(int t);
void gen_cvt_ftof(int t);
void ggoto(void);

// asm.c
int tcc_assemble(TCCState *s1, int do_preprocess);
void asm_instr(void);
void masm_instr(TCCState *s1);
void asm_global_instr(void);
int find_constraint(ASMOperand *operands, int nb_operands, const char *name, const char **pp);
int asm_int_expr(TCCState *s1);
void asm_expr(TCCState *s1, ExprValue *pe);
void asm_expr_logic(TCCState *s1, ExprValue *pe);

static void *resolve_sym(TCCState *s1, const char *symbol, int type) { return NULL; }

// asm386.c
void gen_expr32(ExprValue *pe);
void asm_opcode(TCCState *s1, int opcode);
void subst_asm_operand(CString *add_str, SValue *sv, int modifier);
void asm_clobber(uint8_t *clobber_regs, const char *str);
void asm_compute_constraints(ASMOperand *operands, 
                             int nb_operands, int nb_outputs, 
                             const uint8_t *clobber_regs, int *pout_reg);
void asm_gen_code(ASMOperand *operands, int nb_operands, int nb_outputs, int is_output,
                  uint8_t *clobber_regs, int out_reg);

// elf.c
Section *new_symtab(TCCState *s1,
                    const char *symtab_name, int sh_type, int sh_flags,
                    const char *strtab_name, 
                    const char *hash_name, int hash_sh_flags);
int put_elf_str(Section *s, const char *sym);
int put_elf_sym(Section *s, unsigned long value, unsigned long size, int info, int other, int shndx, const char *name);
int add_elf_sym(Section *s, unsigned long value, unsigned long size, int info, int other, int sh_num, const char *name);
int find_elf_sym(Section *s, const char *name);

void put_elf_reloc(Section *symtab, Section *s, unsigned long offset, int type, int symbol);
void relocate_common_syms(void);
void relocate_syms(TCCState *s1, int do_resolve);
void relocate_section(TCCState *s1, Section *s);

void put_stabs(const char *str, int type, int other, int desc, unsigned long value);
void put_stabs_r(const char *str, int type, int other, int desc, unsigned long value, Section *sec, int sym_index);
void put_stabn(int type, int other, int desc, int value);
void put_stabd(int type, int other, int desc);

void tcc_add_linker_symbols(TCCState *s1);
int tcc_output_file(TCCState *s1, const char *filename);
int tcc_load_object_file(TCCState *s1, int fd, unsigned long file_offset);
int tcc_load_archive(TCCState *s1, int fd);
int tcc_load_dll(TCCState *s1, int fd, const char *filename, int level);
int tcc_load_ldscript(TCCState *s1);
void *tcc_get_symbol_err(TCCState *s, const char *name);

// pe.c
int pe_output_file(TCCState * s1, const char *filename);
int pe_load_def_file(struct TCCState *s1, int fd);
int pe_test_res_file(void *v, int size);
int pe_load_res_file(struct TCCState *s1, int fd);

// util.c
void *tcc_malloc(unsigned long size);
void *tcc_mallocz(unsigned long size);
void *tcc_realloc(void *ptr, unsigned long size);
char *tcc_strdup(const char *str);
void tcc_free(void *ptr);

char *tcc_basename(const char *name);
char *tcc_fileextension(const char *p);

char *pstrcpy(char *buf, int buf_size, const char *s);
char *pstrcat(char *buf, int buf_size, const char *s);
int strstart(const char *str, const char *val, const char **ptr);

int is_space(int c);
int is_id(int c);
int is_num(int c);
int is_idnum(int c);
int is_oct(int c);
int to_upper(int c);

void dynarray_add(void ***ptab, int *nb_ptr, void *data);
void dynarray_reset(void *pp, int *n);
int dynarray_assoc(void **pp, int n, int key);

void cstr_new(CString *cstr);
void cstr_free(CString *cstr);
void cstr_reset(CString *cstr);
void cstr_realloc(CString *cstr, int new_size);
void cstr_ccat(CString *cstr, int ch);
void cstr_cat(CString *cstr, const char *str);
void cstr_wccat(CString *cstr, int ch);
void add_char(CString *cstr, int c);

void error(const char *fmt, ...);
void warning(const char *fmt, ...);
void error_noabort(const char *fmt, ...);
void expect(const char *msg);

void *load_data(int fd, unsigned long file_offset, unsigned long size);

void init_util(void);

#endif

