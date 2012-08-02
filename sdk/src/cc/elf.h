//
// elf.h - Standard ELF types, structures, and macros.
//
// Copyright (C) 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
// This file is part of the GNU C Library.
// Contributed by Ian Lance Taylor <ian@cygnus.com>.
// Modified for Sanos C compiler by Michael Ringgaard.
//
// The GNU C Library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// The GNU C Library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with the GNU C Library; see the file COPYING.LIB.  If not,
// write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#ifndef _ELF_H
#define _ELF_H

#ifndef _WIN32
#include <inttypes.h>
#else
#ifndef __int8_t_defined
#define __int8_t_defined
typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long long int int64_t;
#endif

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int  uint64_t;
#endif

// Standard ELF types

// Type for a 16-bit quantity
typedef uint16_t Elf32_Half;

// Types for signed and unsigned 32-bit quantities
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;

// Types for signed and unsigned 64-bit quantities
typedef uint64_t Elf32_Xword;
typedef int64_t  Elf32_Sxword;

// Type of addresses
typedef uint32_t Elf32_Addr;

// Type of file offsets
typedef uint32_t Elf32_Off;

// Type for section indices, which are 16-bit quantities
typedef uint16_t Elf32_Section;

// Type of symbol indices
typedef uint32_t Elf32_Symndx;

// The ELF file header. This appears at the start of every ELF file.

#define EI_NIDENT (16)

typedef struct {
  unsigned char e_ident[EI_NIDENT];     // Magic number and other info
  Elf32_Half    e_type;                 // Object file type
  Elf32_Half    e_machine;              // Architecture
  Elf32_Word    e_version;              // Object file version
  Elf32_Addr    e_entry;                // Entry point virtual address
  Elf32_Off     e_phoff;                // Program header table file offset
  Elf32_Off     e_shoff;                // Section header table file offset
  Elf32_Word    e_flags;                // Processor-specific flags
  Elf32_Half    e_ehsize;               // ELF header size in bytes
  Elf32_Half    e_phentsize;            // Program header table entry size
  Elf32_Half    e_phnum;                // Program header table entry count
  Elf32_Half    e_shentsize;            // Section header table entry size
  Elf32_Half    e_shnum;                // Section header table entry count
  Elf32_Half    e_shstrndx;             // Section header string table index
} Elf32_Ehdr;

// Fields in the e_ident array.  The EI_* macros are indices into the
// array.  The macros under each EI_* macro are the values the byte
// may have. 

#define EI_MAG0         0               // File identification byte 0 index
#define ELFMAG0         0x7f            // Magic number byte 0

#define EI_MAG1         1               // File identification byte 1 index
#define ELFMAG1         'E'             // Magic number byte 1

#define EI_MAG2         2               // File identification byte 2 index
#define ELFMAG2         'L'             // Magic number byte 2

#define EI_MAG3         3               // File identification byte 3 index
#define ELFMAG3         'F'             // Magic number byte 3

// Conglomeration of the identification bytes, for easy testing as a word
#define ELFMAG          "\177ELF"
#define SELFMAG         4

#define EI_CLASS        4               // File class byte index
#define ELFCLASSNONE    0               // Invalid class
#define ELFCLASS32      1               // 32-bit objects
#define ELFCLASS64      2               // 64-bit objects
#define ELFCLASSNUM     3

#define EI_DATA         5               // Data encoding byte index
#define ELFDATANONE     0               // Invalid data encoding
#define ELFDATA2LSB     1               // 2's complement, little endian
#define ELFDATA2MSB     2               // 2's complement, big endian
#define ELFDATANUM      3

#define EI_VERSION      6               // File version byte index
                                        // Value must be EV_CURRENT

#define EI_OSABI        7               // OS ABI identification
#define ELFOSABI_SYSV           0       // UNIX System V ABI
#define ELFOSABI_HPUX           1       // HP-UX
#define ELFOSABI_FREEBSD        9       // Free BSD
#define ELFOSABI_ARM            97      // ARM
#define ELFOSABI_STANDALONE     255     // Standalone (embedded) application

#define EI_ABIVERSION   8               // ABI version

#define EI_PAD          9               // Byte index of padding bytes

// Legal values for e_type (object file type)

#define ET_NONE         0               // No file type
#define ET_REL          1               // Relocatable file
#define ET_EXEC         2               // Executable file
#define ET_DYN          3               // Shared object file
#define ET_CORE         4               // Core file
#define ET_NUM          5               // Number of defined types
#define ET_LOPROC       0xff00          // Processor-specific
#define ET_HIPROC       0xffff          // Processor-specific

// Legal values for e_machine (architecture) 

#define EM_NONE          0              // No machine
#define EM_M32           1              // AT&T WE 32100
#define EM_SPARC         2              // SUN SPARC
#define EM_386           3              // Intel 80386
#define EM_68K           4              // Motorola m68k family
#define EM_88K           5              // Motorola m88k family
#define EM_486           6              // Intel 80486
#define EM_860           7              // Intel 80860
#define EM_MIPS          8              // MIPS R3000 big-endian
#define EM_S370          9              // Amdahl
#define EM_MIPS_RS4_BE  10              // MIPS R4000 big-endian
#define EM_RS6000       11              // RS6000

#define EM_PARISC       15              // HPPA
#define EM_nCUBE        16              // nCUBE
#define EM_VPP500       17              // Fujitsu VPP500
#define EM_SPARC32PLUS  18              // Sun's "v8plus"
#define EM_960          19              // Intel 80960
#define EM_PPC          20              // PowerPC

#define EM_V800         36              // NEC V800 series
#define EM_FR20         37              // Fujitsu FR20
#define EM_RH32         38              // TRW RH32
#define EM_MMA          39              // Fujitsu MMA
#define EM_ARM          40              // ARM
#define EM_FAKE_ALPHA   41              // Digital Alpha
#define EM_SH           42              // Hitachi SH
#define EM_SPARCV9      43              // SPARC v9 64-bit
#define EM_TRICORE      44              // Siemens Tricore
#define EM_ARC          45              // Argonaut RISC Core
#define EM_H8_300       46              // Hitachi H8/300
#define EM_H8_300H      47              // Hitachi H8/300H
#define EM_H8S          48              // Hitachi H8S
#define EM_H8_500       49              // Hitachi H8/500
#define EM_IA_64        50              // Intel Merced
#define EM_MIPS_X       51              // Stanford MIPS-X
#define EM_COLDFIRE     52              // Motorola Coldfire
#define EM_68HC12       53              // Motorola M68HC12
#define EM_NUM          54

// If it is necessary to assign new unofficial EM_* values, please
// pick large random numbers (0x8523, 0xa7f2, etc.) to minimize the
// chances of collision with official or non-GNU unofficial values. 

#define EM_ALPHA        0x9026
#define EM_C60          0x9c60

// Legal values for e_version (version)

#define EV_NONE         0               // Invalid ELF version
#define EV_CURRENT      1               // Current version
#define EV_NUM          2

// Section header

typedef struct {
  Elf32_Word    sh_name;                // Section name (string tbl index)
  Elf32_Word    sh_type;                // Section type
  Elf32_Word    sh_flags;               // Section flags
  Elf32_Addr    sh_addr;                // Section virtual addr at execution
  Elf32_Off     sh_offset;              // Section file offset
  Elf32_Word    sh_size;                // Section size in bytes
  Elf32_Word    sh_link;                // Link to another section
  Elf32_Word    sh_info;                // Additional section information
  Elf32_Word    sh_addralign;           // Section alignment
  Elf32_Word    sh_entsize;             // Entry size if section holds table
} Elf32_Shdr;

// Special section indices

#define SHN_UNDEF       0               // Undefined section
#define SHN_LORESERVE   0xff00          // Start of reserved indices
#define SHN_LOPROC      0xff00          // Start of processor-specific
#define SHN_HIPROC      0xff1f          // End of processor-specific
#define SHN_ABS         0xfff1          // Associated symbol is absolute
#define SHN_COMMON      0xfff2          // Associated symbol is common
#define SHN_HIRESERVE   0xffff          // End of reserved indices

// Legal values for sh_type (section type)

#define SHT_NULL         0              // Section header table entry unused
#define SHT_PROGBITS     1              // Program data
#define SHT_SYMTAB       2              // Symbol table
#define SHT_STRTAB       3              // String table
#define SHT_RELA         4              // Relocation entries with addends
#define SHT_HASH         5              // Symbol hash table
#define SHT_DYNAMIC      6              // Dynamic linking information
#define SHT_NOTE         7              // Notes
#define SHT_NOBITS       8              // Program space with no data (bss)
#define SHT_REL          9              // Relocation entries, no addends
#define SHT_SHLIB        10             // Reserved
#define SHT_DYNSYM       11             // Dynamic linker symbol table
#define SHT_NUM          12             // Number of defined types. 
#define SHT_LOOS         0x60000000     // Start OS-specific
#define SHT_LOSUNW       0x6ffffffb     // Sun-specific low bound. 
#define SHT_SUNW_COMDAT  0x6ffffffb
#define SHT_SUNW_syminfo 0x6ffffffc
#define SHT_GNU_verdef   0x6ffffffd     // Version definition section. 
#define SHT_GNU_verneed  0x6ffffffe     // Version needs section. 
#define SHT_GNU_versym   0x6fffffff     // Version symbol table. 
#define SHT_HISUNW       0x6fffffff     // Sun-specific high bound. 
#define SHT_HIOS         0x6fffffff     // End OS-specific type
#define SHT_LOPROC       0x70000000     // Start of processor-specific
#define SHT_ARM_EXIDX    0x70000001     // Exception Index table
#define SHT_ARM_PREEMPTMAP 0x70000002   // dynamic linking pre-emption map
#define SHT_ARM_ATTRIBUTES 0x70000003   // Object file compatibility attrs
#define SHT_HIPROC       0x7fffffff     // End of processor-specific
#define SHT_LOUSER       0x80000000     // Start of application-specific
#define SHT_HIUSER       0x8fffffff     // End of application-specific

// Legal values for sh_flags (section flags)

#define SHF_WRITE       (1 << 0)        // Writable
#define SHF_ALLOC       (1 << 1)        // Occupies memory during execution
#define SHF_EXECINSTR   (1 << 2)        // Executable
#define SHF_MASKPROC    0xf0000000      // Processor-specific

// Symbol table entry. 

typedef struct {
  Elf32_Word    st_name;                // Symbol name (string tbl index)
  Elf32_Addr    st_value;               // Symbol value
  Elf32_Word    st_size;                // Symbol size
  unsigned char st_info;                // Symbol type and binding
  unsigned char st_other;               // No defined meaning, 0
  Elf32_Section st_shndx;               // Section index
} Elf32_Sym;

// The syminfo section if available contains additional information about
// every dynamic symbol. 

typedef struct {
  Elf32_Half si_boundto;                // Direct bindings, symbol bound to
  Elf32_Half si_flags;                  // Per symbol flags
} Elf32_Syminfo;

// Possible values for si_boundto
#define SYMINFO_BT_SELF         0xffff  // Symbol bound to self
#define SYMINFO_BT_PARENT       0xfffe  // Symbol bound to parent
#define SYMINFO_BT_LOWRESERVE   0xff00  // Beginning of reserved entries

// Possible bitmasks for si_flags
#define SYMINFO_FLG_DIRECT      0x0001  // Direct bound symbol
#define SYMINFO_FLG_PASSTHRU    0x0002  // Pass-thru symbol for translator
#define SYMINFO_FLG_COPY        0x0004  // Symbol is a copy-reloc
#define SYMINFO_FLG_LAZYLOAD    0x0008  // Symbol bound to object to be lazy
                                        // loaded
// Syminfo version values
#define SYMINFO_NONE            0
#define SYMINFO_CURRENT         1
#define SYMINFO_NUM             2

// Special section index

#define SHN_UNDEF       0               // No section, undefined symbol

// How to extract and insert information held in the st_info field

#define ELF32_ST_BIND(val)              (((unsigned char) (val)) >> 4)
#define ELF32_ST_TYPE(val)              ((val) & 0xf)
#define ELF32_ST_INFO(bind, type)       (((bind) << 4) + ((type) & 0xf))

// Legal values for ST_BIND subfield of st_info (symbol binding)

#define STB_LOCAL       0               // Local symbol
#define STB_GLOBAL      1               // Global symbol
#define STB_WEAK        2               // Weak symbol
#define STB_NUM         3               // Number of defined types. 
#define STB_LOOS        10              // Start of OS-specific
#define STB_HIOS        12              // End of OS-specific
#define STB_LOPROC      13              // Start of processor-specific
#define STB_HIPROC      15              // End of processor-specific

// Legal values for ST_TYPE subfield of st_info (symbol type)

#define STT_NOTYPE      0               // Symbol type is unspecified
#define STT_OBJECT      1               // Symbol is a data object
#define STT_FUNC        2               // Symbol is a code object
#define STT_SECTION     3               // Symbol associated with a section
#define STT_FILE        4               // Symbol's name is file name
#define STT_NUM         5               // Number of defined types. 
#define STT_LOOS        11              // Start of OS-specific
#define STT_HIOS        12              // End of OS-specific
#define STT_LOPROC      13              // Start of processor-specific
#define STT_HIPROC      15              // End of processor-specific


// Symbol table indices are found in the hash buckets and chain table
// of a symbol hash table section.  This special index value indicates
// the end of a chain, meaning no further symbols are found in that bucket. 

#define STN_UNDEF       0               // End of a chain

// How to extract and insert information held in the st_other field

#define ELF32_ST_VISIBILITY(o)  ((o) & 0x03)

// Symbol visibility specification encoded in the st_other field
#define STV_DEFAULT     0               // Default symbol visibility rules
#define STV_INTERNAL    1               // Processor specific hidden class
#define STV_HIDDEN      2               // Sym unavailable in other modules
#define STV_PROTECTED   3               // Not preemptible, not exported


// Relocation table entry without addend (in section of type SHT_REL)

typedef struct {
  Elf32_Addr    r_offset;               // Address
  Elf32_Word    r_info;                 // Relocation type and symbol index
} Elf32_Rel;

// Relocation table entry with addend (in section of type SHT_RELA)

typedef struct {
  Elf32_Addr    r_offset;               // Address
  Elf32_Word    r_info;                 // Relocation type and symbol index
  Elf32_Sword   r_addend;               // Addend
} Elf32_Rela;

// How to extract and insert information held in the r_info field

#define ELF32_R_SYM(val)                ((val) >> 8)
#define ELF32_R_TYPE(val)               ((val) & 0xff)
#define ELF32_R_INFO(sym, type)         (((sym) << 8) + ((type) & 0xff))

// Program segment header

typedef struct {
  Elf32_Word    p_type;                 // Segment type
  Elf32_Off     p_offset;               // Segment file offset
  Elf32_Addr    p_vaddr;                // Segment virtual address
  Elf32_Addr    p_paddr;                // Segment physical address
  Elf32_Word    p_filesz;               // Segment size in file
  Elf32_Word    p_memsz;                // Segment size in memory
  Elf32_Word    p_flags;                // Segment flags
  Elf32_Word    p_align;                // Segment alignment
} Elf32_Phdr;

// Legal values for p_type (segment type)

#define PT_NULL         0               // Program header table entry unused
#define PT_LOAD         1               // Loadable program segment
#define PT_DYNAMIC      2               // Dynamic linking information
#define PT_INTERP       3               // Program interpreter
#define PT_NOTE         4               // Auxiliary information
#define PT_SHLIB        5               // Reserved
#define PT_PHDR         6               // Entry for header table itself
#define PT_NUM          7               // Number of defined types. 
#define PT_LOOS         0x60000000      // Start of OS-specific
#define PT_HIOS         0x6fffffff      // End of OS-specific
#define PT_LOPROC       0x70000000      // Start of processor-specific
#define PT_HIPROC       0x7fffffff      // End of processor-specific

// Legal values for p_flags (segment flags)

#define PF_X            (1 << 0)        // Segment is executable
#define PF_W            (1 << 1)        // Segment is writable
#define PF_R            (1 << 2)        // Segment is readable
#define PF_MASKPROC     0xf0000000      // Processor-specific

// Legal values for note segment descriptor types for core files

#define NT_PRSTATUS     1               // Contains copy of prstatus struct
#define NT_FPREGSET     2               // Contains copy of fpregset struct
#define NT_PRPSINFO     3               // Contains copy of prpsinfo struct
#define NT_PRXREG       4               // Contains copy of prxregset struct
#define NT_PLATFORM     5               // String from sysinfo(SI_PLATFORM)
#define NT_AUXV         6               // Contains copy of auxv array
#define NT_GWINDOWS     7               // Contains copy of gwindows struct
#define NT_PSTATUS      10              // Contains copy of pstatus struct
#define NT_PSINFO       13              // Contains copy of psinfo struct
#define NT_PRCRED       14              // Contains copy of prcred struct
#define NT_UTSNAME      15              // Contains copy of utsname struct
#define NT_LWPSTATUS    16              // Contains copy of lwpstatus struct
#define NT_LWPSINFO     17              // Contains copy of lwpinfo struct

// Legal values for the  note segment descriptor types for object files

#define NT_VERSION      1               // Contains a version string. 

// Dynamic section entry

typedef struct {
  Elf32_Sword   d_tag;                  // Dynamic entry type
  union {
    Elf32_Word d_val;                   // Integer value
    Elf32_Addr d_ptr;                   // Address value
  } d_un;
} Elf32_Dyn;

// Legal values for d_tag (dynamic entry type)

#define DT_NULL         0               // Marks end of dynamic section
#define DT_NEEDED       1               // Name of needed library
#define DT_PLTRELSZ     2               // Size in bytes of PLT relocs
#define DT_PLTGOT       3               // Processor defined value
#define DT_HASH         4               // Address of symbol hash table
#define DT_STRTAB       5               // Address of string table
#define DT_SYMTAB       6               // Address of symbol table
#define DT_RELA         7               // Address of Rela relocs
#define DT_RELASZ       8               // Total size of Rela relocs
#define DT_RELAENT      9               // Size of one Rela reloc
#define DT_STRSZ        10              // Size of string table
#define DT_SYMENT       11              // Size of one symbol table entry
#define DT_INIT         12              // Address of init function
#define DT_FINI         13              // Address of termination function
#define DT_SONAME       14              // Name of shared object
#define DT_RPATH        15              // Library search path
#define DT_SYMBOLIC     16              // Start symbol search here
#define DT_REL          17              // Address of Rel relocs
#define DT_RELSZ        18              // Total size of Rel relocs
#define DT_RELENT       19              // Size of one Rel reloc
#define DT_PLTREL       20              // Type of reloc in PLT
#define DT_DEBUG        21              // For debugging; unspecified
#define DT_TEXTREL      22              // Reloc might modify .text
#define DT_JMPREL       23              // Address of PLT relocs
#define DT_BIND_NOW     24              // Process relocations of object
#define DT_INIT_ARRAY   25              // Array with addresses of init fct
#define DT_FINI_ARRAY   26              // Array with addresses of fini fct
#define DT_INIT_ARRAYSZ 27              // Size in bytes of DT_INIT_ARRAY
#define DT_FINI_ARRAYSZ 28              // Size in bytes of DT_FINI_ARRAY
#define DT_NUM          29              // Number used
#define DT_LOOS         0x60000000      // Start of OS-specific
#define DT_HIOS         0x6fffffff      // End of OS-specific
#define DT_LOPROC       0x70000000      // Start of processor-specific
#define DT_HIPROC       0x7fffffff      // End of processor-specific
#define DT_PROCNUM      DT_MIPS_NUM     // Most used by any processor

// DT_* entries which fall between DT_VALRNGHI & DT_VALRNGLO use the
// Dyn.d_un.d_val field of the Elf*_Dyn structure.  This follows Sun's
// approach. 
#define DT_VALRNGLO     0x6ffffd00
#define DT_POSFLAG_1    0x6ffffdfd      // Flags for DT_* entries, effecting
                                        // the following DT_* entry. 
#define DT_SYMINSZ      0x6ffffdfe      // Size of syminfo table (in bytes)
#define DT_SYMINENT     0x6ffffdff      // Entry size of syminfo
#define DT_VALRNGHI     0x6ffffdff

// DT_* entries which fall between DT_ADDRRNGHI & DT_ADDRRNGLO use the
// Dyn.d_un.d_ptr field of the Elf*_Dyn structure.
//
// If any adjustment is made to the ELF object after it has been
// built these entries will need to be adjusted. 
#define DT_ADDRRNGLO    0x6ffffe00
#define DT_SYMINFO      0x6ffffeff      // syminfo table
#define DT_ADDRRNGHI    0x6ffffeff

// The versioning entry types.  The next are defined as part of the
// GNU extension. 
#define DT_VERSYM       0x6ffffff0

// These were chosen by Sun
#define DT_FLAGS_1      0x6ffffffb      // State flags, see DF_1_* below. 
#define DT_VERDEF       0x6ffffffc      // Address of version definition
                                        // table
#define DT_VERDEFNUM    0x6ffffffd      // Number of version definitions
#define DT_VERNEED      0x6ffffffe      // Address of table with needed
                                        // versions
#define DT_VERNEEDNUM   0x6fffffff      // Number of needed versions
#define DT_VERSIONTAGIDX(tag)   (DT_VERNEEDNUM - (tag)) // Reverse order!
#define DT_VERSIONTAGNUM 16

// Sun added these machine-independent extensions in the "processor-specific"
// range.  Be compatible. 
#define DT_AUXILIARY    0x7ffffffd      // Shared object to load before self
#define DT_FILTER       0x7fffffff      // Shared object to get values from
#define DT_EXTRATAGIDX(tag)     ((Elf32_Word)-((Elf32_Sword) (tag) <<1>>1)-1)
#define DT_EXTRANUM     3

// State flags selectable in the `d_un.d_val' element of the DT_FLAGS_1
// entry in the dynamic section. 
#define DF_1_NOW        0x00000001      // Set RTLD_NOW for this object. 
#define DF_1_GLOBAL     0x00000002      // Set RTLD_GLOBAL for this object. 
#define DF_1_GROUP      0x00000004      // Set RTLD_GROUP for this object. 
#define DF_1_NODELETE   0x00000008      // Set RTLD_NODELETE for this object.*/
#define DF_1_LOADFLTR   0x00000010      // Trigger filtee loading at runtime.*/
#define DF_1_INITFIRST  0x00000020      // Set RTLD_INITFIRST for this object*/
#define DF_1_NOOPEN     0x00000040      // Set RTLD_NOOPEN for this object. 

// Version definition sections

typedef struct {
  Elf32_Half    vd_version;             // Version revision
  Elf32_Half    vd_flags;               // Version information
  Elf32_Half    vd_ndx;                 // Version Index
  Elf32_Half    vd_cnt;                 // Number of associated aux entries
  Elf32_Word    vd_hash;                // Version name hash value
  Elf32_Word    vd_aux;                 // Offset in bytes to verdaux array
  Elf32_Word    vd_next;                // Offset in bytes to next verdef
                                        // entry
} Elf32_Verdef;

// Legal values for vd_version (version revision)
#define VER_DEF_NONE    0               // No version
#define VER_DEF_CURRENT 1               // Current version
#define VER_DEF_NUM     2               // Given version number

// Legal values for vd_flags (version information flags)
#define VER_FLG_BASE    0x1             // Version definition of file itself
#define VER_FLG_WEAK    0x2             // Weak version identifier

// Auxialiary version information. 

typedef struct {
  Elf32_Word    vda_name;               // Version or dependency names
  Elf32_Word    vda_next;               // Offset in bytes to next verdaux
                                        // entry
} Elf32_Verdaux;

// Version dependency section

typedef struct {
  Elf32_Half    vn_version;             // Version of structure
  Elf32_Half    vn_cnt;                 // Number of associated aux entries
  Elf32_Word    vn_file;                // Offset of filename for this
                                        // dependency
  Elf32_Word    vn_aux;                 // Offset in bytes to vernaux array
  Elf32_Word    vn_next;                // Offset in bytes to next verneed
                                        // entry
} Elf32_Verneed;

// Legal values for vn_version (version revision)
#define VER_NEED_NONE    0              // No version
#define VER_NEED_CURRENT 1              // Current version
#define VER_NEED_NUM     2              // Given version number

// Auxiliary needed version information

typedef struct {
  Elf32_Word    vna_hash;               // Hash value of dependency name
  Elf32_Half    vna_flags;              // Dependency specific information
  Elf32_Half    vna_other;              // Unused
  Elf32_Word    vna_name;               // Dependency name string offset
  Elf32_Word    vna_next;               // Offset in bytes to next vernaux
                                        // entry
} Elf32_Vernaux;

// Legal values for vna_flags
#define VER_FLG_WEAK    0x2             // Weak version identifier

// Auxiliary vector

// This vector is normally only used by the program interpreter.  The
// usual definition in an ABI supplement uses the name auxv_t.  The
// vector is not usually defined in a standard <elf.h> file, but it
// can't hurt.  We rename it to avoid conflicts.  The sizes of these
// types are an arrangement between the exec server and the program
// interpreter, so we don't fully specify them here. 

typedef struct {
  int a_type;                   // Entry type
  union {
    long int a_val;             // Integer value
    void *a_ptr;                // Pointer value
    void (*a_fcn) (void);       // Function pointer value
  } a_un;
} Elf32_auxv_t;

// Legal values for a_type (entry type)

#define AT_NULL         0               // End of vector
#define AT_IGNORE       1               // Entry should be ignored
#define AT_EXECFD       2               // File descriptor of program
#define AT_PHDR         3               // Program headers for program
#define AT_PHENT        4               // Size of program header entry
#define AT_PHNUM        5               // Number of program headers
#define AT_PAGESZ       6               // System page size
#define AT_BASE         7               // Base address of interpreter
#define AT_FLAGS        8               // Flags
#define AT_ENTRY        9               // Entry point of program
#define AT_NOTELF       10              // Program is not ELF
#define AT_UID          11              // Real uid
#define AT_EUID         12              // Effective uid
#define AT_GID          13              // Real gid
#define AT_EGID         14              // Effective gid

// Some more special a_type values describing the hardware
#define AT_PLATFORM     15              // String identifying platform
#define AT_HWCAP        16              // Machine dependent hints about
                                        // processor capabilities

// This entry gives some information about the FPU initialization
// performed by the kernel
#define AT_FPUCW        17              // Used FPU control word


// Note section contents.  Each entry in the note section begins with
// a header of a fixed form. 

typedef struct {
  Elf32_Word n_namesz;                  // Length of the note's name 
  Elf32_Word n_descsz;                  // Length of the note's descriptor
  Elf32_Word n_type;                    // Type of the note
} Elf32_Nhdr;

// Known names of notes

// Solaris entries in the note section have this name
#define ELF_NOTE_SOLARIS        "SUNW Solaris"

// Note entries for GNU systems have this name
#define ELF_NOTE_GNU            "GNU"

// Defined types of notes for Solaris

// Value of descriptor (one word) is desired pagesize for the binary
#define ELF_NOTE_PAGESIZE_HINT  1


// Defined note types for GNU systems

// ABI information.  The descriptor consists of words:
// word 0: OS descriptor
// word 1: major version of the ABI
// word 2: minor version of the ABI
// word 3: subminor version of the ABI

#define ELF_NOTE_ABI            1

// Known OSes.  These value can appear in word 0 of an ELF_NOTE_ABI
// note section entry.
#define ELF_NOTE_OS_LINUX       0
#define ELF_NOTE_OS_GNU         1
#define ELF_NOTE_OS_SOLARIS2    2

// Intel 80386 specific definitions

// i386 relocs

#define R_386_NONE      0               // No reloc
#define R_386_32        1               // Direct 32 bit 
#define R_386_PC32      2               // PC relative 32 bit
#define R_386_GOT32     3               // 32 bit GOT entry
#define R_386_PLT32     4               // 32 bit PLT address
#define R_386_COPY      5               // Copy symbol at runtime
#define R_386_GLOB_DAT  6               // Create GOT entry
#define R_386_JMP_SLOT  7               // Create PLT entry
#define R_386_RELATIVE  8               // Adjust by program base
#define R_386_GOTOFF    9               // 32 bit offset to GOT
#define R_386_GOTPC     10              // 32 bit PC relative offset to GOT
// Keep this the last entry
#define R_386_NUM       11

#endif
