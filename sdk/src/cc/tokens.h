//
//  opcodes.h - Tiny C Compiler for Sanos
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

//
// Keywords
//

DEF(TOK_INT, "int")
DEF(TOK_VOID, "void")
DEF(TOK_CHAR, "char")
DEF(TOK_IF, "if")
DEF(TOK_ELSE, "else")
DEF(TOK_WHILE, "while")
DEF(TOK_BREAK, "break")
DEF(TOK_RETURN, "return")
DEF(TOK_FOR, "for")
DEF(TOK_EXTERN, "extern")
DEF(TOK_STATIC, "static")
DEF(TOK_UNSIGNED, "unsigned")
DEF(TOK_GOTO, "goto")
DEF(TOK_DO, "do")
DEF(TOK_CONTINUE, "continue")
DEF(TOK_SWITCH, "switch")
DEF(TOK_CASE, "case")

DEF(TOK_CONST1, "const")
DEF(TOK_CONST2, "__const") // gcc keyword
DEF(TOK_CONST3, "__const__") // gcc keyword
DEF(TOK_VOLATILE1, "volatile")
DEF(TOK_VOLATILE2, "__volatile") // gcc keyword
DEF(TOK_VOLATILE3, "__volatile__") // gcc keyword
DEF(TOK_LONG, "long")
DEF(TOK_REGISTER, "register")
DEF(TOK_SIGNED1, "signed")
DEF(TOK_SIGNED2, "__signed") // gcc keyword
DEF(TOK_SIGNED3, "__signed__") // gcc keyword
DEF(TOK_AUTO, "auto")
DEF(TOK_INLINE1, "inline")
DEF(TOK_INLINE2, "__inline") // gcc keyword
DEF(TOK_INLINE3, "__inline__") // gcc keyword
DEF(TOK_RESTRICT1, "restrict")
DEF(TOK_RESTRICT2, "__restrict")
DEF(TOK_RESTRICT3, "__restrict__")
DEF(TOK_EXTENSION, "__extension__") // gcc keyword

DEF(TOK_FLOAT, "float")
DEF(TOK_DOUBLE, "double")
DEF(TOK_BOOL, "_Bool")
DEF(TOK_SHORT, "short")
DEF(TOK_STRUCT, "struct")
DEF(TOK_UNION, "union")
DEF(TOK_TYPEDEF, "typedef")
DEF(TOK_DEFAULT, "default")
DEF(TOK_ENUM, "enum")
DEF(TOK_SIZEOF, "sizeof")
DEF(TOK_ATTRIBUTE1, "__attribute")
DEF(TOK_ATTRIBUTE2, "__attribute__")
DEF(TOK_ALIGNOF1, "__alignof")
DEF(TOK_ALIGNOF2, "__alignof__")
DEF(TOK_TYPEOF1, "typeof")
DEF(TOK_TYPEOF2, "__typeof")
DEF(TOK_TYPEOF3, "__typeof__")
DEF(TOK_LABEL, "__label__")
DEF(TOK_ASM1, "asm")
DEF(TOK_ASM2, "__asm")
DEF(TOK_ASM3, "__asm__")

DEF(TOK_INT64, "__int64")
DEF(TOK_DECLSPEC, "__declspec")
DEF(TOK_DLLIMPORT, "dllimport")
DEF(TOK_NAKED, "naked")

//
// The following are not keywords. They are included to ease parsing
// preprocessor only
//

DEF(TOK_DEFINE, "define")
DEF(TOK_INCLUDE, "include")
DEF(TOK_INCLUDE_NEXT, "include_next")
DEF(TOK_IFDEF, "ifdef")
DEF(TOK_IFNDEF, "ifndef")
DEF(TOK_ELIF, "elif")
DEF(TOK_ENDIF, "endif")
DEF(TOK_DEFINED, "defined")
DEF(TOK_UNDEF, "undef")
DEF(TOK_ERROR, "error")
DEF(TOK_WARNING, "warning")
DEF(TOK_LINE, "line")
DEF(TOK_PRAGMA, "pragma")
DEF(TOK___LINE__, "__LINE__")
DEF(TOK___FILE__, "__FILE__")
DEF(TOK___DATE__, "__DATE__")
DEF(TOK___TIME__, "__TIME__")
DEF(TOK___FUNCTION__, "__FUNCTION__")
DEF(TOK___VA_ARGS__, "__VA_ARGS__")

//
// Special identifiers
//

DEF(TOK___FUNC__, "__func__")

//
// Attribute identifiers
// TODO: Handle all tokens generically since speed is not critical
//

DEF(TOK_SECTION1, "section")
DEF(TOK_SECTION2, "__section__")
DEF(TOK_ALIGNED1, "aligned")
DEF(TOK_ALIGNED2, "__aligned__")
DEF(TOK_PACKED1, "packed")
DEF(TOK_PACKED2, "__packed__")
DEF(TOK_UNUSED1, "unused")
DEF(TOK_UNUSED2, "__unused__")
DEF(TOK_CDECL1, "cdecl")
DEF(TOK_CDECL2, "__cdecl")
DEF(TOK_CDECL3, "__cdecl__")
DEF(TOK_STDCALL1, "stdcall")
DEF(TOK_STDCALL2, "__stdcall")
DEF(TOK_STDCALL3, "__stdcall__")
DEF(TOK_FASTCALL1, "fastcall")
DEF(TOK_FASTCALL2, "__fastcall")
DEF(TOK_FASTCALL3, "__fastcall__")
DEF(TOK_DLLEXPORT, "dllexport")
DEF(TOK_NORETURN1, "noreturn")
DEF(TOK_NORETURN2, "__noreturn__")
DEF(TOK_builtin_types_compatible_p, "__builtin_types_compatible_p")
DEF(TOK_builtin_constant_p, "__builtin_constant_p")
DEF(TOK_REGPARM1, "regparm")
DEF(TOK_REGPARM2, "__regparm__")

//
// Pragma
//

DEF(TOK_pack, "pack")

//
// Builtin functions or variables
//

DEF(TOK_memcpy, "memcpy")
DEF(TOK_memset, "memset")
DEF(TOK___divdi3, "__divdi3")
DEF(TOK___moddi3, "__moddi3")
DEF(TOK___udivdi3, "__udivdi3")
DEF(TOK___umoddi3, "__umoddi3")
DEF(TOK___sardi3, "__sardi3")
DEF(TOK___shrdi3, "__shrdi3")
DEF(TOK___shldi3, "__shldi3")
DEF(TOK___tcc_int_fpu_control, "__tcc_int_fpu_control")
DEF(TOK___tcc_fpu_control, "__tcc_fpu_control")
DEF(TOK___ulltof, "__ulltof")
DEF(TOK___ulltod, "__ulltod")
DEF(TOK___ulltold, "__ulltold")
DEF(TOK___fixunssfdi, "__fixunssfdi")
DEF(TOK___fixunsdfdi, "__fixunsdfdi")
DEF(TOK___fixunsxfdi, "__fixunsxfdi")
DEF(TOK___chkstk, "__chkstk")

//
// Tiny Assembler
//

#define DEF_ASM(x) DEF(TOK_ASM_ ## x, #x)

#define DEF_BWL(x) \
 DEF(TOK_ASM_ ## x ## b, #x "b") \
 DEF(TOK_ASM_ ## x ## w, #x "w") \
 DEF(TOK_ASM_ ## x ## l, #x "l") \
 DEF(TOK_ASM_ ## x, #x)

#define DEF_WL(x) \
 DEF(TOK_ASM_ ## x ## w, #x "w") \
 DEF(TOK_ASM_ ## x ## l, #x "l") \
 DEF(TOK_ASM_ ## x, #x)

#define DEF_FP1(x) \
 DEF(TOK_ASM_ ## f ## x ## s, "f" #x "s") \
 DEF(TOK_ASM_ ## fi ## x ## l, "fi" #x "l") \
 DEF(TOK_ASM_ ## f ## x ## l, "f" #x "l") \
 DEF(TOK_ASM_ ## fi ## x ## s, "fi" #x "s")

#define DEF_FP(x) \
 DEF(TOK_ASM_ ## f ## x, "f" #x ) \
 DEF(TOK_ASM_ ## f ## x ## p, "f" #x "p") \
 DEF_FP1(x)

#define DEF_ASMTEST(x) \
 DEF_ASM(x ## o) \
 DEF_ASM(x ## no) \
 DEF_ASM(x ## b) \
 DEF_ASM(x ## c) \
 DEF_ASM(x ## nae) \
 DEF_ASM(x ## nb) \
 DEF_ASM(x ## nc) \
 DEF_ASM(x ## ae) \
 DEF_ASM(x ## e) \
 DEF_ASM(x ## z) \
 DEF_ASM(x ## ne) \
 DEF_ASM(x ## nz) \
 DEF_ASM(x ## be) \
 DEF_ASM(x ## na) \
 DEF_ASM(x ## nbe) \
 DEF_ASM(x ## a) \
 DEF_ASM(x ## s) \
 DEF_ASM(x ## ns) \
 DEF_ASM(x ## p) \
 DEF_ASM(x ## pe) \
 DEF_ASM(x ## np) \
 DEF_ASM(x ## po) \
 DEF_ASM(x ## l) \
 DEF_ASM(x ## nge) \
 DEF_ASM(x ## nl) \
 DEF_ASM(x ## ge) \
 DEF_ASM(x ## le) \
 DEF_ASM(x ## ng) \
 DEF_ASM(x ## nle) \
 DEF_ASM(x ## g)

DEF_ASM(byte)
DEF_ASM(align)
DEF_ASM(skip)
DEF_ASM(space)
DEF_ASM(string)
DEF_ASM(asciz)
DEF_ASM(ascii)
DEF_ASM(globl)
DEF_ASM(global)
DEF_ASM(text)
DEF_ASM(data)
DEF_ASM(bss)
DEF_ASM(previous)
DEF_ASM(fill)
DEF_ASM(quad)

DEF_ASM(_emit)
DEF_ASM(offset)
DEF_ASM(dword)
DEF_ASM(fword)
DEF_ASM(far)
DEF_ASM(ptr)

// WARNING: relative order of tokens is important.
DEF_ASM(al)
DEF_ASM(cl)
DEF_ASM(dl)
DEF_ASM(bl)
DEF_ASM(ah)
DEF_ASM(ch)
DEF_ASM(dh)
DEF_ASM(bh)
DEF_ASM(ax)
DEF_ASM(cx)
DEF_ASM(dx)
DEF_ASM(bx)
DEF_ASM(sp)
DEF_ASM(bp)
DEF_ASM(si)
DEF_ASM(di)
DEF_ASM(eax)
DEF_ASM(ecx)
DEF_ASM(edx)
DEF_ASM(ebx)
DEF_ASM(esp)
DEF_ASM(ebp)
DEF_ASM(esi)
DEF_ASM(edi)
DEF_ASM(mm0)
DEF_ASM(mm1)
DEF_ASM(mm2)
DEF_ASM(mm3)
DEF_ASM(mm4)
DEF_ASM(mm5)
DEF_ASM(mm6)
DEF_ASM(mm7)
DEF_ASM(xmm0)
DEF_ASM(xmm1)
DEF_ASM(xmm2)
DEF_ASM(xmm3)
DEF_ASM(xmm4)
DEF_ASM(xmm5)
DEF_ASM(xmm6)
DEF_ASM(xmm7)
DEF_ASM(cr0)
DEF_ASM(cr1)
DEF_ASM(cr2)
DEF_ASM(cr3)
DEF_ASM(cr4)
DEF_ASM(cr5)
DEF_ASM(cr6)
DEF_ASM(cr7)
DEF_ASM(tr0)
DEF_ASM(tr1)
DEF_ASM(tr2)
DEF_ASM(tr3)
DEF_ASM(tr4)
DEF_ASM(tr5)
DEF_ASM(tr6)
DEF_ASM(tr7)
DEF_ASM(db0)
DEF_ASM(db1)
DEF_ASM(db2)
DEF_ASM(db3)
DEF_ASM(db4)
DEF_ASM(db5)
DEF_ASM(db6)
DEF_ASM(db7)
DEF_ASM(dr0)
DEF_ASM(dr1)
DEF_ASM(dr2)
DEF_ASM(dr3)
DEF_ASM(dr4)
DEF_ASM(dr5)
DEF_ASM(dr6)
DEF_ASM(dr7)
DEF_ASM(es)
DEF_ASM(cs)
DEF_ASM(ss)
DEF_ASM(ds)
DEF_ASM(fs)
DEF_ASM(gs)
DEF_ASM(st)

DEF_BWL(mov)

//
// Generic two operands
//

DEF_BWL(add)
DEF_BWL(or)
DEF_BWL(adc)
DEF_BWL(sbb)
DEF_BWL(and)
DEF_BWL(sub)
DEF_BWL(xor)
DEF_BWL(cmp)

//
// Unary ops
//

DEF_BWL(inc)
DEF_BWL(dec)
DEF_BWL(not)
DEF_BWL(neg)
DEF_BWL(mul)
DEF_BWL(imul)
DEF_BWL(div)
DEF_BWL(idiv)

DEF_BWL(xchg)
DEF_BWL(test)

//
// Shifts
//

DEF_BWL(rol)
DEF_BWL(ror)
DEF_BWL(rcl)
DEF_BWL(rcr)
DEF_BWL(shl)
DEF_BWL(shr)
DEF_BWL(sar)

DEF_ASM(shldw)
DEF_ASM(shldl)
DEF_ASM(shld)
DEF_ASM(shrdw)
DEF_ASM(shrdl)
DEF_ASM(shrd)

DEF_ASM(pushw)
DEF_ASM(pushl)
DEF_ASM(push)
DEF_ASM(popw)
DEF_ASM(popl)
DEF_ASM(pop)
DEF_BWL(in)
DEF_BWL(out)

DEF_WL(movzb)

DEF_ASM(movzwl)
DEF_ASM(movsbw)
DEF_ASM(movsbl)
DEF_ASM(movswl)
DEF_ASM(movsd)

DEF_WL(lea) 

DEF_ASM(les) 
DEF_ASM(lds) 
DEF_ASM(lss) 
DEF_ASM(lfs) 
DEF_ASM(lgs) 

DEF_ASM(call)
DEF_ASM(jmp)
DEF_ASM(lcall)
DEF_ASM(ljmp)
 
DEF_ASMTEST(j)

DEF_ASMTEST(set)
DEF_ASMTEST(cmov)

DEF_WL(bsf)
DEF_WL(bsr)
DEF_WL(bt)
DEF_WL(bts)
DEF_WL(btr)
DEF_WL(btc)

DEF_WL(lsl)

//
// Generic FP ops
//

DEF_FP(add)
DEF_FP(mul)

DEF_ASM(fcom)
DEF_ASM(fcom_1) // Non existant op, just to have a regular table
DEF_FP1(com)

DEF_FP(comp)
DEF_FP(sub)
DEF_FP(subr)
DEF_FP(div)
DEF_FP(divr)

DEF_BWL(xadd)
DEF_BWL(cmpxchg)

//
// String ops
//

DEF_BWL(cmps)
DEF_BWL(scmp)
DEF_BWL(ins)
DEF_BWL(outs)
DEF_BWL(lods)
DEF_BWL(slod)
DEF_BWL(movs)
DEF_BWL(smov)
DEF_BWL(scas)
DEF_BWL(ssca)
DEF_BWL(stos)
DEF_BWL(ssto)

//
// Generic asm ops
//

#define ALT(x)
#define DEF_ASM_OP0(name, opcode) DEF_ASM(name)
#define DEF_ASM_OP0L(name, opcode, group, instr_type)
#define DEF_ASM_OP1(name, opcode, group, instr_type, op0)
#define DEF_ASM_OP2(name, opcode, group, instr_type, op0, op1)
#define DEF_ASM_OP3(name, opcode, group, instr_type, op0, op1, op2)
#include "opcodes.h"

#define ALT(x)
#define DEF_ASM_OP0(name, opcode)
#define DEF_ASM_OP0L(name, opcode, group, instr_type) DEF_ASM(name)
#define DEF_ASM_OP1(name, opcode, group, instr_type, op0) DEF_ASM(name)
#define DEF_ASM_OP2(name, opcode, group, instr_type, op0, op1) DEF_ASM(name)
#define DEF_ASM_OP3(name, opcode, group, instr_type, op0, op1, op2) DEF_ASM(name)
#include "opcodes.h"

