/* This file auto-generated from insns.dat by insns.pl - don't edit it */

#include "nasm.h"
#include "insns.h"

static const struct itemplate instrux_AAA[] = {
    {I_AAA, 0, {0,0,0,0,0}, nasm_bytecodes+19680, IF_8086|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_AAD[] = {
    {I_AAD, 0, {0,0,0,0,0}, nasm_bytecodes+18632, IF_8086|IF_NOLONG},
    {I_AAD, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+18636, IF_8086|IF_SB|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_AAM[] = {
    {I_AAM, 0, {0,0,0,0,0}, nasm_bytecodes+18640, IF_8086|IF_NOLONG},
    {I_AAM, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+18644, IF_8086|IF_SB|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_AAS[] = {
    {I_AAS, 0, {0,0,0,0,0}, nasm_bytecodes+19683, IF_8086|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_ADC[] = {
    {I_ADC, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+18648, IF_8086|IF_SM},
    {I_ADC, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+18648, IF_8086},
    {I_ADC, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+16772, IF_8086|IF_SM},
    {I_ADC, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+16772, IF_8086},
    {I_ADC, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+16777, IF_386|IF_SM},
    {I_ADC, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+16777, IF_386},
    {I_ADC, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+16782, IF_X64|IF_SM},
    {I_ADC, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+16782, IF_X64},
    {I_ADC, 2, {REG_GPR|BITS8,MEMORY,0,0,0}, nasm_bytecodes+9884, IF_8086|IF_SM},
    {I_ADC, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+9884, IF_8086},
    {I_ADC, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+16787, IF_8086|IF_SM},
    {I_ADC, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+16787, IF_8086},
    {I_ADC, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+16792, IF_386|IF_SM},
    {I_ADC, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+16792, IF_386},
    {I_ADC, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+16797, IF_X64|IF_SM},
    {I_ADC, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+16797, IF_X64},
    {I_ADC, 2, {RM_GPR|BITS16,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13220, IF_8086},
    {I_ADC, 2, {RM_GPR|BITS32,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13226, IF_386},
    {I_ADC, 2, {RM_GPR|BITS64,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13232, IF_X64},
    {I_ADC, 2, {REG_AL,IMMEDIATE,0,0,0}, nasm_bytecodes+18652, IF_8086|IF_SM},
    {I_ADC, 2, {REG_AX,SBYTE16,0,0,0}, nasm_bytecodes+13220, IF_8086|IF_SM},
    {I_ADC, 2, {REG_AX,IMMEDIATE,0,0,0}, nasm_bytecodes+16802, IF_8086|IF_SM},
    {I_ADC, 2, {REG_EAX,SBYTE32,0,0,0}, nasm_bytecodes+13226, IF_386|IF_SM},
    {I_ADC, 2, {REG_EAX,IMMEDIATE,0,0,0}, nasm_bytecodes+16807, IF_386|IF_SM},
    {I_ADC, 2, {REG_RAX,SBYTE64,0,0,0}, nasm_bytecodes+13232, IF_X64|IF_SM},
    {I_ADC, 2, {REG_RAX,IMMEDIATE,0,0,0}, nasm_bytecodes+16812, IF_X64|IF_SM},
    {I_ADC, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+16817, IF_8086|IF_SM},
    {I_ADC, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+13238, IF_8086|IF_SM},
    {I_ADC, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+13244, IF_386|IF_SM},
    {I_ADC, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+13250, IF_X64|IF_SM},
    {I_ADC, 2, {MEMORY,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+16817, IF_8086|IF_SM},
    {I_ADC, 2, {MEMORY,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+13238, IF_8086|IF_SM},
    {I_ADC, 2, {MEMORY,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+13244, IF_386|IF_SM},
    ITEMPLATE_END
};

static const struct itemplate instrux_ADD[] = {
    {I_ADD, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+18656, IF_8086|IF_SM},
    {I_ADD, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+18656, IF_8086},
    {I_ADD, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+16822, IF_8086|IF_SM},
    {I_ADD, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+16822, IF_8086},
    {I_ADD, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+16827, IF_386|IF_SM},
    {I_ADD, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+16827, IF_386},
    {I_ADD, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+16832, IF_X64|IF_SM},
    {I_ADD, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+16832, IF_X64},
    {I_ADD, 2, {REG_GPR|BITS8,MEMORY,0,0,0}, nasm_bytecodes+10535, IF_8086|IF_SM},
    {I_ADD, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+10535, IF_8086},
    {I_ADD, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+16837, IF_8086|IF_SM},
    {I_ADD, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+16837, IF_8086},
    {I_ADD, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+16842, IF_386|IF_SM},
    {I_ADD, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+16842, IF_386},
    {I_ADD, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+16847, IF_X64|IF_SM},
    {I_ADD, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+16847, IF_X64},
    {I_ADD, 2, {RM_GPR|BITS16,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13256, IF_8086},
    {I_ADD, 2, {RM_GPR|BITS32,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13262, IF_386},
    {I_ADD, 2, {RM_GPR|BITS64,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13268, IF_X64},
    {I_ADD, 2, {REG_AL,IMMEDIATE,0,0,0}, nasm_bytecodes+18660, IF_8086|IF_SM},
    {I_ADD, 2, {REG_AX,SBYTE16,0,0,0}, nasm_bytecodes+13256, IF_8086|IF_SM},
    {I_ADD, 2, {REG_AX,IMMEDIATE,0,0,0}, nasm_bytecodes+16852, IF_8086|IF_SM},
    {I_ADD, 2, {REG_EAX,SBYTE32,0,0,0}, nasm_bytecodes+13262, IF_386|IF_SM},
    {I_ADD, 2, {REG_EAX,IMMEDIATE,0,0,0}, nasm_bytecodes+16857, IF_386|IF_SM},
    {I_ADD, 2, {REG_RAX,SBYTE64,0,0,0}, nasm_bytecodes+13268, IF_X64|IF_SM},
    {I_ADD, 2, {REG_RAX,IMMEDIATE,0,0,0}, nasm_bytecodes+16862, IF_X64|IF_SM},
    {I_ADD, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+16867, IF_8086|IF_SM},
    {I_ADD, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+13274, IF_8086|IF_SM},
    {I_ADD, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+13280, IF_386|IF_SM},
    {I_ADD, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+13286, IF_X64|IF_SM},
    {I_ADD, 2, {MEMORY,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+16867, IF_8086|IF_SM},
    {I_ADD, 2, {MEMORY,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+13274, IF_8086|IF_SM},
    {I_ADD, 2, {MEMORY,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+13280, IF_386|IF_SM},
    ITEMPLATE_END
};

static const struct itemplate instrux_ADDPD[] = {
    {I_ADDPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15188, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_ADDPS[] = {
    {I_ADDPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14468, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_ADDSD[] = {
    {I_ADDSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15194, IF_WILLAMETTE|IF_SSE2|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_ADDSS[] = {
    {I_ADDSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14474, IF_KATMAI|IF_SSE|IF_SD},
    ITEMPLATE_END
};

static const struct itemplate instrux_ADDSUBPD[] = {
    {I_ADDSUBPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15464, IF_PRESCOTT|IF_SSE3|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_ADDSUBPS[] = {
    {I_ADDSUBPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15470, IF_PRESCOTT|IF_SSE3|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_AESDEC[] = {
    {I_AESDEC, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8411, IF_SSE|IF_WESTMERE},
    ITEMPLATE_END
};

static const struct itemplate instrux_AESDECLAST[] = {
    {I_AESDECLAST, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8418, IF_SSE|IF_WESTMERE},
    ITEMPLATE_END
};

static const struct itemplate instrux_AESENC[] = {
    {I_AESENC, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8397, IF_SSE|IF_WESTMERE},
    ITEMPLATE_END
};

static const struct itemplate instrux_AESENCLAST[] = {
    {I_AESENCLAST, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8404, IF_SSE|IF_WESTMERE},
    ITEMPLATE_END
};

static const struct itemplate instrux_AESIMC[] = {
    {I_AESIMC, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8425, IF_SSE|IF_WESTMERE},
    ITEMPLATE_END
};

static const struct itemplate instrux_AESKEYGENASSIST[] = {
    {I_AESKEYGENASSIST, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+5856, IF_SSE|IF_WESTMERE},
    ITEMPLATE_END
};

static const struct itemplate instrux_AND[] = {
    {I_AND, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+18664, IF_8086|IF_SM},
    {I_AND, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+18664, IF_8086},
    {I_AND, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+16872, IF_8086|IF_SM},
    {I_AND, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+16872, IF_8086},
    {I_AND, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+16877, IF_386|IF_SM},
    {I_AND, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+16877, IF_386},
    {I_AND, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+16882, IF_X64|IF_SM},
    {I_AND, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+16882, IF_X64},
    {I_AND, 2, {REG_GPR|BITS8,MEMORY,0,0,0}, nasm_bytecodes+10822, IF_8086|IF_SM},
    {I_AND, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+10822, IF_8086},
    {I_AND, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+16887, IF_8086|IF_SM},
    {I_AND, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+16887, IF_8086},
    {I_AND, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+16892, IF_386|IF_SM},
    {I_AND, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+16892, IF_386},
    {I_AND, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+16897, IF_X64|IF_SM},
    {I_AND, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+16897, IF_X64},
    {I_AND, 2, {RM_GPR|BITS16,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13292, IF_8086},
    {I_AND, 2, {RM_GPR|BITS32,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13298, IF_386},
    {I_AND, 2, {RM_GPR|BITS64,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13304, IF_X64},
    {I_AND, 2, {REG_AL,IMMEDIATE,0,0,0}, nasm_bytecodes+18668, IF_8086|IF_SM},
    {I_AND, 2, {REG_AX,SBYTE16,0,0,0}, nasm_bytecodes+13292, IF_8086|IF_SM},
    {I_AND, 2, {REG_AX,IMMEDIATE,0,0,0}, nasm_bytecodes+16902, IF_8086|IF_SM},
    {I_AND, 2, {REG_EAX,SBYTE32,0,0,0}, nasm_bytecodes+13298, IF_386|IF_SM},
    {I_AND, 2, {REG_EAX,IMMEDIATE,0,0,0}, nasm_bytecodes+16907, IF_386|IF_SM},
    {I_AND, 2, {REG_RAX,SBYTE64,0,0,0}, nasm_bytecodes+13304, IF_X64|IF_SM},
    {I_AND, 2, {REG_RAX,IMMEDIATE,0,0,0}, nasm_bytecodes+16912, IF_X64|IF_SM},
    {I_AND, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+16917, IF_8086|IF_SM},
    {I_AND, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+13310, IF_8086|IF_SM},
    {I_AND, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+13316, IF_386|IF_SM},
    {I_AND, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+13322, IF_X64|IF_SM},
    {I_AND, 2, {MEMORY,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+16917, IF_8086|IF_SM},
    {I_AND, 2, {MEMORY,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+13310, IF_8086|IF_SM},
    {I_AND, 2, {MEMORY,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+13316, IF_386|IF_SM},
    ITEMPLATE_END
};

static const struct itemplate instrux_ANDNPD[] = {
    {I_ANDNPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15200, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_ANDNPS[] = {
    {I_ANDNPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14480, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_ANDPD[] = {
    {I_ANDPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15206, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_ANDPS[] = {
    {I_ANDPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14486, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_ARPL[] = {
    {I_ARPL, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18672, IF_286|IF_PROT|IF_SM|IF_NOLONG},
    {I_ARPL, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18672, IF_286|IF_PROT|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_BB0_RESET[] = {
    {I_BB0_RESET, 0, {0,0,0,0,0}, nasm_bytecodes+18676, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_BB1_RESET[] = {
    {I_BB1_RESET, 0, {0,0,0,0,0}, nasm_bytecodes+18680, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_BLENDPD[] = {
    {I_BLENDPD, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5688, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_BLENDPS[] = {
    {I_BLENDPS, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5696, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_BLENDVPD[] = {
    {I_BLENDVPD, 3, {XMMREG,RM_XMM,XMM0,0,0}, nasm_bytecodes+8117, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_BLENDVPS[] = {
    {I_BLENDVPS, 3, {XMMREG,RM_XMM,XMM0,0,0}, nasm_bytecodes+8124, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_BOUND[] = {
    {I_BOUND, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+16922, IF_186|IF_NOLONG},
    {I_BOUND, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+16927, IF_386|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_BSF[] = {
    {I_BSF, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+13328, IF_386|IF_SM},
    {I_BSF, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13328, IF_386},
    {I_BSF, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+13334, IF_386|IF_SM},
    {I_BSF, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13334, IF_386},
    {I_BSF, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+13340, IF_X64|IF_SM},
    {I_BSF, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13340, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_BSR[] = {
    {I_BSR, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+13346, IF_386|IF_SM},
    {I_BSR, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13346, IF_386},
    {I_BSR, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+13352, IF_386|IF_SM},
    {I_BSR, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13352, IF_386},
    {I_BSR, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+13358, IF_X64|IF_SM},
    {I_BSR, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13358, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_BSWAP[] = {
    {I_BSWAP, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+13364, IF_486},
    {I_BSWAP, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+13370, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_BT[] = {
    {I_BT, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13376, IF_386|IF_SM},
    {I_BT, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13376, IF_386},
    {I_BT, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13382, IF_386|IF_SM},
    {I_BT, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13382, IF_386},
    {I_BT, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13388, IF_X64|IF_SM},
    {I_BT, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13388, IF_X64},
    {I_BT, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+6920, IF_386|IF_SB},
    {I_BT, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+6927, IF_386|IF_SB},
    {I_BT, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+6934, IF_X64|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_BTC[] = {
    {I_BTC, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13394, IF_386|IF_SM},
    {I_BTC, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13394, IF_386},
    {I_BTC, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13400, IF_386|IF_SM},
    {I_BTC, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13400, IF_386},
    {I_BTC, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13406, IF_X64|IF_SM},
    {I_BTC, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13406, IF_X64},
    {I_BTC, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+6941, IF_386|IF_SB},
    {I_BTC, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+6948, IF_386|IF_SB},
    {I_BTC, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+6955, IF_X64|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_BTR[] = {
    {I_BTR, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13412, IF_386|IF_SM},
    {I_BTR, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13412, IF_386},
    {I_BTR, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13418, IF_386|IF_SM},
    {I_BTR, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13418, IF_386},
    {I_BTR, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13424, IF_X64|IF_SM},
    {I_BTR, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13424, IF_X64},
    {I_BTR, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+6962, IF_386|IF_SB},
    {I_BTR, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+6969, IF_386|IF_SB},
    {I_BTR, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+6976, IF_X64|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_BTS[] = {
    {I_BTS, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13430, IF_386|IF_SM},
    {I_BTS, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13430, IF_386},
    {I_BTS, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13436, IF_386|IF_SM},
    {I_BTS, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13436, IF_386},
    {I_BTS, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13442, IF_X64|IF_SM},
    {I_BTS, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13442, IF_X64},
    {I_BTS, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+6983, IF_386|IF_SB},
    {I_BTS, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+6990, IF_386|IF_SB},
    {I_BTS, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+6997, IF_X64|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_CALL[] = {
    {I_CALL, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+16932, IF_8086},
    {I_CALL, 1, {IMMEDIATE|NEAR,0,0,0,0}, nasm_bytecodes+16932, IF_8086},
    {I_CALL, 1, {IMMEDIATE|FAR,0,0,0,0}, nasm_bytecodes+13448, IF_8086|IF_NOLONG},
    {I_CALL, 1, {IMMEDIATE|BITS16,0,0,0,0}, nasm_bytecodes+16937, IF_8086},
    {I_CALL, 1, {IMMEDIATE|BITS16|NEAR,0,0,0,0}, nasm_bytecodes+16937, IF_8086},
    {I_CALL, 1, {IMMEDIATE|BITS16|FAR,0,0,0,0}, nasm_bytecodes+13454, IF_8086|IF_NOLONG},
    {I_CALL, 1, {IMMEDIATE|BITS32,0,0,0,0}, nasm_bytecodes+16942, IF_386},
    {I_CALL, 1, {IMMEDIATE|BITS32|NEAR,0,0,0,0}, nasm_bytecodes+16942, IF_386},
    {I_CALL, 1, {IMMEDIATE|BITS32|FAR,0,0,0,0}, nasm_bytecodes+13460, IF_386|IF_NOLONG},
    {I_CALL, 2, {IMMEDIATE|COLON,IMMEDIATE,0,0,0}, nasm_bytecodes+13466, IF_8086|IF_NOLONG},
    {I_CALL, 2, {IMMEDIATE|BITS16|COLON,IMMEDIATE,0,0,0}, nasm_bytecodes+13472, IF_8086|IF_NOLONG},
    {I_CALL, 2, {IMMEDIATE|COLON,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+13472, IF_8086|IF_NOLONG},
    {I_CALL, 2, {IMMEDIATE|BITS32|COLON,IMMEDIATE,0,0,0}, nasm_bytecodes+13478, IF_386|IF_NOLONG},
    {I_CALL, 2, {IMMEDIATE|COLON,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+13478, IF_386|IF_NOLONG},
    {I_CALL, 1, {MEMORY|FAR,0,0,0,0}, nasm_bytecodes+16947, IF_8086|IF_NOLONG},
    {I_CALL, 1, {MEMORY|FAR,0,0,0,0}, nasm_bytecodes+16952, IF_X64},
    {I_CALL, 1, {MEMORY|BITS16|FAR,0,0,0,0}, nasm_bytecodes+16957, IF_8086},
    {I_CALL, 1, {MEMORY|BITS32|FAR,0,0,0,0}, nasm_bytecodes+16962, IF_386},
    {I_CALL, 1, {MEMORY|BITS64|FAR,0,0,0,0}, nasm_bytecodes+16952, IF_X64},
    {I_CALL, 1, {MEMORY|NEAR,0,0,0,0}, nasm_bytecodes+16967, IF_8086},
    {I_CALL, 1, {MEMORY|BITS16|NEAR,0,0,0,0}, nasm_bytecodes+16972, IF_8086},
    {I_CALL, 1, {MEMORY|BITS32|NEAR,0,0,0,0}, nasm_bytecodes+16977, IF_386|IF_NOLONG},
    {I_CALL, 1, {MEMORY|BITS64|NEAR,0,0,0,0}, nasm_bytecodes+16982, IF_X64},
    {I_CALL, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16972, IF_8086},
    {I_CALL, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16977, IF_386|IF_NOLONG},
    {I_CALL, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16987, IF_X64},
    {I_CALL, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+16967, IF_8086},
    {I_CALL, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+16972, IF_8086},
    {I_CALL, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+16977, IF_386|IF_NOLONG},
    {I_CALL, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+16987, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_CBW[] = {
    {I_CBW, 0, {0,0,0,0,0}, nasm_bytecodes+18684, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_CDQ[] = {
    {I_CDQ, 0, {0,0,0,0,0}, nasm_bytecodes+18688, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_CDQE[] = {
    {I_CDQE, 0, {0,0,0,0,0}, nasm_bytecodes+18692, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_CLC[] = {
    {I_CLC, 0, {0,0,0,0,0}, nasm_bytecodes+18424, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_CLD[] = {
    {I_CLD, 0, {0,0,0,0,0}, nasm_bytecodes+19686, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_CLFLUSH[] = {
    {I_CLFLUSH, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+18577, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CLGI[] = {
    {I_CLGI, 0, {0,0,0,0,0}, nasm_bytecodes+16992, IF_X64|IF_AMD},
    ITEMPLATE_END
};

static const struct itemplate instrux_CLI[] = {
    {I_CLI, 0, {0,0,0,0,0}, nasm_bytecodes+19689, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_CLTS[] = {
    {I_CLTS, 0, {0,0,0,0,0}, nasm_bytecodes+18696, IF_286|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMC[] = {
    {I_CMC, 0, {0,0,0,0,0}, nasm_bytecodes+19692, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMP[] = {
    {I_CMP, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+18700, IF_8086|IF_SM},
    {I_CMP, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+18700, IF_8086},
    {I_CMP, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+16997, IF_8086|IF_SM},
    {I_CMP, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+16997, IF_8086},
    {I_CMP, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+17002, IF_386|IF_SM},
    {I_CMP, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+17002, IF_386},
    {I_CMP, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+17007, IF_X64|IF_SM},
    {I_CMP, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+17007, IF_X64},
    {I_CMP, 2, {REG_GPR|BITS8,MEMORY,0,0,0}, nasm_bytecodes+10780, IF_8086|IF_SM},
    {I_CMP, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+10780, IF_8086},
    {I_CMP, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+17012, IF_8086|IF_SM},
    {I_CMP, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+17012, IF_8086},
    {I_CMP, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+17017, IF_386|IF_SM},
    {I_CMP, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+17017, IF_386},
    {I_CMP, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+17022, IF_X64|IF_SM},
    {I_CMP, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+17022, IF_X64},
    {I_CMP, 2, {RM_GPR|BITS16,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13484, IF_8086},
    {I_CMP, 2, {RM_GPR|BITS32,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13490, IF_386},
    {I_CMP, 2, {RM_GPR|BITS64,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13496, IF_X64},
    {I_CMP, 2, {REG_AL,IMMEDIATE,0,0,0}, nasm_bytecodes+18704, IF_8086|IF_SM},
    {I_CMP, 2, {REG_AX,SBYTE16,0,0,0}, nasm_bytecodes+13484, IF_8086|IF_SM},
    {I_CMP, 2, {REG_AX,IMMEDIATE,0,0,0}, nasm_bytecodes+17027, IF_8086|IF_SM},
    {I_CMP, 2, {REG_EAX,SBYTE32,0,0,0}, nasm_bytecodes+13490, IF_386|IF_SM},
    {I_CMP, 2, {REG_EAX,IMMEDIATE,0,0,0}, nasm_bytecodes+17032, IF_386|IF_SM},
    {I_CMP, 2, {REG_RAX,SBYTE64,0,0,0}, nasm_bytecodes+13496, IF_X64|IF_SM},
    {I_CMP, 2, {REG_RAX,IMMEDIATE,0,0,0}, nasm_bytecodes+17037, IF_X64|IF_SM},
    {I_CMP, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+17042, IF_8086|IF_SM},
    {I_CMP, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+13502, IF_8086|IF_SM},
    {I_CMP, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+13508, IF_386|IF_SM},
    {I_CMP, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+13514, IF_X64|IF_SM},
    {I_CMP, 2, {MEMORY,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+17042, IF_8086|IF_SM},
    {I_CMP, 2, {MEMORY,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+13502, IF_8086|IF_SM},
    {I_CMP, 2, {MEMORY,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+13508, IF_386|IF_SM},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPEQPD[] = {
    {I_CMPEQPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5512, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPEQPS[] = {
    {I_CMPEQPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5336, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPEQSD[] = {
    {I_CMPEQSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5520, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPEQSS[] = {
    {I_CMPEQSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5344, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPLEPD[] = {
    {I_CMPLEPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5528, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPLEPS[] = {
    {I_CMPLEPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5352, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPLESD[] = {
    {I_CMPLESD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5536, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPLESS[] = {
    {I_CMPLESS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5360, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPLTPD[] = {
    {I_CMPLTPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5544, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPLTPS[] = {
    {I_CMPLTPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5368, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPLTSD[] = {
    {I_CMPLTSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5552, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPLTSS[] = {
    {I_CMPLTSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5376, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPNEQPD[] = {
    {I_CMPNEQPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5560, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPNEQPS[] = {
    {I_CMPNEQPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5384, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPNEQSD[] = {
    {I_CMPNEQSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5568, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPNEQSS[] = {
    {I_CMPNEQSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5392, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPNLEPD[] = {
    {I_CMPNLEPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5576, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPNLEPS[] = {
    {I_CMPNLEPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5400, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPNLESD[] = {
    {I_CMPNLESD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5584, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPNLESS[] = {
    {I_CMPNLESS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5408, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPNLTPD[] = {
    {I_CMPNLTPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5592, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPNLTPS[] = {
    {I_CMPNLTPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5416, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPNLTSD[] = {
    {I_CMPNLTSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5600, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPNLTSS[] = {
    {I_CMPNLTSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5424, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPORDPD[] = {
    {I_CMPORDPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5608, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPORDPS[] = {
    {I_CMPORDPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5432, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPORDSD[] = {
    {I_CMPORDSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5616, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPORDSS[] = {
    {I_CMPORDSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5440, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPPD[] = {
    {I_CMPPD, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+7823, IF_WILLAMETTE|IF_SSE2|IF_SM2|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPPS[] = {
    {I_CMPPS, 3, {XMMREG,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+7550, IF_KATMAI|IF_SSE|IF_SB|IF_AR2},
    {I_CMPPS, 3, {XMMREG,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+7550, IF_KATMAI|IF_SSE|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPSB[] = {
    {I_CMPSB, 0, {0,0,0,0,0}, nasm_bytecodes+18708, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPSD[] = {
    {I_CMPSD, 0, {0,0,0,0,0}, nasm_bytecodes+17047, IF_386},
    {I_CMPSD, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+7830, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPSQ[] = {
    {I_CMPSQ, 0, {0,0,0,0,0}, nasm_bytecodes+17052, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPSS[] = {
    {I_CMPSS, 3, {XMMREG,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+7557, IF_KATMAI|IF_SSE|IF_SB|IF_AR2},
    {I_CMPSS, 3, {XMMREG,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+7557, IF_KATMAI|IF_SSE|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPSW[] = {
    {I_CMPSW, 0, {0,0,0,0,0}, nasm_bytecodes+17057, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPUNORDPD[] = {
    {I_CMPUNORDPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5624, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPUNORDPS[] = {
    {I_CMPUNORDPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5448, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPUNORDSD[] = {
    {I_CMPUNORDSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5632, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPUNORDSS[] = {
    {I_CMPUNORDSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+5456, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPXCHG[] = {
    {I_CMPXCHG, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+17062, IF_PENT|IF_SM},
    {I_CMPXCHG, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+17062, IF_PENT},
    {I_CMPXCHG, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13520, IF_PENT|IF_SM},
    {I_CMPXCHG, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13520, IF_PENT},
    {I_CMPXCHG, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13526, IF_PENT|IF_SM},
    {I_CMPXCHG, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13526, IF_PENT},
    {I_CMPXCHG, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13532, IF_X64|IF_SM},
    {I_CMPXCHG, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13532, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPXCHG16B[] = {
    {I_CMPXCHG16B, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+13550, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPXCHG486[] = {
    {I_CMPXCHG486, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+17067, IF_486|IF_SM|IF_UNDOC},
    {I_CMPXCHG486, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+17067, IF_486|IF_UNDOC},
    {I_CMPXCHG486, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13538, IF_486|IF_SM|IF_UNDOC},
    {I_CMPXCHG486, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13538, IF_486|IF_UNDOC},
    {I_CMPXCHG486, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13544, IF_486|IF_SM|IF_UNDOC},
    {I_CMPXCHG486, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13544, IF_486|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMPXCHG8B[] = {
    {I_CMPXCHG8B, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+13551, IF_PENT},
    ITEMPLATE_END
};

static const struct itemplate instrux_COMISD[] = {
    {I_COMISD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15212, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_COMISS[] = {
    {I_COMISS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14492, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_CPUID[] = {
    {I_CPUID, 0, {0,0,0,0,0}, nasm_bytecodes+18712, IF_PENT},
    ITEMPLATE_END
};

static const struct itemplate instrux_CPU_READ[] = {
    {I_CPU_READ, 0, {0,0,0,0,0}, nasm_bytecodes+18716, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_CPU_WRITE[] = {
    {I_CPU_WRITE, 0, {0,0,0,0,0}, nasm_bytecodes+18720, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_CQO[] = {
    {I_CQO, 0, {0,0,0,0,0}, nasm_bytecodes+18724, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_CRC32[] = {
    {I_CRC32, 2, {REG_GPR|BITS32,RM_GPR|BITS8,0,0,0}, nasm_bytecodes+5793, IF_SSE42},
    {I_CRC32, 2, {REG_GPR|BITS32,RM_GPR|BITS16,0,0,0}, nasm_bytecodes+5776, IF_SSE42},
    {I_CRC32, 2, {REG_GPR|BITS32,RM_GPR|BITS32,0,0,0}, nasm_bytecodes+5784, IF_SSE42},
    {I_CRC32, 2, {REG_GPR|BITS64,RM_GPR|BITS8,0,0,0}, nasm_bytecodes+5792, IF_SSE42|IF_X64},
    {I_CRC32, 2, {REG_GPR|BITS64,RM_GPR|BITS64,0,0,0}, nasm_bytecodes+5800, IF_SSE42|IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTDQ2PD[] = {
    {I_CVTDQ2PD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15218, IF_WILLAMETTE|IF_SSE2|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTDQ2PS[] = {
    {I_CVTDQ2PS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15224, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTPD2DQ[] = {
    {I_CVTPD2DQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15230, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTPD2PI[] = {
    {I_CVTPD2PI, 2, {MMXREG,RM_XMM,0,0,0}, nasm_bytecodes+15236, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTPD2PS[] = {
    {I_CVTPD2PS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15242, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTPI2PD[] = {
    {I_CVTPI2PD, 2, {XMMREG,RM_MMX,0,0,0}, nasm_bytecodes+15248, IF_WILLAMETTE|IF_SSE2|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTPI2PS[] = {
    {I_CVTPI2PS, 2, {XMMREG,RM_MMX,0,0,0}, nasm_bytecodes+14498, IF_KATMAI|IF_SSE|IF_MMX|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTPS2DQ[] = {
    {I_CVTPS2DQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15254, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTPS2PD[] = {
    {I_CVTPS2PD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15260, IF_WILLAMETTE|IF_SSE2|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTPS2PI[] = {
    {I_CVTPS2PI, 2, {MMXREG,RM_XMM,0,0,0}, nasm_bytecodes+14504, IF_KATMAI|IF_SSE|IF_MMX|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTSD2SI[] = {
    {I_CVTSD2SI, 2, {REG_GPR|BITS32,XMMREG,0,0,0}, nasm_bytecodes+7838, IF_WILLAMETTE|IF_SSE2|IF_SQ|IF_AR1},
    {I_CVTSD2SI, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+7838, IF_WILLAMETTE|IF_SSE2|IF_SQ|IF_AR1},
    {I_CVTSD2SI, 2, {REG_GPR|BITS64,XMMREG,0,0,0}, nasm_bytecodes+7837, IF_X64|IF_SSE2|IF_SQ|IF_AR1},
    {I_CVTSD2SI, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+7837, IF_X64|IF_SSE2|IF_SQ|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTSD2SS[] = {
    {I_CVTSD2SS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15266, IF_WILLAMETTE|IF_SSE2|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTSI2SD[] = {
    {I_CVTSI2SD, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+7845, IF_WILLAMETTE|IF_SSE2|IF_SD|IF_AR1},
    {I_CVTSI2SD, 2, {XMMREG,RM_GPR|BITS32,0,0,0}, nasm_bytecodes+7845, IF_WILLAMETTE|IF_SSE2|IF_SD|IF_AR1},
    {I_CVTSI2SD, 2, {XMMREG,RM_GPR|BITS64,0,0,0}, nasm_bytecodes+7844, IF_X64|IF_SSE2|IF_SQ|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTSI2SS[] = {
    {I_CVTSI2SS, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+7565, IF_KATMAI|IF_SSE|IF_SD|IF_AR1},
    {I_CVTSI2SS, 2, {XMMREG,RM_GPR|BITS32,0,0,0}, nasm_bytecodes+7565, IF_KATMAI|IF_SSE|IF_SD|IF_AR1},
    {I_CVTSI2SS, 2, {XMMREG,RM_GPR|BITS64,0,0,0}, nasm_bytecodes+7564, IF_X64|IF_SSE|IF_SQ|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTSS2SD[] = {
    {I_CVTSS2SD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15272, IF_WILLAMETTE|IF_SSE2|IF_SD},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTSS2SI[] = {
    {I_CVTSS2SI, 2, {REG_GPR|BITS32,XMMREG,0,0,0}, nasm_bytecodes+7572, IF_KATMAI|IF_SSE|IF_SD|IF_AR1},
    {I_CVTSS2SI, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+7572, IF_KATMAI|IF_SSE|IF_SD|IF_AR1},
    {I_CVTSS2SI, 2, {REG_GPR|BITS64,XMMREG,0,0,0}, nasm_bytecodes+7571, IF_X64|IF_SSE|IF_SD|IF_AR1},
    {I_CVTSS2SI, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+7571, IF_X64|IF_SSE|IF_SD|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTTPD2DQ[] = {
    {I_CVTTPD2DQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15284, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTTPD2PI[] = {
    {I_CVTTPD2PI, 2, {MMXREG,RM_XMM,0,0,0}, nasm_bytecodes+15278, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTTPS2DQ[] = {
    {I_CVTTPS2DQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15290, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTTPS2PI[] = {
    {I_CVTTPS2PI, 2, {MMXREG,RM_XMM,0,0,0}, nasm_bytecodes+14510, IF_KATMAI|IF_SSE|IF_MMX|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTTSD2SI[] = {
    {I_CVTTSD2SI, 2, {REG_GPR|BITS32,XMMREG,0,0,0}, nasm_bytecodes+7852, IF_WILLAMETTE|IF_SSE2|IF_SQ|IF_AR1},
    {I_CVTTSD2SI, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+7852, IF_WILLAMETTE|IF_SSE2|IF_SQ|IF_AR1},
    {I_CVTTSD2SI, 2, {REG_GPR|BITS64,XMMREG,0,0,0}, nasm_bytecodes+7851, IF_X64|IF_SSE2|IF_SQ|IF_AR1},
    {I_CVTTSD2SI, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+7851, IF_X64|IF_SSE2|IF_SQ|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_CVTTSS2SI[] = {
    {I_CVTTSS2SI, 2, {REG_GPR|BITS32,RM_XMM,0,0,0}, nasm_bytecodes+7579, IF_KATMAI|IF_SSE|IF_SD|IF_AR1},
    {I_CVTTSS2SI, 2, {REG_GPR|BITS64,RM_XMM,0,0,0}, nasm_bytecodes+7578, IF_X64|IF_SSE|IF_SD|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_CWD[] = {
    {I_CWD, 0, {0,0,0,0,0}, nasm_bytecodes+18728, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_CWDE[] = {
    {I_CWDE, 0, {0,0,0,0,0}, nasm_bytecodes+18732, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_DAA[] = {
    {I_DAA, 0, {0,0,0,0,0}, nasm_bytecodes+19695, IF_8086|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_DAS[] = {
    {I_DAS, 0, {0,0,0,0,0}, nasm_bytecodes+19698, IF_8086|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_DB[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_DD[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_DEC[] = {
    {I_DEC, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+18736, IF_8086|IF_NOLONG},
    {I_DEC, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+18740, IF_386|IF_NOLONG},
    {I_DEC, 1, {RM_GPR|BITS8,0,0,0,0}, nasm_bytecodes+18744, IF_8086},
    {I_DEC, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17072, IF_8086},
    {I_DEC, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+17077, IF_386},
    {I_DEC, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+17082, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_DIV[] = {
    {I_DIV, 1, {RM_GPR|BITS8,0,0,0,0}, nasm_bytecodes+18748, IF_8086},
    {I_DIV, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17087, IF_8086},
    {I_DIV, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+17092, IF_386},
    {I_DIV, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+17097, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_DIVPD[] = {
    {I_DIVPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15296, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_DIVPS[] = {
    {I_DIVPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14516, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_DIVSD[] = {
    {I_DIVSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15302, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_DIVSS[] = {
    {I_DIVSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14522, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_DMINT[] = {
    {I_DMINT, 0, {0,0,0,0,0}, nasm_bytecodes+18752, IF_P6|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_DO[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_DPPD[] = {
    {I_DPPD, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5704, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_DPPS[] = {
    {I_DPPS, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5712, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_DQ[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_DT[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_DW[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_DY[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_EMMS[] = {
    {I_EMMS, 0, {0,0,0,0,0}, nasm_bytecodes+18756, IF_PENT|IF_MMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_ENTER[] = {
    {I_ENTER, 2, {IMMEDIATE,IMMEDIATE,0,0,0}, nasm_bytecodes+17102, IF_186},
    ITEMPLATE_END
};

static const struct itemplate instrux_EQU[] = {
    {I_EQU, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+5526, IF_8086},
    {I_EQU, 2, {IMMEDIATE|COLON,IMMEDIATE,0,0,0}, nasm_bytecodes+5526, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_EXTRACTPS[] = {
    {I_EXTRACTPS, 3, {RM_GPR|BITS32,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+1, IF_SSE41},
    {I_EXTRACTPS, 3, {REG_GPR|BITS64,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+0, IF_SSE41|IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_EXTRQ[] = {
    {I_EXTRQ, 3, {XMMREG,IMMEDIATE,IMMEDIATE,0,0}, nasm_bytecodes+5672, IF_SSE4A|IF_AMD},
    {I_EXTRQ, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+15536, IF_SSE4A|IF_AMD},
    ITEMPLATE_END
};

static const struct itemplate instrux_F2XM1[] = {
    {I_F2XM1, 0, {0,0,0,0,0}, nasm_bytecodes+18760, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FABS[] = {
    {I_FABS, 0, {0,0,0,0,0}, nasm_bytecodes+18764, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FADD[] = {
    {I_FADD, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18768, IF_8086|IF_FPU},
    {I_FADD, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+18772, IF_8086|IF_FPU},
    {I_FADD, 1, {FPUREG|TO,0,0,0,0}, nasm_bytecodes+17107, IF_8086|IF_FPU},
    {I_FADD, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17112, IF_8086|IF_FPU},
    {I_FADD, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17107, IF_8086|IF_FPU},
    {I_FADD, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17117, IF_8086|IF_FPU},
    {I_FADD, 0, {0,0,0,0,0}, nasm_bytecodes+18776, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FADDP[] = {
    {I_FADDP, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17122, IF_8086|IF_FPU},
    {I_FADDP, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17122, IF_8086|IF_FPU},
    {I_FADDP, 0, {0,0,0,0,0}, nasm_bytecodes+18776, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FBLD[] = {
    {I_FBLD, 1, {MEMORY|BITS80,0,0,0,0}, nasm_bytecodes+18780, IF_8086|IF_FPU},
    {I_FBLD, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+18780, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FBSTP[] = {
    {I_FBSTP, 1, {MEMORY|BITS80,0,0,0,0}, nasm_bytecodes+18784, IF_8086|IF_FPU},
    {I_FBSTP, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+18784, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCHS[] = {
    {I_FCHS, 0, {0,0,0,0,0}, nasm_bytecodes+18788, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCLEX[] = {
    {I_FCLEX, 0, {0,0,0,0,0}, nasm_bytecodes+17127, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCMOVB[] = {
    {I_FCMOVB, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17132, IF_P6|IF_FPU},
    {I_FCMOVB, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17137, IF_P6|IF_FPU},
    {I_FCMOVB, 0, {0,0,0,0,0}, nasm_bytecodes+18792, IF_P6|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCMOVBE[] = {
    {I_FCMOVBE, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17142, IF_P6|IF_FPU},
    {I_FCMOVBE, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17147, IF_P6|IF_FPU},
    {I_FCMOVBE, 0, {0,0,0,0,0}, nasm_bytecodes+18796, IF_P6|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCMOVE[] = {
    {I_FCMOVE, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17152, IF_P6|IF_FPU},
    {I_FCMOVE, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17157, IF_P6|IF_FPU},
    {I_FCMOVE, 0, {0,0,0,0,0}, nasm_bytecodes+18800, IF_P6|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCMOVNB[] = {
    {I_FCMOVNB, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17162, IF_P6|IF_FPU},
    {I_FCMOVNB, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17167, IF_P6|IF_FPU},
    {I_FCMOVNB, 0, {0,0,0,0,0}, nasm_bytecodes+18804, IF_P6|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCMOVNBE[] = {
    {I_FCMOVNBE, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17172, IF_P6|IF_FPU},
    {I_FCMOVNBE, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17177, IF_P6|IF_FPU},
    {I_FCMOVNBE, 0, {0,0,0,0,0}, nasm_bytecodes+18808, IF_P6|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCMOVNE[] = {
    {I_FCMOVNE, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17182, IF_P6|IF_FPU},
    {I_FCMOVNE, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17187, IF_P6|IF_FPU},
    {I_FCMOVNE, 0, {0,0,0,0,0}, nasm_bytecodes+18812, IF_P6|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCMOVNU[] = {
    {I_FCMOVNU, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17192, IF_P6|IF_FPU},
    {I_FCMOVNU, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17197, IF_P6|IF_FPU},
    {I_FCMOVNU, 0, {0,0,0,0,0}, nasm_bytecodes+18816, IF_P6|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCMOVU[] = {
    {I_FCMOVU, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17202, IF_P6|IF_FPU},
    {I_FCMOVU, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17207, IF_P6|IF_FPU},
    {I_FCMOVU, 0, {0,0,0,0,0}, nasm_bytecodes+18820, IF_P6|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCOM[] = {
    {I_FCOM, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18824, IF_8086|IF_FPU},
    {I_FCOM, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+18828, IF_8086|IF_FPU},
    {I_FCOM, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17212, IF_8086|IF_FPU},
    {I_FCOM, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17217, IF_8086|IF_FPU},
    {I_FCOM, 0, {0,0,0,0,0}, nasm_bytecodes+18832, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCOMI[] = {
    {I_FCOMI, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17222, IF_P6|IF_FPU},
    {I_FCOMI, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17227, IF_P6|IF_FPU},
    {I_FCOMI, 0, {0,0,0,0,0}, nasm_bytecodes+18836, IF_P6|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCOMIP[] = {
    {I_FCOMIP, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17232, IF_P6|IF_FPU},
    {I_FCOMIP, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17237, IF_P6|IF_FPU},
    {I_FCOMIP, 0, {0,0,0,0,0}, nasm_bytecodes+18840, IF_P6|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCOMP[] = {
    {I_FCOMP, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18844, IF_8086|IF_FPU},
    {I_FCOMP, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+18848, IF_8086|IF_FPU},
    {I_FCOMP, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17242, IF_8086|IF_FPU},
    {I_FCOMP, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17247, IF_8086|IF_FPU},
    {I_FCOMP, 0, {0,0,0,0,0}, nasm_bytecodes+18852, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCOMPP[] = {
    {I_FCOMPP, 0, {0,0,0,0,0}, nasm_bytecodes+18856, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FCOS[] = {
    {I_FCOS, 0, {0,0,0,0,0}, nasm_bytecodes+18860, IF_386|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FDECSTP[] = {
    {I_FDECSTP, 0, {0,0,0,0,0}, nasm_bytecodes+18864, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FDISI[] = {
    {I_FDISI, 0, {0,0,0,0,0}, nasm_bytecodes+17252, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FDIV[] = {
    {I_FDIV, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18868, IF_8086|IF_FPU},
    {I_FDIV, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+18872, IF_8086|IF_FPU},
    {I_FDIV, 1, {FPUREG|TO,0,0,0,0}, nasm_bytecodes+17257, IF_8086|IF_FPU},
    {I_FDIV, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17262, IF_8086|IF_FPU},
    {I_FDIV, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17257, IF_8086|IF_FPU},
    {I_FDIV, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17267, IF_8086|IF_FPU},
    {I_FDIV, 0, {0,0,0,0,0}, nasm_bytecodes+18876, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FDIVP[] = {
    {I_FDIVP, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17272, IF_8086|IF_FPU},
    {I_FDIVP, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17272, IF_8086|IF_FPU},
    {I_FDIVP, 0, {0,0,0,0,0}, nasm_bytecodes+18876, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FDIVR[] = {
    {I_FDIVR, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18880, IF_8086|IF_FPU},
    {I_FDIVR, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+18884, IF_8086|IF_FPU},
    {I_FDIVR, 1, {FPUREG|TO,0,0,0,0}, nasm_bytecodes+17277, IF_8086|IF_FPU},
    {I_FDIVR, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17277, IF_8086|IF_FPU},
    {I_FDIVR, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17282, IF_8086|IF_FPU},
    {I_FDIVR, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17287, IF_8086|IF_FPU},
    {I_FDIVR, 0, {0,0,0,0,0}, nasm_bytecodes+18888, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FDIVRP[] = {
    {I_FDIVRP, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17292, IF_8086|IF_FPU},
    {I_FDIVRP, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17292, IF_8086|IF_FPU},
    {I_FDIVRP, 0, {0,0,0,0,0}, nasm_bytecodes+18888, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FEMMS[] = {
    {I_FEMMS, 0, {0,0,0,0,0}, nasm_bytecodes+18892, IF_PENT|IF_3DNOW},
    ITEMPLATE_END
};

static const struct itemplate instrux_FENI[] = {
    {I_FENI, 0, {0,0,0,0,0}, nasm_bytecodes+17297, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FFREE[] = {
    {I_FFREE, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17302, IF_8086|IF_FPU},
    {I_FFREE, 0, {0,0,0,0,0}, nasm_bytecodes+18896, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FFREEP[] = {
    {I_FFREEP, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17307, IF_286|IF_FPU|IF_UNDOC},
    {I_FFREEP, 0, {0,0,0,0,0}, nasm_bytecodes+18900, IF_286|IF_FPU|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_FIADD[] = {
    {I_FIADD, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18904, IF_8086|IF_FPU},
    {I_FIADD, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+18908, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FICOM[] = {
    {I_FICOM, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18912, IF_8086|IF_FPU},
    {I_FICOM, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+18916, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FICOMP[] = {
    {I_FICOMP, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18920, IF_8086|IF_FPU},
    {I_FICOMP, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+18924, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FIDIV[] = {
    {I_FIDIV, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18928, IF_8086|IF_FPU},
    {I_FIDIV, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+18932, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FIDIVR[] = {
    {I_FIDIVR, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18936, IF_8086|IF_FPU},
    {I_FIDIVR, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+18940, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FILD[] = {
    {I_FILD, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18944, IF_8086|IF_FPU},
    {I_FILD, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+18948, IF_8086|IF_FPU},
    {I_FILD, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+18952, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FIMUL[] = {
    {I_FIMUL, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18956, IF_8086|IF_FPU},
    {I_FIMUL, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+18960, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FINCSTP[] = {
    {I_FINCSTP, 0, {0,0,0,0,0}, nasm_bytecodes+18964, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FINIT[] = {
    {I_FINIT, 0, {0,0,0,0,0}, nasm_bytecodes+17312, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FIST[] = {
    {I_FIST, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18968, IF_8086|IF_FPU},
    {I_FIST, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+18972, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FISTP[] = {
    {I_FISTP, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18976, IF_8086|IF_FPU},
    {I_FISTP, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+18980, IF_8086|IF_FPU},
    {I_FISTP, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+18984, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FISTTP[] = {
    {I_FISTTP, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+18988, IF_PRESCOTT|IF_FPU},
    {I_FISTTP, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+18992, IF_PRESCOTT|IF_FPU},
    {I_FISTTP, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+18996, IF_PRESCOTT|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FISUB[] = {
    {I_FISUB, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+19000, IF_8086|IF_FPU},
    {I_FISUB, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+19004, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FISUBR[] = {
    {I_FISUBR, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+19008, IF_8086|IF_FPU},
    {I_FISUBR, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+19012, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FLD[] = {
    {I_FLD, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+19016, IF_8086|IF_FPU},
    {I_FLD, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+19020, IF_8086|IF_FPU},
    {I_FLD, 1, {MEMORY|BITS80,0,0,0,0}, nasm_bytecodes+19024, IF_8086|IF_FPU},
    {I_FLD, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17317, IF_8086|IF_FPU},
    {I_FLD, 0, {0,0,0,0,0}, nasm_bytecodes+19028, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FLD1[] = {
    {I_FLD1, 0, {0,0,0,0,0}, nasm_bytecodes+19032, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FLDCW[] = {
    {I_FLDCW, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+19036, IF_8086|IF_FPU|IF_SW},
    ITEMPLATE_END
};

static const struct itemplate instrux_FLDENV[] = {
    {I_FLDENV, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+19040, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FLDL2E[] = {
    {I_FLDL2E, 0, {0,0,0,0,0}, nasm_bytecodes+19044, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FLDL2T[] = {
    {I_FLDL2T, 0, {0,0,0,0,0}, nasm_bytecodes+19048, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FLDLG2[] = {
    {I_FLDLG2, 0, {0,0,0,0,0}, nasm_bytecodes+19052, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FLDLN2[] = {
    {I_FLDLN2, 0, {0,0,0,0,0}, nasm_bytecodes+19056, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FLDPI[] = {
    {I_FLDPI, 0, {0,0,0,0,0}, nasm_bytecodes+19060, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FLDZ[] = {
    {I_FLDZ, 0, {0,0,0,0,0}, nasm_bytecodes+19064, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FMUL[] = {
    {I_FMUL, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+19068, IF_8086|IF_FPU},
    {I_FMUL, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+19072, IF_8086|IF_FPU},
    {I_FMUL, 1, {FPUREG|TO,0,0,0,0}, nasm_bytecodes+17322, IF_8086|IF_FPU},
    {I_FMUL, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17322, IF_8086|IF_FPU},
    {I_FMUL, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17327, IF_8086|IF_FPU},
    {I_FMUL, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17332, IF_8086|IF_FPU},
    {I_FMUL, 0, {0,0,0,0,0}, nasm_bytecodes+19076, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FMULP[] = {
    {I_FMULP, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17337, IF_8086|IF_FPU},
    {I_FMULP, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17337, IF_8086|IF_FPU},
    {I_FMULP, 0, {0,0,0,0,0}, nasm_bytecodes+19076, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FNCLEX[] = {
    {I_FNCLEX, 0, {0,0,0,0,0}, nasm_bytecodes+17128, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FNDISI[] = {
    {I_FNDISI, 0, {0,0,0,0,0}, nasm_bytecodes+17253, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FNENI[] = {
    {I_FNENI, 0, {0,0,0,0,0}, nasm_bytecodes+17298, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FNINIT[] = {
    {I_FNINIT, 0, {0,0,0,0,0}, nasm_bytecodes+17313, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FNOP[] = {
    {I_FNOP, 0, {0,0,0,0,0}, nasm_bytecodes+19080, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FNSAVE[] = {
    {I_FNSAVE, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17343, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FNSTCW[] = {
    {I_FNSTCW, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17353, IF_8086|IF_FPU|IF_SW},
    ITEMPLATE_END
};

static const struct itemplate instrux_FNSTENV[] = {
    {I_FNSTENV, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17358, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FNSTSW[] = {
    {I_FNSTSW, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17368, IF_8086|IF_FPU|IF_SW},
    {I_FNSTSW, 1, {REG_AX,0,0,0,0}, nasm_bytecodes+17373, IF_286|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FPATAN[] = {
    {I_FPATAN, 0, {0,0,0,0,0}, nasm_bytecodes+19084, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FPREM[] = {
    {I_FPREM, 0, {0,0,0,0,0}, nasm_bytecodes+19088, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FPREM1[] = {
    {I_FPREM1, 0, {0,0,0,0,0}, nasm_bytecodes+19092, IF_386|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FPTAN[] = {
    {I_FPTAN, 0, {0,0,0,0,0}, nasm_bytecodes+19096, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FRNDINT[] = {
    {I_FRNDINT, 0, {0,0,0,0,0}, nasm_bytecodes+19100, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FRSTOR[] = {
    {I_FRSTOR, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+19104, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSAVE[] = {
    {I_FSAVE, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17342, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSCALE[] = {
    {I_FSCALE, 0, {0,0,0,0,0}, nasm_bytecodes+19108, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSETPM[] = {
    {I_FSETPM, 0, {0,0,0,0,0}, nasm_bytecodes+19112, IF_286|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSIN[] = {
    {I_FSIN, 0, {0,0,0,0,0}, nasm_bytecodes+19116, IF_386|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSINCOS[] = {
    {I_FSINCOS, 0, {0,0,0,0,0}, nasm_bytecodes+19120, IF_386|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSQRT[] = {
    {I_FSQRT, 0, {0,0,0,0,0}, nasm_bytecodes+19124, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FST[] = {
    {I_FST, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+19128, IF_8086|IF_FPU},
    {I_FST, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+19132, IF_8086|IF_FPU},
    {I_FST, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17347, IF_8086|IF_FPU},
    {I_FST, 0, {0,0,0,0,0}, nasm_bytecodes+19136, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSTCW[] = {
    {I_FSTCW, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17352, IF_8086|IF_FPU|IF_SW},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSTENV[] = {
    {I_FSTENV, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17357, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSTP[] = {
    {I_FSTP, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+19140, IF_8086|IF_FPU},
    {I_FSTP, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+19144, IF_8086|IF_FPU},
    {I_FSTP, 1, {MEMORY|BITS80,0,0,0,0}, nasm_bytecodes+19148, IF_8086|IF_FPU},
    {I_FSTP, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17362, IF_8086|IF_FPU},
    {I_FSTP, 0, {0,0,0,0,0}, nasm_bytecodes+19152, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSTSW[] = {
    {I_FSTSW, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17367, IF_8086|IF_FPU|IF_SW},
    {I_FSTSW, 1, {REG_AX,0,0,0,0}, nasm_bytecodes+17372, IF_286|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSUB[] = {
    {I_FSUB, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+19156, IF_8086|IF_FPU},
    {I_FSUB, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+19160, IF_8086|IF_FPU},
    {I_FSUB, 1, {FPUREG|TO,0,0,0,0}, nasm_bytecodes+17377, IF_8086|IF_FPU},
    {I_FSUB, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17377, IF_8086|IF_FPU},
    {I_FSUB, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17382, IF_8086|IF_FPU},
    {I_FSUB, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17387, IF_8086|IF_FPU},
    {I_FSUB, 0, {0,0,0,0,0}, nasm_bytecodes+19164, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSUBP[] = {
    {I_FSUBP, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17392, IF_8086|IF_FPU},
    {I_FSUBP, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17392, IF_8086|IF_FPU},
    {I_FSUBP, 0, {0,0,0,0,0}, nasm_bytecodes+19164, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSUBR[] = {
    {I_FSUBR, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+19168, IF_8086|IF_FPU},
    {I_FSUBR, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+19172, IF_8086|IF_FPU},
    {I_FSUBR, 1, {FPUREG|TO,0,0,0,0}, nasm_bytecodes+17397, IF_8086|IF_FPU},
    {I_FSUBR, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17397, IF_8086|IF_FPU},
    {I_FSUBR, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17402, IF_8086|IF_FPU},
    {I_FSUBR, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17407, IF_8086|IF_FPU},
    {I_FSUBR, 0, {0,0,0,0,0}, nasm_bytecodes+19176, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FSUBRP[] = {
    {I_FSUBRP, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17412, IF_8086|IF_FPU},
    {I_FSUBRP, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17412, IF_8086|IF_FPU},
    {I_FSUBRP, 0, {0,0,0,0,0}, nasm_bytecodes+19176, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FTST[] = {
    {I_FTST, 0, {0,0,0,0,0}, nasm_bytecodes+19180, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FUCOM[] = {
    {I_FUCOM, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17417, IF_386|IF_FPU},
    {I_FUCOM, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17422, IF_386|IF_FPU},
    {I_FUCOM, 0, {0,0,0,0,0}, nasm_bytecodes+19184, IF_386|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FUCOMI[] = {
    {I_FUCOMI, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17427, IF_P6|IF_FPU},
    {I_FUCOMI, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17432, IF_P6|IF_FPU},
    {I_FUCOMI, 0, {0,0,0,0,0}, nasm_bytecodes+19188, IF_P6|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FUCOMIP[] = {
    {I_FUCOMIP, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17437, IF_P6|IF_FPU},
    {I_FUCOMIP, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17442, IF_P6|IF_FPU},
    {I_FUCOMIP, 0, {0,0,0,0,0}, nasm_bytecodes+19192, IF_P6|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FUCOMP[] = {
    {I_FUCOMP, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17447, IF_386|IF_FPU},
    {I_FUCOMP, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17452, IF_386|IF_FPU},
    {I_FUCOMP, 0, {0,0,0,0,0}, nasm_bytecodes+19196, IF_386|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FUCOMPP[] = {
    {I_FUCOMPP, 0, {0,0,0,0,0}, nasm_bytecodes+19200, IF_386|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FWAIT[] = {
    {I_FWAIT, 0, {0,0,0,0,0}, nasm_bytecodes+19186, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_FXAM[] = {
    {I_FXAM, 0, {0,0,0,0,0}, nasm_bytecodes+19204, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FXCH[] = {
    {I_FXCH, 1, {FPUREG,0,0,0,0}, nasm_bytecodes+17457, IF_8086|IF_FPU},
    {I_FXCH, 2, {FPUREG,FPU0,0,0,0}, nasm_bytecodes+17457, IF_8086|IF_FPU},
    {I_FXCH, 2, {FPU0,FPUREG,0,0,0}, nasm_bytecodes+17462, IF_8086|IF_FPU},
    {I_FXCH, 0, {0,0,0,0,0}, nasm_bytecodes+19208, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FXRSTOR[] = {
    {I_FXRSTOR, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14709, IF_P6|IF_SSE|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FXRSTOR64[] = {
    {I_FXRSTOR64, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14708, IF_X64|IF_SSE|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FXSAVE[] = {
    {I_FXSAVE, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14715, IF_P6|IF_SSE|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FXSAVE64[] = {
    {I_FXSAVE64, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14714, IF_X64|IF_SSE|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FXTRACT[] = {
    {I_FXTRACT, 0, {0,0,0,0,0}, nasm_bytecodes+19212, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FYL2X[] = {
    {I_FYL2X, 0, {0,0,0,0,0}, nasm_bytecodes+19216, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_FYL2XP1[] = {
    {I_FYL2XP1, 0, {0,0,0,0,0}, nasm_bytecodes+19220, IF_8086|IF_FPU},
    ITEMPLATE_END
};

static const struct itemplate instrux_GETSEC[] = {
    {I_GETSEC, 0, {0,0,0,0,0}, nasm_bytecodes+19676, IF_KATMAI},
    ITEMPLATE_END
};

static const struct itemplate instrux_HADDPD[] = {
    {I_HADDPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15476, IF_PRESCOTT|IF_SSE3|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_HADDPS[] = {
    {I_HADDPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15482, IF_PRESCOTT|IF_SSE3|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP0[] = {
    {I_HINT_NOP0, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15638, IF_P6|IF_UNDOC},
    {I_HINT_NOP0, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15644, IF_P6|IF_UNDOC},
    {I_HINT_NOP0, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15650, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP1[] = {
    {I_HINT_NOP1, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15656, IF_P6|IF_UNDOC},
    {I_HINT_NOP1, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15662, IF_P6|IF_UNDOC},
    {I_HINT_NOP1, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15668, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP10[] = {
    {I_HINT_NOP10, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15818, IF_P6|IF_UNDOC},
    {I_HINT_NOP10, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15824, IF_P6|IF_UNDOC},
    {I_HINT_NOP10, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15830, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP11[] = {
    {I_HINT_NOP11, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15836, IF_P6|IF_UNDOC},
    {I_HINT_NOP11, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15842, IF_P6|IF_UNDOC},
    {I_HINT_NOP11, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15848, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP12[] = {
    {I_HINT_NOP12, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15854, IF_P6|IF_UNDOC},
    {I_HINT_NOP12, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15860, IF_P6|IF_UNDOC},
    {I_HINT_NOP12, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15866, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP13[] = {
    {I_HINT_NOP13, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15872, IF_P6|IF_UNDOC},
    {I_HINT_NOP13, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15878, IF_P6|IF_UNDOC},
    {I_HINT_NOP13, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15884, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP14[] = {
    {I_HINT_NOP14, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15890, IF_P6|IF_UNDOC},
    {I_HINT_NOP14, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15896, IF_P6|IF_UNDOC},
    {I_HINT_NOP14, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15902, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP15[] = {
    {I_HINT_NOP15, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15908, IF_P6|IF_UNDOC},
    {I_HINT_NOP15, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15914, IF_P6|IF_UNDOC},
    {I_HINT_NOP15, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15920, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP16[] = {
    {I_HINT_NOP16, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15926, IF_P6|IF_UNDOC},
    {I_HINT_NOP16, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15932, IF_P6|IF_UNDOC},
    {I_HINT_NOP16, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15938, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP17[] = {
    {I_HINT_NOP17, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15944, IF_P6|IF_UNDOC},
    {I_HINT_NOP17, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15950, IF_P6|IF_UNDOC},
    {I_HINT_NOP17, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15956, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP18[] = {
    {I_HINT_NOP18, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15962, IF_P6|IF_UNDOC},
    {I_HINT_NOP18, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15968, IF_P6|IF_UNDOC},
    {I_HINT_NOP18, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15974, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP19[] = {
    {I_HINT_NOP19, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15980, IF_P6|IF_UNDOC},
    {I_HINT_NOP19, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15986, IF_P6|IF_UNDOC},
    {I_HINT_NOP19, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15992, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP2[] = {
    {I_HINT_NOP2, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15674, IF_P6|IF_UNDOC},
    {I_HINT_NOP2, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15680, IF_P6|IF_UNDOC},
    {I_HINT_NOP2, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15686, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP20[] = {
    {I_HINT_NOP20, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15998, IF_P6|IF_UNDOC},
    {I_HINT_NOP20, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16004, IF_P6|IF_UNDOC},
    {I_HINT_NOP20, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16010, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP21[] = {
    {I_HINT_NOP21, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16016, IF_P6|IF_UNDOC},
    {I_HINT_NOP21, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16022, IF_P6|IF_UNDOC},
    {I_HINT_NOP21, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16028, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP22[] = {
    {I_HINT_NOP22, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16034, IF_P6|IF_UNDOC},
    {I_HINT_NOP22, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16040, IF_P6|IF_UNDOC},
    {I_HINT_NOP22, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16046, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP23[] = {
    {I_HINT_NOP23, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16052, IF_P6|IF_UNDOC},
    {I_HINT_NOP23, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16058, IF_P6|IF_UNDOC},
    {I_HINT_NOP23, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16064, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP24[] = {
    {I_HINT_NOP24, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16070, IF_P6|IF_UNDOC},
    {I_HINT_NOP24, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16076, IF_P6|IF_UNDOC},
    {I_HINT_NOP24, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16082, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP25[] = {
    {I_HINT_NOP25, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16088, IF_P6|IF_UNDOC},
    {I_HINT_NOP25, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16094, IF_P6|IF_UNDOC},
    {I_HINT_NOP25, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16100, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP26[] = {
    {I_HINT_NOP26, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16106, IF_P6|IF_UNDOC},
    {I_HINT_NOP26, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16112, IF_P6|IF_UNDOC},
    {I_HINT_NOP26, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16118, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP27[] = {
    {I_HINT_NOP27, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16124, IF_P6|IF_UNDOC},
    {I_HINT_NOP27, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16130, IF_P6|IF_UNDOC},
    {I_HINT_NOP27, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16136, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP28[] = {
    {I_HINT_NOP28, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16142, IF_P6|IF_UNDOC},
    {I_HINT_NOP28, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16148, IF_P6|IF_UNDOC},
    {I_HINT_NOP28, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16154, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP29[] = {
    {I_HINT_NOP29, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16160, IF_P6|IF_UNDOC},
    {I_HINT_NOP29, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16166, IF_P6|IF_UNDOC},
    {I_HINT_NOP29, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16172, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP3[] = {
    {I_HINT_NOP3, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15692, IF_P6|IF_UNDOC},
    {I_HINT_NOP3, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15698, IF_P6|IF_UNDOC},
    {I_HINT_NOP3, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15704, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP30[] = {
    {I_HINT_NOP30, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16178, IF_P6|IF_UNDOC},
    {I_HINT_NOP30, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16184, IF_P6|IF_UNDOC},
    {I_HINT_NOP30, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16190, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP31[] = {
    {I_HINT_NOP31, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16196, IF_P6|IF_UNDOC},
    {I_HINT_NOP31, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16202, IF_P6|IF_UNDOC},
    {I_HINT_NOP31, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16208, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP32[] = {
    {I_HINT_NOP32, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16214, IF_P6|IF_UNDOC},
    {I_HINT_NOP32, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16220, IF_P6|IF_UNDOC},
    {I_HINT_NOP32, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16226, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP33[] = {
    {I_HINT_NOP33, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16232, IF_P6|IF_UNDOC},
    {I_HINT_NOP33, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16238, IF_P6|IF_UNDOC},
    {I_HINT_NOP33, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16244, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP34[] = {
    {I_HINT_NOP34, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16250, IF_P6|IF_UNDOC},
    {I_HINT_NOP34, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16256, IF_P6|IF_UNDOC},
    {I_HINT_NOP34, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16262, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP35[] = {
    {I_HINT_NOP35, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16268, IF_P6|IF_UNDOC},
    {I_HINT_NOP35, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16274, IF_P6|IF_UNDOC},
    {I_HINT_NOP35, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16280, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP36[] = {
    {I_HINT_NOP36, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16286, IF_P6|IF_UNDOC},
    {I_HINT_NOP36, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16292, IF_P6|IF_UNDOC},
    {I_HINT_NOP36, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16298, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP37[] = {
    {I_HINT_NOP37, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16304, IF_P6|IF_UNDOC},
    {I_HINT_NOP37, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16310, IF_P6|IF_UNDOC},
    {I_HINT_NOP37, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16316, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP38[] = {
    {I_HINT_NOP38, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16322, IF_P6|IF_UNDOC},
    {I_HINT_NOP38, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16328, IF_P6|IF_UNDOC},
    {I_HINT_NOP38, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16334, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP39[] = {
    {I_HINT_NOP39, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16340, IF_P6|IF_UNDOC},
    {I_HINT_NOP39, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16346, IF_P6|IF_UNDOC},
    {I_HINT_NOP39, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16352, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP4[] = {
    {I_HINT_NOP4, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15710, IF_P6|IF_UNDOC},
    {I_HINT_NOP4, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15716, IF_P6|IF_UNDOC},
    {I_HINT_NOP4, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15722, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP40[] = {
    {I_HINT_NOP40, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16358, IF_P6|IF_UNDOC},
    {I_HINT_NOP40, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16364, IF_P6|IF_UNDOC},
    {I_HINT_NOP40, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16370, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP41[] = {
    {I_HINT_NOP41, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16376, IF_P6|IF_UNDOC},
    {I_HINT_NOP41, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16382, IF_P6|IF_UNDOC},
    {I_HINT_NOP41, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16388, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP42[] = {
    {I_HINT_NOP42, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16394, IF_P6|IF_UNDOC},
    {I_HINT_NOP42, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16400, IF_P6|IF_UNDOC},
    {I_HINT_NOP42, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16406, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP43[] = {
    {I_HINT_NOP43, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16412, IF_P6|IF_UNDOC},
    {I_HINT_NOP43, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16418, IF_P6|IF_UNDOC},
    {I_HINT_NOP43, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16424, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP44[] = {
    {I_HINT_NOP44, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16430, IF_P6|IF_UNDOC},
    {I_HINT_NOP44, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16436, IF_P6|IF_UNDOC},
    {I_HINT_NOP44, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16442, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP45[] = {
    {I_HINT_NOP45, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16448, IF_P6|IF_UNDOC},
    {I_HINT_NOP45, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16454, IF_P6|IF_UNDOC},
    {I_HINT_NOP45, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16460, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP46[] = {
    {I_HINT_NOP46, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16466, IF_P6|IF_UNDOC},
    {I_HINT_NOP46, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16472, IF_P6|IF_UNDOC},
    {I_HINT_NOP46, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16478, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP47[] = {
    {I_HINT_NOP47, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16484, IF_P6|IF_UNDOC},
    {I_HINT_NOP47, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16490, IF_P6|IF_UNDOC},
    {I_HINT_NOP47, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16496, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP48[] = {
    {I_HINT_NOP48, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16502, IF_P6|IF_UNDOC},
    {I_HINT_NOP48, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16508, IF_P6|IF_UNDOC},
    {I_HINT_NOP48, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16514, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP49[] = {
    {I_HINT_NOP49, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16520, IF_P6|IF_UNDOC},
    {I_HINT_NOP49, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16526, IF_P6|IF_UNDOC},
    {I_HINT_NOP49, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16532, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP5[] = {
    {I_HINT_NOP5, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15728, IF_P6|IF_UNDOC},
    {I_HINT_NOP5, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15734, IF_P6|IF_UNDOC},
    {I_HINT_NOP5, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15740, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP50[] = {
    {I_HINT_NOP50, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16538, IF_P6|IF_UNDOC},
    {I_HINT_NOP50, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16544, IF_P6|IF_UNDOC},
    {I_HINT_NOP50, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16550, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP51[] = {
    {I_HINT_NOP51, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16556, IF_P6|IF_UNDOC},
    {I_HINT_NOP51, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16562, IF_P6|IF_UNDOC},
    {I_HINT_NOP51, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16568, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP52[] = {
    {I_HINT_NOP52, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16574, IF_P6|IF_UNDOC},
    {I_HINT_NOP52, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16580, IF_P6|IF_UNDOC},
    {I_HINT_NOP52, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16586, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP53[] = {
    {I_HINT_NOP53, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16592, IF_P6|IF_UNDOC},
    {I_HINT_NOP53, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16598, IF_P6|IF_UNDOC},
    {I_HINT_NOP53, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16604, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP54[] = {
    {I_HINT_NOP54, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16610, IF_P6|IF_UNDOC},
    {I_HINT_NOP54, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16616, IF_P6|IF_UNDOC},
    {I_HINT_NOP54, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16622, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP55[] = {
    {I_HINT_NOP55, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16628, IF_P6|IF_UNDOC},
    {I_HINT_NOP55, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16634, IF_P6|IF_UNDOC},
    {I_HINT_NOP55, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16640, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP56[] = {
    {I_HINT_NOP56, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+13976, IF_P6|IF_UNDOC},
    {I_HINT_NOP56, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+13982, IF_P6|IF_UNDOC},
    {I_HINT_NOP56, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+13988, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP57[] = {
    {I_HINT_NOP57, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16646, IF_P6|IF_UNDOC},
    {I_HINT_NOP57, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16652, IF_P6|IF_UNDOC},
    {I_HINT_NOP57, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16658, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP58[] = {
    {I_HINT_NOP58, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16664, IF_P6|IF_UNDOC},
    {I_HINT_NOP58, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16670, IF_P6|IF_UNDOC},
    {I_HINT_NOP58, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16676, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP59[] = {
    {I_HINT_NOP59, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16682, IF_P6|IF_UNDOC},
    {I_HINT_NOP59, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16688, IF_P6|IF_UNDOC},
    {I_HINT_NOP59, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16694, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP6[] = {
    {I_HINT_NOP6, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15746, IF_P6|IF_UNDOC},
    {I_HINT_NOP6, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15752, IF_P6|IF_UNDOC},
    {I_HINT_NOP6, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15758, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP60[] = {
    {I_HINT_NOP60, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16700, IF_P6|IF_UNDOC},
    {I_HINT_NOP60, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16706, IF_P6|IF_UNDOC},
    {I_HINT_NOP60, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16712, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP61[] = {
    {I_HINT_NOP61, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16718, IF_P6|IF_UNDOC},
    {I_HINT_NOP61, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16724, IF_P6|IF_UNDOC},
    {I_HINT_NOP61, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16730, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP62[] = {
    {I_HINT_NOP62, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16736, IF_P6|IF_UNDOC},
    {I_HINT_NOP62, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16742, IF_P6|IF_UNDOC},
    {I_HINT_NOP62, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16748, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP63[] = {
    {I_HINT_NOP63, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+16754, IF_P6|IF_UNDOC},
    {I_HINT_NOP63, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+16760, IF_P6|IF_UNDOC},
    {I_HINT_NOP63, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+16766, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP7[] = {
    {I_HINT_NOP7, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15764, IF_P6|IF_UNDOC},
    {I_HINT_NOP7, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15770, IF_P6|IF_UNDOC},
    {I_HINT_NOP7, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15776, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP8[] = {
    {I_HINT_NOP8, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15782, IF_P6|IF_UNDOC},
    {I_HINT_NOP8, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15788, IF_P6|IF_UNDOC},
    {I_HINT_NOP8, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15794, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HINT_NOP9[] = {
    {I_HINT_NOP9, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15800, IF_P6|IF_UNDOC},
    {I_HINT_NOP9, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15806, IF_P6|IF_UNDOC},
    {I_HINT_NOP9, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15812, IF_X64|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_HLT[] = {
    {I_HLT, 0, {0,0,0,0,0}, nasm_bytecodes+19701, IF_8086|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_HSUBPD[] = {
    {I_HSUBPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15488, IF_PRESCOTT|IF_SSE3|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_HSUBPS[] = {
    {I_HSUBPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15494, IF_PRESCOTT|IF_SSE3|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_IBTS[] = {
    {I_IBTS, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13538, IF_386|IF_SW|IF_UNDOC},
    {I_IBTS, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13538, IF_386|IF_UNDOC},
    {I_IBTS, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13544, IF_386|IF_SD|IF_UNDOC},
    {I_IBTS, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13544, IF_386|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_ICEBP[] = {
    {I_ICEBP, 0, {0,0,0,0,0}, nasm_bytecodes+19704, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_IDIV[] = {
    {I_IDIV, 1, {RM_GPR|BITS8,0,0,0,0}, nasm_bytecodes+19224, IF_8086},
    {I_IDIV, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17467, IF_8086},
    {I_IDIV, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+17472, IF_386},
    {I_IDIV, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+17477, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_IMUL[] = {
    {I_IMUL, 1, {RM_GPR|BITS8,0,0,0,0}, nasm_bytecodes+19228, IF_8086},
    {I_IMUL, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17482, IF_8086},
    {I_IMUL, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+17487, IF_386},
    {I_IMUL, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+17492, IF_X64},
    {I_IMUL, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+13556, IF_386|IF_SM},
    {I_IMUL, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13556, IF_386},
    {I_IMUL, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+13562, IF_386|IF_SM},
    {I_IMUL, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13562, IF_386},
    {I_IMUL, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+13568, IF_X64|IF_SM},
    {I_IMUL, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13568, IF_X64},
    {I_IMUL, 3, {REG_GPR|BITS16,MEMORY,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+13574, IF_186|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS16,MEMORY,SBYTE16,0,0}, nasm_bytecodes+13574, IF_186|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS16,MEMORY,IMMEDIATE|BITS16,0,0}, nasm_bytecodes+13580, IF_186|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS16,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+13586, IF_186|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS16,REG_GPR|BITS16,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+13574, IF_186},
    {I_IMUL, 3, {REG_GPR|BITS16,REG_GPR|BITS16,SBYTE16,0,0}, nasm_bytecodes+13574, IF_186|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS16,REG_GPR|BITS16,IMMEDIATE|BITS16,0,0}, nasm_bytecodes+13580, IF_186},
    {I_IMUL, 3, {REG_GPR|BITS16,REG_GPR|BITS16,IMMEDIATE,0,0}, nasm_bytecodes+13586, IF_186|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS32,MEMORY,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+13592, IF_386|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS32,MEMORY,SBYTE32,0,0}, nasm_bytecodes+13592, IF_386|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS32,MEMORY,IMMEDIATE|BITS32,0,0}, nasm_bytecodes+13598, IF_386|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS32,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+13604, IF_386|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS32,REG_GPR|BITS32,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+13592, IF_386},
    {I_IMUL, 3, {REG_GPR|BITS32,REG_GPR|BITS32,SBYTE32,0,0}, nasm_bytecodes+13592, IF_386|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS32,REG_GPR|BITS32,IMMEDIATE|BITS32,0,0}, nasm_bytecodes+13598, IF_386},
    {I_IMUL, 3, {REG_GPR|BITS32,REG_GPR|BITS32,IMMEDIATE,0,0}, nasm_bytecodes+13604, IF_386|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS64,MEMORY,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+13610, IF_X64|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS64,MEMORY,SBYTE64,0,0}, nasm_bytecodes+13610, IF_X64|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS64,MEMORY,IMMEDIATE|BITS32,0,0}, nasm_bytecodes+13616, IF_X64|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS64,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+13622, IF_X64|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS64,REG_GPR|BITS64,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+13610, IF_X64},
    {I_IMUL, 3, {REG_GPR|BITS64,REG_GPR|BITS64,SBYTE64,0,0}, nasm_bytecodes+13610, IF_X64|IF_SM},
    {I_IMUL, 3, {REG_GPR|BITS64,REG_GPR|BITS64,IMMEDIATE|BITS32,0,0}, nasm_bytecodes+13616, IF_X64},
    {I_IMUL, 3, {REG_GPR|BITS64,REG_GPR|BITS64,IMMEDIATE,0,0}, nasm_bytecodes+13622, IF_X64|IF_SM},
    {I_IMUL, 2, {REG_GPR|BITS16,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13628, IF_186},
    {I_IMUL, 2, {REG_GPR|BITS16,SBYTE16,0,0,0}, nasm_bytecodes+13628, IF_186|IF_SM},
    {I_IMUL, 2, {REG_GPR|BITS16,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+13634, IF_186},
    {I_IMUL, 2, {REG_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+13640, IF_186|IF_SM},
    {I_IMUL, 2, {REG_GPR|BITS32,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13646, IF_386},
    {I_IMUL, 2, {REG_GPR|BITS32,SBYTE32,0,0,0}, nasm_bytecodes+13646, IF_386|IF_SM},
    {I_IMUL, 2, {REG_GPR|BITS32,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+13652, IF_386},
    {I_IMUL, 2, {REG_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+13658, IF_386|IF_SM},
    {I_IMUL, 2, {REG_GPR|BITS64,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13664, IF_X64},
    {I_IMUL, 2, {REG_GPR|BITS64,SBYTE64,0,0,0}, nasm_bytecodes+13664, IF_X64|IF_SM},
    {I_IMUL, 2, {REG_GPR|BITS64,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+13670, IF_X64},
    {I_IMUL, 2, {REG_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+13676, IF_X64|IF_SM},
    ITEMPLATE_END
};

static const struct itemplate instrux_IN[] = {
    {I_IN, 2, {REG_AL,IMMEDIATE,0,0,0}, nasm_bytecodes+19232, IF_8086|IF_SB},
    {I_IN, 2, {REG_AX,IMMEDIATE,0,0,0}, nasm_bytecodes+17497, IF_8086|IF_SB},
    {I_IN, 2, {REG_EAX,IMMEDIATE,0,0,0}, nasm_bytecodes+17502, IF_386|IF_SB},
    {I_IN, 2, {REG_AL,REG_DX,0,0,0}, nasm_bytecodes+19707, IF_8086},
    {I_IN, 2, {REG_AX,REG_DX,0,0,0}, nasm_bytecodes+19236, IF_8086},
    {I_IN, 2, {REG_EAX,REG_DX,0,0,0}, nasm_bytecodes+19240, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_INC[] = {
    {I_INC, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+19244, IF_8086|IF_NOLONG},
    {I_INC, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+19248, IF_386|IF_NOLONG},
    {I_INC, 1, {RM_GPR|BITS8,0,0,0,0}, nasm_bytecodes+19252, IF_8086},
    {I_INC, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17507, IF_8086},
    {I_INC, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+17512, IF_386},
    {I_INC, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+17517, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_INCBIN[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_INSB[] = {
    {I_INSB, 0, {0,0,0,0,0}, nasm_bytecodes+19710, IF_186},
    ITEMPLATE_END
};

static const struct itemplate instrux_INSD[] = {
    {I_INSD, 0, {0,0,0,0,0}, nasm_bytecodes+19256, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_INSERTPS[] = {
    {I_INSERTPS, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5720, IF_SSE41|IF_SD},
    ITEMPLATE_END
};

static const struct itemplate instrux_INSERTQ[] = {
    {I_INSERTQ, 4, {XMMREG,XMMREG,IMMEDIATE,IMMEDIATE,0}, nasm_bytecodes+5680, IF_SSE4A|IF_AMD},
    {I_INSERTQ, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+15542, IF_SSE4A|IF_AMD},
    ITEMPLATE_END
};

static const struct itemplate instrux_INSW[] = {
    {I_INSW, 0, {0,0,0,0,0}, nasm_bytecodes+19260, IF_186},
    ITEMPLATE_END
};

static const struct itemplate instrux_INT[] = {
    {I_INT, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+19264, IF_8086|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_INT01[] = {
    {I_INT01, 0, {0,0,0,0,0}, nasm_bytecodes+19704, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_INT03[] = {
    {I_INT03, 0, {0,0,0,0,0}, nasm_bytecodes+19713, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_INT1[] = {
    {I_INT1, 0, {0,0,0,0,0}, nasm_bytecodes+19704, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_INT3[] = {
    {I_INT3, 0, {0,0,0,0,0}, nasm_bytecodes+19713, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_INTO[] = {
    {I_INTO, 0, {0,0,0,0,0}, nasm_bytecodes+19716, IF_8086|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_INVD[] = {
    {I_INVD, 0, {0,0,0,0,0}, nasm_bytecodes+19268, IF_486|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_INVEPT[] = {
    {I_INVEPT, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+5641, IF_VMX|IF_SO|IF_NOLONG},
    {I_INVEPT, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+5640, IF_VMX|IF_SO|IF_LONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_INVLPG[] = {
    {I_INVLPG, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17522, IF_486|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_INVLPGA[] = {
    {I_INVLPGA, 2, {REG_AX,REG_ECX,0,0,0}, nasm_bytecodes+13682, IF_X86_64|IF_AMD|IF_NOLONG},
    {I_INVLPGA, 2, {REG_EAX,REG_ECX,0,0,0}, nasm_bytecodes+13688, IF_X86_64|IF_AMD},
    {I_INVLPGA, 2, {REG_RAX,REG_ECX,0,0,0}, nasm_bytecodes+7004, IF_X64|IF_AMD},
    {I_INVLPGA, 0, {0,0,0,0,0}, nasm_bytecodes+13689, IF_X86_64|IF_AMD},
    ITEMPLATE_END
};

static const struct itemplate instrux_INVVPID[] = {
    {I_INVVPID, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+5649, IF_VMX|IF_SO|IF_NOLONG},
    {I_INVVPID, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+5648, IF_VMX|IF_SO|IF_LONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_IRET[] = {
    {I_IRET, 0, {0,0,0,0,0}, nasm_bytecodes+19272, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_IRETD[] = {
    {I_IRETD, 0, {0,0,0,0,0}, nasm_bytecodes+19276, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_IRETQ[] = {
    {I_IRETQ, 0, {0,0,0,0,0}, nasm_bytecodes+19280, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_IRETW[] = {
    {I_IRETW, 0, {0,0,0,0,0}, nasm_bytecodes+19284, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_JCXZ[] = {
    {I_JCXZ, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+17527, IF_8086|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_JECXZ[] = {
    {I_JECXZ, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+17532, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_JMP[] = {
    {I_JMP, 1, {IMMEDIATE|SHORT,0,0,0,0}, nasm_bytecodes+17543, IF_8086},
    {I_JMP, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+17542, IF_8086},
    {I_JMP, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+17547, IF_8086},
    {I_JMP, 1, {IMMEDIATE|NEAR,0,0,0,0}, nasm_bytecodes+17547, IF_8086},
    {I_JMP, 1, {IMMEDIATE|FAR,0,0,0,0}, nasm_bytecodes+13694, IF_8086|IF_NOLONG},
    {I_JMP, 1, {IMMEDIATE|BITS16,0,0,0,0}, nasm_bytecodes+17552, IF_8086},
    {I_JMP, 1, {IMMEDIATE|BITS16|NEAR,0,0,0,0}, nasm_bytecodes+17552, IF_8086},
    {I_JMP, 1, {IMMEDIATE|BITS16|FAR,0,0,0,0}, nasm_bytecodes+13700, IF_8086|IF_NOLONG},
    {I_JMP, 1, {IMMEDIATE|BITS32,0,0,0,0}, nasm_bytecodes+17557, IF_386},
    {I_JMP, 1, {IMMEDIATE|BITS32|NEAR,0,0,0,0}, nasm_bytecodes+17557, IF_386},
    {I_JMP, 1, {IMMEDIATE|BITS32|FAR,0,0,0,0}, nasm_bytecodes+13706, IF_386|IF_NOLONG},
    {I_JMP, 2, {IMMEDIATE|COLON,IMMEDIATE,0,0,0}, nasm_bytecodes+13712, IF_8086|IF_NOLONG},
    {I_JMP, 2, {IMMEDIATE|BITS16|COLON,IMMEDIATE,0,0,0}, nasm_bytecodes+13718, IF_8086|IF_NOLONG},
    {I_JMP, 2, {IMMEDIATE|COLON,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+13718, IF_8086|IF_NOLONG},
    {I_JMP, 2, {IMMEDIATE|BITS32|COLON,IMMEDIATE,0,0,0}, nasm_bytecodes+13724, IF_386|IF_NOLONG},
    {I_JMP, 2, {IMMEDIATE|COLON,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+13724, IF_386|IF_NOLONG},
    {I_JMP, 1, {MEMORY|FAR,0,0,0,0}, nasm_bytecodes+17562, IF_8086|IF_NOLONG},
    {I_JMP, 1, {MEMORY|FAR,0,0,0,0}, nasm_bytecodes+17567, IF_X64},
    {I_JMP, 1, {MEMORY|BITS16|FAR,0,0,0,0}, nasm_bytecodes+17572, IF_8086},
    {I_JMP, 1, {MEMORY|BITS32|FAR,0,0,0,0}, nasm_bytecodes+17577, IF_386},
    {I_JMP, 1, {MEMORY|BITS64|FAR,0,0,0,0}, nasm_bytecodes+17567, IF_X64},
    {I_JMP, 1, {MEMORY|NEAR,0,0,0,0}, nasm_bytecodes+17582, IF_8086},
    {I_JMP, 1, {MEMORY|BITS16|NEAR,0,0,0,0}, nasm_bytecodes+17587, IF_8086},
    {I_JMP, 1, {MEMORY|BITS32|NEAR,0,0,0,0}, nasm_bytecodes+17592, IF_386|IF_NOLONG},
    {I_JMP, 1, {MEMORY|BITS64|NEAR,0,0,0,0}, nasm_bytecodes+17597, IF_X64},
    {I_JMP, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17587, IF_8086},
    {I_JMP, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+17592, IF_386|IF_NOLONG},
    {I_JMP, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+17597, IF_X64},
    {I_JMP, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17582, IF_8086},
    {I_JMP, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+17587, IF_8086},
    {I_JMP, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+17592, IF_386|IF_NOLONG},
    {I_JMP, 1, {MEMORY|BITS64,0,0,0,0}, nasm_bytecodes+17597, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_JMPE[] = {
    {I_JMPE, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+13730, IF_IA64},
    {I_JMPE, 1, {IMMEDIATE|BITS16,0,0,0,0}, nasm_bytecodes+13736, IF_IA64},
    {I_JMPE, 1, {IMMEDIATE|BITS32,0,0,0,0}, nasm_bytecodes+13742, IF_IA64},
    {I_JMPE, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+13748, IF_IA64},
    {I_JMPE, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+13754, IF_IA64},
    ITEMPLATE_END
};

static const struct itemplate instrux_JRCXZ[] = {
    {I_JRCXZ, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+17537, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LAHF[] = {
    {I_LAHF, 0, {0,0,0,0,0}, nasm_bytecodes+19719, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_LAR[] = {
    {I_LAR, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+13760, IF_286|IF_PROT|IF_SW},
    {I_LAR, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13760, IF_286|IF_PROT},
    {I_LAR, 2, {REG_GPR|BITS16,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13760, IF_386|IF_PROT},
    {I_LAR, 2, {REG_GPR|BITS16,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+7011, IF_X64|IF_PROT},
    {I_LAR, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+13766, IF_386|IF_PROT|IF_SW},
    {I_LAR, 2, {REG_GPR|BITS32,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13766, IF_386|IF_PROT},
    {I_LAR, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13766, IF_386|IF_PROT},
    {I_LAR, 2, {REG_GPR|BITS32,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+7018, IF_X64|IF_PROT},
    {I_LAR, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+13772, IF_X64|IF_PROT|IF_SW},
    {I_LAR, 2, {REG_GPR|BITS64,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13772, IF_X64|IF_PROT},
    {I_LAR, 2, {REG_GPR|BITS64,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13772, IF_X64|IF_PROT},
    {I_LAR, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13772, IF_X64|IF_PROT},
    ITEMPLATE_END
};

static const struct itemplate instrux_LDDQU[] = {
    {I_LDDQU, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+15500, IF_PRESCOTT|IF_SSE3|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_LDMXCSR[] = {
    {I_LDMXCSR, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+12536, IF_KATMAI|IF_SSE|IF_SD},
    ITEMPLATE_END
};

static const struct itemplate instrux_LDS[] = {
    {I_LDS, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+17602, IF_8086|IF_NOLONG},
    {I_LDS, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+17607, IF_386|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_LEA[] = {
    {I_LEA, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+17612, IF_8086},
    {I_LEA, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+17617, IF_386},
    {I_LEA, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+17622, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LEAVE[] = {
    {I_LEAVE, 0, {0,0,0,0,0}, nasm_bytecodes+17859, IF_186},
    ITEMPLATE_END
};

static const struct itemplate instrux_LES[] = {
    {I_LES, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+17627, IF_8086|IF_NOLONG},
    {I_LES, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+17632, IF_386|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_LFENCE[] = {
    {I_LFENCE, 0, {0,0,0,0,0}, nasm_bytecodes+17637, IF_X64|IF_AMD},
    {I_LFENCE, 0, {0,0,0,0,0}, nasm_bytecodes+17637, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_LFS[] = {
    {I_LFS, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+13778, IF_386},
    {I_LFS, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+13784, IF_386},
    {I_LFS, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+13790, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LGDT[] = {
    {I_LGDT, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17642, IF_286|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_LGS[] = {
    {I_LGS, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+13796, IF_386},
    {I_LGS, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+13802, IF_386},
    {I_LGS, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+13808, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LIDT[] = {
    {I_LIDT, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17647, IF_286|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_LLDT[] = {
    {I_LLDT, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17652, IF_286|IF_PROT|IF_PRIV},
    {I_LLDT, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+17652, IF_286|IF_PROT|IF_PRIV},
    {I_LLDT, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17652, IF_286|IF_PROT|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_LLWPCB[] = {
    {I_LLWPCB, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+12562, IF_AMD|IF_386},
    {I_LLWPCB, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+12569, IF_AMD|IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LMSW[] = {
    {I_LMSW, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17657, IF_286|IF_PRIV},
    {I_LMSW, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+17657, IF_286|IF_PRIV},
    {I_LMSW, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17657, IF_286|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_LOADALL[] = {
    {I_LOADALL, 0, {0,0,0,0,0}, nasm_bytecodes+19288, IF_386|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_LOADALL286[] = {
    {I_LOADALL286, 0, {0,0,0,0,0}, nasm_bytecodes+19292, IF_286|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_LODSB[] = {
    {I_LODSB, 0, {0,0,0,0,0}, nasm_bytecodes+19722, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_LODSD[] = {
    {I_LODSD, 0, {0,0,0,0,0}, nasm_bytecodes+19296, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_LODSQ[] = {
    {I_LODSQ, 0, {0,0,0,0,0}, nasm_bytecodes+19300, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LODSW[] = {
    {I_LODSW, 0, {0,0,0,0,0}, nasm_bytecodes+19304, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_LOOP[] = {
    {I_LOOP, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+17662, IF_8086},
    {I_LOOP, 2, {IMMEDIATE,REG_CX,0,0,0}, nasm_bytecodes+17667, IF_8086|IF_NOLONG},
    {I_LOOP, 2, {IMMEDIATE,REG_ECX,0,0,0}, nasm_bytecodes+17672, IF_386},
    {I_LOOP, 2, {IMMEDIATE,REG_RCX,0,0,0}, nasm_bytecodes+17677, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LOOPE[] = {
    {I_LOOPE, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+17682, IF_8086},
    {I_LOOPE, 2, {IMMEDIATE,REG_CX,0,0,0}, nasm_bytecodes+17687, IF_8086|IF_NOLONG},
    {I_LOOPE, 2, {IMMEDIATE,REG_ECX,0,0,0}, nasm_bytecodes+17692, IF_386},
    {I_LOOPE, 2, {IMMEDIATE,REG_RCX,0,0,0}, nasm_bytecodes+17697, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LOOPNE[] = {
    {I_LOOPNE, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+17702, IF_8086},
    {I_LOOPNE, 2, {IMMEDIATE,REG_CX,0,0,0}, nasm_bytecodes+17707, IF_8086|IF_NOLONG},
    {I_LOOPNE, 2, {IMMEDIATE,REG_ECX,0,0,0}, nasm_bytecodes+17712, IF_386},
    {I_LOOPNE, 2, {IMMEDIATE,REG_RCX,0,0,0}, nasm_bytecodes+17717, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LOOPNZ[] = {
    {I_LOOPNZ, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+17702, IF_8086},
    {I_LOOPNZ, 2, {IMMEDIATE,REG_CX,0,0,0}, nasm_bytecodes+17707, IF_8086|IF_NOLONG},
    {I_LOOPNZ, 2, {IMMEDIATE,REG_ECX,0,0,0}, nasm_bytecodes+17712, IF_386},
    {I_LOOPNZ, 2, {IMMEDIATE,REG_RCX,0,0,0}, nasm_bytecodes+17717, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LOOPZ[] = {
    {I_LOOPZ, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+17682, IF_8086},
    {I_LOOPZ, 2, {IMMEDIATE,REG_CX,0,0,0}, nasm_bytecodes+17687, IF_8086|IF_NOLONG},
    {I_LOOPZ, 2, {IMMEDIATE,REG_ECX,0,0,0}, nasm_bytecodes+17692, IF_386},
    {I_LOOPZ, 2, {IMMEDIATE,REG_RCX,0,0,0}, nasm_bytecodes+17697, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LSL[] = {
    {I_LSL, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+13814, IF_286|IF_PROT|IF_SW},
    {I_LSL, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13814, IF_286|IF_PROT},
    {I_LSL, 2, {REG_GPR|BITS16,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13814, IF_386|IF_PROT},
    {I_LSL, 2, {REG_GPR|BITS16,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+7025, IF_X64|IF_PROT},
    {I_LSL, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+13820, IF_386|IF_PROT|IF_SW},
    {I_LSL, 2, {REG_GPR|BITS32,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13820, IF_386|IF_PROT},
    {I_LSL, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13820, IF_386|IF_PROT},
    {I_LSL, 2, {REG_GPR|BITS32,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+7032, IF_X64|IF_PROT},
    {I_LSL, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+13826, IF_X64|IF_PROT|IF_SW},
    {I_LSL, 2, {REG_GPR|BITS64,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+13826, IF_X64|IF_PROT},
    {I_LSL, 2, {REG_GPR|BITS64,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13826, IF_X64|IF_PROT},
    {I_LSL, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13826, IF_X64|IF_PROT},
    ITEMPLATE_END
};

static const struct itemplate instrux_LSS[] = {
    {I_LSS, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+13832, IF_386},
    {I_LSS, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+13838, IF_386},
    {I_LSS, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+13844, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LTR[] = {
    {I_LTR, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17722, IF_286|IF_PROT|IF_PRIV},
    {I_LTR, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+17722, IF_286|IF_PROT|IF_PRIV},
    {I_LTR, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17722, IF_286|IF_PROT|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_LWPINS[] = {
    {I_LWPINS, 3, {REG_GPR|BITS32,RM_GPR|BITS32,IMMEDIATE|BITS32,0,0}, nasm_bytecodes+6712, IF_AMD|IF_386},
    {I_LWPINS, 3, {REG_GPR|BITS64,RM_GPR|BITS32,IMMEDIATE|BITS32,0,0}, nasm_bytecodes+6720, IF_AMD|IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LWPVAL[] = {
    {I_LWPVAL, 3, {REG_GPR|BITS32,RM_GPR|BITS32,IMMEDIATE|BITS32,0,0}, nasm_bytecodes+6696, IF_AMD|IF_386},
    {I_LWPVAL, 3, {REG_GPR|BITS64,RM_GPR|BITS32,IMMEDIATE|BITS32,0,0}, nasm_bytecodes+6704, IF_AMD|IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_LZCNT[] = {
    {I_LZCNT, 2, {REG_GPR|BITS16,RM_GPR|BITS16,0,0,0}, nasm_bytecodes+8096, IF_P6|IF_AMD},
    {I_LZCNT, 2, {REG_GPR|BITS32,RM_GPR|BITS32,0,0,0}, nasm_bytecodes+8103, IF_P6|IF_AMD},
    {I_LZCNT, 2, {REG_GPR|BITS64,RM_GPR|BITS64,0,0,0}, nasm_bytecodes+8110, IF_X64|IF_AMD},
    ITEMPLATE_END
};

static const struct itemplate instrux_MASKMOVDQU[] = {
    {I_MASKMOVDQU, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14768, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MASKMOVQ[] = {
    {I_MASKMOVQ, 2, {MMXREG,MMXREG,0,0,0}, nasm_bytecodes+14750, IF_KATMAI|IF_MMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_MAXPD[] = {
    {I_MAXPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15308, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_MAXPS[] = {
    {I_MAXPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14528, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MAXSD[] = {
    {I_MAXSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15314, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MAXSS[] = {
    {I_MAXSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14534, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MFENCE[] = {
    {I_MFENCE, 0, {0,0,0,0,0}, nasm_bytecodes+17727, IF_X64|IF_AMD},
    {I_MFENCE, 0, {0,0,0,0,0}, nasm_bytecodes+17727, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MINPD[] = {
    {I_MINPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15320, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_MINPS[] = {
    {I_MINPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14540, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MINSD[] = {
    {I_MINSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15326, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MINSS[] = {
    {I_MINSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14546, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MONITOR[] = {
    {I_MONITOR, 0, {0,0,0,0,0}, nasm_bytecodes+17732, IF_PRESCOTT},
    {I_MONITOR, 3, {REG_EAX,REG_ECX,REG_EDX,0,0}, nasm_bytecodes+17732, IF_PRESCOTT},
    {I_MONITOR, 3, {REG_RAX,REG_ECX,REG_EDX,0,0}, nasm_bytecodes+17732, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_MONTMUL[] = {
    {I_MONTMUL, 0, {0,0,0,0,0}, nasm_bytecodes+15620, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOV[] = {
    {I_MOV, 2, {MEMORY,REG_SREG,0,0,0}, nasm_bytecodes+17743, IF_8086|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS16,REG_SREG,0,0,0}, nasm_bytecodes+17737, IF_8086},
    {I_MOV, 2, {REG_GPR|BITS32,REG_SREG,0,0,0}, nasm_bytecodes+17742, IF_386},
    {I_MOV, 2, {REG_SREG,MEMORY,0,0,0}, nasm_bytecodes+19308, IF_8086|IF_SM},
    {I_MOV, 2, {REG_SREG,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+19308, IF_8086},
    {I_MOV, 2, {REG_SREG,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+19308, IF_386},
    {I_MOV, 2, {REG_AL,MEM_OFFS,0,0,0}, nasm_bytecodes+19312, IF_8086|IF_SM},
    {I_MOV, 2, {REG_AX,MEM_OFFS,0,0,0}, nasm_bytecodes+17747, IF_8086|IF_SM},
    {I_MOV, 2, {REG_EAX,MEM_OFFS,0,0,0}, nasm_bytecodes+17752, IF_386|IF_SM},
    {I_MOV, 2, {REG_RAX,MEM_OFFS,0,0,0}, nasm_bytecodes+17757, IF_X64|IF_SM},
    {I_MOV, 2, {MEM_OFFS,REG_AL,0,0,0}, nasm_bytecodes+19316, IF_8086|IF_SM},
    {I_MOV, 2, {MEM_OFFS,REG_AX,0,0,0}, nasm_bytecodes+17762, IF_8086|IF_SM},
    {I_MOV, 2, {MEM_OFFS,REG_EAX,0,0,0}, nasm_bytecodes+17767, IF_386|IF_SM},
    {I_MOV, 2, {MEM_OFFS,REG_RAX,0,0,0}, nasm_bytecodes+17772, IF_X64|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS32,REG_CREG,0,0,0}, nasm_bytecodes+13850, IF_386|IF_PRIV|IF_NOLONG},
    {I_MOV, 2, {REG_GPR|BITS64,REG_CREG,0,0,0}, nasm_bytecodes+13856, IF_X64|IF_PRIV},
    {I_MOV, 2, {REG_CREG,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13862, IF_386|IF_PRIV|IF_NOLONG},
    {I_MOV, 2, {REG_CREG,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13868, IF_X64|IF_PRIV},
    {I_MOV, 2, {REG_GPR|BITS32,REG_DREG,0,0,0}, nasm_bytecodes+13875, IF_386|IF_PRIV|IF_NOLONG},
    {I_MOV, 2, {REG_GPR|BITS64,REG_DREG,0,0,0}, nasm_bytecodes+13874, IF_X64|IF_PRIV},
    {I_MOV, 2, {REG_DREG,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13881, IF_386|IF_PRIV|IF_NOLONG},
    {I_MOV, 2, {REG_DREG,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+13880, IF_X64|IF_PRIV},
    {I_MOV, 2, {REG_GPR|BITS32,REG_TREG,0,0,0}, nasm_bytecodes+17777, IF_386|IF_NOLONG},
    {I_MOV, 2, {REG_TREG,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+17782, IF_386|IF_NOLONG},
    {I_MOV, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19320, IF_8086|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19320, IF_8086},
    {I_MOV, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+17787, IF_8086|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+17787, IF_8086},
    {I_MOV, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+17792, IF_386|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+17792, IF_386},
    {I_MOV, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+17797, IF_X64|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+17797, IF_X64},
    {I_MOV, 2, {REG_GPR|BITS8,MEMORY,0,0,0}, nasm_bytecodes+19324, IF_8086|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19324, IF_8086},
    {I_MOV, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+17802, IF_8086|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+17802, IF_8086},
    {I_MOV, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+17807, IF_386|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+17807, IF_386},
    {I_MOV, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+17812, IF_X64|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+17812, IF_X64},
    {I_MOV, 2, {REG_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+19328, IF_8086|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+17817, IF_8086|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+17822, IF_386|IF_SM},
    {I_MOV, 2, {REG_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+17827, IF_X64|IF_SM},
    {I_MOV, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+17832, IF_8086|IF_SM},
    {I_MOV, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+13886, IF_8086|IF_SM},
    {I_MOV, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+13892, IF_386|IF_SM},
    {I_MOV, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+13898, IF_X64|IF_SM},
    {I_MOV, 2, {RM_GPR|BITS64,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+13898, IF_X64},
    {I_MOV, 2, {MEMORY,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+17832, IF_8086|IF_SM},
    {I_MOV, 2, {MEMORY,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+13886, IF_8086|IF_SM},
    {I_MOV, 2, {MEMORY,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+13892, IF_386|IF_SM},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVAPD[] = {
    {I_MOVAPD, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+15332, IF_WILLAMETTE|IF_SSE2},
    {I_MOVAPD, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+15338, IF_WILLAMETTE|IF_SSE2},
    {I_MOVAPD, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+15338, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_MOVAPD, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+15332, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVAPS[] = {
    {I_MOVAPS, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+14552, IF_KATMAI|IF_SSE},
    {I_MOVAPS, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+14558, IF_KATMAI|IF_SSE},
    {I_MOVAPS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14552, IF_KATMAI|IF_SSE},
    {I_MOVAPS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14558, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVBE[] = {
    {I_MOVBE, 2, {REG_GPR|BITS16,MEMORY|BITS16,0,0,0}, nasm_bytecodes+8355, IF_NEHALEM|IF_SM},
    {I_MOVBE, 2, {REG_GPR|BITS32,MEMORY|BITS32,0,0,0}, nasm_bytecodes+8362, IF_NEHALEM|IF_SM},
    {I_MOVBE, 2, {REG_GPR|BITS64,MEMORY|BITS64,0,0,0}, nasm_bytecodes+8369, IF_NEHALEM|IF_SM},
    {I_MOVBE, 2, {MEMORY|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+8376, IF_NEHALEM|IF_SM},
    {I_MOVBE, 2, {MEMORY|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+8383, IF_NEHALEM|IF_SM},
    {I_MOVBE, 2, {MEMORY|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+8390, IF_NEHALEM|IF_SM},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVD[] = {
    {I_MOVD, 2, {MMXREG,MEMORY,0,0,0}, nasm_bytecodes+13904, IF_PENT|IF_MMX|IF_SD},
    {I_MOVD, 2, {MMXREG,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+13904, IF_PENT|IF_MMX},
    {I_MOVD, 2, {MEMORY,MMXREG,0,0,0}, nasm_bytecodes+13910, IF_PENT|IF_MMX|IF_SD},
    {I_MOVD, 2, {REG_GPR|BITS32,MMXREG,0,0,0}, nasm_bytecodes+13910, IF_PENT|IF_MMX},
    {I_MOVD, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+7039, IF_X64|IF_SD},
    {I_MOVD, 2, {XMMREG,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+7039, IF_X64},
    {I_MOVD, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+7046, IF_X64|IF_SD},
    {I_MOVD, 2, {REG_GPR|BITS32,XMMREG,0,0,0}, nasm_bytecodes+7046, IF_X64|IF_SSE},
    {I_MOVD, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+7676, IF_WILLAMETTE|IF_SSE2|IF_SD},
    {I_MOVD, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+7683, IF_WILLAMETTE|IF_SSE2|IF_SD},
    {I_MOVD, 2, {XMMREG,RM_GPR|BITS32,0,0,0}, nasm_bytecodes+7683, IF_WILLAMETTE|IF_SSE2},
    {I_MOVD, 2, {RM_GPR|BITS32,XMMREG,0,0,0}, nasm_bytecodes+7676, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVDDUP[] = {
    {I_MOVDDUP, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15506, IF_PRESCOTT|IF_SSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVDQ2Q[] = {
    {I_MOVDQ2Q, 2, {MMXREG,XMMREG,0,0,0}, nasm_bytecodes+14810, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVDQA[] = {
    {I_MOVDQA, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14786, IF_WILLAMETTE|IF_SSE2},
    {I_MOVDQA, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+14792, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_MOVDQA, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+14786, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_MOVDQA, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14792, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVDQU[] = {
    {I_MOVDQU, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14798, IF_WILLAMETTE|IF_SSE2},
    {I_MOVDQU, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+14804, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_MOVDQU, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+14798, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_MOVDQU, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14804, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVHLPS[] = {
    {I_MOVHLPS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14384, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVHPD[] = {
    {I_MOVHPD, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+15344, IF_WILLAMETTE|IF_SSE2},
    {I_MOVHPD, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+15350, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVHPS[] = {
    {I_MOVHPS, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+14564, IF_KATMAI|IF_SSE},
    {I_MOVHPS, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+14570, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVLHPS[] = {
    {I_MOVLHPS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14564, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVLPD[] = {
    {I_MOVLPD, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+15356, IF_WILLAMETTE|IF_SSE2},
    {I_MOVLPD, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+15362, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVLPS[] = {
    {I_MOVLPS, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+14384, IF_KATMAI|IF_SSE},
    {I_MOVLPS, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+14576, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVMSKPD[] = {
    {I_MOVMSKPD, 2, {REG_GPR|BITS32,XMMREG,0,0,0}, nasm_bytecodes+15368, IF_WILLAMETTE|IF_SSE2},
    {I_MOVMSKPD, 2, {REG_GPR|BITS64,XMMREG,0,0,0}, nasm_bytecodes+7858, IF_X64|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVMSKPS[] = {
    {I_MOVMSKPS, 2, {REG_GPR|BITS32,XMMREG,0,0,0}, nasm_bytecodes+14582, IF_KATMAI|IF_SSE},
    {I_MOVMSKPS, 2, {REG_GPR|BITS64,XMMREG,0,0,0}, nasm_bytecodes+7585, IF_X64|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVNTDQ[] = {
    {I_MOVNTDQ, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+14774, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVNTDQA[] = {
    {I_MOVNTDQA, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+8131, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVNTI[] = {
    {I_MOVNTI, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+7670, IF_WILLAMETTE|IF_SD},
    {I_MOVNTI, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+7669, IF_X64|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVNTPD[] = {
    {I_MOVNTPD, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+14780, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVNTPS[] = {
    {I_MOVNTPS, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+14588, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVNTQ[] = {
    {I_MOVNTQ, 2, {MEMORY,MMXREG,0,0,0}, nasm_bytecodes+14756, IF_KATMAI|IF_MMX|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVNTSD[] = {
    {I_MOVNTSD, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+15548, IF_SSE4A|IF_AMD|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVNTSS[] = {
    {I_MOVNTSS, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+15554, IF_SSE4A|IF_AMD|IF_SD},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVQ[] = {
    {I_MOVQ, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7053, IF_PENT|IF_MMX|IF_SQ},
    {I_MOVQ, 2, {RM_MMX,MMXREG,0,0,0}, nasm_bytecodes+7060, IF_PENT|IF_MMX|IF_SQ},
    {I_MOVQ, 2, {MMXREG,RM_GPR|BITS64,0,0,0}, nasm_bytecodes+13904, IF_X64|IF_MMX},
    {I_MOVQ, 2, {RM_GPR|BITS64,MMXREG,0,0,0}, nasm_bytecodes+13910, IF_X64|IF_MMX},
    {I_MOVQ, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14816, IF_WILLAMETTE|IF_SSE2},
    {I_MOVQ, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14822, IF_WILLAMETTE|IF_SSE2},
    {I_MOVQ, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+14822, IF_WILLAMETTE|IF_SSE2|IF_SQ},
    {I_MOVQ, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+14816, IF_WILLAMETTE|IF_SSE2|IF_SQ},
    {I_MOVQ, 2, {XMMREG,RM_GPR|BITS64,0,0,0}, nasm_bytecodes+7690, IF_X64|IF_SSE2},
    {I_MOVQ, 2, {RM_GPR|BITS64,XMMREG,0,0,0}, nasm_bytecodes+7697, IF_X64|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVQ2DQ[] = {
    {I_MOVQ2DQ, 2, {XMMREG,MMXREG,0,0,0}, nasm_bytecodes+14828, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVSB[] = {
    {I_MOVSB, 0, {0,0,0,0,0}, nasm_bytecodes+5245, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVSD[] = {
    {I_MOVSD, 0, {0,0,0,0,0}, nasm_bytecodes+19332, IF_386},
    {I_MOVSD, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+15374, IF_WILLAMETTE|IF_SSE2},
    {I_MOVSD, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+15380, IF_WILLAMETTE|IF_SSE2},
    {I_MOVSD, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+15380, IF_WILLAMETTE|IF_SSE2},
    {I_MOVSD, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+15374, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVSHDUP[] = {
    {I_MOVSHDUP, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15512, IF_PRESCOTT|IF_SSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVSLDUP[] = {
    {I_MOVSLDUP, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15518, IF_PRESCOTT|IF_SSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVSQ[] = {
    {I_MOVSQ, 0, {0,0,0,0,0}, nasm_bytecodes+19336, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVSS[] = {
    {I_MOVSS, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+14594, IF_KATMAI|IF_SSE},
    {I_MOVSS, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+14600, IF_KATMAI|IF_SSE},
    {I_MOVSS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14594, IF_KATMAI|IF_SSE},
    {I_MOVSS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14600, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVSW[] = {
    {I_MOVSW, 0, {0,0,0,0,0}, nasm_bytecodes+19340, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVSX[] = {
    {I_MOVSX, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+13916, IF_386|IF_SB},
    {I_MOVSX, 2, {REG_GPR|BITS16,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+13916, IF_386},
    {I_MOVSX, 2, {REG_GPR|BITS32,RM_GPR|BITS8,0,0,0}, nasm_bytecodes+13922, IF_386},
    {I_MOVSX, 2, {REG_GPR|BITS32,RM_GPR|BITS16,0,0,0}, nasm_bytecodes+13928, IF_386},
    {I_MOVSX, 2, {REG_GPR|BITS64,RM_GPR|BITS8,0,0,0}, nasm_bytecodes+13934, IF_X64},
    {I_MOVSX, 2, {REG_GPR|BITS64,RM_GPR|BITS16,0,0,0}, nasm_bytecodes+13940, IF_X64},
    {I_MOVSX, 2, {REG_GPR|BITS64,RM_GPR|BITS32,0,0,0}, nasm_bytecodes+17837, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVSXD[] = {
    {I_MOVSXD, 2, {REG_GPR|BITS64,RM_GPR|BITS32,0,0,0}, nasm_bytecodes+17837, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVUPD[] = {
    {I_MOVUPD, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+15386, IF_WILLAMETTE|IF_SSE2},
    {I_MOVUPD, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+15392, IF_WILLAMETTE|IF_SSE2},
    {I_MOVUPD, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+15392, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_MOVUPD, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+15386, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVUPS[] = {
    {I_MOVUPS, 2, {XMMREG,MEMORY,0,0,0}, nasm_bytecodes+14606, IF_KATMAI|IF_SSE},
    {I_MOVUPS, 2, {MEMORY,XMMREG,0,0,0}, nasm_bytecodes+14612, IF_KATMAI|IF_SSE},
    {I_MOVUPS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14606, IF_KATMAI|IF_SSE},
    {I_MOVUPS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+14612, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MOVZX[] = {
    {I_MOVZX, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+13946, IF_386|IF_SB},
    {I_MOVZX, 2, {REG_GPR|BITS16,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+13946, IF_386},
    {I_MOVZX, 2, {REG_GPR|BITS32,RM_GPR|BITS8,0,0,0}, nasm_bytecodes+13952, IF_386},
    {I_MOVZX, 2, {REG_GPR|BITS32,RM_GPR|BITS16,0,0,0}, nasm_bytecodes+13958, IF_386},
    {I_MOVZX, 2, {REG_GPR|BITS64,RM_GPR|BITS8,0,0,0}, nasm_bytecodes+13964, IF_X64},
    {I_MOVZX, 2, {REG_GPR|BITS64,RM_GPR|BITS16,0,0,0}, nasm_bytecodes+13970, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_MPSADBW[] = {
    {I_MPSADBW, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5728, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_MUL[] = {
    {I_MUL, 1, {RM_GPR|BITS8,0,0,0,0}, nasm_bytecodes+19344, IF_8086},
    {I_MUL, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17842, IF_8086},
    {I_MUL, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+17847, IF_386},
    {I_MUL, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+17852, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_MULPD[] = {
    {I_MULPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15398, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_MULPS[] = {
    {I_MULPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14618, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MULSD[] = {
    {I_MULSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15404, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_MULSS[] = {
    {I_MULSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14624, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_MWAIT[] = {
    {I_MWAIT, 0, {0,0,0,0,0}, nasm_bytecodes+17857, IF_PRESCOTT},
    {I_MWAIT, 2, {REG_EAX,REG_ECX,0,0,0}, nasm_bytecodes+17857, IF_PRESCOTT},
    ITEMPLATE_END
};

static const struct itemplate instrux_NEG[] = {
    {I_NEG, 1, {RM_GPR|BITS8,0,0,0,0}, nasm_bytecodes+19348, IF_8086},
    {I_NEG, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17862, IF_8086},
    {I_NEG, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+17867, IF_386},
    {I_NEG, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+17872, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_NOP[] = {
    {I_NOP, 0, {0,0,0,0,0}, nasm_bytecodes+19352, IF_8086},
    {I_NOP, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+13976, IF_P6},
    {I_NOP, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+13982, IF_P6},
    {I_NOP, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+13988, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_NOT[] = {
    {I_NOT, 1, {RM_GPR|BITS8,0,0,0,0}, nasm_bytecodes+19356, IF_8086},
    {I_NOT, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17877, IF_8086},
    {I_NOT, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+17882, IF_386},
    {I_NOT, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+17887, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_OR[] = {
    {I_OR, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19360, IF_8086|IF_SM},
    {I_OR, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19360, IF_8086},
    {I_OR, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+17892, IF_8086|IF_SM},
    {I_OR, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+17892, IF_8086},
    {I_OR, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+17897, IF_386|IF_SM},
    {I_OR, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+17897, IF_386},
    {I_OR, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+17902, IF_X64|IF_SM},
    {I_OR, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+17902, IF_X64},
    {I_OR, 2, {REG_GPR|BITS8,MEMORY,0,0,0}, nasm_bytecodes+11067, IF_8086|IF_SM},
    {I_OR, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+11067, IF_8086},
    {I_OR, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+17907, IF_8086|IF_SM},
    {I_OR, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+17907, IF_8086},
    {I_OR, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+17912, IF_386|IF_SM},
    {I_OR, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+17912, IF_386},
    {I_OR, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+17917, IF_X64|IF_SM},
    {I_OR, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+17917, IF_X64},
    {I_OR, 2, {RM_GPR|BITS16,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+13994, IF_8086},
    {I_OR, 2, {RM_GPR|BITS32,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+14000, IF_386},
    {I_OR, 2, {RM_GPR|BITS64,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+14006, IF_X64},
    {I_OR, 2, {REG_AL,IMMEDIATE,0,0,0}, nasm_bytecodes+19364, IF_8086|IF_SM},
    {I_OR, 2, {REG_AX,SBYTE16,0,0,0}, nasm_bytecodes+13994, IF_8086|IF_SM},
    {I_OR, 2, {REG_AX,IMMEDIATE,0,0,0}, nasm_bytecodes+17922, IF_8086|IF_SM},
    {I_OR, 2, {REG_EAX,SBYTE32,0,0,0}, nasm_bytecodes+14000, IF_386|IF_SM},
    {I_OR, 2, {REG_EAX,IMMEDIATE,0,0,0}, nasm_bytecodes+17927, IF_386|IF_SM},
    {I_OR, 2, {REG_RAX,SBYTE64,0,0,0}, nasm_bytecodes+14006, IF_X64|IF_SM},
    {I_OR, 2, {REG_RAX,IMMEDIATE,0,0,0}, nasm_bytecodes+17932, IF_X64|IF_SM},
    {I_OR, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+17937, IF_8086|IF_SM},
    {I_OR, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14012, IF_8086|IF_SM},
    {I_OR, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14018, IF_386|IF_SM},
    {I_OR, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14024, IF_X64|IF_SM},
    {I_OR, 2, {MEMORY,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+17937, IF_8086|IF_SM},
    {I_OR, 2, {MEMORY,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+14012, IF_8086|IF_SM},
    {I_OR, 2, {MEMORY,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+14018, IF_386|IF_SM},
    ITEMPLATE_END
};

static const struct itemplate instrux_ORPD[] = {
    {I_ORPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15410, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_ORPS[] = {
    {I_ORPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14630, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_OUT[] = {
    {I_OUT, 2, {IMMEDIATE,REG_AL,0,0,0}, nasm_bytecodes+19368, IF_8086|IF_SB},
    {I_OUT, 2, {IMMEDIATE,REG_AX,0,0,0}, nasm_bytecodes+17942, IF_8086|IF_SB},
    {I_OUT, 2, {IMMEDIATE,REG_EAX,0,0,0}, nasm_bytecodes+17947, IF_386|IF_SB},
    {I_OUT, 2, {REG_DX,REG_AL,0,0,0}, nasm_bytecodes+19725, IF_8086},
    {I_OUT, 2, {REG_DX,REG_AX,0,0,0}, nasm_bytecodes+19372, IF_8086},
    {I_OUT, 2, {REG_DX,REG_EAX,0,0,0}, nasm_bytecodes+19376, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_OUTSB[] = {
    {I_OUTSB, 0, {0,0,0,0,0}, nasm_bytecodes+19728, IF_186},
    ITEMPLATE_END
};

static const struct itemplate instrux_OUTSD[] = {
    {I_OUTSD, 0, {0,0,0,0,0}, nasm_bytecodes+19380, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_OUTSW[] = {
    {I_OUTSW, 0, {0,0,0,0,0}, nasm_bytecodes+19384, IF_186},
    ITEMPLATE_END
};

static const struct itemplate instrux_PABSB[] = {
    {I_PABSB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7886, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PABSB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+7893, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PABSD[] = {
    {I_PABSD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7914, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PABSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+7921, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PABSW[] = {
    {I_PABSW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7900, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PABSW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+7907, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PACKSSDW[] = {
    {I_PACKSSDW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7067, IF_PENT|IF_MMX|IF_SQ},
    {I_PACKSSDW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14840, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PACKSSWB[] = {
    {I_PACKSSWB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7074, IF_PENT|IF_MMX|IF_SQ},
    {I_PACKSSWB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14834, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PACKUSDW[] = {
    {I_PACKUSDW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8138, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PACKUSWB[] = {
    {I_PACKUSWB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7081, IF_PENT|IF_MMX|IF_SQ},
    {I_PACKUSWB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14846, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PADDB[] = {
    {I_PADDB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7088, IF_PENT|IF_MMX|IF_SQ},
    {I_PADDB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14852, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PADDD[] = {
    {I_PADDD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7095, IF_PENT|IF_MMX|IF_SQ},
    {I_PADDD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14864, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PADDQ[] = {
    {I_PADDQ, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+14870, IF_WILLAMETTE|IF_MMX|IF_SQ},
    {I_PADDQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14876, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PADDSB[] = {
    {I_PADDSB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7102, IF_PENT|IF_MMX|IF_SQ},
    {I_PADDSB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14882, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PADDSIW[] = {
    {I_PADDSIW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+14030, IF_PENT|IF_MMX|IF_SQ|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PADDSW[] = {
    {I_PADDSW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7109, IF_PENT|IF_MMX|IF_SQ},
    {I_PADDSW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14888, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PADDUSB[] = {
    {I_PADDUSB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7116, IF_PENT|IF_MMX|IF_SQ},
    {I_PADDUSB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14894, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PADDUSW[] = {
    {I_PADDUSW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7123, IF_PENT|IF_MMX|IF_SQ},
    {I_PADDUSW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14900, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PADDW[] = {
    {I_PADDW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7130, IF_PENT|IF_MMX|IF_SQ},
    {I_PADDW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14858, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PALIGNR[] = {
    {I_PALIGNR, 3, {MMXREG,RM_MMX,IMMEDIATE,0,0}, nasm_bytecodes+5656, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PALIGNR, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5664, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PAND[] = {
    {I_PAND, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7137, IF_PENT|IF_MMX|IF_SQ},
    {I_PAND, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14906, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PANDN[] = {
    {I_PANDN, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7144, IF_PENT|IF_MMX|IF_SQ},
    {I_PANDN, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14912, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PAUSE[] = {
    {I_PAUSE, 0, {0,0,0,0,0}, nasm_bytecodes+17952, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_PAVEB[] = {
    {I_PAVEB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+14036, IF_PENT|IF_MMX|IF_SQ|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PAVGB[] = {
    {I_PAVGB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7599, IF_KATMAI|IF_MMX|IF_SQ},
    {I_PAVGB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14918, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PAVGUSB[] = {
    {I_PAVGUSB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5184, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PAVGW[] = {
    {I_PAVGW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7606, IF_KATMAI|IF_MMX|IF_SQ},
    {I_PAVGW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14924, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PBLENDVB[] = {
    {I_PBLENDVB, 3, {XMMREG,RM_XMM,XMM0,0,0}, nasm_bytecodes+8145, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PBLENDW[] = {
    {I_PBLENDW, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5736, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCLMULHQHQDQ[] = {
    {I_PCLMULHQHQDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+3627, IF_SSE|IF_WESTMERE},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCLMULHQLQDQ[] = {
    {I_PCLMULHQLQDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+3609, IF_SSE|IF_WESTMERE},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCLMULLQHQDQ[] = {
    {I_PCLMULLQHQDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+3618, IF_SSE|IF_WESTMERE},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCLMULLQLQDQ[] = {
    {I_PCLMULLQLQDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+3600, IF_SSE|IF_WESTMERE},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCLMULQDQ[] = {
    {I_PCLMULQDQ, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6656, IF_SSE|IF_WESTMERE},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCMPEQB[] = {
    {I_PCMPEQB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7151, IF_PENT|IF_MMX|IF_SQ},
    {I_PCMPEQB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14930, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCMPEQD[] = {
    {I_PCMPEQD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7158, IF_PENT|IF_MMX|IF_SQ},
    {I_PCMPEQD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14942, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCMPEQQ[] = {
    {I_PCMPEQQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8152, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCMPEQW[] = {
    {I_PCMPEQW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7165, IF_PENT|IF_MMX|IF_SQ},
    {I_PCMPEQW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14936, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCMPESTRI[] = {
    {I_PCMPESTRI, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5808, IF_SSE42},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCMPESTRM[] = {
    {I_PCMPESTRM, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5816, IF_SSE42},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCMPGTB[] = {
    {I_PCMPGTB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7172, IF_PENT|IF_MMX|IF_SQ},
    {I_PCMPGTB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14948, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCMPGTD[] = {
    {I_PCMPGTD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7179, IF_PENT|IF_MMX|IF_SQ},
    {I_PCMPGTD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14960, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCMPGTQ[] = {
    {I_PCMPGTQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8327, IF_SSE42},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCMPGTW[] = {
    {I_PCMPGTW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7186, IF_PENT|IF_MMX|IF_SQ},
    {I_PCMPGTW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14954, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCMPISTRI[] = {
    {I_PCMPISTRI, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5824, IF_SSE42},
    ITEMPLATE_END
};

static const struct itemplate instrux_PCMPISTRM[] = {
    {I_PCMPISTRM, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5832, IF_SSE42},
    ITEMPLATE_END
};

static const struct itemplate instrux_PDISTIB[] = {
    {I_PDISTIB, 2, {MMXREG,MEMORY,0,0,0}, nasm_bytecodes+15207, IF_PENT|IF_MMX|IF_SM|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PEXTRB[] = {
    {I_PEXTRB, 3, {REG_GPR|BITS32,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+10, IF_SSE41},
    {I_PEXTRB, 3, {MEMORY|BITS8,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+10, IF_SSE41},
    {I_PEXTRB, 3, {REG_GPR|BITS64,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+9, IF_SSE41|IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_PEXTRD[] = {
    {I_PEXTRD, 3, {RM_GPR|BITS32,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+19, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PEXTRQ[] = {
    {I_PEXTRQ, 3, {RM_GPR|BITS64,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+18, IF_SSE41|IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_PEXTRW[] = {
    {I_PEXTRW, 3, {REG_GPR|BITS32,MMXREG,IMMEDIATE,0,0}, nasm_bytecodes+7613, IF_KATMAI|IF_MMX|IF_SB|IF_AR2},
    {I_PEXTRW, 3, {REG_GPR|BITS32,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+7704, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR2},
    {I_PEXTRW, 3, {REG_GPR|BITS32,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+28, IF_SSE41},
    {I_PEXTRW, 3, {MEMORY|BITS16,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+28, IF_SSE41},
    {I_PEXTRW, 3, {REG_GPR|BITS64,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+27, IF_SSE41|IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_PF2ID[] = {
    {I_PF2ID, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5192, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PF2IW[] = {
    {I_PF2IW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5472, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFACC[] = {
    {I_PFACC, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5200, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFADD[] = {
    {I_PFADD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5208, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFCMPEQ[] = {
    {I_PFCMPEQ, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5216, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFCMPGE[] = {
    {I_PFCMPGE, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5224, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFCMPGT[] = {
    {I_PFCMPGT, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5232, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFMAX[] = {
    {I_PFMAX, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5240, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFMIN[] = {
    {I_PFMIN, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5248, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFMUL[] = {
    {I_PFMUL, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5256, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFNACC[] = {
    {I_PFNACC, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5480, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFPNACC[] = {
    {I_PFPNACC, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5488, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFRCP[] = {
    {I_PFRCP, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5264, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFRCPIT1[] = {
    {I_PFRCPIT1, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5272, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFRCPIT2[] = {
    {I_PFRCPIT2, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5280, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFRCPV[] = {
    {I_PFRCPV, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5840, IF_PENT|IF_3DNOW|IF_SQ|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFRSQIT1[] = {
    {I_PFRSQIT1, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5288, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFRSQRT[] = {
    {I_PFRSQRT, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5296, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFRSQRTV[] = {
    {I_PFRSQRTV, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5848, IF_PENT|IF_3DNOW|IF_SQ|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFSUB[] = {
    {I_PFSUB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5304, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PFSUBR[] = {
    {I_PFSUBR, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5312, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PHADDD[] = {
    {I_PHADDD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7942, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PHADDD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+7949, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PHADDSW[] = {
    {I_PHADDSW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7956, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PHADDSW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+7963, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PHADDW[] = {
    {I_PHADDW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7928, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PHADDW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+7935, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PHMINPOSUW[] = {
    {I_PHMINPOSUW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8159, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PHSUBD[] = {
    {I_PHSUBD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7984, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PHSUBD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+7991, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PHSUBSW[] = {
    {I_PHSUBSW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7998, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PHSUBSW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8005, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PHSUBW[] = {
    {I_PHSUBW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7970, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PHSUBW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+7977, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PI2FD[] = {
    {I_PI2FD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5320, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PI2FW[] = {
    {I_PI2FW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5496, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PINSRB[] = {
    {I_PINSRB, 3, {XMMREG,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+37, IF_SSE41|IF_SB|IF_AR2},
    {I_PINSRB, 3, {XMMREG,RM_GPR|BITS8,IMMEDIATE,0,0}, nasm_bytecodes+36, IF_SSE41|IF_SB|IF_AR2},
    {I_PINSRB, 3, {XMMREG,REG_GPR|BITS32,IMMEDIATE,0,0}, nasm_bytecodes+37, IF_SSE41|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_PINSRD[] = {
    {I_PINSRD, 3, {XMMREG,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+46, IF_SSE41|IF_SB|IF_AR2},
    {I_PINSRD, 3, {XMMREG,RM_GPR|BITS32,IMMEDIATE,0,0}, nasm_bytecodes+46, IF_SSE41|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_PINSRQ[] = {
    {I_PINSRQ, 3, {XMMREG,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+45, IF_SSE41|IF_X64|IF_SB|IF_AR2},
    {I_PINSRQ, 3, {XMMREG,RM_GPR|BITS64,IMMEDIATE,0,0}, nasm_bytecodes+45, IF_SSE41|IF_X64|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_PINSRW[] = {
    {I_PINSRW, 3, {MMXREG,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+7620, IF_KATMAI|IF_MMX|IF_SB|IF_AR2},
    {I_PINSRW, 3, {MMXREG,RM_GPR|BITS16,IMMEDIATE,0,0}, nasm_bytecodes+7620, IF_KATMAI|IF_MMX|IF_SB|IF_AR2},
    {I_PINSRW, 3, {MMXREG,REG_GPR|BITS32,IMMEDIATE,0,0}, nasm_bytecodes+7620, IF_KATMAI|IF_MMX|IF_SB|IF_AR2},
    {I_PINSRW, 3, {XMMREG,REG_GPR|BITS16,IMMEDIATE,0,0}, nasm_bytecodes+7711, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR2},
    {I_PINSRW, 3, {XMMREG,REG_GPR|BITS32,IMMEDIATE,0,0}, nasm_bytecodes+7711, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR2},
    {I_PINSRW, 3, {XMMREG,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+7711, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR2},
    {I_PINSRW, 3, {XMMREG,MEMORY|BITS16,IMMEDIATE,0,0}, nasm_bytecodes+7711, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMACHRIW[] = {
    {I_PMACHRIW, 2, {MMXREG,MEMORY,0,0,0}, nasm_bytecodes+15303, IF_PENT|IF_MMX|IF_SM|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMADDUBSW[] = {
    {I_PMADDUBSW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+8012, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PMADDUBSW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8019, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMADDWD[] = {
    {I_PMADDWD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7193, IF_PENT|IF_MMX|IF_SQ},
    {I_PMADDWD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14966, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMAGW[] = {
    {I_PMAGW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+14042, IF_PENT|IF_MMX|IF_SQ|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMAXSB[] = {
    {I_PMAXSB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8166, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMAXSD[] = {
    {I_PMAXSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8173, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMAXSW[] = {
    {I_PMAXSW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7627, IF_KATMAI|IF_MMX|IF_SQ},
    {I_PMAXSW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14972, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMAXUB[] = {
    {I_PMAXUB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7634, IF_KATMAI|IF_MMX|IF_SQ},
    {I_PMAXUB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14978, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMAXUD[] = {
    {I_PMAXUD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8180, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMAXUW[] = {
    {I_PMAXUW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8187, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMINSB[] = {
    {I_PMINSB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8194, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMINSD[] = {
    {I_PMINSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8201, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMINSW[] = {
    {I_PMINSW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7641, IF_KATMAI|IF_MMX|IF_SQ},
    {I_PMINSW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14984, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMINUB[] = {
    {I_PMINUB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7648, IF_KATMAI|IF_MMX|IF_SQ},
    {I_PMINUB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14990, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMINUD[] = {
    {I_PMINUD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8208, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMINUW[] = {
    {I_PMINUW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8215, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVMSKB[] = {
    {I_PMOVMSKB, 2, {REG_GPR|BITS32,MMXREG,0,0,0}, nasm_bytecodes+14762, IF_KATMAI|IF_MMX},
    {I_PMOVMSKB, 2, {REG_GPR|BITS32,XMMREG,0,0,0}, nasm_bytecodes+14996, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVSXBD[] = {
    {I_PMOVSXBD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8229, IF_SSE41|IF_SD},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVSXBQ[] = {
    {I_PMOVSXBQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8236, IF_SSE41|IF_SW},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVSXBW[] = {
    {I_PMOVSXBW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8222, IF_SSE41|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVSXDQ[] = {
    {I_PMOVSXDQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8257, IF_SSE41|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVSXWD[] = {
    {I_PMOVSXWD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8243, IF_SSE41|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVSXWQ[] = {
    {I_PMOVSXWQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8250, IF_SSE41|IF_SD},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVZXBD[] = {
    {I_PMOVZXBD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8271, IF_SSE41|IF_SD},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVZXBQ[] = {
    {I_PMOVZXBQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8278, IF_SSE41|IF_SW},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVZXBW[] = {
    {I_PMOVZXBW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8264, IF_SSE41|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVZXDQ[] = {
    {I_PMOVZXDQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8299, IF_SSE41|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVZXWD[] = {
    {I_PMOVZXWD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8285, IF_SSE41|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMOVZXWQ[] = {
    {I_PMOVZXWQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8292, IF_SSE41|IF_SD},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMULDQ[] = {
    {I_PMULDQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8306, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMULHRIW[] = {
    {I_PMULHRIW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+14048, IF_PENT|IF_MMX|IF_SQ|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMULHRSW[] = {
    {I_PMULHRSW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+8026, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PMULHRSW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8033, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMULHRWA[] = {
    {I_PMULHRWA, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5328, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMULHRWC[] = {
    {I_PMULHRWC, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+14054, IF_PENT|IF_MMX|IF_SQ|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMULHUW[] = {
    {I_PMULHUW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7655, IF_KATMAI|IF_MMX|IF_SQ},
    {I_PMULHUW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15002, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMULHW[] = {
    {I_PMULHW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7200, IF_PENT|IF_MMX|IF_SQ},
    {I_PMULHW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15008, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMULLD[] = {
    {I_PMULLD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8313, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMULLW[] = {
    {I_PMULLW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7207, IF_PENT|IF_MMX|IF_SQ},
    {I_PMULLW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15014, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMULUDQ[] = {
    {I_PMULUDQ, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7718, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_PMULUDQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15020, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMVGEZB[] = {
    {I_PMVGEZB, 2, {MMXREG,MEMORY,0,0,0}, nasm_bytecodes+15435, IF_PENT|IF_MMX|IF_SQ|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMVLZB[] = {
    {I_PMVLZB, 2, {MMXREG,MEMORY,0,0,0}, nasm_bytecodes+15291, IF_PENT|IF_MMX|IF_SQ|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMVNZB[] = {
    {I_PMVNZB, 2, {MMXREG,MEMORY,0,0,0}, nasm_bytecodes+15273, IF_PENT|IF_MMX|IF_SQ|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PMVZB[] = {
    {I_PMVZB, 2, {MMXREG,MEMORY,0,0,0}, nasm_bytecodes+15195, IF_PENT|IF_MMX|IF_SQ|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_POP[] = {
    {I_POP, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+19388, IF_8086},
    {I_POP, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+19392, IF_386|IF_NOLONG},
    {I_POP, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+19396, IF_X64},
    {I_POP, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17957, IF_8086},
    {I_POP, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+17962, IF_386|IF_NOLONG},
    {I_POP, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+17967, IF_X64},
    {I_POP, 1, {REG_CS,0,0,0,0}, nasm_bytecodes+3291, IF_8086|IF_UNDOC},
    {I_POP, 1, {REG_DESS,0,0,0,0}, nasm_bytecodes+19206, IF_8086|IF_NOLONG},
    {I_POP, 1, {REG_FSGS,0,0,0,0}, nasm_bytecodes+19400, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_POPA[] = {
    {I_POPA, 0, {0,0,0,0,0}, nasm_bytecodes+19404, IF_186|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_POPAD[] = {
    {I_POPAD, 0, {0,0,0,0,0}, nasm_bytecodes+19408, IF_386|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_POPAW[] = {
    {I_POPAW, 0, {0,0,0,0,0}, nasm_bytecodes+19412, IF_186|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_POPCNT[] = {
    {I_POPCNT, 2, {REG_GPR|BITS16,RM_GPR|BITS16,0,0,0}, nasm_bytecodes+8334, IF_NEHALEM|IF_SW},
    {I_POPCNT, 2, {REG_GPR|BITS32,RM_GPR|BITS32,0,0,0}, nasm_bytecodes+8341, IF_NEHALEM|IF_SD},
    {I_POPCNT, 2, {REG_GPR|BITS64,RM_GPR|BITS64,0,0,0}, nasm_bytecodes+8348, IF_NEHALEM|IF_SQ|IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_POPF[] = {
    {I_POPF, 0, {0,0,0,0,0}, nasm_bytecodes+19416, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_POPFD[] = {
    {I_POPFD, 0, {0,0,0,0,0}, nasm_bytecodes+19420, IF_386|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_POPFQ[] = {
    {I_POPFQ, 0, {0,0,0,0,0}, nasm_bytecodes+19420, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_POPFW[] = {
    {I_POPFW, 0, {0,0,0,0,0}, nasm_bytecodes+19424, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_POR[] = {
    {I_POR, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7214, IF_PENT|IF_MMX|IF_SQ},
    {I_POR, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15026, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PREFETCH[] = {
    {I_PREFETCH, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17972, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PREFETCHNTA[] = {
    {I_PREFETCHNTA, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+15651, IF_KATMAI},
    ITEMPLATE_END
};

static const struct itemplate instrux_PREFETCHT0[] = {
    {I_PREFETCHT0, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+15669, IF_KATMAI},
    ITEMPLATE_END
};

static const struct itemplate instrux_PREFETCHT1[] = {
    {I_PREFETCHT1, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+15687, IF_KATMAI},
    ITEMPLATE_END
};

static const struct itemplate instrux_PREFETCHT2[] = {
    {I_PREFETCHT2, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+15705, IF_KATMAI},
    ITEMPLATE_END
};

static const struct itemplate instrux_PREFETCHW[] = {
    {I_PREFETCHW, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+17977, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSADBW[] = {
    {I_PSADBW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7662, IF_KATMAI|IF_MMX|IF_SQ},
    {I_PSADBW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15032, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSHUFB[] = {
    {I_PSHUFB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+8040, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PSHUFB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8047, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSHUFD[] = {
    {I_PSHUFD, 3, {XMMREG,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+7725, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR2},
    {I_PSHUFD, 3, {XMMREG,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+7725, IF_WILLAMETTE|IF_SSE2|IF_SM2|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSHUFHW[] = {
    {I_PSHUFHW, 3, {XMMREG,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+7732, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR2},
    {I_PSHUFHW, 3, {XMMREG,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+7732, IF_WILLAMETTE|IF_SSE2|IF_SM2|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSHUFLW[] = {
    {I_PSHUFLW, 3, {XMMREG,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+7739, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR2},
    {I_PSHUFLW, 3, {XMMREG,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+7739, IF_WILLAMETTE|IF_SSE2|IF_SM2|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSHUFW[] = {
    {I_PSHUFW, 3, {MMXREG,RM_MMX,IMMEDIATE,0,0}, nasm_bytecodes+5464, IF_KATMAI|IF_MMX|IF_SM2|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSIGNB[] = {
    {I_PSIGNB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+8054, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PSIGNB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8061, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSIGND[] = {
    {I_PSIGND, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+8082, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PSIGND, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8089, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSIGNW[] = {
    {I_PSIGNW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+8068, IF_SSSE3|IF_MMX|IF_SQ},
    {I_PSIGNW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8075, IF_SSSE3},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSLLD[] = {
    {I_PSLLD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7221, IF_PENT|IF_MMX|IF_SQ},
    {I_PSLLD, 2, {MMXREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7228, IF_PENT|IF_MMX},
    {I_PSLLD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15044, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_PSLLD, 2, {XMMREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7760, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSLLDQ[] = {
    {I_PSLLDQ, 2, {XMMREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7746, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSLLQ[] = {
    {I_PSLLQ, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7235, IF_PENT|IF_MMX|IF_SQ},
    {I_PSLLQ, 2, {MMXREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7242, IF_PENT|IF_MMX},
    {I_PSLLQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15050, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_PSLLQ, 2, {XMMREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7767, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSLLW[] = {
    {I_PSLLW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7249, IF_PENT|IF_MMX|IF_SQ},
    {I_PSLLW, 2, {MMXREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7256, IF_PENT|IF_MMX},
    {I_PSLLW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15038, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_PSLLW, 2, {XMMREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7753, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSRAD[] = {
    {I_PSRAD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7263, IF_PENT|IF_MMX|IF_SQ},
    {I_PSRAD, 2, {MMXREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7270, IF_PENT|IF_MMX},
    {I_PSRAD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15062, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_PSRAD, 2, {XMMREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7781, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSRAW[] = {
    {I_PSRAW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7277, IF_PENT|IF_MMX|IF_SQ},
    {I_PSRAW, 2, {MMXREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7284, IF_PENT|IF_MMX},
    {I_PSRAW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15056, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_PSRAW, 2, {XMMREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7774, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSRLD[] = {
    {I_PSRLD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7291, IF_PENT|IF_MMX|IF_SQ},
    {I_PSRLD, 2, {MMXREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7298, IF_PENT|IF_MMX},
    {I_PSRLD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15074, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_PSRLD, 2, {XMMREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7802, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSRLDQ[] = {
    {I_PSRLDQ, 2, {XMMREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7788, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSRLQ[] = {
    {I_PSRLQ, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7305, IF_PENT|IF_MMX|IF_SQ},
    {I_PSRLQ, 2, {MMXREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7312, IF_PENT|IF_MMX},
    {I_PSRLQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15080, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_PSRLQ, 2, {XMMREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7809, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSRLW[] = {
    {I_PSRLW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7319, IF_PENT|IF_MMX|IF_SQ},
    {I_PSRLW, 2, {MMXREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7326, IF_PENT|IF_MMX},
    {I_PSRLW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15068, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_PSRLW, 2, {XMMREG,IMMEDIATE,0,0,0}, nasm_bytecodes+7795, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR1},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSUBB[] = {
    {I_PSUBB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7333, IF_PENT|IF_MMX|IF_SQ},
    {I_PSUBB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15086, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSUBD[] = {
    {I_PSUBD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7340, IF_PENT|IF_MMX|IF_SQ},
    {I_PSUBD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15098, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSUBQ[] = {
    {I_PSUBQ, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7816, IF_WILLAMETTE|IF_SSE2|IF_SO},
    {I_PSUBQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15104, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSUBSB[] = {
    {I_PSUBSB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7347, IF_PENT|IF_MMX|IF_SQ},
    {I_PSUBSB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15110, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSUBSIW[] = {
    {I_PSUBSIW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+14060, IF_PENT|IF_MMX|IF_SQ|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSUBSW[] = {
    {I_PSUBSW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7354, IF_PENT|IF_MMX|IF_SQ},
    {I_PSUBSW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15116, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSUBUSB[] = {
    {I_PSUBUSB, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7361, IF_PENT|IF_MMX|IF_SQ},
    {I_PSUBUSB, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15122, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSUBUSW[] = {
    {I_PSUBUSW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7368, IF_PENT|IF_MMX|IF_SQ},
    {I_PSUBUSW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15128, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSUBW[] = {
    {I_PSUBW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7375, IF_PENT|IF_MMX|IF_SQ},
    {I_PSUBW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15092, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PSWAPD[] = {
    {I_PSWAPD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+5504, IF_PENT|IF_3DNOW|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PTEST[] = {
    {I_PTEST, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+8320, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUNPCKHBW[] = {
    {I_PUNPCKHBW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7382, IF_PENT|IF_MMX|IF_SQ},
    {I_PUNPCKHBW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15134, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUNPCKHDQ[] = {
    {I_PUNPCKHDQ, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7389, IF_PENT|IF_MMX|IF_SQ},
    {I_PUNPCKHDQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15146, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUNPCKHQDQ[] = {
    {I_PUNPCKHQDQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15152, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUNPCKHWD[] = {
    {I_PUNPCKHWD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7396, IF_PENT|IF_MMX|IF_SQ},
    {I_PUNPCKHWD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15140, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUNPCKLBW[] = {
    {I_PUNPCKLBW, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7403, IF_PENT|IF_MMX|IF_SQ},
    {I_PUNPCKLBW, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15158, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUNPCKLDQ[] = {
    {I_PUNPCKLDQ, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7410, IF_PENT|IF_MMX|IF_SQ},
    {I_PUNPCKLDQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15170, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUNPCKLQDQ[] = {
    {I_PUNPCKLQDQ, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15176, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUNPCKLWD[] = {
    {I_PUNPCKLWD, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7417, IF_PENT|IF_MMX|IF_SQ},
    {I_PUNPCKLWD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15164, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUSH[] = {
    {I_PUSH, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+19428, IF_8086},
    {I_PUSH, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+19432, IF_386|IF_NOLONG},
    {I_PUSH, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+19436, IF_X64},
    {I_PUSH, 1, {RM_GPR|BITS16,0,0,0,0}, nasm_bytecodes+17982, IF_8086},
    {I_PUSH, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+17987, IF_386|IF_NOLONG},
    {I_PUSH, 1, {RM_GPR|BITS64,0,0,0,0}, nasm_bytecodes+17992, IF_X64},
    {I_PUSH, 1, {REG_CS,0,0,0,0}, nasm_bytecodes+19182, IF_8086|IF_NOLONG},
    {I_PUSH, 1, {REG_DESS,0,0,0,0}, nasm_bytecodes+19182, IF_8086|IF_NOLONG},
    {I_PUSH, 1, {REG_FSGS,0,0,0,0}, nasm_bytecodes+19440, IF_386},
    {I_PUSH, 1, {IMMEDIATE|BITS8,0,0,0,0}, nasm_bytecodes+19444, IF_186},
    {I_PUSH, 1, {IMMEDIATE|BITS16,0,0,0,0}, nasm_bytecodes+17997, IF_186|IF_AR0|IF_SZ},
    {I_PUSH, 1, {IMMEDIATE|BITS32,0,0,0,0}, nasm_bytecodes+18002, IF_386|IF_NOLONG|IF_AR0|IF_SZ},
    {I_PUSH, 1, {IMMEDIATE|BITS32,0,0,0,0}, nasm_bytecodes+18002, IF_386|IF_NOLONG|IF_SD},
    {I_PUSH, 1, {IMMEDIATE|BITS32,0,0,0,0}, nasm_bytecodes+18007, IF_X64|IF_AR0|IF_SZ},
    {I_PUSH, 1, {IMMEDIATE|BITS64,0,0,0,0}, nasm_bytecodes+18007, IF_X64|IF_AR0|IF_SZ},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUSHA[] = {
    {I_PUSHA, 0, {0,0,0,0,0}, nasm_bytecodes+19448, IF_186|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUSHAD[] = {
    {I_PUSHAD, 0, {0,0,0,0,0}, nasm_bytecodes+19452, IF_386|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUSHAW[] = {
    {I_PUSHAW, 0, {0,0,0,0,0}, nasm_bytecodes+19456, IF_186|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUSHF[] = {
    {I_PUSHF, 0, {0,0,0,0,0}, nasm_bytecodes+19460, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUSHFD[] = {
    {I_PUSHFD, 0, {0,0,0,0,0}, nasm_bytecodes+19464, IF_386|IF_NOLONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUSHFQ[] = {
    {I_PUSHFQ, 0, {0,0,0,0,0}, nasm_bytecodes+19464, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_PUSHFW[] = {
    {I_PUSHFW, 0, {0,0,0,0,0}, nasm_bytecodes+19468, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_PXOR[] = {
    {I_PXOR, 2, {MMXREG,RM_MMX,0,0,0}, nasm_bytecodes+7424, IF_PENT|IF_MMX|IF_SQ},
    {I_PXOR, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15182, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_RCL[] = {
    {I_RCL, 2, {RM_GPR|BITS8,UNITY,0,0,0}, nasm_bytecodes+19472, IF_8086},
    {I_RCL, 2, {RM_GPR|BITS8,REG_CL,0,0,0}, nasm_bytecodes+19476, IF_8086},
    {I_RCL, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+18012, IF_186|IF_SB},
    {I_RCL, 2, {RM_GPR|BITS16,UNITY,0,0,0}, nasm_bytecodes+18017, IF_8086},
    {I_RCL, 2, {RM_GPR|BITS16,REG_CL,0,0,0}, nasm_bytecodes+18022, IF_8086},
    {I_RCL, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14066, IF_186|IF_SB},
    {I_RCL, 2, {RM_GPR|BITS32,UNITY,0,0,0}, nasm_bytecodes+18027, IF_386},
    {I_RCL, 2, {RM_GPR|BITS32,REG_CL,0,0,0}, nasm_bytecodes+18032, IF_386},
    {I_RCL, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14072, IF_386|IF_SB},
    {I_RCL, 2, {RM_GPR|BITS64,UNITY,0,0,0}, nasm_bytecodes+18037, IF_X64},
    {I_RCL, 2, {RM_GPR|BITS64,REG_CL,0,0,0}, nasm_bytecodes+18042, IF_X64},
    {I_RCL, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14078, IF_X64|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_RCPPS[] = {
    {I_RCPPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14636, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_RCPSS[] = {
    {I_RCPSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14642, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_RCR[] = {
    {I_RCR, 2, {RM_GPR|BITS8,UNITY,0,0,0}, nasm_bytecodes+19480, IF_8086},
    {I_RCR, 2, {RM_GPR|BITS8,REG_CL,0,0,0}, nasm_bytecodes+19484, IF_8086},
    {I_RCR, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+18047, IF_186|IF_SB},
    {I_RCR, 2, {RM_GPR|BITS16,UNITY,0,0,0}, nasm_bytecodes+18052, IF_8086},
    {I_RCR, 2, {RM_GPR|BITS16,REG_CL,0,0,0}, nasm_bytecodes+18057, IF_8086},
    {I_RCR, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14084, IF_186|IF_SB},
    {I_RCR, 2, {RM_GPR|BITS32,UNITY,0,0,0}, nasm_bytecodes+18062, IF_386},
    {I_RCR, 2, {RM_GPR|BITS32,REG_CL,0,0,0}, nasm_bytecodes+18067, IF_386},
    {I_RCR, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14090, IF_386|IF_SB},
    {I_RCR, 2, {RM_GPR|BITS64,UNITY,0,0,0}, nasm_bytecodes+18072, IF_X64},
    {I_RCR, 2, {RM_GPR|BITS64,REG_CL,0,0,0}, nasm_bytecodes+18077, IF_X64},
    {I_RCR, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14096, IF_X64|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_RDFSBASE[] = {
    {I_RDFSBASE, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+12521, IF_LONG|IF_FUTURE},
    {I_RDFSBASE, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+12520, IF_LONG|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_RDGSBASE[] = {
    {I_RDGSBASE, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+12528, IF_LONG|IF_FUTURE},
    {I_RDGSBASE, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+12527, IF_LONG|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_RDM[] = {
    {I_RDM, 0, {0,0,0,0,0}, nasm_bytecodes+18676, IF_P6|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_RDMSR[] = {
    {I_RDMSR, 0, {0,0,0,0,0}, nasm_bytecodes+19488, IF_PENT|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_RDPMC[] = {
    {I_RDPMC, 0, {0,0,0,0,0}, nasm_bytecodes+19492, IF_P6},
    ITEMPLATE_END
};

static const struct itemplate instrux_RDRAND[] = {
    {I_RDRAND, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+15572, IF_FUTURE},
    {I_RDRAND, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+15578, IF_FUTURE},
    {I_RDRAND, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+15584, IF_LONG|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_RDSHR[] = {
    {I_RDSHR, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+14102, IF_P6|IF_CYRIX|IF_SMM},
    ITEMPLATE_END
};

static const struct itemplate instrux_RDTSC[] = {
    {I_RDTSC, 0, {0,0,0,0,0}, nasm_bytecodes+19496, IF_PENT},
    ITEMPLATE_END
};

static const struct itemplate instrux_RDTSCP[] = {
    {I_RDTSCP, 0, {0,0,0,0,0}, nasm_bytecodes+18082, IF_X86_64},
    ITEMPLATE_END
};

static const struct itemplate instrux_RESB[] = {
    {I_RESB, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+18790, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_RESD[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_RESO[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_RESQ[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_REST[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_RESW[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_RESY[] = {
    ITEMPLATE_END
};

static const struct itemplate instrux_RET[] = {
    {I_RET, 0, {0,0,0,0,0}, nasm_bytecodes+18609, IF_8086},
    {I_RET, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+19500, IF_8086|IF_SW},
    ITEMPLATE_END
};

static const struct itemplate instrux_RETF[] = {
    {I_RETF, 0, {0,0,0,0,0}, nasm_bytecodes+19731, IF_8086},
    {I_RETF, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+19504, IF_8086|IF_SW},
    ITEMPLATE_END
};

static const struct itemplate instrux_RETN[] = {
    {I_RETN, 0, {0,0,0,0,0}, nasm_bytecodes+18609, IF_8086},
    {I_RETN, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+19500, IF_8086|IF_SW},
    ITEMPLATE_END
};

static const struct itemplate instrux_ROL[] = {
    {I_ROL, 2, {RM_GPR|BITS8,UNITY,0,0,0}, nasm_bytecodes+19508, IF_8086},
    {I_ROL, 2, {RM_GPR|BITS8,REG_CL,0,0,0}, nasm_bytecodes+19512, IF_8086},
    {I_ROL, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+18087, IF_186|IF_SB},
    {I_ROL, 2, {RM_GPR|BITS16,UNITY,0,0,0}, nasm_bytecodes+18092, IF_8086},
    {I_ROL, 2, {RM_GPR|BITS16,REG_CL,0,0,0}, nasm_bytecodes+18097, IF_8086},
    {I_ROL, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14108, IF_186|IF_SB},
    {I_ROL, 2, {RM_GPR|BITS32,UNITY,0,0,0}, nasm_bytecodes+18102, IF_386},
    {I_ROL, 2, {RM_GPR|BITS32,REG_CL,0,0,0}, nasm_bytecodes+18107, IF_386},
    {I_ROL, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14114, IF_386|IF_SB},
    {I_ROL, 2, {RM_GPR|BITS64,UNITY,0,0,0}, nasm_bytecodes+18112, IF_X64},
    {I_ROL, 2, {RM_GPR|BITS64,REG_CL,0,0,0}, nasm_bytecodes+18117, IF_X64},
    {I_ROL, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14120, IF_X64|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_ROR[] = {
    {I_ROR, 2, {RM_GPR|BITS8,UNITY,0,0,0}, nasm_bytecodes+19516, IF_8086},
    {I_ROR, 2, {RM_GPR|BITS8,REG_CL,0,0,0}, nasm_bytecodes+19520, IF_8086},
    {I_ROR, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+18122, IF_186|IF_SB},
    {I_ROR, 2, {RM_GPR|BITS16,UNITY,0,0,0}, nasm_bytecodes+18127, IF_8086},
    {I_ROR, 2, {RM_GPR|BITS16,REG_CL,0,0,0}, nasm_bytecodes+18132, IF_8086},
    {I_ROR, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14126, IF_186|IF_SB},
    {I_ROR, 2, {RM_GPR|BITS32,UNITY,0,0,0}, nasm_bytecodes+18137, IF_386},
    {I_ROR, 2, {RM_GPR|BITS32,REG_CL,0,0,0}, nasm_bytecodes+18142, IF_386},
    {I_ROR, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14132, IF_386|IF_SB},
    {I_ROR, 2, {RM_GPR|BITS64,UNITY,0,0,0}, nasm_bytecodes+18147, IF_X64},
    {I_ROR, 2, {RM_GPR|BITS64,REG_CL,0,0,0}, nasm_bytecodes+18152, IF_X64},
    {I_ROR, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14138, IF_X64|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_ROUNDPD[] = {
    {I_ROUNDPD, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5744, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_ROUNDPS[] = {
    {I_ROUNDPS, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5752, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_ROUNDSD[] = {
    {I_ROUNDSD, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5760, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_ROUNDSS[] = {
    {I_ROUNDSS, 3, {XMMREG,RM_XMM,IMMEDIATE,0,0}, nasm_bytecodes+5768, IF_SSE41},
    ITEMPLATE_END
};

static const struct itemplate instrux_RSDC[] = {
    {I_RSDC, 2, {REG_SREG,MEMORY|BITS80,0,0,0}, nasm_bytecodes+15543, IF_486|IF_CYRIX|IF_SMM},
    ITEMPLATE_END
};

static const struct itemplate instrux_RSLDT[] = {
    {I_RSLDT, 1, {MEMORY|BITS80,0,0,0,0}, nasm_bytecodes+18157, IF_486|IF_CYRIX|IF_SMM},
    ITEMPLATE_END
};

static const struct itemplate instrux_RSM[] = {
    {I_RSM, 0, {0,0,0,0,0}, nasm_bytecodes+19524, IF_PENT|IF_SMM},
    ITEMPLATE_END
};

static const struct itemplate instrux_RSQRTPS[] = {
    {I_RSQRTPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14648, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_RSQRTSS[] = {
    {I_RSQRTSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14654, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_RSTS[] = {
    {I_RSTS, 1, {MEMORY|BITS80,0,0,0,0}, nasm_bytecodes+18162, IF_486|IF_CYRIX|IF_SMM},
    ITEMPLATE_END
};

static const struct itemplate instrux_SAHF[] = {
    {I_SAHF, 0, {0,0,0,0,0}, nasm_bytecodes+5213, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_SAL[] = {
    {I_SAL, 2, {RM_GPR|BITS8,UNITY,0,0,0}, nasm_bytecodes+19528, IF_8086},
    {I_SAL, 2, {RM_GPR|BITS8,REG_CL,0,0,0}, nasm_bytecodes+19532, IF_8086},
    {I_SAL, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+18167, IF_186|IF_SB},
    {I_SAL, 2, {RM_GPR|BITS16,UNITY,0,0,0}, nasm_bytecodes+18172, IF_8086},
    {I_SAL, 2, {RM_GPR|BITS16,REG_CL,0,0,0}, nasm_bytecodes+18177, IF_8086},
    {I_SAL, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14144, IF_186|IF_SB},
    {I_SAL, 2, {RM_GPR|BITS32,UNITY,0,0,0}, nasm_bytecodes+18182, IF_386},
    {I_SAL, 2, {RM_GPR|BITS32,REG_CL,0,0,0}, nasm_bytecodes+18187, IF_386},
    {I_SAL, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14150, IF_386|IF_SB},
    {I_SAL, 2, {RM_GPR|BITS64,UNITY,0,0,0}, nasm_bytecodes+18192, IF_X64},
    {I_SAL, 2, {RM_GPR|BITS64,REG_CL,0,0,0}, nasm_bytecodes+18197, IF_X64},
    {I_SAL, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14156, IF_X64|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_SALC[] = {
    {I_SALC, 0, {0,0,0,0,0}, nasm_bytecodes+19734, IF_8086|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_SAR[] = {
    {I_SAR, 2, {RM_GPR|BITS8,UNITY,0,0,0}, nasm_bytecodes+19536, IF_8086},
    {I_SAR, 2, {RM_GPR|BITS8,REG_CL,0,0,0}, nasm_bytecodes+19540, IF_8086},
    {I_SAR, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+18202, IF_186|IF_SB},
    {I_SAR, 2, {RM_GPR|BITS16,UNITY,0,0,0}, nasm_bytecodes+18207, IF_8086},
    {I_SAR, 2, {RM_GPR|BITS16,REG_CL,0,0,0}, nasm_bytecodes+18212, IF_8086},
    {I_SAR, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14162, IF_186|IF_SB},
    {I_SAR, 2, {RM_GPR|BITS32,UNITY,0,0,0}, nasm_bytecodes+18217, IF_386},
    {I_SAR, 2, {RM_GPR|BITS32,REG_CL,0,0,0}, nasm_bytecodes+18222, IF_386},
    {I_SAR, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14168, IF_386|IF_SB},
    {I_SAR, 2, {RM_GPR|BITS64,UNITY,0,0,0}, nasm_bytecodes+18227, IF_X64},
    {I_SAR, 2, {RM_GPR|BITS64,REG_CL,0,0,0}, nasm_bytecodes+18232, IF_X64},
    {I_SAR, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14174, IF_X64|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_SBB[] = {
    {I_SBB, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19544, IF_8086|IF_SM},
    {I_SBB, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19544, IF_8086},
    {I_SBB, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18237, IF_8086|IF_SM},
    {I_SBB, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18237, IF_8086},
    {I_SBB, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18242, IF_386|IF_SM},
    {I_SBB, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18242, IF_386},
    {I_SBB, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18247, IF_X64|IF_SM},
    {I_SBB, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18247, IF_X64},
    {I_SBB, 2, {REG_GPR|BITS8,MEMORY,0,0,0}, nasm_bytecodes+8771, IF_8086|IF_SM},
    {I_SBB, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+8771, IF_8086},
    {I_SBB, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+18252, IF_8086|IF_SM},
    {I_SBB, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18252, IF_8086},
    {I_SBB, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+18257, IF_386|IF_SM},
    {I_SBB, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18257, IF_386},
    {I_SBB, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+18262, IF_X64|IF_SM},
    {I_SBB, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18262, IF_X64},
    {I_SBB, 2, {RM_GPR|BITS16,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+14180, IF_8086},
    {I_SBB, 2, {RM_GPR|BITS32,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+14186, IF_386},
    {I_SBB, 2, {RM_GPR|BITS64,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+14192, IF_X64},
    {I_SBB, 2, {REG_AL,IMMEDIATE,0,0,0}, nasm_bytecodes+19548, IF_8086|IF_SM},
    {I_SBB, 2, {REG_AX,SBYTE16,0,0,0}, nasm_bytecodes+14180, IF_8086|IF_SM},
    {I_SBB, 2, {REG_AX,IMMEDIATE,0,0,0}, nasm_bytecodes+18267, IF_8086|IF_SM},
    {I_SBB, 2, {REG_EAX,SBYTE32,0,0,0}, nasm_bytecodes+14186, IF_386|IF_SM},
    {I_SBB, 2, {REG_EAX,IMMEDIATE,0,0,0}, nasm_bytecodes+18272, IF_386|IF_SM},
    {I_SBB, 2, {REG_RAX,SBYTE64,0,0,0}, nasm_bytecodes+14192, IF_X64|IF_SM},
    {I_SBB, 2, {REG_RAX,IMMEDIATE,0,0,0}, nasm_bytecodes+18277, IF_X64|IF_SM},
    {I_SBB, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+18282, IF_8086|IF_SM},
    {I_SBB, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14198, IF_8086|IF_SM},
    {I_SBB, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14204, IF_386|IF_SM},
    {I_SBB, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14210, IF_X64|IF_SM},
    {I_SBB, 2, {MEMORY,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+18282, IF_8086|IF_SM},
    {I_SBB, 2, {MEMORY,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+14198, IF_8086|IF_SM},
    {I_SBB, 2, {MEMORY,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+14204, IF_386|IF_SM},
    ITEMPLATE_END
};

static const struct itemplate instrux_SCASB[] = {
    {I_SCASB, 0, {0,0,0,0,0}, nasm_bytecodes+19552, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_SCASD[] = {
    {I_SCASD, 0, {0,0,0,0,0}, nasm_bytecodes+18287, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_SCASQ[] = {
    {I_SCASQ, 0, {0,0,0,0,0}, nasm_bytecodes+18292, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_SCASW[] = {
    {I_SCASW, 0, {0,0,0,0,0}, nasm_bytecodes+18297, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_SFENCE[] = {
    {I_SFENCE, 0, {0,0,0,0,0}, nasm_bytecodes+18302, IF_X64|IF_AMD},
    {I_SFENCE, 0, {0,0,0,0,0}, nasm_bytecodes+18302, IF_KATMAI},
    ITEMPLATE_END
};

static const struct itemplate instrux_SGDT[] = {
    {I_SGDT, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+18307, IF_286},
    ITEMPLATE_END
};

static const struct itemplate instrux_SHL[] = {
    {I_SHL, 2, {RM_GPR|BITS8,UNITY,0,0,0}, nasm_bytecodes+19528, IF_8086},
    {I_SHL, 2, {RM_GPR|BITS8,REG_CL,0,0,0}, nasm_bytecodes+19532, IF_8086},
    {I_SHL, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+18167, IF_186|IF_SB},
    {I_SHL, 2, {RM_GPR|BITS16,UNITY,0,0,0}, nasm_bytecodes+18172, IF_8086},
    {I_SHL, 2, {RM_GPR|BITS16,REG_CL,0,0,0}, nasm_bytecodes+18177, IF_8086},
    {I_SHL, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14144, IF_186|IF_SB},
    {I_SHL, 2, {RM_GPR|BITS32,UNITY,0,0,0}, nasm_bytecodes+18182, IF_386},
    {I_SHL, 2, {RM_GPR|BITS32,REG_CL,0,0,0}, nasm_bytecodes+18187, IF_386},
    {I_SHL, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14150, IF_386|IF_SB},
    {I_SHL, 2, {RM_GPR|BITS64,UNITY,0,0,0}, nasm_bytecodes+18192, IF_X64},
    {I_SHL, 2, {RM_GPR|BITS64,REG_CL,0,0,0}, nasm_bytecodes+18197, IF_X64},
    {I_SHL, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14156, IF_X64|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_SHLD[] = {
    {I_SHLD, 3, {MEMORY,REG_GPR|BITS16,IMMEDIATE,0,0}, nasm_bytecodes+7431, IF_386|IF_SM2|IF_SB|IF_AR2},
    {I_SHLD, 3, {REG_GPR|BITS16,REG_GPR|BITS16,IMMEDIATE,0,0}, nasm_bytecodes+7431, IF_386|IF_SM2|IF_SB|IF_AR2},
    {I_SHLD, 3, {MEMORY,REG_GPR|BITS32,IMMEDIATE,0,0}, nasm_bytecodes+7438, IF_386|IF_SM2|IF_SB|IF_AR2},
    {I_SHLD, 3, {REG_GPR|BITS32,REG_GPR|BITS32,IMMEDIATE,0,0}, nasm_bytecodes+7438, IF_386|IF_SM2|IF_SB|IF_AR2},
    {I_SHLD, 3, {MEMORY,REG_GPR|BITS64,IMMEDIATE,0,0}, nasm_bytecodes+7445, IF_X64|IF_SM2|IF_SB|IF_AR2},
    {I_SHLD, 3, {REG_GPR|BITS64,REG_GPR|BITS64,IMMEDIATE,0,0}, nasm_bytecodes+7445, IF_X64|IF_SM2|IF_SB|IF_AR2},
    {I_SHLD, 3, {MEMORY,REG_GPR|BITS16,REG_CL,0,0}, nasm_bytecodes+14216, IF_386|IF_SM},
    {I_SHLD, 3, {REG_GPR|BITS16,REG_GPR|BITS16,REG_CL,0,0}, nasm_bytecodes+14216, IF_386},
    {I_SHLD, 3, {MEMORY,REG_GPR|BITS32,REG_CL,0,0}, nasm_bytecodes+14222, IF_386|IF_SM},
    {I_SHLD, 3, {REG_GPR|BITS32,REG_GPR|BITS32,REG_CL,0,0}, nasm_bytecodes+14222, IF_386},
    {I_SHLD, 3, {MEMORY,REG_GPR|BITS64,REG_CL,0,0}, nasm_bytecodes+14228, IF_X64|IF_SM},
    {I_SHLD, 3, {REG_GPR|BITS64,REG_GPR|BITS64,REG_CL,0,0}, nasm_bytecodes+14228, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_SHR[] = {
    {I_SHR, 2, {RM_GPR|BITS8,UNITY,0,0,0}, nasm_bytecodes+19556, IF_8086},
    {I_SHR, 2, {RM_GPR|BITS8,REG_CL,0,0,0}, nasm_bytecodes+19560, IF_8086},
    {I_SHR, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+18312, IF_186|IF_SB},
    {I_SHR, 2, {RM_GPR|BITS16,UNITY,0,0,0}, nasm_bytecodes+18317, IF_8086},
    {I_SHR, 2, {RM_GPR|BITS16,REG_CL,0,0,0}, nasm_bytecodes+18322, IF_8086},
    {I_SHR, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14234, IF_186|IF_SB},
    {I_SHR, 2, {RM_GPR|BITS32,UNITY,0,0,0}, nasm_bytecodes+18327, IF_386},
    {I_SHR, 2, {RM_GPR|BITS32,REG_CL,0,0,0}, nasm_bytecodes+18332, IF_386},
    {I_SHR, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14240, IF_386|IF_SB},
    {I_SHR, 2, {RM_GPR|BITS64,UNITY,0,0,0}, nasm_bytecodes+18337, IF_X64},
    {I_SHR, 2, {RM_GPR|BITS64,REG_CL,0,0,0}, nasm_bytecodes+18342, IF_X64},
    {I_SHR, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14246, IF_X64|IF_SB},
    ITEMPLATE_END
};

static const struct itemplate instrux_SHRD[] = {
    {I_SHRD, 3, {MEMORY,REG_GPR|BITS16,IMMEDIATE,0,0}, nasm_bytecodes+7452, IF_386|IF_SM2|IF_SB|IF_AR2},
    {I_SHRD, 3, {REG_GPR|BITS16,REG_GPR|BITS16,IMMEDIATE,0,0}, nasm_bytecodes+7452, IF_386|IF_SM2|IF_SB|IF_AR2},
    {I_SHRD, 3, {MEMORY,REG_GPR|BITS32,IMMEDIATE,0,0}, nasm_bytecodes+7459, IF_386|IF_SM2|IF_SB|IF_AR2},
    {I_SHRD, 3, {REG_GPR|BITS32,REG_GPR|BITS32,IMMEDIATE,0,0}, nasm_bytecodes+7459, IF_386|IF_SM2|IF_SB|IF_AR2},
    {I_SHRD, 3, {MEMORY,REG_GPR|BITS64,IMMEDIATE,0,0}, nasm_bytecodes+7466, IF_X64|IF_SM2|IF_SB|IF_AR2},
    {I_SHRD, 3, {REG_GPR|BITS64,REG_GPR|BITS64,IMMEDIATE,0,0}, nasm_bytecodes+7466, IF_X64|IF_SM2|IF_SB|IF_AR2},
    {I_SHRD, 3, {MEMORY,REG_GPR|BITS16,REG_CL,0,0}, nasm_bytecodes+14252, IF_386|IF_SM},
    {I_SHRD, 3, {REG_GPR|BITS16,REG_GPR|BITS16,REG_CL,0,0}, nasm_bytecodes+14252, IF_386},
    {I_SHRD, 3, {MEMORY,REG_GPR|BITS32,REG_CL,0,0}, nasm_bytecodes+14258, IF_386|IF_SM},
    {I_SHRD, 3, {REG_GPR|BITS32,REG_GPR|BITS32,REG_CL,0,0}, nasm_bytecodes+14258, IF_386},
    {I_SHRD, 3, {MEMORY,REG_GPR|BITS64,REG_CL,0,0}, nasm_bytecodes+14264, IF_X64|IF_SM},
    {I_SHRD, 3, {REG_GPR|BITS64,REG_GPR|BITS64,REG_CL,0,0}, nasm_bytecodes+14264, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_SHUFPD[] = {
    {I_SHUFPD, 3, {XMMREG,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+7865, IF_WILLAMETTE|IF_SSE2|IF_SB|IF_AR2},
    {I_SHUFPD, 3, {XMMREG,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+7865, IF_WILLAMETTE|IF_SSE2|IF_SM|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_SHUFPS[] = {
    {I_SHUFPS, 3, {XMMREG,MEMORY,IMMEDIATE,0,0}, nasm_bytecodes+7592, IF_KATMAI|IF_SSE|IF_SB|IF_AR2},
    {I_SHUFPS, 3, {XMMREG,XMMREG,IMMEDIATE,0,0}, nasm_bytecodes+7592, IF_KATMAI|IF_SSE|IF_SB|IF_AR2},
    ITEMPLATE_END
};

static const struct itemplate instrux_SIDT[] = {
    {I_SIDT, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+18347, IF_286},
    ITEMPLATE_END
};

static const struct itemplate instrux_SKINIT[] = {
    {I_SKINIT, 0, {0,0,0,0,0}, nasm_bytecodes+18352, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_SLDT[] = {
    {I_SLDT, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14289, IF_286},
    {I_SLDT, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+14289, IF_286},
    {I_SLDT, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+14270, IF_286},
    {I_SLDT, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+14276, IF_386},
    {I_SLDT, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+14282, IF_X64},
    {I_SLDT, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+14288, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_SLWPCB[] = {
    {I_SLWPCB, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+12576, IF_AMD|IF_386},
    {I_SLWPCB, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+12583, IF_AMD|IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_SMI[] = {
    {I_SMI, 0, {0,0,0,0,0}, nasm_bytecodes+19704, IF_386|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_SMINT[] = {
    {I_SMINT, 0, {0,0,0,0,0}, nasm_bytecodes+19564, IF_P6|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_SMINTOLD[] = {
    {I_SMINTOLD, 0, {0,0,0,0,0}, nasm_bytecodes+19568, IF_486|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_SMSW[] = {
    {I_SMSW, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14301, IF_286},
    {I_SMSW, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+14301, IF_286},
    {I_SMSW, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+14294, IF_286},
    {I_SMSW, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+14300, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_SQRTPD[] = {
    {I_SQRTPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15416, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_SQRTPS[] = {
    {I_SQRTPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14660, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_SQRTSD[] = {
    {I_SQRTSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15422, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_SQRTSS[] = {
    {I_SQRTSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14666, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_STC[] = {
    {I_STC, 0, {0,0,0,0,0}, nasm_bytecodes+18084, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_STD[] = {
    {I_STD, 0, {0,0,0,0,0}, nasm_bytecodes+19737, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_STGI[] = {
    {I_STGI, 0, {0,0,0,0,0}, nasm_bytecodes+18357, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_STI[] = {
    {I_STI, 0, {0,0,0,0,0}, nasm_bytecodes+19740, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_STMXCSR[] = {
    {I_STMXCSR, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+12543, IF_KATMAI|IF_SSE|IF_SD},
    ITEMPLATE_END
};

static const struct itemplate instrux_STOSB[] = {
    {I_STOSB, 0, {0,0,0,0,0}, nasm_bytecodes+5317, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_STOSD[] = {
    {I_STOSD, 0, {0,0,0,0,0}, nasm_bytecodes+19572, IF_386},
    ITEMPLATE_END
};

static const struct itemplate instrux_STOSQ[] = {
    {I_STOSQ, 0, {0,0,0,0,0}, nasm_bytecodes+19576, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_STOSW[] = {
    {I_STOSW, 0, {0,0,0,0,0}, nasm_bytecodes+19580, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_STR[] = {
    {I_STR, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14319, IF_286|IF_PROT},
    {I_STR, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+14319, IF_286|IF_PROT},
    {I_STR, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+14306, IF_286|IF_PROT},
    {I_STR, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+14312, IF_386|IF_PROT},
    {I_STR, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+14318, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_SUB[] = {
    {I_SUB, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19584, IF_8086|IF_SM},
    {I_SUB, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19584, IF_8086},
    {I_SUB, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18362, IF_8086|IF_SM},
    {I_SUB, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18362, IF_8086},
    {I_SUB, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18367, IF_386|IF_SM},
    {I_SUB, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18367, IF_386},
    {I_SUB, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18372, IF_X64|IF_SM},
    {I_SUB, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18372, IF_X64},
    {I_SUB, 2, {REG_GPR|BITS8,MEMORY,0,0,0}, nasm_bytecodes+9786, IF_8086|IF_SM},
    {I_SUB, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+9786, IF_8086},
    {I_SUB, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+18377, IF_8086|IF_SM},
    {I_SUB, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18377, IF_8086},
    {I_SUB, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+18382, IF_386|IF_SM},
    {I_SUB, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18382, IF_386},
    {I_SUB, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+18387, IF_X64|IF_SM},
    {I_SUB, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18387, IF_X64},
    {I_SUB, 2, {RM_GPR|BITS16,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+14324, IF_8086},
    {I_SUB, 2, {RM_GPR|BITS32,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+14330, IF_386},
    {I_SUB, 2, {RM_GPR|BITS64,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+14336, IF_X64},
    {I_SUB, 2, {REG_AL,IMMEDIATE,0,0,0}, nasm_bytecodes+19588, IF_8086|IF_SM},
    {I_SUB, 2, {REG_AX,SBYTE16,0,0,0}, nasm_bytecodes+14324, IF_8086|IF_SM},
    {I_SUB, 2, {REG_AX,IMMEDIATE,0,0,0}, nasm_bytecodes+18392, IF_8086|IF_SM},
    {I_SUB, 2, {REG_EAX,SBYTE32,0,0,0}, nasm_bytecodes+14330, IF_386|IF_SM},
    {I_SUB, 2, {REG_EAX,IMMEDIATE,0,0,0}, nasm_bytecodes+18397, IF_386|IF_SM},
    {I_SUB, 2, {REG_RAX,SBYTE64,0,0,0}, nasm_bytecodes+14336, IF_X64|IF_SM},
    {I_SUB, 2, {REG_RAX,IMMEDIATE,0,0,0}, nasm_bytecodes+18402, IF_X64|IF_SM},
    {I_SUB, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+18407, IF_8086|IF_SM},
    {I_SUB, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14342, IF_8086|IF_SM},
    {I_SUB, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14348, IF_386|IF_SM},
    {I_SUB, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14354, IF_X64|IF_SM},
    {I_SUB, 2, {MEMORY,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+18407, IF_8086|IF_SM},
    {I_SUB, 2, {MEMORY,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+14342, IF_8086|IF_SM},
    {I_SUB, 2, {MEMORY,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+14348, IF_386|IF_SM},
    ITEMPLATE_END
};

static const struct itemplate instrux_SUBPD[] = {
    {I_SUBPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15428, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_SUBPS[] = {
    {I_SUBPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14672, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_SUBSD[] = {
    {I_SUBSD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15434, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_SUBSS[] = {
    {I_SUBSS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14678, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_SVDC[] = {
    {I_SVDC, 2, {MEMORY|BITS80,REG_SREG,0,0,0}, nasm_bytecodes+7874, IF_486|IF_CYRIX|IF_SMM},
    ITEMPLATE_END
};

static const struct itemplate instrux_SVLDT[] = {
    {I_SVLDT, 1, {MEMORY|BITS80,0,0,0,0}, nasm_bytecodes+18412, IF_486|IF_CYRIX|IF_SMM},
    ITEMPLATE_END
};

static const struct itemplate instrux_SVTS[] = {
    {I_SVTS, 1, {MEMORY|BITS80,0,0,0,0}, nasm_bytecodes+18417, IF_486|IF_CYRIX|IF_SMM},
    ITEMPLATE_END
};

static const struct itemplate instrux_SWAPGS[] = {
    {I_SWAPGS, 0, {0,0,0,0,0}, nasm_bytecodes+18422, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_SYSCALL[] = {
    {I_SYSCALL, 0, {0,0,0,0,0}, nasm_bytecodes+19292, IF_P6|IF_AMD},
    ITEMPLATE_END
};

static const struct itemplate instrux_SYSENTER[] = {
    {I_SYSENTER, 0, {0,0,0,0,0}, nasm_bytecodes+19592, IF_P6},
    ITEMPLATE_END
};

static const struct itemplate instrux_SYSEXIT[] = {
    {I_SYSEXIT, 0, {0,0,0,0,0}, nasm_bytecodes+19596, IF_P6|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_SYSRET[] = {
    {I_SYSRET, 0, {0,0,0,0,0}, nasm_bytecodes+19288, IF_P6|IF_PRIV|IF_AMD},
    ITEMPLATE_END
};

static const struct itemplate instrux_TEST[] = {
    {I_TEST, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19600, IF_8086|IF_SM},
    {I_TEST, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19600, IF_8086},
    {I_TEST, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18427, IF_8086|IF_SM},
    {I_TEST, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18427, IF_8086},
    {I_TEST, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18432, IF_386|IF_SM},
    {I_TEST, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18432, IF_386},
    {I_TEST, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18437, IF_X64|IF_SM},
    {I_TEST, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18437, IF_X64},
    {I_TEST, 2, {REG_GPR|BITS8,MEMORY,0,0,0}, nasm_bytecodes+19604, IF_8086|IF_SM},
    {I_TEST, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+18442, IF_8086|IF_SM},
    {I_TEST, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+18447, IF_386|IF_SM},
    {I_TEST, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+18452, IF_X64|IF_SM},
    {I_TEST, 2, {REG_AL,IMMEDIATE,0,0,0}, nasm_bytecodes+19608, IF_8086|IF_SM},
    {I_TEST, 2, {REG_AX,IMMEDIATE,0,0,0}, nasm_bytecodes+18457, IF_8086|IF_SM},
    {I_TEST, 2, {REG_EAX,IMMEDIATE,0,0,0}, nasm_bytecodes+18462, IF_386|IF_SM},
    {I_TEST, 2, {REG_RAX,IMMEDIATE,0,0,0}, nasm_bytecodes+18467, IF_X64|IF_SM},
    {I_TEST, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+18472, IF_8086|IF_SM},
    {I_TEST, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14360, IF_8086|IF_SM},
    {I_TEST, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14366, IF_386|IF_SM},
    {I_TEST, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14372, IF_X64|IF_SM},
    {I_TEST, 2, {MEMORY,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+18472, IF_8086|IF_SM},
    {I_TEST, 2, {MEMORY,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+14360, IF_8086|IF_SM},
    {I_TEST, 2, {MEMORY,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+14366, IF_386|IF_SM},
    ITEMPLATE_END
};

static const struct itemplate instrux_UCOMISD[] = {
    {I_UCOMISD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15440, IF_WILLAMETTE|IF_SSE2},
    ITEMPLATE_END
};

static const struct itemplate instrux_UCOMISS[] = {
    {I_UCOMISS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14684, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_UD0[] = {
    {I_UD0, 0, {0,0,0,0,0}, nasm_bytecodes+19612, IF_186|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_UD1[] = {
    {I_UD1, 0, {0,0,0,0,0}, nasm_bytecodes+19616, IF_186|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_UD2[] = {
    {I_UD2, 0, {0,0,0,0,0}, nasm_bytecodes+19620, IF_186},
    ITEMPLATE_END
};

static const struct itemplate instrux_UD2A[] = {
    {I_UD2A, 0, {0,0,0,0,0}, nasm_bytecodes+19620, IF_186},
    ITEMPLATE_END
};

static const struct itemplate instrux_UD2B[] = {
    {I_UD2B, 0, {0,0,0,0,0}, nasm_bytecodes+19616, IF_186|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_UMOV[] = {
    {I_UMOV, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+14378, IF_386|IF_UNDOC|IF_SM},
    {I_UMOV, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+14378, IF_386|IF_UNDOC},
    {I_UMOV, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+7473, IF_386|IF_UNDOC|IF_SM},
    {I_UMOV, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+7473, IF_386|IF_UNDOC},
    {I_UMOV, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+7480, IF_386|IF_UNDOC|IF_SM},
    {I_UMOV, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+7480, IF_386|IF_UNDOC},
    {I_UMOV, 2, {REG_GPR|BITS8,MEMORY,0,0,0}, nasm_bytecodes+14384, IF_386|IF_UNDOC|IF_SM},
    {I_UMOV, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+14384, IF_386|IF_UNDOC},
    {I_UMOV, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+7487, IF_386|IF_UNDOC|IF_SM},
    {I_UMOV, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+7487, IF_386|IF_UNDOC},
    {I_UMOV, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+7494, IF_386|IF_UNDOC|IF_SM},
    {I_UMOV, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+7494, IF_386|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_UNPCKHPD[] = {
    {I_UNPCKHPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15446, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_UNPCKHPS[] = {
    {I_UNPCKHPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14690, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_UNPCKLPD[] = {
    {I_UNPCKLPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15452, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_UNPCKLPS[] = {
    {I_UNPCKLPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14696, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VADDPD[] = {
    {I_VADDPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+8495, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8502, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+8509, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+8516, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VADDPS[] = {
    {I_VADDPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+8523, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8530, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+8537, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+8544, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VADDSD[] = {
    {I_VADDSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+8551, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+8558, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VADDSS[] = {
    {I_VADDSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+8565, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDSS, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+8572, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VADDSUBPD[] = {
    {I_VADDSUBPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+8579, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDSUBPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8586, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDSUBPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+8593, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDSUBPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+8600, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VADDSUBPS[] = {
    {I_VADDSUBPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+8607, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDSUBPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8614, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDSUBPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+8621, IF_AVX|IF_SANDYBRIDGE},
    {I_VADDSUBPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+8628, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VAESDEC[] = {
    {I_VAESDEC, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+8460, IF_AVX|IF_SANDYBRIDGE},
    {I_VAESDEC, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8467, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VAESDECLAST[] = {
    {I_VAESDECLAST, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+8474, IF_AVX|IF_SANDYBRIDGE},
    {I_VAESDECLAST, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8481, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VAESENC[] = {
    {I_VAESENC, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+8432, IF_AVX|IF_SANDYBRIDGE},
    {I_VAESENC, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8439, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VAESENCLAST[] = {
    {I_VAESENCLAST, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+8446, IF_AVX|IF_SANDYBRIDGE},
    {I_VAESENCLAST, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8453, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VAESIMC[] = {
    {I_VAESIMC, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8488, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VAESKEYGENASSIST[] = {
    {I_VAESKEYGENASSIST, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+5864, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VANDNPD[] = {
    {I_VANDNPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+8691, IF_AVX|IF_SANDYBRIDGE},
    {I_VANDNPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8698, IF_AVX|IF_SANDYBRIDGE},
    {I_VANDNPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+8705, IF_AVX|IF_SANDYBRIDGE},
    {I_VANDNPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+8712, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VANDNPS[] = {
    {I_VANDNPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+8719, IF_AVX|IF_SANDYBRIDGE},
    {I_VANDNPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8726, IF_AVX|IF_SANDYBRIDGE},
    {I_VANDNPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+8733, IF_AVX|IF_SANDYBRIDGE},
    {I_VANDNPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+8740, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VANDPD[] = {
    {I_VANDPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+8635, IF_AVX|IF_SANDYBRIDGE},
    {I_VANDPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8642, IF_AVX|IF_SANDYBRIDGE},
    {I_VANDPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+8649, IF_AVX|IF_SANDYBRIDGE},
    {I_VANDPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+8656, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VANDPS[] = {
    {I_VANDPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+8663, IF_AVX|IF_SANDYBRIDGE},
    {I_VANDPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8670, IF_AVX|IF_SANDYBRIDGE},
    {I_VANDPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+8677, IF_AVX|IF_SANDYBRIDGE},
    {I_VANDPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+8684, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VBLENDPD[] = {
    {I_VBLENDPD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+5872, IF_AVX|IF_SANDYBRIDGE},
    {I_VBLENDPD, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+5880, IF_AVX|IF_SANDYBRIDGE},
    {I_VBLENDPD, 4, {YMMREG,YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0}, nasm_bytecodes+5888, IF_AVX|IF_SANDYBRIDGE},
    {I_VBLENDPD, 3, {YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+5896, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VBLENDPS[] = {
    {I_VBLENDPS, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+5904, IF_AVX|IF_SANDYBRIDGE},
    {I_VBLENDPS, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+5912, IF_AVX|IF_SANDYBRIDGE},
    {I_VBLENDPS, 4, {YMMREG,YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0}, nasm_bytecodes+5920, IF_AVX|IF_SANDYBRIDGE},
    {I_VBLENDPS, 3, {YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+5928, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VBLENDVPD[] = {
    {I_VBLENDVPD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+54, IF_AVX|IF_SANDYBRIDGE},
    {I_VBLENDVPD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+63, IF_AVX|IF_SANDYBRIDGE},
    {I_VBLENDVPD, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+72, IF_AVX|IF_SANDYBRIDGE},
    {I_VBLENDVPD, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+81, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VBLENDVPS[] = {
    {I_VBLENDVPS, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+90, IF_AVX|IF_SANDYBRIDGE},
    {I_VBLENDVPS, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+99, IF_AVX|IF_SANDYBRIDGE},
    {I_VBLENDVPS, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+108, IF_AVX|IF_SANDYBRIDGE},
    {I_VBLENDVPS, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+117, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VBROADCASTF128[] = {
    {I_VBROADCASTF128, 2, {YMMREG,MEMORY|BITS128,0,0,0}, nasm_bytecodes+8768, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VBROADCASTSD[] = {
    {I_VBROADCASTSD, 2, {YMMREG,MEMORY|BITS64,0,0,0}, nasm_bytecodes+8761, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VBROADCASTSS[] = {
    {I_VBROADCASTSS, 2, {XMMREG,MEMORY|BITS32,0,0,0}, nasm_bytecodes+8747, IF_AVX|IF_SANDYBRIDGE},
    {I_VBROADCASTSS, 2, {YMMREG,MEMORY|BITS32,0,0,0}, nasm_bytecodes+8754, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQPD[] = {
    {I_VCMPEQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+126, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+135, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+144, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+153, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQPS[] = {
    {I_VCMPEQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1278, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1287, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1296, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1305, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQSD[] = {
    {I_VCMPEQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2430, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2439, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQSS[] = {
    {I_VCMPEQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3006, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3015, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQ_OSPD[] = {
    {I_VCMPEQ_OSPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+126, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+135, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+144, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+153, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+702, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+711, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+720, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+729, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQ_OSPS[] = {
    {I_VCMPEQ_OSPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1278, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1287, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1296, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1305, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1854, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1863, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1872, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1881, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQ_OSSD[] = {
    {I_VCMPEQ_OSSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2430, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2439, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2718, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2727, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQ_OSSS[] = {
    {I_VCMPEQ_OSSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3006, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3015, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3294, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_OSSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3303, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQ_UQPD[] = {
    {I_VCMPEQ_UQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+414, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_UQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+423, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_UQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+432, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_UQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+441, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQ_UQPS[] = {
    {I_VCMPEQ_UQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1566, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_UQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1575, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_UQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1584, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_UQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1593, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQ_UQSD[] = {
    {I_VCMPEQ_UQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2574, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_UQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2583, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQ_UQSS[] = {
    {I_VCMPEQ_UQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3150, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_UQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3159, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQ_USPD[] = {
    {I_VCMPEQ_USPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+990, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_USPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+999, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_USPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1008, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_USPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1017, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQ_USPS[] = {
    {I_VCMPEQ_USPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+2142, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_USPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+2151, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_USPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+2160, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_USPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+2169, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQ_USSD[] = {
    {I_VCMPEQ_USSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2862, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_USSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2871, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPEQ_USSS[] = {
    {I_VCMPEQ_USSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3438, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPEQ_USSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3447, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPFALSEPD[] = {
    {I_VCMPFALSEPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+522, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSEPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+531, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSEPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+540, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSEPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+549, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPFALSEPS[] = {
    {I_VCMPFALSEPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1674, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSEPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1683, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSEPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1692, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSEPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1701, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPFALSESD[] = {
    {I_VCMPFALSESD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2628, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSESD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2637, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPFALSESS[] = {
    {I_VCMPFALSESS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3204, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSESS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3213, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPFALSE_OQPD[] = {
    {I_VCMPFALSE_OQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+522, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+531, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+540, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+549, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPFALSE_OQPS[] = {
    {I_VCMPFALSE_OQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1674, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1683, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1692, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1701, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPFALSE_OQSD[] = {
    {I_VCMPFALSE_OQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2628, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2637, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPFALSE_OQSS[] = {
    {I_VCMPFALSE_OQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3204, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3213, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPFALSE_OSPD[] = {
    {I_VCMPFALSE_OSPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1098, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OSPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1107, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OSPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1116, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OSPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1125, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPFALSE_OSPS[] = {
    {I_VCMPFALSE_OSPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+2250, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OSPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+2259, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OSPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+2268, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OSPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+2277, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPFALSE_OSSD[] = {
    {I_VCMPFALSE_OSSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2916, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OSSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2925, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPFALSE_OSSS[] = {
    {I_VCMPFALSE_OSSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3492, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPFALSE_OSSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3501, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGEPD[] = {
    {I_VCMPGEPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+594, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGEPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+603, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGEPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+612, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGEPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+621, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGEPS[] = {
    {I_VCMPGEPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1746, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGEPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1755, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGEPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1764, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGEPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1773, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGESD[] = {
    {I_VCMPGESD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2664, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGESD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2673, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGESS[] = {
    {I_VCMPGESS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3240, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGESS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3249, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGE_OQPD[] = {
    {I_VCMPGE_OQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1170, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1179, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1188, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1197, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGE_OQPS[] = {
    {I_VCMPGE_OQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+2322, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+2331, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+2340, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+2349, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGE_OQSD[] = {
    {I_VCMPGE_OQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2952, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2961, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGE_OQSS[] = {
    {I_VCMPGE_OQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3528, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3537, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGE_OSPD[] = {
    {I_VCMPGE_OSPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+594, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OSPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+603, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OSPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+612, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OSPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+621, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGE_OSPS[] = {
    {I_VCMPGE_OSPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1746, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OSPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1755, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OSPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1764, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OSPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1773, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGE_OSSD[] = {
    {I_VCMPGE_OSSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2664, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OSSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2673, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGE_OSSS[] = {
    {I_VCMPGE_OSSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3240, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGE_OSSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3249, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGTPD[] = {
    {I_VCMPGTPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+630, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGTPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+639, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGTPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+648, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGTPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+657, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGTPS[] = {
    {I_VCMPGTPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1782, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGTPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1791, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGTPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1800, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGTPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1809, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGTSD[] = {
    {I_VCMPGTSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2682, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGTSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2691, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGTSS[] = {
    {I_VCMPGTSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3258, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGTSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3267, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGT_OQPD[] = {
    {I_VCMPGT_OQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1206, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1215, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1224, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1233, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGT_OQPS[] = {
    {I_VCMPGT_OQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+2358, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+2367, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+2376, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+2385, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGT_OQSD[] = {
    {I_VCMPGT_OQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2970, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2979, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGT_OQSS[] = {
    {I_VCMPGT_OQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3546, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3555, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGT_OSPD[] = {
    {I_VCMPGT_OSPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+630, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OSPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+639, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OSPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+648, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OSPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+657, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGT_OSPS[] = {
    {I_VCMPGT_OSPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1782, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OSPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1791, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OSPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1800, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OSPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1809, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGT_OSSD[] = {
    {I_VCMPGT_OSSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2682, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OSSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2691, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPGT_OSSS[] = {
    {I_VCMPGT_OSSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3258, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPGT_OSSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3267, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLEPD[] = {
    {I_VCMPLEPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+198, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLEPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+207, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLEPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+216, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLEPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+225, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLEPS[] = {
    {I_VCMPLEPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1350, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLEPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1359, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLEPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1368, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLEPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1377, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLESD[] = {
    {I_VCMPLESD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2466, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLESD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2475, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLESS[] = {
    {I_VCMPLESS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3042, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLESS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3051, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLE_OQPD[] = {
    {I_VCMPLE_OQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+774, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+783, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+792, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+801, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLE_OQPS[] = {
    {I_VCMPLE_OQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1926, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1935, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1944, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1953, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLE_OQSD[] = {
    {I_VCMPLE_OQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2754, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2763, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLE_OQSS[] = {
    {I_VCMPLE_OQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3330, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3339, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLE_OSPD[] = {
    {I_VCMPLE_OSPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+198, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OSPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+207, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OSPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+216, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OSPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+225, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLE_OSPS[] = {
    {I_VCMPLE_OSPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1350, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OSPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1359, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OSPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1368, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OSPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1377, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLE_OSSD[] = {
    {I_VCMPLE_OSSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2466, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OSSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2475, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLE_OSSS[] = {
    {I_VCMPLE_OSSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3042, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLE_OSSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3051, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLTPD[] = {
    {I_VCMPLTPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+162, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLTPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+171, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLTPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+180, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLTPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+189, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLTPS[] = {
    {I_VCMPLTPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1314, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLTPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1323, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLTPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1332, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLTPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1341, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLTSD[] = {
    {I_VCMPLTSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2448, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLTSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2457, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLTSS[] = {
    {I_VCMPLTSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3024, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLTSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3033, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLT_OQPD[] = {
    {I_VCMPLT_OQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+738, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+747, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+756, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+765, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLT_OQPS[] = {
    {I_VCMPLT_OQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1890, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1899, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1908, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1917, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLT_OQSD[] = {
    {I_VCMPLT_OQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2736, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2745, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLT_OQSS[] = {
    {I_VCMPLT_OQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3312, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3321, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLT_OSPD[] = {
    {I_VCMPLT_OSPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+162, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OSPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+171, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OSPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+180, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OSPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+189, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLT_OSPS[] = {
    {I_VCMPLT_OSPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1314, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OSPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1323, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OSPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1332, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OSPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1341, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLT_OSSD[] = {
    {I_VCMPLT_OSSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2448, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OSSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2457, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPLT_OSSS[] = {
    {I_VCMPLT_OSSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3024, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPLT_OSSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3033, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQPD[] = {
    {I_VCMPNEQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+270, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+279, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+288, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+297, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQPS[] = {
    {I_VCMPNEQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1422, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1431, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1440, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1449, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQSD[] = {
    {I_VCMPNEQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2502, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2511, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQSS[] = {
    {I_VCMPNEQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3078, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3087, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_OQPD[] = {
    {I_VCMPNEQ_OQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+558, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+567, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+576, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+585, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_OQPS[] = {
    {I_VCMPNEQ_OQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1710, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1719, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1728, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1737, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_OQSD[] = {
    {I_VCMPNEQ_OQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2646, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2655, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_OQSS[] = {
    {I_VCMPNEQ_OQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3222, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3231, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_OSPD[] = {
    {I_VCMPNEQ_OSPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1134, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OSPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1143, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OSPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1152, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OSPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1161, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_OSPS[] = {
    {I_VCMPNEQ_OSPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+2286, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OSPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+2295, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OSPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+2304, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OSPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+2313, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_OSSD[] = {
    {I_VCMPNEQ_OSSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2934, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OSSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2943, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_OSSS[] = {
    {I_VCMPNEQ_OSSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3510, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_OSSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3519, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_UQPD[] = {
    {I_VCMPNEQ_UQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+270, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_UQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+279, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_UQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+288, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_UQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+297, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_UQPS[] = {
    {I_VCMPNEQ_UQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1422, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_UQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1431, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_UQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1440, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_UQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1449, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_UQSD[] = {
    {I_VCMPNEQ_UQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2502, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_UQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2511, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_UQSS[] = {
    {I_VCMPNEQ_UQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3078, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_UQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3087, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_USPD[] = {
    {I_VCMPNEQ_USPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+846, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_USPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+855, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_USPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+864, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_USPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+873, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_USPS[] = {
    {I_VCMPNEQ_USPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1998, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_USPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+2007, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_USPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+2016, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_USPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+2025, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_USSD[] = {
    {I_VCMPNEQ_USSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2790, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_USSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2799, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNEQ_USSS[] = {
    {I_VCMPNEQ_USSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3366, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNEQ_USSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3375, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGEPD[] = {
    {I_VCMPNGEPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+450, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGEPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+459, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGEPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+468, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGEPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+477, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGEPS[] = {
    {I_VCMPNGEPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1602, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGEPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1611, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGEPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1620, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGEPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1629, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGESD[] = {
    {I_VCMPNGESD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2592, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGESD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2601, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGESS[] = {
    {I_VCMPNGESS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3168, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGESS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3177, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGE_UQPD[] = {
    {I_VCMPNGE_UQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1026, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_UQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1035, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_UQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1044, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_UQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1053, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGE_UQPS[] = {
    {I_VCMPNGE_UQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+2178, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_UQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+2187, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_UQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+2196, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_UQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+2205, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGE_UQSD[] = {
    {I_VCMPNGE_UQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2880, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_UQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2889, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGE_UQSS[] = {
    {I_VCMPNGE_UQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3456, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_UQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3465, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGE_USPD[] = {
    {I_VCMPNGE_USPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+450, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_USPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+459, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_USPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+468, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_USPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+477, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGE_USPS[] = {
    {I_VCMPNGE_USPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1602, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_USPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1611, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_USPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1620, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_USPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1629, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGE_USSD[] = {
    {I_VCMPNGE_USSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2592, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_USSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2601, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGE_USSS[] = {
    {I_VCMPNGE_USSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3168, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGE_USSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3177, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGTPD[] = {
    {I_VCMPNGTPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+486, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGTPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+495, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGTPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+504, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGTPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+513, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGTPS[] = {
    {I_VCMPNGTPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1638, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGTPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1647, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGTPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1656, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGTPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1665, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGTSD[] = {
    {I_VCMPNGTSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2610, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGTSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2619, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGTSS[] = {
    {I_VCMPNGTSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3186, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGTSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3195, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGT_UQPD[] = {
    {I_VCMPNGT_UQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1062, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_UQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1071, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_UQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1080, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_UQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1089, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGT_UQPS[] = {
    {I_VCMPNGT_UQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+2214, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_UQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+2223, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_UQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+2232, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_UQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+2241, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGT_UQSD[] = {
    {I_VCMPNGT_UQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2898, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_UQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2907, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGT_UQSS[] = {
    {I_VCMPNGT_UQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3474, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_UQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3483, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGT_USPD[] = {
    {I_VCMPNGT_USPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+486, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_USPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+495, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_USPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+504, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_USPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+513, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGT_USPS[] = {
    {I_VCMPNGT_USPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1638, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_USPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1647, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_USPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1656, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_USPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1665, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGT_USSD[] = {
    {I_VCMPNGT_USSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2610, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_USSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2619, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNGT_USSS[] = {
    {I_VCMPNGT_USSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3186, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNGT_USSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3195, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLEPD[] = {
    {I_VCMPNLEPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+342, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLEPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+351, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLEPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+360, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLEPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+369, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLEPS[] = {
    {I_VCMPNLEPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1494, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLEPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1503, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLEPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1512, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLEPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1521, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLESD[] = {
    {I_VCMPNLESD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2538, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLESD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2547, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLESS[] = {
    {I_VCMPNLESS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3114, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLESS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3123, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLE_UQPD[] = {
    {I_VCMPNLE_UQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+918, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_UQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+927, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_UQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+936, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_UQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+945, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLE_UQPS[] = {
    {I_VCMPNLE_UQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+2070, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_UQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+2079, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_UQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+2088, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_UQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+2097, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLE_UQSD[] = {
    {I_VCMPNLE_UQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2826, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_UQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2835, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLE_UQSS[] = {
    {I_VCMPNLE_UQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3402, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_UQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3411, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLE_USPD[] = {
    {I_VCMPNLE_USPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+342, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_USPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+351, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_USPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+360, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_USPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+369, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLE_USPS[] = {
    {I_VCMPNLE_USPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1494, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_USPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1503, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_USPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1512, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_USPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1521, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLE_USSD[] = {
    {I_VCMPNLE_USSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2538, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_USSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2547, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLE_USSS[] = {
    {I_VCMPNLE_USSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3114, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLE_USSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3123, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLTPD[] = {
    {I_VCMPNLTPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+306, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLTPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+315, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLTPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+324, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLTPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+333, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLTPS[] = {
    {I_VCMPNLTPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1458, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLTPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1467, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLTPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1476, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLTPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1485, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLTSD[] = {
    {I_VCMPNLTSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2520, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLTSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2529, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLTSS[] = {
    {I_VCMPNLTSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3096, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLTSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3105, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLT_UQPD[] = {
    {I_VCMPNLT_UQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+882, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_UQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+891, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_UQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+900, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_UQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+909, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLT_UQPS[] = {
    {I_VCMPNLT_UQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+2034, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_UQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+2043, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_UQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+2052, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_UQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+2061, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLT_UQSD[] = {
    {I_VCMPNLT_UQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2808, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_UQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2817, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLT_UQSS[] = {
    {I_VCMPNLT_UQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3384, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_UQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3393, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLT_USPD[] = {
    {I_VCMPNLT_USPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+306, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_USPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+315, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_USPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+324, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_USPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+333, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLT_USPS[] = {
    {I_VCMPNLT_USPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1458, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_USPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1467, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_USPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1476, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_USPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1485, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLT_USSD[] = {
    {I_VCMPNLT_USSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2520, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_USSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2529, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPNLT_USSS[] = {
    {I_VCMPNLT_USSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3096, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPNLT_USSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3105, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPORDPD[] = {
    {I_VCMPORDPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+378, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORDPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+387, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORDPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+396, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORDPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+405, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPORDPS[] = {
    {I_VCMPORDPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1530, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORDPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1539, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORDPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1548, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORDPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1557, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPORDSD[] = {
    {I_VCMPORDSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2556, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORDSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2565, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPORDSS[] = {
    {I_VCMPORDSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3132, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORDSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3141, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPORD_QPD[] = {
    {I_VCMPORD_QPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+378, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_QPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+387, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_QPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+396, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_QPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+405, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPORD_QPS[] = {
    {I_VCMPORD_QPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1530, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_QPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1539, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_QPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1548, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_QPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1557, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPORD_QSD[] = {
    {I_VCMPORD_QSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2556, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_QSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2565, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPORD_QSS[] = {
    {I_VCMPORD_QSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3132, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_QSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3141, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPORD_SPD[] = {
    {I_VCMPORD_SPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+954, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_SPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+963, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_SPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+972, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_SPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+981, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPORD_SPS[] = {
    {I_VCMPORD_SPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+2106, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_SPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+2115, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_SPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+2124, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_SPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+2133, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPORD_SSD[] = {
    {I_VCMPORD_SSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2844, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_SSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2853, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPORD_SSS[] = {
    {I_VCMPORD_SSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3420, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPORD_SSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3429, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPPD[] = {
    {I_VCMPPD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+5936, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPPD, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+5944, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPPD, 4, {YMMREG,YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0}, nasm_bytecodes+5952, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPPD, 3, {YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+5960, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPPS[] = {
    {I_VCMPPS, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+5968, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPPS, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+5976, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPPS, 4, {YMMREG,YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0}, nasm_bytecodes+5984, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPPS, 3, {YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+5992, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPSD[] = {
    {I_VCMPSD, 4, {XMMREG,XMMREG,RM_XMM|BITS64,IMMEDIATE|BITS8,0}, nasm_bytecodes+6000, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPSD, 3, {XMMREG,RM_XMM|BITS64,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6008, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPSS[] = {
    {I_VCMPSS, 4, {XMMREG,XMMREG,RM_XMM|BITS64,IMMEDIATE|BITS8,0}, nasm_bytecodes+6016, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPSS, 3, {XMMREG,RM_XMM|BITS64,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6024, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPTRUEPD[] = {
    {I_VCMPTRUEPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+666, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUEPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+675, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUEPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+684, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUEPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+693, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPTRUEPS[] = {
    {I_VCMPTRUEPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1818, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUEPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1827, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUEPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1836, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUEPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1845, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPTRUESD[] = {
    {I_VCMPTRUESD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2700, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUESD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2709, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPTRUESS[] = {
    {I_VCMPTRUESS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3276, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUESS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3285, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPTRUE_UQPD[] = {
    {I_VCMPTRUE_UQPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+666, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_UQPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+675, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_UQPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+684, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_UQPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+693, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPTRUE_UQPS[] = {
    {I_VCMPTRUE_UQPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1818, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_UQPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1827, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_UQPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1836, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_UQPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1845, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPTRUE_UQSD[] = {
    {I_VCMPTRUE_UQSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2700, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_UQSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2709, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPTRUE_UQSS[] = {
    {I_VCMPTRUE_UQSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3276, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_UQSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3285, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPTRUE_USPD[] = {
    {I_VCMPTRUE_USPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1242, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_USPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1251, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_USPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1260, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_USPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1269, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPTRUE_USPS[] = {
    {I_VCMPTRUE_USPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+2394, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_USPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+2403, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_USPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+2412, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_USPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+2421, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPTRUE_USSD[] = {
    {I_VCMPTRUE_USSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2988, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_USSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2997, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPTRUE_USSS[] = {
    {I_VCMPTRUE_USSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3564, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPTRUE_USSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3573, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPUNORDPD[] = {
    {I_VCMPUNORDPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+234, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORDPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+243, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORDPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+252, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORDPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+261, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPUNORDPS[] = {
    {I_VCMPUNORDPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1386, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORDPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1395, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORDPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1404, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORDPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1413, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPUNORDSD[] = {
    {I_VCMPUNORDSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2484, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORDSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2493, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPUNORDSS[] = {
    {I_VCMPUNORDSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3060, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORDSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3069, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPUNORD_QPD[] = {
    {I_VCMPUNORD_QPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+234, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_QPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+243, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_QPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+252, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_QPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+261, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPUNORD_QPS[] = {
    {I_VCMPUNORD_QPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1386, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_QPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1395, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_QPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1404, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_QPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1413, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPUNORD_QSD[] = {
    {I_VCMPUNORD_QSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2484, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_QSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2493, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPUNORD_QSS[] = {
    {I_VCMPUNORD_QSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3060, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_QSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3069, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPUNORD_SPD[] = {
    {I_VCMPUNORD_SPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+810, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_SPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+819, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_SPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+828, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_SPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+837, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPUNORD_SPS[] = {
    {I_VCMPUNORD_SPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+1962, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_SPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+1971, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_SPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+1980, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_SPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+1989, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPUNORD_SSD[] = {
    {I_VCMPUNORD_SSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+2772, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_SSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+2781, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCMPUNORD_SSS[] = {
    {I_VCMPUNORD_SSS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3348, IF_AVX|IF_SANDYBRIDGE},
    {I_VCMPUNORD_SSS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+3357, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCOMISD[] = {
    {I_VCOMISD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+8775, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCOMISS[] = {
    {I_VCOMISS, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+8782, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTDQ2PD[] = {
    {I_VCVTDQ2PD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+8789, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTDQ2PD, 2, {YMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8796, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTDQ2PS[] = {
    {I_VCVTDQ2PS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8803, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTDQ2PS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+8810, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTPD2DQ[] = {
    {I_VCVTPD2DQ, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+8817, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTPD2DQ, 2, {XMMREG,MEMORY|BITS128,0,0,0}, nasm_bytecodes+8817, IF_AVX|IF_SANDYBRIDGE|IF_SO},
    {I_VCVTPD2DQ, 2, {XMMREG,YMMREG,0,0,0}, nasm_bytecodes+8824, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTPD2DQ, 2, {XMMREG,MEMORY|BITS256,0,0,0}, nasm_bytecodes+8824, IF_AVX|IF_SANDYBRIDGE|IF_SY},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTPD2PS[] = {
    {I_VCVTPD2PS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+8831, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTPD2PS, 2, {XMMREG,MEMORY|BITS128,0,0,0}, nasm_bytecodes+8831, IF_AVX|IF_SANDYBRIDGE|IF_SO},
    {I_VCVTPD2PS, 2, {XMMREG,YMMREG,0,0,0}, nasm_bytecodes+8838, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTPD2PS, 2, {XMMREG,MEMORY|BITS256,0,0,0}, nasm_bytecodes+8838, IF_AVX|IF_SANDYBRIDGE|IF_SY},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTPH2PS[] = {
    {I_VCVTPH2PS, 2, {YMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12548, IF_AVX|IF_FUTURE},
    {I_VCVTPH2PS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+12555, IF_AVX|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTPS2DQ[] = {
    {I_VCVTPS2DQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8845, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTPS2DQ, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+8852, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTPS2PD[] = {
    {I_VCVTPS2PD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+8859, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTPS2PD, 2, {YMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8866, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTPS2PH[] = {
    {I_VCVTPS2PH, 3, {RM_XMM|BITS128,YMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6680, IF_AVX|IF_FUTURE},
    {I_VCVTPS2PH, 3, {RM_XMM|BITS64,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6688, IF_AVX|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTSD2SI[] = {
    {I_VCVTSD2SI, 2, {REG_GPR|BITS32,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+8873, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTSD2SI, 2, {REG_GPR|BITS64,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+8880, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTSD2SS[] = {
    {I_VCVTSD2SS, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+8887, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTSD2SS, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+8894, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTSI2SD[] = {
    {I_VCVTSI2SD, 3, {XMMREG,XMMREG,RM_GPR|BITS32,0,0}, nasm_bytecodes+8901, IF_AVX|IF_SANDYBRIDGE|IF_SD},
    {I_VCVTSI2SD, 2, {XMMREG,RM_GPR|BITS32,0,0,0}, nasm_bytecodes+8908, IF_AVX|IF_SANDYBRIDGE|IF_SD},
    {I_VCVTSI2SD, 3, {XMMREG,XMMREG,MEMORY|BITS32,0,0}, nasm_bytecodes+8901, IF_AVX|IF_SANDYBRIDGE|IF_SD},
    {I_VCVTSI2SD, 2, {XMMREG,MEMORY|BITS32,0,0,0}, nasm_bytecodes+8908, IF_AVX|IF_SANDYBRIDGE|IF_SD},
    {I_VCVTSI2SD, 3, {XMMREG,XMMREG,RM_GPR|BITS64,0,0}, nasm_bytecodes+8915, IF_AVX|IF_SANDYBRIDGE|IF_LONG|IF_SQ},
    {I_VCVTSI2SD, 2, {XMMREG,RM_GPR|BITS64,0,0,0}, nasm_bytecodes+8922, IF_AVX|IF_SANDYBRIDGE|IF_LONG|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTSI2SS[] = {
    {I_VCVTSI2SS, 3, {XMMREG,XMMREG,RM_GPR|BITS32,0,0}, nasm_bytecodes+8929, IF_AVX|IF_SANDYBRIDGE|IF_SD},
    {I_VCVTSI2SS, 2, {XMMREG,RM_GPR|BITS32,0,0,0}, nasm_bytecodes+8936, IF_AVX|IF_SANDYBRIDGE|IF_SD},
    {I_VCVTSI2SS, 3, {XMMREG,XMMREG,MEMORY|BITS32,0,0}, nasm_bytecodes+8929, IF_AVX|IF_SANDYBRIDGE|IF_SD},
    {I_VCVTSI2SS, 2, {XMMREG,MEMORY|BITS32,0,0,0}, nasm_bytecodes+8936, IF_AVX|IF_SANDYBRIDGE|IF_SD},
    {I_VCVTSI2SS, 3, {XMMREG,XMMREG,RM_GPR|BITS64,0,0}, nasm_bytecodes+8943, IF_AVX|IF_SANDYBRIDGE|IF_LONG|IF_SQ},
    {I_VCVTSI2SS, 2, {XMMREG,RM_GPR|BITS64,0,0,0}, nasm_bytecodes+8950, IF_AVX|IF_SANDYBRIDGE|IF_LONG|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTSS2SD[] = {
    {I_VCVTSS2SD, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+8957, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTSS2SD, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+8964, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTSS2SI[] = {
    {I_VCVTSS2SI, 2, {REG_GPR|BITS32,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+8971, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTSS2SI, 2, {REG_GPR|BITS64,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+8978, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTTPD2DQ[] = {
    {I_VCVTTPD2DQ, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+8985, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTTPD2DQ, 2, {XMMREG,MEMORY|BITS128,0,0,0}, nasm_bytecodes+8985, IF_AVX|IF_SANDYBRIDGE|IF_SO},
    {I_VCVTTPD2DQ, 2, {XMMREG,YMMREG,0,0,0}, nasm_bytecodes+8992, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTTPD2DQ, 2, {XMMREG,MEMORY|BITS256,0,0,0}, nasm_bytecodes+8992, IF_AVX|IF_SANDYBRIDGE|IF_SY},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTTPS2DQ[] = {
    {I_VCVTTPS2DQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+8999, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTTPS2DQ, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9006, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTTSD2SI[] = {
    {I_VCVTTSD2SI, 2, {REG_GPR|BITS32,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+9013, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTTSD2SI, 2, {REG_GPR|BITS64,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+9020, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_VCVTTSS2SI[] = {
    {I_VCVTTSS2SI, 2, {REG_GPR|BITS32,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+9027, IF_AVX|IF_SANDYBRIDGE},
    {I_VCVTTSS2SI, 2, {REG_GPR|BITS64,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+9034, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_VDIVPD[] = {
    {I_VDIVPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+9041, IF_AVX|IF_SANDYBRIDGE},
    {I_VDIVPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9048, IF_AVX|IF_SANDYBRIDGE},
    {I_VDIVPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+9055, IF_AVX|IF_SANDYBRIDGE},
    {I_VDIVPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9062, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VDIVPS[] = {
    {I_VDIVPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+9069, IF_AVX|IF_SANDYBRIDGE},
    {I_VDIVPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9076, IF_AVX|IF_SANDYBRIDGE},
    {I_VDIVPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+9083, IF_AVX|IF_SANDYBRIDGE},
    {I_VDIVPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9090, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VDIVSD[] = {
    {I_VDIVSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+9097, IF_AVX|IF_SANDYBRIDGE},
    {I_VDIVSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+9104, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VDIVSS[] = {
    {I_VDIVSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+9111, IF_AVX|IF_SANDYBRIDGE},
    {I_VDIVSS, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+9118, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VDPPD[] = {
    {I_VDPPD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6032, IF_AVX|IF_SANDYBRIDGE},
    {I_VDPPD, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6040, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VDPPS[] = {
    {I_VDPPS, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6048, IF_AVX|IF_SANDYBRIDGE},
    {I_VDPPS, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6056, IF_AVX|IF_SANDYBRIDGE},
    {I_VDPPS, 4, {YMMREG,YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0}, nasm_bytecodes+6064, IF_AVX|IF_SANDYBRIDGE},
    {I_VDPPS, 3, {YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6072, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VERR[] = {
    {I_VERR, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+18477, IF_286|IF_PROT},
    {I_VERR, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+18477, IF_286|IF_PROT},
    {I_VERR, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+18477, IF_286|IF_PROT},
    ITEMPLATE_END
};

static const struct itemplate instrux_VERW[] = {
    {I_VERW, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+18482, IF_286|IF_PROT},
    {I_VERW, 1, {MEMORY|BITS16,0,0,0,0}, nasm_bytecodes+18482, IF_286|IF_PROT},
    {I_VERW, 1, {REG_GPR|BITS16,0,0,0,0}, nasm_bytecodes+18482, IF_286|IF_PROT},
    ITEMPLATE_END
};

static const struct itemplate instrux_VEXTRACTF128[] = {
    {I_VEXTRACTF128, 3, {RM_XMM|BITS128,YMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6080, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VEXTRACTPS[] = {
    {I_VEXTRACTPS, 3, {RM_GPR|BITS32,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6088, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD123PD[] = {
    {I_VFMADD123PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11890, IF_FMA|IF_FUTURE},
    {I_VFMADD123PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11897, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD123PS[] = {
    {I_VFMADD123PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11876, IF_FMA|IF_FUTURE},
    {I_VFMADD123PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11883, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD123SD[] = {
    {I_VFMADD123SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12373, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD123SS[] = {
    {I_VFMADD123SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12366, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD132PD[] = {
    {I_VFMADD132PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11862, IF_FMA|IF_FUTURE},
    {I_VFMADD132PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11869, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD132PS[] = {
    {I_VFMADD132PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11848, IF_FMA|IF_FUTURE},
    {I_VFMADD132PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11855, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD132SD[] = {
    {I_VFMADD132SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12359, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD132SS[] = {
    {I_VFMADD132SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12352, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD213PD[] = {
    {I_VFMADD213PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11890, IF_FMA|IF_FUTURE},
    {I_VFMADD213PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11897, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD213PS[] = {
    {I_VFMADD213PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11876, IF_FMA|IF_FUTURE},
    {I_VFMADD213PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11883, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD213SD[] = {
    {I_VFMADD213SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12373, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD213SS[] = {
    {I_VFMADD213SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12366, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD231PD[] = {
    {I_VFMADD231PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11918, IF_FMA|IF_FUTURE},
    {I_VFMADD231PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11925, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD231PS[] = {
    {I_VFMADD231PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11904, IF_FMA|IF_FUTURE},
    {I_VFMADD231PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11911, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD231SD[] = {
    {I_VFMADD231SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12387, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD231SS[] = {
    {I_VFMADD231SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12380, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD312PD[] = {
    {I_VFMADD312PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11862, IF_FMA|IF_FUTURE},
    {I_VFMADD312PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11869, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD312PS[] = {
    {I_VFMADD312PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11848, IF_FMA|IF_FUTURE},
    {I_VFMADD312PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11855, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD312SD[] = {
    {I_VFMADD312SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12359, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD312SS[] = {
    {I_VFMADD312SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12352, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD321PD[] = {
    {I_VFMADD321PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11918, IF_FMA|IF_FUTURE},
    {I_VFMADD321PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11925, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD321PS[] = {
    {I_VFMADD321PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11904, IF_FMA|IF_FUTURE},
    {I_VFMADD321PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11911, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD321SD[] = {
    {I_VFMADD321SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12387, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADD321SS[] = {
    {I_VFMADD321SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12380, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDPD[] = {
    {I_VFMADDPD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+3708, IF_AMD|IF_SSE5},
    {I_VFMADDPD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+3717, IF_AMD|IF_SSE5},
    {I_VFMADDPD, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+3726, IF_AMD|IF_SSE5},
    {I_VFMADDPD, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+3735, IF_AMD|IF_SSE5},
    {I_VFMADDPD, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+3744, IF_AMD|IF_SSE5},
    {I_VFMADDPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+3753, IF_AMD|IF_SSE5},
    {I_VFMADDPD, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+3762, IF_AMD|IF_SSE5},
    {I_VFMADDPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+3771, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDPS[] = {
    {I_VFMADDPS, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+3780, IF_AMD|IF_SSE5},
    {I_VFMADDPS, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+3789, IF_AMD|IF_SSE5},
    {I_VFMADDPS, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+3798, IF_AMD|IF_SSE5},
    {I_VFMADDPS, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+3807, IF_AMD|IF_SSE5},
    {I_VFMADDPS, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+3816, IF_AMD|IF_SSE5},
    {I_VFMADDPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+3825, IF_AMD|IF_SSE5},
    {I_VFMADDPS, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+3834, IF_AMD|IF_SSE5},
    {I_VFMADDPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+3843, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSD[] = {
    {I_VFMADDSD, 4, {XMMREG,XMMREG,RM_XMM|BITS64,XMMREG,0}, nasm_bytecodes+3852, IF_AMD|IF_SSE5},
    {I_VFMADDSD, 3, {XMMREG,RM_XMM|BITS64,XMMREG,0,0}, nasm_bytecodes+3861, IF_AMD|IF_SSE5},
    {I_VFMADDSD, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS64,0}, nasm_bytecodes+3870, IF_AMD|IF_SSE5},
    {I_VFMADDSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+3879, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSS[] = {
    {I_VFMADDSS, 4, {XMMREG,XMMREG,RM_XMM|BITS32,XMMREG,0}, nasm_bytecodes+3888, IF_AMD|IF_SSE5},
    {I_VFMADDSS, 3, {XMMREG,RM_XMM|BITS32,XMMREG,0,0}, nasm_bytecodes+3897, IF_AMD|IF_SSE5},
    {I_VFMADDSS, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS32,0}, nasm_bytecodes+3906, IF_AMD|IF_SSE5},
    {I_VFMADDSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+3915, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUB123PD[] = {
    {I_VFMADDSUB123PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11974, IF_FMA|IF_FUTURE},
    {I_VFMADDSUB123PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11981, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUB123PS[] = {
    {I_VFMADDSUB123PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11960, IF_FMA|IF_FUTURE},
    {I_VFMADDSUB123PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11967, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUB132PD[] = {
    {I_VFMADDSUB132PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11946, IF_FMA|IF_FUTURE},
    {I_VFMADDSUB132PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11953, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUB132PS[] = {
    {I_VFMADDSUB132PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11932, IF_FMA|IF_FUTURE},
    {I_VFMADDSUB132PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11939, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUB213PD[] = {
    {I_VFMADDSUB213PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11974, IF_FMA|IF_FUTURE},
    {I_VFMADDSUB213PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11981, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUB213PS[] = {
    {I_VFMADDSUB213PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11960, IF_FMA|IF_FUTURE},
    {I_VFMADDSUB213PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11967, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUB231PD[] = {
    {I_VFMADDSUB231PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12002, IF_FMA|IF_FUTURE},
    {I_VFMADDSUB231PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12009, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUB231PS[] = {
    {I_VFMADDSUB231PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11988, IF_FMA|IF_FUTURE},
    {I_VFMADDSUB231PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11995, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUB312PD[] = {
    {I_VFMADDSUB312PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11946, IF_FMA|IF_FUTURE},
    {I_VFMADDSUB312PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11953, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUB312PS[] = {
    {I_VFMADDSUB312PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11932, IF_FMA|IF_FUTURE},
    {I_VFMADDSUB312PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11939, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUB321PD[] = {
    {I_VFMADDSUB321PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12002, IF_FMA|IF_FUTURE},
    {I_VFMADDSUB321PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12009, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUB321PS[] = {
    {I_VFMADDSUB321PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11988, IF_FMA|IF_FUTURE},
    {I_VFMADDSUB321PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11995, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUBPD[] = {
    {I_VFMADDSUBPD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+3924, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+3933, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPD, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+3942, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPD, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+3951, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPD, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+3960, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+3969, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPD, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+3978, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+3987, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMADDSUBPS[] = {
    {I_VFMADDSUBPS, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+3996, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPS, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4005, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPS, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+4014, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPS, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+4023, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPS, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+4032, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+4041, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPS, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+4050, IF_AMD|IF_SSE5},
    {I_VFMADDSUBPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+4059, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB123PD[] = {
    {I_VFMSUB123PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12058, IF_FMA|IF_FUTURE},
    {I_VFMSUB123PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12065, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB123PS[] = {
    {I_VFMSUB123PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12044, IF_FMA|IF_FUTURE},
    {I_VFMSUB123PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12051, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB123SD[] = {
    {I_VFMSUB123SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12415, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB123SS[] = {
    {I_VFMSUB123SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12408, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB132PD[] = {
    {I_VFMSUB132PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12030, IF_FMA|IF_FUTURE},
    {I_VFMSUB132PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12037, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB132PS[] = {
    {I_VFMSUB132PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12016, IF_FMA|IF_FUTURE},
    {I_VFMSUB132PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12023, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB132SD[] = {
    {I_VFMSUB132SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12401, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB132SS[] = {
    {I_VFMSUB132SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12394, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB213PD[] = {
    {I_VFMSUB213PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12058, IF_FMA|IF_FUTURE},
    {I_VFMSUB213PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12065, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB213PS[] = {
    {I_VFMSUB213PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12044, IF_FMA|IF_FUTURE},
    {I_VFMSUB213PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12051, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB213SD[] = {
    {I_VFMSUB213SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12415, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB213SS[] = {
    {I_VFMSUB213SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12408, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB231PD[] = {
    {I_VFMSUB231PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12086, IF_FMA|IF_FUTURE},
    {I_VFMSUB231PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12093, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB231PS[] = {
    {I_VFMSUB231PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12072, IF_FMA|IF_FUTURE},
    {I_VFMSUB231PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12079, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB231SD[] = {
    {I_VFMSUB231SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12429, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB231SS[] = {
    {I_VFMSUB231SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12422, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB312PD[] = {
    {I_VFMSUB312PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12030, IF_FMA|IF_FUTURE},
    {I_VFMSUB312PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12037, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB312PS[] = {
    {I_VFMSUB312PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12016, IF_FMA|IF_FUTURE},
    {I_VFMSUB312PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12023, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB312SD[] = {
    {I_VFMSUB312SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12401, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB312SS[] = {
    {I_VFMSUB312SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12394, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB321PD[] = {
    {I_VFMSUB321PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12086, IF_FMA|IF_FUTURE},
    {I_VFMSUB321PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12093, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB321PS[] = {
    {I_VFMSUB321PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12072, IF_FMA|IF_FUTURE},
    {I_VFMSUB321PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12079, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB321SD[] = {
    {I_VFMSUB321SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12429, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUB321SS[] = {
    {I_VFMSUB321SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12422, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADD123PD[] = {
    {I_VFMSUBADD123PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12142, IF_FMA|IF_FUTURE},
    {I_VFMSUBADD123PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12149, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADD123PS[] = {
    {I_VFMSUBADD123PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12128, IF_FMA|IF_FUTURE},
    {I_VFMSUBADD123PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12135, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADD132PD[] = {
    {I_VFMSUBADD132PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12114, IF_FMA|IF_FUTURE},
    {I_VFMSUBADD132PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12121, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADD132PS[] = {
    {I_VFMSUBADD132PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12100, IF_FMA|IF_FUTURE},
    {I_VFMSUBADD132PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12107, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADD213PD[] = {
    {I_VFMSUBADD213PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12142, IF_FMA|IF_FUTURE},
    {I_VFMSUBADD213PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12149, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADD213PS[] = {
    {I_VFMSUBADD213PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12128, IF_FMA|IF_FUTURE},
    {I_VFMSUBADD213PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12135, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADD231PD[] = {
    {I_VFMSUBADD231PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12170, IF_FMA|IF_FUTURE},
    {I_VFMSUBADD231PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12177, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADD231PS[] = {
    {I_VFMSUBADD231PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12156, IF_FMA|IF_FUTURE},
    {I_VFMSUBADD231PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12163, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADD312PD[] = {
    {I_VFMSUBADD312PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12114, IF_FMA|IF_FUTURE},
    {I_VFMSUBADD312PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12121, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADD312PS[] = {
    {I_VFMSUBADD312PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12100, IF_FMA|IF_FUTURE},
    {I_VFMSUBADD312PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12107, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADD321PD[] = {
    {I_VFMSUBADD321PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12170, IF_FMA|IF_FUTURE},
    {I_VFMSUBADD321PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12177, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADD321PS[] = {
    {I_VFMSUBADD321PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12156, IF_FMA|IF_FUTURE},
    {I_VFMSUBADD321PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12163, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADDPD[] = {
    {I_VFMSUBADDPD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4068, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4077, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPD, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+4086, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPD, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+4095, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPD, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+4104, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+4113, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPD, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+4122, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+4131, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBADDPS[] = {
    {I_VFMSUBADDPS, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4140, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPS, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4149, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPS, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+4158, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPS, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+4167, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPS, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+4176, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+4185, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPS, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+4194, IF_AMD|IF_SSE5},
    {I_VFMSUBADDPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+4203, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBPD[] = {
    {I_VFMSUBPD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4212, IF_AMD|IF_SSE5},
    {I_VFMSUBPD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4221, IF_AMD|IF_SSE5},
    {I_VFMSUBPD, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+4230, IF_AMD|IF_SSE5},
    {I_VFMSUBPD, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+4239, IF_AMD|IF_SSE5},
    {I_VFMSUBPD, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+4248, IF_AMD|IF_SSE5},
    {I_VFMSUBPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+4257, IF_AMD|IF_SSE5},
    {I_VFMSUBPD, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+4266, IF_AMD|IF_SSE5},
    {I_VFMSUBPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+4275, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBPS[] = {
    {I_VFMSUBPS, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4284, IF_AMD|IF_SSE5},
    {I_VFMSUBPS, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4293, IF_AMD|IF_SSE5},
    {I_VFMSUBPS, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+4302, IF_AMD|IF_SSE5},
    {I_VFMSUBPS, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+4311, IF_AMD|IF_SSE5},
    {I_VFMSUBPS, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+4320, IF_AMD|IF_SSE5},
    {I_VFMSUBPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+4329, IF_AMD|IF_SSE5},
    {I_VFMSUBPS, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+4338, IF_AMD|IF_SSE5},
    {I_VFMSUBPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+4347, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBSD[] = {
    {I_VFMSUBSD, 4, {XMMREG,XMMREG,RM_XMM|BITS64,XMMREG,0}, nasm_bytecodes+4356, IF_AMD|IF_SSE5},
    {I_VFMSUBSD, 3, {XMMREG,RM_XMM|BITS64,XMMREG,0,0}, nasm_bytecodes+4365, IF_AMD|IF_SSE5},
    {I_VFMSUBSD, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS64,0}, nasm_bytecodes+4374, IF_AMD|IF_SSE5},
    {I_VFMSUBSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+4383, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFMSUBSS[] = {
    {I_VFMSUBSS, 4, {XMMREG,XMMREG,RM_XMM|BITS32,XMMREG,0}, nasm_bytecodes+4392, IF_AMD|IF_SSE5},
    {I_VFMSUBSS, 3, {XMMREG,RM_XMM|BITS32,XMMREG,0,0}, nasm_bytecodes+4401, IF_AMD|IF_SSE5},
    {I_VFMSUBSS, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS32,0}, nasm_bytecodes+4410, IF_AMD|IF_SSE5},
    {I_VFMSUBSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+4419, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD123PD[] = {
    {I_VFNMADD123PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12226, IF_FMA|IF_FUTURE},
    {I_VFNMADD123PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12233, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD123PS[] = {
    {I_VFNMADD123PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12212, IF_FMA|IF_FUTURE},
    {I_VFNMADD123PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12219, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD123SD[] = {
    {I_VFNMADD123SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12457, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD123SS[] = {
    {I_VFNMADD123SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12450, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD132PD[] = {
    {I_VFNMADD132PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12198, IF_FMA|IF_FUTURE},
    {I_VFNMADD132PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12205, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD132PS[] = {
    {I_VFNMADD132PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12184, IF_FMA|IF_FUTURE},
    {I_VFNMADD132PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12191, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD132SD[] = {
    {I_VFNMADD132SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12443, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD132SS[] = {
    {I_VFNMADD132SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12436, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD213PD[] = {
    {I_VFNMADD213PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12226, IF_FMA|IF_FUTURE},
    {I_VFNMADD213PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12233, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD213PS[] = {
    {I_VFNMADD213PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12212, IF_FMA|IF_FUTURE},
    {I_VFNMADD213PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12219, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD213SD[] = {
    {I_VFNMADD213SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12457, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD213SS[] = {
    {I_VFNMADD213SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12450, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD231PD[] = {
    {I_VFNMADD231PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12254, IF_FMA|IF_FUTURE},
    {I_VFNMADD231PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12261, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD231PS[] = {
    {I_VFNMADD231PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12240, IF_FMA|IF_FUTURE},
    {I_VFNMADD231PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12247, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD231SD[] = {
    {I_VFNMADD231SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12471, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD231SS[] = {
    {I_VFNMADD231SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12464, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD312PD[] = {
    {I_VFNMADD312PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12198, IF_FMA|IF_FUTURE},
    {I_VFNMADD312PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12205, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD312PS[] = {
    {I_VFNMADD312PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12184, IF_FMA|IF_FUTURE},
    {I_VFNMADD312PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12191, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD312SD[] = {
    {I_VFNMADD312SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12443, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD312SS[] = {
    {I_VFNMADD312SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12436, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD321PD[] = {
    {I_VFNMADD321PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12254, IF_FMA|IF_FUTURE},
    {I_VFNMADD321PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12261, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD321PS[] = {
    {I_VFNMADD321PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12240, IF_FMA|IF_FUTURE},
    {I_VFNMADD321PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12247, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD321SD[] = {
    {I_VFNMADD321SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12471, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADD321SS[] = {
    {I_VFNMADD321SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12464, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADDPD[] = {
    {I_VFNMADDPD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4428, IF_AMD|IF_SSE5},
    {I_VFNMADDPD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4437, IF_AMD|IF_SSE5},
    {I_VFNMADDPD, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+4446, IF_AMD|IF_SSE5},
    {I_VFNMADDPD, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+4455, IF_AMD|IF_SSE5},
    {I_VFNMADDPD, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+4464, IF_AMD|IF_SSE5},
    {I_VFNMADDPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+4473, IF_AMD|IF_SSE5},
    {I_VFNMADDPD, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+4482, IF_AMD|IF_SSE5},
    {I_VFNMADDPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+4491, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADDPS[] = {
    {I_VFNMADDPS, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4500, IF_AMD|IF_SSE5},
    {I_VFNMADDPS, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4509, IF_AMD|IF_SSE5},
    {I_VFNMADDPS, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+4518, IF_AMD|IF_SSE5},
    {I_VFNMADDPS, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+4527, IF_AMD|IF_SSE5},
    {I_VFNMADDPS, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+4536, IF_AMD|IF_SSE5},
    {I_VFNMADDPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+4545, IF_AMD|IF_SSE5},
    {I_VFNMADDPS, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+4554, IF_AMD|IF_SSE5},
    {I_VFNMADDPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+4563, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADDSD[] = {
    {I_VFNMADDSD, 4, {XMMREG,XMMREG,RM_XMM|BITS64,XMMREG,0}, nasm_bytecodes+4572, IF_AMD|IF_SSE5},
    {I_VFNMADDSD, 3, {XMMREG,RM_XMM|BITS64,XMMREG,0,0}, nasm_bytecodes+4581, IF_AMD|IF_SSE5},
    {I_VFNMADDSD, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS64,0}, nasm_bytecodes+4590, IF_AMD|IF_SSE5},
    {I_VFNMADDSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+4599, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMADDSS[] = {
    {I_VFNMADDSS, 4, {XMMREG,XMMREG,RM_XMM|BITS32,XMMREG,0}, nasm_bytecodes+4608, IF_AMD|IF_SSE5},
    {I_VFNMADDSS, 3, {XMMREG,RM_XMM|BITS32,XMMREG,0,0}, nasm_bytecodes+4617, IF_AMD|IF_SSE5},
    {I_VFNMADDSS, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS32,0}, nasm_bytecodes+4626, IF_AMD|IF_SSE5},
    {I_VFNMADDSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+4635, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB123PD[] = {
    {I_VFNMSUB123PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12310, IF_FMA|IF_FUTURE},
    {I_VFNMSUB123PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12317, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB123PS[] = {
    {I_VFNMSUB123PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12296, IF_FMA|IF_FUTURE},
    {I_VFNMSUB123PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12303, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB123SD[] = {
    {I_VFNMSUB123SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12499, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB123SS[] = {
    {I_VFNMSUB123SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12492, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB132PD[] = {
    {I_VFNMSUB132PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12282, IF_FMA|IF_FUTURE},
    {I_VFNMSUB132PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12289, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB132PS[] = {
    {I_VFNMSUB132PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12268, IF_FMA|IF_FUTURE},
    {I_VFNMSUB132PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12275, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB132SD[] = {
    {I_VFNMSUB132SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12485, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB132SS[] = {
    {I_VFNMSUB132SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12478, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB213PD[] = {
    {I_VFNMSUB213PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12310, IF_FMA|IF_FUTURE},
    {I_VFNMSUB213PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12317, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB213PS[] = {
    {I_VFNMSUB213PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12296, IF_FMA|IF_FUTURE},
    {I_VFNMSUB213PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12303, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB213SD[] = {
    {I_VFNMSUB213SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12499, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB213SS[] = {
    {I_VFNMSUB213SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12492, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB231PD[] = {
    {I_VFNMSUB231PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12338, IF_FMA|IF_FUTURE},
    {I_VFNMSUB231PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12345, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB231PS[] = {
    {I_VFNMSUB231PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12324, IF_FMA|IF_FUTURE},
    {I_VFNMSUB231PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12331, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB231SD[] = {
    {I_VFNMSUB231SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12513, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB231SS[] = {
    {I_VFNMSUB231SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12506, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB312PD[] = {
    {I_VFNMSUB312PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12282, IF_FMA|IF_FUTURE},
    {I_VFNMSUB312PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12289, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB312PS[] = {
    {I_VFNMSUB312PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12268, IF_FMA|IF_FUTURE},
    {I_VFNMSUB312PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12275, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB312SD[] = {
    {I_VFNMSUB312SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12485, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB312SS[] = {
    {I_VFNMSUB312SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12478, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB321PD[] = {
    {I_VFNMSUB321PD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12338, IF_FMA|IF_FUTURE},
    {I_VFNMSUB321PD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12345, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB321PS[] = {
    {I_VFNMSUB321PS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12324, IF_FMA|IF_FUTURE},
    {I_VFNMSUB321PS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+12331, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB321SD[] = {
    {I_VFNMSUB321SD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+12513, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUB321SS[] = {
    {I_VFNMSUB321SS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+12506, IF_FMA|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUBPD[] = {
    {I_VFNMSUBPD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4644, IF_AMD|IF_SSE5},
    {I_VFNMSUBPD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4653, IF_AMD|IF_SSE5},
    {I_VFNMSUBPD, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+4662, IF_AMD|IF_SSE5},
    {I_VFNMSUBPD, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+4671, IF_AMD|IF_SSE5},
    {I_VFNMSUBPD, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+4680, IF_AMD|IF_SSE5},
    {I_VFNMSUBPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+4689, IF_AMD|IF_SSE5},
    {I_VFNMSUBPD, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+4698, IF_AMD|IF_SSE5},
    {I_VFNMSUBPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+4707, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUBPS[] = {
    {I_VFNMSUBPS, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4716, IF_AMD|IF_SSE5},
    {I_VFNMSUBPS, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4725, IF_AMD|IF_SSE5},
    {I_VFNMSUBPS, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+4734, IF_AMD|IF_SSE5},
    {I_VFNMSUBPS, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+4743, IF_AMD|IF_SSE5},
    {I_VFNMSUBPS, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+4752, IF_AMD|IF_SSE5},
    {I_VFNMSUBPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+4761, IF_AMD|IF_SSE5},
    {I_VFNMSUBPS, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+4770, IF_AMD|IF_SSE5},
    {I_VFNMSUBPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+4779, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUBSD[] = {
    {I_VFNMSUBSD, 4, {XMMREG,XMMREG,RM_XMM|BITS64,XMMREG,0}, nasm_bytecodes+4788, IF_AMD|IF_SSE5},
    {I_VFNMSUBSD, 3, {XMMREG,RM_XMM|BITS64,XMMREG,0,0}, nasm_bytecodes+4797, IF_AMD|IF_SSE5},
    {I_VFNMSUBSD, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS64,0}, nasm_bytecodes+4806, IF_AMD|IF_SSE5},
    {I_VFNMSUBSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+4815, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFNMSUBSS[] = {
    {I_VFNMSUBSS, 4, {XMMREG,XMMREG,RM_XMM|BITS32,XMMREG,0}, nasm_bytecodes+4824, IF_AMD|IF_SSE5},
    {I_VFNMSUBSS, 3, {XMMREG,RM_XMM|BITS32,XMMREG,0,0}, nasm_bytecodes+4833, IF_AMD|IF_SSE5},
    {I_VFNMSUBSS, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS32,0}, nasm_bytecodes+4842, IF_AMD|IF_SSE5},
    {I_VFNMSUBSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+4851, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFRCZPD[] = {
    {I_VFRCZPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12590, IF_AMD|IF_SSE5},
    {I_VFRCZPD, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12597, IF_AMD|IF_SSE5},
    {I_VFRCZPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+12604, IF_AMD|IF_SSE5},
    {I_VFRCZPD, 1, {YMMREG,0,0,0,0}, nasm_bytecodes+12611, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFRCZPS[] = {
    {I_VFRCZPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12618, IF_AMD|IF_SSE5},
    {I_VFRCZPS, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12625, IF_AMD|IF_SSE5},
    {I_VFRCZPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+12632, IF_AMD|IF_SSE5},
    {I_VFRCZPS, 1, {YMMREG,0,0,0,0}, nasm_bytecodes+12639, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFRCZSD[] = {
    {I_VFRCZSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+12646, IF_AMD|IF_SSE5},
    {I_VFRCZSD, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12653, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VFRCZSS[] = {
    {I_VFRCZSS, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+12660, IF_AMD|IF_SSE5},
    {I_VFRCZSS, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12667, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VHADDPD[] = {
    {I_VHADDPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+9125, IF_AVX|IF_SANDYBRIDGE},
    {I_VHADDPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9132, IF_AVX|IF_SANDYBRIDGE},
    {I_VHADDPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+9139, IF_AVX|IF_SANDYBRIDGE},
    {I_VHADDPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9146, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VHADDPS[] = {
    {I_VHADDPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+9153, IF_AVX|IF_SANDYBRIDGE},
    {I_VHADDPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9160, IF_AVX|IF_SANDYBRIDGE},
    {I_VHADDPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+9167, IF_AVX|IF_SANDYBRIDGE},
    {I_VHADDPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9174, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VHSUBPD[] = {
    {I_VHSUBPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+9181, IF_AVX|IF_SANDYBRIDGE},
    {I_VHSUBPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9188, IF_AVX|IF_SANDYBRIDGE},
    {I_VHSUBPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+9195, IF_AVX|IF_SANDYBRIDGE},
    {I_VHSUBPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9202, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VHSUBPS[] = {
    {I_VHSUBPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+9209, IF_AVX|IF_SANDYBRIDGE},
    {I_VHSUBPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9216, IF_AVX|IF_SANDYBRIDGE},
    {I_VHSUBPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+9223, IF_AVX|IF_SANDYBRIDGE},
    {I_VHSUBPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9230, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VINSERTF128[] = {
    {I_VINSERTF128, 4, {YMMREG,YMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6096, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VINSERTPS[] = {
    {I_VINSERTPS, 4, {XMMREG,XMMREG,RM_XMM|BITS32,IMMEDIATE|BITS8,0}, nasm_bytecodes+6104, IF_AVX|IF_SANDYBRIDGE},
    {I_VINSERTPS, 3, {XMMREG,RM_XMM|BITS32,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6112, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VLDDQU[] = {
    {I_VLDDQU, 2, {XMMREG,MEMORY|BITS128,0,0,0}, nasm_bytecodes+9237, IF_AVX|IF_SANDYBRIDGE},
    {I_VLDDQU, 2, {YMMREG,MEMORY|BITS256,0,0,0}, nasm_bytecodes+9244, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VLDMXCSR[] = {
    {I_VLDMXCSR, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+9251, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VLDQQU[] = {
    {I_VLDQQU, 2, {YMMREG,MEMORY|BITS256,0,0,0}, nasm_bytecodes+9244, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMASKMOVDQU[] = {
    {I_VMASKMOVDQU, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+9258, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMASKMOVPD[] = {
    {I_VMASKMOVPD, 3, {XMMREG,XMMREG,MEMORY|BITS128,0,0}, nasm_bytecodes+9293, IF_AVX|IF_SANDYBRIDGE},
    {I_VMASKMOVPD, 3, {YMMREG,YMMREG,MEMORY|BITS256,0,0}, nasm_bytecodes+9300, IF_AVX|IF_SANDYBRIDGE},
    {I_VMASKMOVPD, 3, {MEMORY|BITS128,XMMREG,XMMREG,0,0}, nasm_bytecodes+9307, IF_AVX|IF_SANDYBRIDGE},
    {I_VMASKMOVPD, 3, {MEMORY|BITS256,YMMREG,YMMREG,0,0}, nasm_bytecodes+9314, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMASKMOVPS[] = {
    {I_VMASKMOVPS, 3, {XMMREG,XMMREG,MEMORY|BITS128,0,0}, nasm_bytecodes+9265, IF_AVX|IF_SANDYBRIDGE},
    {I_VMASKMOVPS, 3, {YMMREG,YMMREG,MEMORY|BITS256,0,0}, nasm_bytecodes+9272, IF_AVX|IF_SANDYBRIDGE},
    {I_VMASKMOVPS, 3, {MEMORY|BITS128,XMMREG,XMMREG,0,0}, nasm_bytecodes+9279, IF_AVX|IF_SANDYBRIDGE|IF_SO},
    {I_VMASKMOVPS, 3, {MEMORY|BITS256,YMMREG,YMMREG,0,0}, nasm_bytecodes+9286, IF_AVX|IF_SANDYBRIDGE|IF_SY},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMAXPD[] = {
    {I_VMAXPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+9321, IF_AVX|IF_SANDYBRIDGE},
    {I_VMAXPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9328, IF_AVX|IF_SANDYBRIDGE},
    {I_VMAXPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+9335, IF_AVX|IF_SANDYBRIDGE},
    {I_VMAXPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9342, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMAXPS[] = {
    {I_VMAXPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+9349, IF_AVX|IF_SANDYBRIDGE},
    {I_VMAXPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9356, IF_AVX|IF_SANDYBRIDGE},
    {I_VMAXPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+9363, IF_AVX|IF_SANDYBRIDGE},
    {I_VMAXPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9370, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMAXSD[] = {
    {I_VMAXSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+9377, IF_AVX|IF_SANDYBRIDGE},
    {I_VMAXSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+9384, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMAXSS[] = {
    {I_VMAXSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+9391, IF_AVX|IF_SANDYBRIDGE},
    {I_VMAXSS, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+9398, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMCALL[] = {
    {I_VMCALL, 0, {0,0,0,0,0}, nasm_bytecodes+18582, IF_VMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMCLEAR[] = {
    {I_VMCLEAR, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+15524, IF_VMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMINPD[] = {
    {I_VMINPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+9405, IF_AVX|IF_SANDYBRIDGE},
    {I_VMINPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9412, IF_AVX|IF_SANDYBRIDGE},
    {I_VMINPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+9419, IF_AVX|IF_SANDYBRIDGE},
    {I_VMINPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9426, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMINPS[] = {
    {I_VMINPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+9433, IF_AVX|IF_SANDYBRIDGE},
    {I_VMINPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9440, IF_AVX|IF_SANDYBRIDGE},
    {I_VMINPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+9447, IF_AVX|IF_SANDYBRIDGE},
    {I_VMINPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9454, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMINSD[] = {
    {I_VMINSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+9461, IF_AVX|IF_SANDYBRIDGE},
    {I_VMINSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+9468, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMINSS[] = {
    {I_VMINSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+9475, IF_AVX|IF_SANDYBRIDGE},
    {I_VMINSS, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+9482, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMLAUNCH[] = {
    {I_VMLAUNCH, 0, {0,0,0,0,0}, nasm_bytecodes+18587, IF_VMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMLOAD[] = {
    {I_VMLOAD, 0, {0,0,0,0,0}, nasm_bytecodes+18592, IF_X64|IF_VMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMMCALL[] = {
    {I_VMMCALL, 0, {0,0,0,0,0}, nasm_bytecodes+18597, IF_X64|IF_VMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVAPD[] = {
    {I_VMOVAPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9489, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVAPD, 2, {RM_XMM|BITS128,XMMREG,0,0,0}, nasm_bytecodes+9496, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVAPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9503, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVAPD, 2, {RM_YMM|BITS256,YMMREG,0,0,0}, nasm_bytecodes+9510, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVAPS[] = {
    {I_VMOVAPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9517, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVAPS, 2, {RM_XMM|BITS128,XMMREG,0,0,0}, nasm_bytecodes+9524, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVAPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9531, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVAPS, 2, {RM_YMM|BITS256,YMMREG,0,0,0}, nasm_bytecodes+9538, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVD[] = {
    {I_VMOVD, 2, {XMMREG,RM_GPR|BITS32,0,0,0}, nasm_bytecodes+9545, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVD, 2, {RM_GPR|BITS32,XMMREG,0,0,0}, nasm_bytecodes+9552, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVDDUP[] = {
    {I_VMOVDDUP, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+9587, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVDDUP, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9594, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVDQA[] = {
    {I_VMOVDQA, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9601, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVDQA, 2, {RM_XMM|BITS128,XMMREG,0,0,0}, nasm_bytecodes+9608, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVDQA, 2, {YMMREG,RM_YMM,0,0,0}, nasm_bytecodes+9615, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVDQA, 2, {RM_YMM|BITS256,YMMREG,0,0,0}, nasm_bytecodes+9622, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVDQU[] = {
    {I_VMOVDQU, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9629, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVDQU, 2, {RM_XMM|BITS128,XMMREG,0,0,0}, nasm_bytecodes+9636, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVDQU, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9643, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVDQU, 2, {RM_YMM|BITS256,YMMREG,0,0,0}, nasm_bytecodes+9650, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVHLPS[] = {
    {I_VMOVHLPS, 3, {XMMREG,XMMREG,XMMREG,0,0}, nasm_bytecodes+9657, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVHLPS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+9664, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVHPD[] = {
    {I_VMOVHPD, 3, {XMMREG,XMMREG,MEMORY|BITS64,0,0}, nasm_bytecodes+9671, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVHPD, 2, {XMMREG,MEMORY|BITS64,0,0,0}, nasm_bytecodes+9678, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVHPD, 2, {MEMORY|BITS64,XMMREG,0,0,0}, nasm_bytecodes+9685, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVHPS[] = {
    {I_VMOVHPS, 3, {XMMREG,XMMREG,MEMORY|BITS64,0,0}, nasm_bytecodes+9692, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVHPS, 2, {XMMREG,MEMORY|BITS64,0,0,0}, nasm_bytecodes+9699, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVHPS, 2, {MEMORY|BITS64,XMMREG,0,0,0}, nasm_bytecodes+9706, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVLHPS[] = {
    {I_VMOVLHPS, 3, {XMMREG,XMMREG,XMMREG,0,0}, nasm_bytecodes+9692, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVLHPS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+9699, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVLPD[] = {
    {I_VMOVLPD, 3, {XMMREG,XMMREG,MEMORY|BITS64,0,0}, nasm_bytecodes+9713, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVLPD, 2, {XMMREG,MEMORY|BITS64,0,0,0}, nasm_bytecodes+9720, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVLPD, 2, {MEMORY|BITS64,XMMREG,0,0,0}, nasm_bytecodes+9727, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVLPS[] = {
    {I_VMOVLPS, 3, {XMMREG,XMMREG,MEMORY|BITS64,0,0}, nasm_bytecodes+9657, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVLPS, 2, {XMMREG,MEMORY|BITS64,0,0,0}, nasm_bytecodes+9664, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVLPS, 2, {MEMORY|BITS64,XMMREG,0,0,0}, nasm_bytecodes+9734, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVMSKPD[] = {
    {I_VMOVMSKPD, 2, {REG_GPR|BITS64,XMMREG,0,0,0}, nasm_bytecodes+9741, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    {I_VMOVMSKPD, 2, {REG_GPR|BITS32,XMMREG,0,0,0}, nasm_bytecodes+9741, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVMSKPD, 2, {REG_GPR|BITS64,YMMREG,0,0,0}, nasm_bytecodes+9748, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    {I_VMOVMSKPD, 2, {REG_GPR|BITS32,YMMREG,0,0,0}, nasm_bytecodes+9748, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVMSKPS[] = {
    {I_VMOVMSKPS, 2, {REG_GPR|BITS64,XMMREG,0,0,0}, nasm_bytecodes+9755, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    {I_VMOVMSKPS, 2, {REG_GPR|BITS32,XMMREG,0,0,0}, nasm_bytecodes+9755, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVMSKPS, 2, {REG_GPR|BITS64,YMMREG,0,0,0}, nasm_bytecodes+9762, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    {I_VMOVMSKPS, 2, {REG_GPR|BITS32,YMMREG,0,0,0}, nasm_bytecodes+9762, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVNTDQ[] = {
    {I_VMOVNTDQ, 2, {MEMORY|BITS128,XMMREG,0,0,0}, nasm_bytecodes+9769, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVNTDQ, 2, {MEMORY|BITS256,YMMREG,0,0,0}, nasm_bytecodes+9776, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVNTDQA[] = {
    {I_VMOVNTDQA, 2, {XMMREG,MEMORY|BITS128,0,0,0}, nasm_bytecodes+9783, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVNTPD[] = {
    {I_VMOVNTPD, 2, {MEMORY|BITS128,XMMREG,0,0,0}, nasm_bytecodes+9790, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVNTPD, 2, {MEMORY|BITS256,YMMREG,0,0,0}, nasm_bytecodes+9797, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVNTPS[] = {
    {I_VMOVNTPS, 2, {MEMORY|BITS128,XMMREG,0,0,0}, nasm_bytecodes+9804, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVNTPS, 2, {MEMORY|BITS128,YMMREG,0,0,0}, nasm_bytecodes+9811, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVNTQQ[] = {
    {I_VMOVNTQQ, 2, {MEMORY|BITS256,YMMREG,0,0,0}, nasm_bytecodes+9776, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVQ[] = {
    {I_VMOVQ, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+9559, IF_AVX|IF_SANDYBRIDGE|IF_SQ},
    {I_VMOVQ, 2, {RM_XMM|BITS64,XMMREG,0,0,0}, nasm_bytecodes+9566, IF_AVX|IF_SANDYBRIDGE|IF_SQ},
    {I_VMOVQ, 2, {XMMREG,RM_GPR|BITS64,0,0,0}, nasm_bytecodes+9573, IF_AVX|IF_SANDYBRIDGE|IF_LONG|IF_SQ},
    {I_VMOVQ, 2, {RM_GPR|BITS64,XMMREG,0,0,0}, nasm_bytecodes+9580, IF_AVX|IF_SANDYBRIDGE|IF_LONG|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVQQA[] = {
    {I_VMOVQQA, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9615, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVQQA, 2, {RM_YMM|BITS256,YMMREG,0,0,0}, nasm_bytecodes+9622, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVQQU[] = {
    {I_VMOVQQU, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9643, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVQQU, 2, {RM_YMM|BITS256,YMMREG,0,0,0}, nasm_bytecodes+9650, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVSD[] = {
    {I_VMOVSD, 3, {XMMREG,XMMREG,XMMREG,0,0}, nasm_bytecodes+9818, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVSD, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+9825, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVSD, 2, {XMMREG,MEMORY|BITS64,0,0,0}, nasm_bytecodes+9832, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVSD, 3, {XMMREG,XMMREG,XMMREG,0,0}, nasm_bytecodes+9839, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVSD, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+9846, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVSD, 2, {MEMORY|BITS64,XMMREG,0,0,0}, nasm_bytecodes+9853, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVSHDUP[] = {
    {I_VMOVSHDUP, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9860, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVSHDUP, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9867, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVSLDUP[] = {
    {I_VMOVSLDUP, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9874, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVSLDUP, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9881, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVSS[] = {
    {I_VMOVSS, 3, {XMMREG,XMMREG,XMMREG,0,0}, nasm_bytecodes+9888, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVSS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+9895, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVSS, 2, {XMMREG,MEMORY|BITS64,0,0,0}, nasm_bytecodes+9902, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVSS, 3, {XMMREG,XMMREG,XMMREG,0,0}, nasm_bytecodes+9909, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVSS, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+9916, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVSS, 2, {MEMORY|BITS64,XMMREG,0,0,0}, nasm_bytecodes+9923, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVUPD[] = {
    {I_VMOVUPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9930, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVUPD, 2, {RM_XMM|BITS128,XMMREG,0,0,0}, nasm_bytecodes+9937, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVUPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9944, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVUPD, 2, {RM_YMM|BITS256,YMMREG,0,0,0}, nasm_bytecodes+9951, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMOVUPS[] = {
    {I_VMOVUPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9958, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVUPS, 2, {RM_XMM|BITS128,XMMREG,0,0,0}, nasm_bytecodes+9965, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVUPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+9972, IF_AVX|IF_SANDYBRIDGE},
    {I_VMOVUPS, 2, {RM_YMM|BITS256,YMMREG,0,0,0}, nasm_bytecodes+9979, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMPSADBW[] = {
    {I_VMPSADBW, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6120, IF_AVX|IF_SANDYBRIDGE},
    {I_VMPSADBW, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6128, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMPTRLD[] = {
    {I_VMPTRLD, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+15585, IF_VMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMPTRST[] = {
    {I_VMPTRST, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+18602, IF_VMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMREAD[] = {
    {I_VMREAD, 2, {RM_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+7873, IF_VMX|IF_NOLONG|IF_SD},
    {I_VMREAD, 2, {RM_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+7872, IF_X64|IF_VMX|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMRESUME[] = {
    {I_VMRESUME, 0, {0,0,0,0,0}, nasm_bytecodes+18607, IF_VMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMRUN[] = {
    {I_VMRUN, 0, {0,0,0,0,0}, nasm_bytecodes+18612, IF_X64|IF_VMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMSAVE[] = {
    {I_VMSAVE, 0, {0,0,0,0,0}, nasm_bytecodes+18617, IF_X64|IF_VMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMULPD[] = {
    {I_VMULPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+9986, IF_AVX|IF_SANDYBRIDGE},
    {I_VMULPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+9993, IF_AVX|IF_SANDYBRIDGE},
    {I_VMULPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+10000, IF_AVX|IF_SANDYBRIDGE},
    {I_VMULPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+10007, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMULPS[] = {
    {I_VMULPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10014, IF_AVX|IF_SANDYBRIDGE},
    {I_VMULPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10021, IF_AVX|IF_SANDYBRIDGE},
    {I_VMULPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+10028, IF_AVX|IF_SANDYBRIDGE},
    {I_VMULPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+10035, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMULSD[] = {
    {I_VMULSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+10042, IF_AVX|IF_SANDYBRIDGE},
    {I_VMULSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+10049, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMULSS[] = {
    {I_VMULSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+10056, IF_AVX|IF_SANDYBRIDGE},
    {I_VMULSS, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+10063, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMWRITE[] = {
    {I_VMWRITE, 2, {REG_GPR|BITS32,RM_GPR|BITS32,0,0,0}, nasm_bytecodes+7880, IF_VMX|IF_NOLONG|IF_SD},
    {I_VMWRITE, 2, {REG_GPR|BITS64,RM_GPR|BITS64,0,0,0}, nasm_bytecodes+7879, IF_X64|IF_VMX|IF_SQ},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMXOFF[] = {
    {I_VMXOFF, 0, {0,0,0,0,0}, nasm_bytecodes+18622, IF_VMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_VMXON[] = {
    {I_VMXON, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+15530, IF_VMX},
    ITEMPLATE_END
};

static const struct itemplate instrux_VORPD[] = {
    {I_VORPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10070, IF_AVX|IF_SANDYBRIDGE},
    {I_VORPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10077, IF_AVX|IF_SANDYBRIDGE},
    {I_VORPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+10084, IF_AVX|IF_SANDYBRIDGE},
    {I_VORPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+10091, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VORPS[] = {
    {I_VORPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10098, IF_AVX|IF_SANDYBRIDGE},
    {I_VORPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10105, IF_AVX|IF_SANDYBRIDGE},
    {I_VORPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+10112, IF_AVX|IF_SANDYBRIDGE},
    {I_VORPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+10119, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPABSB[] = {
    {I_VPABSB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10126, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPABSD[] = {
    {I_VPABSD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10140, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPABSW[] = {
    {I_VPABSW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10133, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPACKSSDW[] = {
    {I_VPACKSSDW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10161, IF_AVX|IF_SANDYBRIDGE},
    {I_VPACKSSDW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10168, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPACKSSWB[] = {
    {I_VPACKSSWB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10147, IF_AVX|IF_SANDYBRIDGE},
    {I_VPACKSSWB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10154, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPACKUSDW[] = {
    {I_VPACKUSDW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10189, IF_AVX|IF_SANDYBRIDGE},
    {I_VPACKUSDW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10196, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPACKUSWB[] = {
    {I_VPACKUSWB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10175, IF_AVX|IF_SANDYBRIDGE},
    {I_VPACKUSWB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10182, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPADDB[] = {
    {I_VPADDB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10203, IF_AVX|IF_SANDYBRIDGE},
    {I_VPADDB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10210, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPADDD[] = {
    {I_VPADDD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10231, IF_AVX|IF_SANDYBRIDGE},
    {I_VPADDD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10238, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPADDQ[] = {
    {I_VPADDQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10245, IF_AVX|IF_SANDYBRIDGE},
    {I_VPADDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10252, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPADDSB[] = {
    {I_VPADDSB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10259, IF_AVX|IF_SANDYBRIDGE},
    {I_VPADDSB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10266, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPADDSW[] = {
    {I_VPADDSW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10273, IF_AVX|IF_SANDYBRIDGE},
    {I_VPADDSW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10280, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPADDUSB[] = {
    {I_VPADDUSB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10287, IF_AVX|IF_SANDYBRIDGE},
    {I_VPADDUSB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10294, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPADDUSW[] = {
    {I_VPADDUSW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10301, IF_AVX|IF_SANDYBRIDGE},
    {I_VPADDUSW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10308, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPADDW[] = {
    {I_VPADDW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10217, IF_AVX|IF_SANDYBRIDGE},
    {I_VPADDW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10224, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPALIGNR[] = {
    {I_VPALIGNR, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6136, IF_AVX|IF_SANDYBRIDGE},
    {I_VPALIGNR, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6144, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPAND[] = {
    {I_VPAND, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10315, IF_AVX|IF_SANDYBRIDGE},
    {I_VPAND, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10322, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPANDN[] = {
    {I_VPANDN, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10329, IF_AVX|IF_SANDYBRIDGE},
    {I_VPANDN, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10336, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPAVGB[] = {
    {I_VPAVGB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10343, IF_AVX|IF_SANDYBRIDGE},
    {I_VPAVGB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10350, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPAVGW[] = {
    {I_VPAVGW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10357, IF_AVX|IF_SANDYBRIDGE},
    {I_VPAVGW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10364, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPBLENDVB[] = {
    {I_VPBLENDVB, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+3582, IF_AVX|IF_SANDYBRIDGE},
    {I_VPBLENDVB, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+3591, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPBLENDW[] = {
    {I_VPBLENDW, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6152, IF_AVX|IF_SANDYBRIDGE},
    {I_VPBLENDW, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6160, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCLMULHQHQDQ[] = {
    {I_VPCLMULHQHQDQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+3690, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCLMULHQHQDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+3699, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCLMULHQLQDQ[] = {
    {I_VPCLMULHQLQDQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+3654, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCLMULHQLQDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+3663, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCLMULLQHQDQ[] = {
    {I_VPCLMULLQHQDQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+3672, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCLMULLQHQDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+3681, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCLMULLQLQDQ[] = {
    {I_VPCLMULLQLQDQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+3636, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCLMULLQLQDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+3645, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCLMULQDQ[] = {
    {I_VPCLMULQDQ, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6664, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCLMULQDQ, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6672, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMOV[] = {
    {I_VPCMOV, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4860, IF_AMD|IF_SSE5},
    {I_VPCMOV, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4869, IF_AMD|IF_SSE5},
    {I_VPCMOV, 4, {YMMREG,YMMREG,RM_YMM|BITS256,YMMREG,0}, nasm_bytecodes+4878, IF_AMD|IF_SSE5},
    {I_VPCMOV, 3, {YMMREG,RM_YMM|BITS256,YMMREG,0,0}, nasm_bytecodes+4887, IF_AMD|IF_SSE5},
    {I_VPCMOV, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+4896, IF_AMD|IF_SSE5},
    {I_VPCMOV, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+4905, IF_AMD|IF_SSE5},
    {I_VPCMOV, 4, {YMMREG,YMMREG,YMMREG,RM_YMM|BITS256,0}, nasm_bytecodes+4914, IF_AMD|IF_SSE5},
    {I_VPCMOV, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+4923, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMPEQB[] = {
    {I_VPCMPEQB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10371, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCMPEQB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10378, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMPEQD[] = {
    {I_VPCMPEQD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10399, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCMPEQD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10406, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMPEQQ[] = {
    {I_VPCMPEQQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10413, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCMPEQQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10420, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMPEQW[] = {
    {I_VPCMPEQW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10385, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCMPEQW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10392, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMPESTRI[] = {
    {I_VPCMPESTRI, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6168, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMPESTRM[] = {
    {I_VPCMPESTRM, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6176, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMPGTB[] = {
    {I_VPCMPGTB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10427, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCMPGTB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10434, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMPGTD[] = {
    {I_VPCMPGTD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10455, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCMPGTD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10462, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMPGTQ[] = {
    {I_VPCMPGTQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10469, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCMPGTQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10476, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMPGTW[] = {
    {I_VPCMPGTW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10441, IF_AVX|IF_SANDYBRIDGE},
    {I_VPCMPGTW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10448, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMPISTRI[] = {
    {I_VPCMPISTRI, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6184, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCMPISTRM[] = {
    {I_VPCMPISTRM, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6192, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCOMB[] = {
    {I_VPCOMB, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6728, IF_AMD|IF_SSE5},
    {I_VPCOMB, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6736, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCOMD[] = {
    {I_VPCOMD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6744, IF_AMD|IF_SSE5},
    {I_VPCOMD, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6752, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCOMQ[] = {
    {I_VPCOMQ, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6760, IF_AMD|IF_SSE5},
    {I_VPCOMQ, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6768, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCOMUB[] = {
    {I_VPCOMUB, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6776, IF_AMD|IF_SSE5},
    {I_VPCOMUB, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6784, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCOMUD[] = {
    {I_VPCOMUD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6792, IF_AMD|IF_SSE5},
    {I_VPCOMUD, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6800, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCOMUQ[] = {
    {I_VPCOMUQ, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6808, IF_AMD|IF_SSE5},
    {I_VPCOMUQ, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6816, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCOMUW[] = {
    {I_VPCOMUW, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6824, IF_AMD|IF_SSE5},
    {I_VPCOMUW, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6832, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPCOMW[] = {
    {I_VPCOMW, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6840, IF_AMD|IF_SSE5},
    {I_VPCOMW, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6848, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPERM2F128[] = {
    {I_VPERM2F128, 4, {YMMREG,YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0}, nasm_bytecodes+6232, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPERMILPD[] = {
    {I_VPERMILPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10483, IF_AVX|IF_SANDYBRIDGE},
    {I_VPERMILPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+10490, IF_AVX|IF_SANDYBRIDGE},
    {I_VPERMILPD, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6200, IF_AVX|IF_SANDYBRIDGE},
    {I_VPERMILPD, 3, {YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6208, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPERMILPS[] = {
    {I_VPERMILPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10497, IF_AVX|IF_SANDYBRIDGE},
    {I_VPERMILPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+10504, IF_AVX|IF_SANDYBRIDGE},
    {I_VPERMILPS, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6216, IF_AVX|IF_SANDYBRIDGE},
    {I_VPERMILPS, 3, {YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6224, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPEXTRB[] = {
    {I_VPEXTRB, 3, {REG_GPR|BITS64,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6240, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    {I_VPEXTRB, 3, {REG_GPR|BITS32,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6240, IF_AVX|IF_SANDYBRIDGE},
    {I_VPEXTRB, 3, {MEMORY|BITS8,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6240, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPEXTRD[] = {
    {I_VPEXTRD, 3, {REG_GPR|BITS64,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6264, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    {I_VPEXTRD, 3, {RM_GPR|BITS32,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6264, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPEXTRQ[] = {
    {I_VPEXTRQ, 3, {RM_GPR|BITS64,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6272, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPEXTRW[] = {
    {I_VPEXTRW, 3, {REG_GPR|BITS64,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6248, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    {I_VPEXTRW, 3, {REG_GPR|BITS32,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6248, IF_AVX|IF_SANDYBRIDGE},
    {I_VPEXTRW, 3, {REG_GPR|BITS64,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6256, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    {I_VPEXTRW, 3, {REG_GPR|BITS32,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6256, IF_AVX|IF_SANDYBRIDGE},
    {I_VPEXTRW, 3, {MEMORY|BITS16,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6256, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDBD[] = {
    {I_VPHADDBD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12674, IF_AMD|IF_SSE5},
    {I_VPHADDBD, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12681, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDBQ[] = {
    {I_VPHADDBQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12688, IF_AMD|IF_SSE5},
    {I_VPHADDBQ, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12695, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDBW[] = {
    {I_VPHADDBW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12702, IF_AMD|IF_SSE5},
    {I_VPHADDBW, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12709, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDD[] = {
    {I_VPHADDD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10525, IF_AVX|IF_SANDYBRIDGE},
    {I_VPHADDD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10532, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDDQ[] = {
    {I_VPHADDDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12716, IF_AMD|IF_SSE5},
    {I_VPHADDDQ, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12723, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDSW[] = {
    {I_VPHADDSW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10539, IF_AVX|IF_SANDYBRIDGE},
    {I_VPHADDSW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10546, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDUBD[] = {
    {I_VPHADDUBD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12730, IF_AMD|IF_SSE5},
    {I_VPHADDUBD, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12737, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDUBQ[] = {
    {I_VPHADDUBQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12744, IF_AMD|IF_SSE5},
    {I_VPHADDUBQ, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12751, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDUBW[] = {
    {I_VPHADDUBW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12758, IF_AMD|IF_SSE5},
    {I_VPHADDUBW, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12765, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDUDQ[] = {
    {I_VPHADDUDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12772, IF_AMD|IF_SSE5},
    {I_VPHADDUDQ, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12779, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDUWD[] = {
    {I_VPHADDUWD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12786, IF_AMD|IF_SSE5},
    {I_VPHADDUWD, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12793, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDUWQ[] = {
    {I_VPHADDUWQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12800, IF_AMD|IF_SSE5},
    {I_VPHADDUWQ, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12807, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDW[] = {
    {I_VPHADDW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10511, IF_AVX|IF_SANDYBRIDGE},
    {I_VPHADDW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10518, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDWD[] = {
    {I_VPHADDWD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12814, IF_AMD|IF_SSE5},
    {I_VPHADDWD, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12821, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHADDWQ[] = {
    {I_VPHADDWQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12828, IF_AMD|IF_SSE5},
    {I_VPHADDWQ, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12835, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHMINPOSUW[] = {
    {I_VPHMINPOSUW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10553, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHSUBBW[] = {
    {I_VPHSUBBW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12842, IF_AMD|IF_SSE5},
    {I_VPHSUBBW, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12849, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHSUBD[] = {
    {I_VPHSUBD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10574, IF_AVX|IF_SANDYBRIDGE},
    {I_VPHSUBD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10581, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHSUBDQ[] = {
    {I_VPHSUBDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12856, IF_AMD|IF_SSE5},
    {I_VPHSUBDQ, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12863, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHSUBSW[] = {
    {I_VPHSUBSW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10588, IF_AVX|IF_SANDYBRIDGE},
    {I_VPHSUBSW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10595, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHSUBW[] = {
    {I_VPHSUBW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10560, IF_AVX|IF_SANDYBRIDGE},
    {I_VPHSUBW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10567, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPHSUBWD[] = {
    {I_VPHSUBWD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12870, IF_AMD|IF_SSE5},
    {I_VPHSUBWD, 1, {XMMREG,0,0,0,0}, nasm_bytecodes+12877, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPINSRB[] = {
    {I_VPINSRB, 4, {XMMREG,XMMREG,MEMORY|BITS8,IMMEDIATE|BITS8,0}, nasm_bytecodes+6280, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRB, 3, {XMMREG,MEMORY|BITS8,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6288, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRB, 4, {XMMREG,XMMREG,RM_GPR|BITS8,IMMEDIATE|BITS8,0}, nasm_bytecodes+6280, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRB, 3, {XMMREG,RM_GPR|BITS8,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6288, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRB, 4, {XMMREG,XMMREG,REG_GPR|BITS32,IMMEDIATE|BITS8,0}, nasm_bytecodes+6280, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRB, 3, {XMMREG,REG_GPR|BITS32,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6288, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPINSRD[] = {
    {I_VPINSRD, 4, {XMMREG,XMMREG,MEMORY|BITS32,IMMEDIATE|BITS8,0}, nasm_bytecodes+6312, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRD, 3, {XMMREG,MEMORY|BITS32,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6320, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRD, 4, {XMMREG,XMMREG,RM_GPR|BITS32,IMMEDIATE|BITS8,0}, nasm_bytecodes+6312, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRD, 3, {XMMREG,RM_GPR|BITS32,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6320, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPINSRQ[] = {
    {I_VPINSRQ, 4, {XMMREG,XMMREG,MEMORY|BITS64,IMMEDIATE|BITS8,0}, nasm_bytecodes+6328, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    {I_VPINSRQ, 3, {XMMREG,MEMORY|BITS64,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6336, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    {I_VPINSRQ, 4, {XMMREG,XMMREG,RM_GPR|BITS64,IMMEDIATE|BITS8,0}, nasm_bytecodes+6328, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    {I_VPINSRQ, 3, {XMMREG,RM_GPR|BITS64,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6336, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPINSRW[] = {
    {I_VPINSRW, 4, {XMMREG,XMMREG,MEMORY|BITS16,IMMEDIATE|BITS8,0}, nasm_bytecodes+6296, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRW, 3, {XMMREG,MEMORY|BITS16,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6304, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRW, 4, {XMMREG,XMMREG,RM_GPR|BITS16,IMMEDIATE|BITS8,0}, nasm_bytecodes+6296, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRW, 3, {XMMREG,RM_GPR|BITS16,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6304, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRW, 4, {XMMREG,XMMREG,REG_GPR|BITS32,IMMEDIATE|BITS8,0}, nasm_bytecodes+6296, IF_AVX|IF_SANDYBRIDGE},
    {I_VPINSRW, 3, {XMMREG,REG_GPR|BITS32,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6304, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMACSDD[] = {
    {I_VPMACSDD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4932, IF_AMD|IF_SSE5},
    {I_VPMACSDD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4941, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMACSDQH[] = {
    {I_VPMACSDQH, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4950, IF_AMD|IF_SSE5},
    {I_VPMACSDQH, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4959, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMACSDQL[] = {
    {I_VPMACSDQL, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4968, IF_AMD|IF_SSE5},
    {I_VPMACSDQL, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4977, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMACSSDD[] = {
    {I_VPMACSSDD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+4986, IF_AMD|IF_SSE5},
    {I_VPMACSSDD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+4995, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMACSSDQH[] = {
    {I_VPMACSSDQH, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+5004, IF_AMD|IF_SSE5},
    {I_VPMACSSDQH, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+5013, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMACSSDQL[] = {
    {I_VPMACSSDQL, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+5022, IF_AMD|IF_SSE5},
    {I_VPMACSSDQL, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+5031, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMACSSWD[] = {
    {I_VPMACSSWD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+5040, IF_AMD|IF_SSE5},
    {I_VPMACSSWD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+5049, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMACSSWW[] = {
    {I_VPMACSSWW, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+5058, IF_AMD|IF_SSE5},
    {I_VPMACSSWW, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+5067, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMACSWD[] = {
    {I_VPMACSWD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+5076, IF_AMD|IF_SSE5},
    {I_VPMACSWD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+5085, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMACSWW[] = {
    {I_VPMACSWW, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+5094, IF_AMD|IF_SSE5},
    {I_VPMACSWW, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+5103, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMADCSSWD[] = {
    {I_VPMADCSSWD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+5112, IF_AMD|IF_SSE5},
    {I_VPMADCSSWD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+5121, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMADCSWD[] = {
    {I_VPMADCSWD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+5130, IF_AMD|IF_SSE5},
    {I_VPMADCSWD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+5139, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMADDUBSW[] = {
    {I_VPMADDUBSW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10616, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMADDUBSW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10623, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMADDWD[] = {
    {I_VPMADDWD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10602, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMADDWD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10609, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMAXSB[] = {
    {I_VPMAXSB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10630, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMAXSB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10637, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMAXSD[] = {
    {I_VPMAXSD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10658, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMAXSD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10665, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMAXSW[] = {
    {I_VPMAXSW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10644, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMAXSW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10651, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMAXUB[] = {
    {I_VPMAXUB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10672, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMAXUB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10679, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMAXUD[] = {
    {I_VPMAXUD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10700, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMAXUD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10707, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMAXUW[] = {
    {I_VPMAXUW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10686, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMAXUW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10693, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMINSB[] = {
    {I_VPMINSB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10714, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMINSB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10721, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMINSD[] = {
    {I_VPMINSD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10742, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMINSD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10749, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMINSW[] = {
    {I_VPMINSW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10728, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMINSW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10735, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMINUB[] = {
    {I_VPMINUB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10756, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMINUB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10763, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMINUD[] = {
    {I_VPMINUD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10784, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMINUD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10791, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMINUW[] = {
    {I_VPMINUW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10770, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMINUW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10777, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVMSKB[] = {
    {I_VPMOVMSKB, 2, {REG_GPR|BITS64,XMMREG,0,0,0}, nasm_bytecodes+10798, IF_AVX|IF_SANDYBRIDGE|IF_LONG},
    {I_VPMOVMSKB, 2, {REG_GPR|BITS32,XMMREG,0,0,0}, nasm_bytecodes+10798, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVSXBD[] = {
    {I_VPMOVSXBD, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+10812, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVSXBQ[] = {
    {I_VPMOVSXBQ, 2, {XMMREG,RM_XMM|BITS16,0,0,0}, nasm_bytecodes+10819, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVSXBW[] = {
    {I_VPMOVSXBW, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+10805, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVSXDQ[] = {
    {I_VPMOVSXDQ, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+10840, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVSXWD[] = {
    {I_VPMOVSXWD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+10826, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVSXWQ[] = {
    {I_VPMOVSXWQ, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+10833, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVZXBD[] = {
    {I_VPMOVZXBD, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+10854, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVZXBQ[] = {
    {I_VPMOVZXBQ, 2, {XMMREG,RM_XMM|BITS16,0,0,0}, nasm_bytecodes+10861, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVZXBW[] = {
    {I_VPMOVZXBW, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+10847, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVZXDQ[] = {
    {I_VPMOVZXDQ, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+10882, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVZXWD[] = {
    {I_VPMOVZXWD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+10868, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMOVZXWQ[] = {
    {I_VPMOVZXWQ, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+10875, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMULDQ[] = {
    {I_VPMULDQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10973, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMULDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10980, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMULHRSW[] = {
    {I_VPMULHRSW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10903, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMULHRSW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10910, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMULHUW[] = {
    {I_VPMULHUW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10889, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMULHUW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10896, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMULHW[] = {
    {I_VPMULHW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10917, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMULHW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10924, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMULLD[] = {
    {I_VPMULLD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10945, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMULLD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10952, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMULLW[] = {
    {I_VPMULLW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10931, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMULLW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10938, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPMULUDQ[] = {
    {I_VPMULUDQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10959, IF_AVX|IF_SANDYBRIDGE},
    {I_VPMULUDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10966, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPOR[] = {
    {I_VPOR, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+10987, IF_AVX|IF_SANDYBRIDGE},
    {I_VPOR, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+10994, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPPERM[] = {
    {I_VPPERM, 4, {XMMREG,XMMREG,XMMREG,RM_XMM|BITS128,0}, nasm_bytecodes+5148, IF_AMD|IF_SSE5},
    {I_VPPERM, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+5157, IF_AMD|IF_SSE5},
    {I_VPPERM, 4, {XMMREG,XMMREG,RM_XMM|BITS128,XMMREG,0}, nasm_bytecodes+5166, IF_AMD|IF_SSE5},
    {I_VPPERM, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+5175, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPROTB[] = {
    {I_VPROTB, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+12884, IF_AMD|IF_SSE5},
    {I_VPROTB, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+12891, IF_AMD|IF_SSE5},
    {I_VPROTB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12898, IF_AMD|IF_SSE5},
    {I_VPROTB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12905, IF_AMD|IF_SSE5},
    {I_VPROTB, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6856, IF_AMD|IF_SSE5},
    {I_VPROTB, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6864, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPROTD[] = {
    {I_VPROTD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+12912, IF_AMD|IF_SSE5},
    {I_VPROTD, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+12919, IF_AMD|IF_SSE5},
    {I_VPROTD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12926, IF_AMD|IF_SSE5},
    {I_VPROTD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12933, IF_AMD|IF_SSE5},
    {I_VPROTD, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6872, IF_AMD|IF_SSE5},
    {I_VPROTD, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6880, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPROTQ[] = {
    {I_VPROTQ, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+12940, IF_AMD|IF_SSE5},
    {I_VPROTQ, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+12947, IF_AMD|IF_SSE5},
    {I_VPROTQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12954, IF_AMD|IF_SSE5},
    {I_VPROTQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12961, IF_AMD|IF_SSE5},
    {I_VPROTQ, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6888, IF_AMD|IF_SSE5},
    {I_VPROTQ, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6896, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPROTW[] = {
    {I_VPROTW, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+12968, IF_AMD|IF_SSE5},
    {I_VPROTW, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+12975, IF_AMD|IF_SSE5},
    {I_VPROTW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+12982, IF_AMD|IF_SSE5},
    {I_VPROTW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+12989, IF_AMD|IF_SSE5},
    {I_VPROTW, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6904, IF_AMD|IF_SSE5},
    {I_VPROTW, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6912, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSADBW[] = {
    {I_VPSADBW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11001, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSADBW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11008, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSHAB[] = {
    {I_VPSHAB, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+12996, IF_AMD|IF_SSE5},
    {I_VPSHAB, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+13003, IF_AMD|IF_SSE5},
    {I_VPSHAB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+13010, IF_AMD|IF_SSE5},
    {I_VPSHAB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+13017, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSHAD[] = {
    {I_VPSHAD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+13024, IF_AMD|IF_SSE5},
    {I_VPSHAD, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+13031, IF_AMD|IF_SSE5},
    {I_VPSHAD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+13038, IF_AMD|IF_SSE5},
    {I_VPSHAD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+13045, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSHAQ[] = {
    {I_VPSHAQ, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+13052, IF_AMD|IF_SSE5},
    {I_VPSHAQ, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+13059, IF_AMD|IF_SSE5},
    {I_VPSHAQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+13066, IF_AMD|IF_SSE5},
    {I_VPSHAQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+13073, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSHAW[] = {
    {I_VPSHAW, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+13080, IF_AMD|IF_SSE5},
    {I_VPSHAW, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+13087, IF_AMD|IF_SSE5},
    {I_VPSHAW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+13094, IF_AMD|IF_SSE5},
    {I_VPSHAW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+13101, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSHLB[] = {
    {I_VPSHLB, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+13108, IF_AMD|IF_SSE5},
    {I_VPSHLB, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+13115, IF_AMD|IF_SSE5},
    {I_VPSHLB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+13122, IF_AMD|IF_SSE5},
    {I_VPSHLB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+13129, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSHLD[] = {
    {I_VPSHLD, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+13136, IF_AMD|IF_SSE5},
    {I_VPSHLD, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+13143, IF_AMD|IF_SSE5},
    {I_VPSHLD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+13150, IF_AMD|IF_SSE5},
    {I_VPSHLD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+13157, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSHLQ[] = {
    {I_VPSHLQ, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+13164, IF_AMD|IF_SSE5},
    {I_VPSHLQ, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+13171, IF_AMD|IF_SSE5},
    {I_VPSHLQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+13178, IF_AMD|IF_SSE5},
    {I_VPSHLQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+13185, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSHLW[] = {
    {I_VPSHLW, 3, {XMMREG,RM_XMM|BITS128,XMMREG,0,0}, nasm_bytecodes+13192, IF_AMD|IF_SSE5},
    {I_VPSHLW, 2, {XMMREG,XMMREG,0,0,0}, nasm_bytecodes+13199, IF_AMD|IF_SSE5},
    {I_VPSHLW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+13206, IF_AMD|IF_SSE5},
    {I_VPSHLW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+13213, IF_AMD|IF_SSE5},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSHUFB[] = {
    {I_VPSHUFB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11015, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSHUFB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11022, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSHUFD[] = {
    {I_VPSHUFD, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6344, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSHUFHW[] = {
    {I_VPSHUFHW, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6352, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSHUFLW[] = {
    {I_VPSHUFLW, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6360, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSIGNB[] = {
    {I_VPSIGNB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11029, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSIGNB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11036, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSIGND[] = {
    {I_VPSIGND, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11057, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSIGND, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11064, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSIGNW[] = {
    {I_VPSIGNW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11043, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSIGNW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11050, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSLLD[] = {
    {I_VPSLLD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11085, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSLLD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11092, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSLLD, 3, {XMMREG,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6416, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSLLD, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6424, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSLLDQ[] = {
    {I_VPSLLDQ, 3, {XMMREG,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6368, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSLLDQ, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6376, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSLLQ[] = {
    {I_VPSLLQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11099, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSLLQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11106, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSLLQ, 3, {XMMREG,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6432, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSLLQ, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6440, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSLLW[] = {
    {I_VPSLLW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11071, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSLLW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11078, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSLLW, 3, {XMMREG,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6400, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSLLW, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6408, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSRAD[] = {
    {I_VPSRAD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11127, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRAD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11134, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRAD, 3, {XMMREG,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6464, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRAD, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6472, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSRAW[] = {
    {I_VPSRAW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11113, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRAW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11120, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRAW, 3, {XMMREG,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6448, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRAW, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6456, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSRLD[] = {
    {I_VPSRLD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11155, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRLD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11162, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRLD, 3, {XMMREG,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6496, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRLD, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6504, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSRLDQ[] = {
    {I_VPSRLDQ, 3, {XMMREG,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6384, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRLDQ, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6392, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSRLQ[] = {
    {I_VPSRLQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11169, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRLQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11176, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRLQ, 3, {XMMREG,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6512, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRLQ, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6520, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSRLW[] = {
    {I_VPSRLW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11141, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRLW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11148, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRLW, 3, {XMMREG,XMMREG,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6480, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSRLW, 2, {XMMREG,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+6488, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSUBB[] = {
    {I_VPSUBB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11197, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSUBB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11204, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSUBD[] = {
    {I_VPSUBD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11225, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSUBD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11232, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSUBQ[] = {
    {I_VPSUBQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11239, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSUBQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11246, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSUBSB[] = {
    {I_VPSUBSB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11253, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSUBSB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11260, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSUBSW[] = {
    {I_VPSUBSW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11267, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSUBSW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11274, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSUBUSB[] = {
    {I_VPSUBUSB, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11281, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSUBUSB, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11288, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSUBUSW[] = {
    {I_VPSUBUSW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11295, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSUBUSW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11302, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPSUBW[] = {
    {I_VPSUBW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11211, IF_AVX|IF_SANDYBRIDGE},
    {I_VPSUBW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11218, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPTEST[] = {
    {I_VPTEST, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11183, IF_AVX|IF_SANDYBRIDGE},
    {I_VPTEST, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11190, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPUNPCKHBW[] = {
    {I_VPUNPCKHBW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11309, IF_AVX|IF_SANDYBRIDGE},
    {I_VPUNPCKHBW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11316, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPUNPCKHDQ[] = {
    {I_VPUNPCKHDQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11337, IF_AVX|IF_SANDYBRIDGE},
    {I_VPUNPCKHDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11344, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPUNPCKHQDQ[] = {
    {I_VPUNPCKHQDQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11351, IF_AVX|IF_SANDYBRIDGE},
    {I_VPUNPCKHQDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11358, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPUNPCKHWD[] = {
    {I_VPUNPCKHWD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11323, IF_AVX|IF_SANDYBRIDGE},
    {I_VPUNPCKHWD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11330, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPUNPCKLBW[] = {
    {I_VPUNPCKLBW, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11365, IF_AVX|IF_SANDYBRIDGE},
    {I_VPUNPCKLBW, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11372, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPUNPCKLDQ[] = {
    {I_VPUNPCKLDQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11393, IF_AVX|IF_SANDYBRIDGE},
    {I_VPUNPCKLDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11400, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPUNPCKLQDQ[] = {
    {I_VPUNPCKLQDQ, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11407, IF_AVX|IF_SANDYBRIDGE},
    {I_VPUNPCKLQDQ, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11414, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPUNPCKLWD[] = {
    {I_VPUNPCKLWD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11379, IF_AVX|IF_SANDYBRIDGE},
    {I_VPUNPCKLWD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11386, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VPXOR[] = {
    {I_VPXOR, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11421, IF_AVX|IF_SANDYBRIDGE},
    {I_VPXOR, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11428, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VRCPPS[] = {
    {I_VRCPPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11435, IF_AVX|IF_SANDYBRIDGE},
    {I_VRCPPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11442, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VRCPSS[] = {
    {I_VRCPSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+11449, IF_AVX|IF_SANDYBRIDGE},
    {I_VRCPSS, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+11456, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VROUNDPD[] = {
    {I_VROUNDPD, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6528, IF_AVX|IF_SANDYBRIDGE},
    {I_VROUNDPD, 3, {YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6536, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VROUNDPS[] = {
    {I_VROUNDPS, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6544, IF_AVX|IF_SANDYBRIDGE},
    {I_VROUNDPS, 3, {YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6552, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VROUNDSD[] = {
    {I_VROUNDSD, 4, {XMMREG,XMMREG,RM_XMM|BITS64,IMMEDIATE|BITS8,0}, nasm_bytecodes+6560, IF_AVX|IF_SANDYBRIDGE},
    {I_VROUNDSD, 3, {XMMREG,RM_XMM|BITS64,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6568, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VROUNDSS[] = {
    {I_VROUNDSS, 4, {XMMREG,XMMREG,RM_XMM|BITS32,IMMEDIATE|BITS8,0}, nasm_bytecodes+6576, IF_AVX|IF_SANDYBRIDGE},
    {I_VROUNDSS, 3, {XMMREG,RM_XMM|BITS32,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6584, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VRSQRTPS[] = {
    {I_VRSQRTPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11463, IF_AVX|IF_SANDYBRIDGE},
    {I_VRSQRTPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11470, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VRSQRTSS[] = {
    {I_VRSQRTSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+11477, IF_AVX|IF_SANDYBRIDGE},
    {I_VRSQRTSS, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+11484, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VSHUFPD[] = {
    {I_VSHUFPD, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6592, IF_AVX|IF_SANDYBRIDGE},
    {I_VSHUFPD, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6600, IF_AVX|IF_SANDYBRIDGE},
    {I_VSHUFPD, 4, {YMMREG,YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0}, nasm_bytecodes+6608, IF_AVX|IF_SANDYBRIDGE},
    {I_VSHUFPD, 3, {YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6616, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VSHUFPS[] = {
    {I_VSHUFPS, 4, {XMMREG,XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0}, nasm_bytecodes+6624, IF_AVX|IF_SANDYBRIDGE},
    {I_VSHUFPS, 3, {XMMREG,RM_XMM|BITS128,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6632, IF_AVX|IF_SANDYBRIDGE},
    {I_VSHUFPS, 4, {YMMREG,YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0}, nasm_bytecodes+6640, IF_AVX|IF_SANDYBRIDGE},
    {I_VSHUFPS, 3, {YMMREG,RM_YMM|BITS256,IMMEDIATE|BITS8,0,0}, nasm_bytecodes+6648, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VSQRTPD[] = {
    {I_VSQRTPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11491, IF_AVX|IF_SANDYBRIDGE},
    {I_VSQRTPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11498, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VSQRTPS[] = {
    {I_VSQRTPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11505, IF_AVX|IF_SANDYBRIDGE},
    {I_VSQRTPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11512, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VSQRTSD[] = {
    {I_VSQRTSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+11519, IF_AVX|IF_SANDYBRIDGE},
    {I_VSQRTSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+11526, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VSQRTSS[] = {
    {I_VSQRTSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+11533, IF_AVX|IF_SANDYBRIDGE},
    {I_VSQRTSS, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+11540, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VSTMXCSR[] = {
    {I_VSTMXCSR, 1, {MEMORY|BITS32,0,0,0,0}, nasm_bytecodes+11547, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VSUBPD[] = {
    {I_VSUBPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11554, IF_AVX|IF_SANDYBRIDGE},
    {I_VSUBPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11561, IF_AVX|IF_SANDYBRIDGE},
    {I_VSUBPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11568, IF_AVX|IF_SANDYBRIDGE},
    {I_VSUBPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11575, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VSUBPS[] = {
    {I_VSUBPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11582, IF_AVX|IF_SANDYBRIDGE},
    {I_VSUBPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11589, IF_AVX|IF_SANDYBRIDGE},
    {I_VSUBPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11596, IF_AVX|IF_SANDYBRIDGE},
    {I_VSUBPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11603, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VSUBSD[] = {
    {I_VSUBSD, 3, {XMMREG,XMMREG,RM_XMM|BITS64,0,0}, nasm_bytecodes+11610, IF_AVX|IF_SANDYBRIDGE},
    {I_VSUBSD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+11617, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VSUBSS[] = {
    {I_VSUBSS, 3, {XMMREG,XMMREG,RM_XMM|BITS32,0,0}, nasm_bytecodes+11624, IF_AVX|IF_SANDYBRIDGE},
    {I_VSUBSS, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+11631, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VTESTPD[] = {
    {I_VTESTPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11652, IF_AVX|IF_SANDYBRIDGE},
    {I_VTESTPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11659, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VTESTPS[] = {
    {I_VTESTPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11638, IF_AVX|IF_SANDYBRIDGE},
    {I_VTESTPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11645, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VUCOMISD[] = {
    {I_VUCOMISD, 2, {XMMREG,RM_XMM|BITS64,0,0,0}, nasm_bytecodes+11666, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VUCOMISS[] = {
    {I_VUCOMISS, 2, {XMMREG,RM_XMM|BITS32,0,0,0}, nasm_bytecodes+11673, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VUNPCKHPD[] = {
    {I_VUNPCKHPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11680, IF_AVX|IF_SANDYBRIDGE},
    {I_VUNPCKHPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11687, IF_AVX|IF_SANDYBRIDGE},
    {I_VUNPCKHPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11694, IF_AVX|IF_SANDYBRIDGE},
    {I_VUNPCKHPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11701, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VUNPCKHPS[] = {
    {I_VUNPCKHPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11708, IF_AVX|IF_SANDYBRIDGE},
    {I_VUNPCKHPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11715, IF_AVX|IF_SANDYBRIDGE},
    {I_VUNPCKHPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11722, IF_AVX|IF_SANDYBRIDGE},
    {I_VUNPCKHPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11729, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VUNPCKLPD[] = {
    {I_VUNPCKLPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11736, IF_AVX|IF_SANDYBRIDGE},
    {I_VUNPCKLPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11743, IF_AVX|IF_SANDYBRIDGE},
    {I_VUNPCKLPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11750, IF_AVX|IF_SANDYBRIDGE},
    {I_VUNPCKLPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11757, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VUNPCKLPS[] = {
    {I_VUNPCKLPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11764, IF_AVX|IF_SANDYBRIDGE},
    {I_VUNPCKLPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11771, IF_AVX|IF_SANDYBRIDGE},
    {I_VUNPCKLPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11778, IF_AVX|IF_SANDYBRIDGE},
    {I_VUNPCKLPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11785, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VXORPD[] = {
    {I_VXORPD, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11792, IF_AVX|IF_SANDYBRIDGE},
    {I_VXORPD, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11799, IF_AVX|IF_SANDYBRIDGE},
    {I_VXORPD, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11806, IF_AVX|IF_SANDYBRIDGE},
    {I_VXORPD, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11813, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VXORPS[] = {
    {I_VXORPS, 3, {XMMREG,XMMREG,RM_XMM|BITS128,0,0}, nasm_bytecodes+11820, IF_AVX|IF_SANDYBRIDGE},
    {I_VXORPS, 2, {XMMREG,RM_XMM|BITS128,0,0,0}, nasm_bytecodes+11827, IF_AVX|IF_SANDYBRIDGE},
    {I_VXORPS, 3, {YMMREG,YMMREG,RM_YMM|BITS256,0,0}, nasm_bytecodes+11834, IF_AVX|IF_SANDYBRIDGE},
    {I_VXORPS, 2, {YMMREG,RM_YMM|BITS256,0,0,0}, nasm_bytecodes+11841, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VZEROALL[] = {
    {I_VZEROALL, 0, {0,0,0,0,0}, nasm_bytecodes+15560, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_VZEROUPPER[] = {
    {I_VZEROUPPER, 0, {0,0,0,0,0}, nasm_bytecodes+15566, IF_AVX|IF_SANDYBRIDGE},
    ITEMPLATE_END
};

static const struct itemplate instrux_WBINVD[] = {
    {I_WBINVD, 0, {0,0,0,0,0}, nasm_bytecodes+19624, IF_486|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_WRFSBASE[] = {
    {I_WRFSBASE, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+12535, IF_LONG|IF_FUTURE},
    {I_WRFSBASE, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+12534, IF_LONG|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_WRGSBASE[] = {
    {I_WRGSBASE, 1, {REG_GPR|BITS32,0,0,0,0}, nasm_bytecodes+12542, IF_LONG|IF_FUTURE},
    {I_WRGSBASE, 1, {REG_GPR|BITS64,0,0,0,0}, nasm_bytecodes+12541, IF_LONG|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_WRMSR[] = {
    {I_WRMSR, 0, {0,0,0,0,0}, nasm_bytecodes+19628, IF_PENT|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_WRSHR[] = {
    {I_WRSHR, 1, {RM_GPR|BITS32,0,0,0,0}, nasm_bytecodes+14390, IF_P6|IF_CYRIX|IF_SMM},
    ITEMPLATE_END
};

static const struct itemplate instrux_XADD[] = {
    {I_XADD, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+18487, IF_486|IF_SM},
    {I_XADD, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+18487, IF_486},
    {I_XADD, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+14396, IF_486|IF_SM},
    {I_XADD, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+14396, IF_486},
    {I_XADD, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+14402, IF_486|IF_SM},
    {I_XADD, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+14402, IF_486},
    {I_XADD, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+14408, IF_X64|IF_SM},
    {I_XADD, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+14408, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_XBTS[] = {
    {I_XBTS, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+14414, IF_386|IF_SW|IF_UNDOC},
    {I_XBTS, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+14414, IF_386|IF_UNDOC},
    {I_XBTS, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+14420, IF_386|IF_SD|IF_UNDOC},
    {I_XBTS, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+14420, IF_386|IF_UNDOC},
    ITEMPLATE_END
};

static const struct itemplate instrux_XCHG[] = {
    {I_XCHG, 2, {REG_AX,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+19632, IF_8086},
    {I_XCHG, 2, {REG_EAX,REG32NA,0,0,0}, nasm_bytecodes+19636, IF_386},
    {I_XCHG, 2, {REG_RAX,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+19640, IF_X64},
    {I_XCHG, 2, {REG_GPR|BITS16,REG_AX,0,0,0}, nasm_bytecodes+19644, IF_8086},
    {I_XCHG, 2, {REG32NA,REG_EAX,0,0,0}, nasm_bytecodes+19648, IF_386},
    {I_XCHG, 2, {REG_GPR|BITS64,REG_RAX,0,0,0}, nasm_bytecodes+19652, IF_X64},
    {I_XCHG, 2, {REG_EAX,REG_EAX,0,0,0}, nasm_bytecodes+19656, IF_386|IF_NOLONG},
    {I_XCHG, 2, {REG_GPR|BITS8,MEMORY,0,0,0}, nasm_bytecodes+19660, IF_8086|IF_SM},
    {I_XCHG, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19660, IF_8086},
    {I_XCHG, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+18492, IF_8086|IF_SM},
    {I_XCHG, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18492, IF_8086},
    {I_XCHG, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+18497, IF_386|IF_SM},
    {I_XCHG, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18497, IF_386},
    {I_XCHG, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+18502, IF_X64|IF_SM},
    {I_XCHG, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18502, IF_X64},
    {I_XCHG, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19664, IF_8086|IF_SM},
    {I_XCHG, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19664, IF_8086},
    {I_XCHG, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18507, IF_8086|IF_SM},
    {I_XCHG, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18507, IF_8086},
    {I_XCHG, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18512, IF_386|IF_SM},
    {I_XCHG, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18512, IF_386},
    {I_XCHG, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18517, IF_X64|IF_SM},
    {I_XCHG, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18517, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_XCRYPTCBC[] = {
    {I_XCRYPTCBC, 0, {0,0,0,0,0}, nasm_bytecodes+15596, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_XCRYPTCFB[] = {
    {I_XCRYPTCFB, 0, {0,0,0,0,0}, nasm_bytecodes+15608, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_XCRYPTCTR[] = {
    {I_XCRYPTCTR, 0, {0,0,0,0,0}, nasm_bytecodes+15602, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_XCRYPTECB[] = {
    {I_XCRYPTECB, 0, {0,0,0,0,0}, nasm_bytecodes+15590, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_XCRYPTOFB[] = {
    {I_XCRYPTOFB, 0, {0,0,0,0,0}, nasm_bytecodes+15614, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_XGETBV[] = {
    {I_XGETBV, 0, {0,0,0,0,0}, nasm_bytecodes+14720, IF_NEHALEM},
    ITEMPLATE_END
};

static const struct itemplate instrux_XLAT[] = {
    {I_XLAT, 0, {0,0,0,0,0}, nasm_bytecodes+19743, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_XLATB[] = {
    {I_XLATB, 0, {0,0,0,0,0}, nasm_bytecodes+19743, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_XOR[] = {
    {I_XOR, 2, {MEMORY,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19668, IF_8086|IF_SM},
    {I_XOR, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+19668, IF_8086},
    {I_XOR, 2, {MEMORY,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18522, IF_8086|IF_SM},
    {I_XOR, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18522, IF_8086},
    {I_XOR, 2, {MEMORY,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18527, IF_386|IF_SM},
    {I_XOR, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18527, IF_386},
    {I_XOR, 2, {MEMORY,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18532, IF_X64|IF_SM},
    {I_XOR, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18532, IF_X64},
    {I_XOR, 2, {REG_GPR|BITS8,MEMORY,0,0,0}, nasm_bytecodes+10864, IF_8086|IF_SM},
    {I_XOR, 2, {REG_GPR|BITS8,REG_GPR|BITS8,0,0,0}, nasm_bytecodes+10864, IF_8086},
    {I_XOR, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+18537, IF_8086|IF_SM},
    {I_XOR, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+18537, IF_8086},
    {I_XOR, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+18542, IF_386|IF_SM},
    {I_XOR, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+18542, IF_386},
    {I_XOR, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+18547, IF_X64|IF_SM},
    {I_XOR, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+18547, IF_X64},
    {I_XOR, 2, {RM_GPR|BITS16,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+14426, IF_8086},
    {I_XOR, 2, {RM_GPR|BITS32,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+14432, IF_386},
    {I_XOR, 2, {RM_GPR|BITS64,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+14438, IF_X64},
    {I_XOR, 2, {REG_AL,IMMEDIATE,0,0,0}, nasm_bytecodes+19672, IF_8086|IF_SM},
    {I_XOR, 2, {REG_AX,SBYTE16,0,0,0}, nasm_bytecodes+14426, IF_8086|IF_SM},
    {I_XOR, 2, {REG_AX,IMMEDIATE,0,0,0}, nasm_bytecodes+18552, IF_8086|IF_SM},
    {I_XOR, 2, {REG_EAX,SBYTE32,0,0,0}, nasm_bytecodes+14432, IF_386|IF_SM},
    {I_XOR, 2, {REG_EAX,IMMEDIATE,0,0,0}, nasm_bytecodes+18557, IF_386|IF_SM},
    {I_XOR, 2, {REG_RAX,SBYTE64,0,0,0}, nasm_bytecodes+14438, IF_X64|IF_SM},
    {I_XOR, 2, {REG_RAX,IMMEDIATE,0,0,0}, nasm_bytecodes+18562, IF_X64|IF_SM},
    {I_XOR, 2, {RM_GPR|BITS8,IMMEDIATE,0,0,0}, nasm_bytecodes+18567, IF_8086|IF_SM},
    {I_XOR, 2, {RM_GPR|BITS16,IMMEDIATE,0,0,0}, nasm_bytecodes+14444, IF_8086|IF_SM},
    {I_XOR, 2, {RM_GPR|BITS32,IMMEDIATE,0,0,0}, nasm_bytecodes+14450, IF_386|IF_SM},
    {I_XOR, 2, {RM_GPR|BITS64,IMMEDIATE,0,0,0}, nasm_bytecodes+14456, IF_X64|IF_SM},
    {I_XOR, 2, {MEMORY,IMMEDIATE|BITS8,0,0,0}, nasm_bytecodes+18567, IF_8086|IF_SM},
    {I_XOR, 2, {MEMORY,IMMEDIATE|BITS16,0,0,0}, nasm_bytecodes+14444, IF_8086|IF_SM},
    {I_XOR, 2, {MEMORY,IMMEDIATE|BITS32,0,0,0}, nasm_bytecodes+14450, IF_386|IF_SM},
    ITEMPLATE_END
};

static const struct itemplate instrux_XORPD[] = {
    {I_XORPD, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+15458, IF_WILLAMETTE|IF_SSE2|IF_SO},
    ITEMPLATE_END
};

static const struct itemplate instrux_XORPS[] = {
    {I_XORPS, 2, {XMMREG,RM_XMM,0,0,0}, nasm_bytecodes+14702, IF_KATMAI|IF_SSE},
    ITEMPLATE_END
};

static const struct itemplate instrux_XRSTOR[] = {
    {I_XRSTOR, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14745, IF_NEHALEM},
    ITEMPLATE_END
};

static const struct itemplate instrux_XRSTOR64[] = {
    {I_XRSTOR64, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14744, IF_LONG|IF_NEHALEM},
    ITEMPLATE_END
};

static const struct itemplate instrux_XSAVE[] = {
    {I_XSAVE, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14733, IF_NEHALEM},
    ITEMPLATE_END
};

static const struct itemplate instrux_XSAVE64[] = {
    {I_XSAVE64, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14732, IF_LONG|IF_NEHALEM},
    ITEMPLATE_END
};

static const struct itemplate instrux_XSAVEOPT[] = {
    {I_XSAVEOPT, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14739, IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_XSAVEOPT64[] = {
    {I_XSAVEOPT64, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14738, IF_LONG|IF_FUTURE},
    ITEMPLATE_END
};

static const struct itemplate instrux_XSETBV[] = {
    {I_XSETBV, 0, {0,0,0,0,0}, nasm_bytecodes+14726, IF_NEHALEM|IF_PRIV},
    ITEMPLATE_END
};

static const struct itemplate instrux_XSHA1[] = {
    {I_XSHA1, 0, {0,0,0,0,0}, nasm_bytecodes+15626, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_XSHA256[] = {
    {I_XSHA256, 0, {0,0,0,0,0}, nasm_bytecodes+15632, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_XSTORE[] = {
    {I_XSTORE, 0, {0,0,0,0,0}, nasm_bytecodes+18627, IF_PENT|IF_CYRIX},
    ITEMPLATE_END
};

static const struct itemplate instrux_CMOVcc[] = {
    {I_CMOVcc, 2, {REG_GPR|BITS16,MEMORY,0,0,0}, nasm_bytecodes+7501, IF_P6|IF_SM},
    {I_CMOVcc, 2, {REG_GPR|BITS16,REG_GPR|BITS16,0,0,0}, nasm_bytecodes+7501, IF_P6},
    {I_CMOVcc, 2, {REG_GPR|BITS32,MEMORY,0,0,0}, nasm_bytecodes+7508, IF_P6|IF_SM},
    {I_CMOVcc, 2, {REG_GPR|BITS32,REG_GPR|BITS32,0,0,0}, nasm_bytecodes+7508, IF_P6},
    {I_CMOVcc, 2, {REG_GPR|BITS64,MEMORY,0,0,0}, nasm_bytecodes+7515, IF_X64|IF_SM},
    {I_CMOVcc, 2, {REG_GPR|BITS64,REG_GPR|BITS64,0,0,0}, nasm_bytecodes+7515, IF_X64},
    ITEMPLATE_END
};

static const struct itemplate instrux_Jcc[] = {
    {I_Jcc, 1, {IMMEDIATE|NEAR,0,0,0,0}, nasm_bytecodes+7522, IF_386},
    {I_Jcc, 1, {IMMEDIATE|BITS16|NEAR,0,0,0,0}, nasm_bytecodes+7529, IF_386},
    {I_Jcc, 1, {IMMEDIATE|BITS32|NEAR,0,0,0,0}, nasm_bytecodes+7536, IF_386},
    {I_Jcc, 1, {IMMEDIATE|SHORT,0,0,0,0}, nasm_bytecodes+18573, IF_8086},
    {I_Jcc, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+18572, IF_8086},
    {I_Jcc, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+7537, IF_386},
    {I_Jcc, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+7543, IF_8086},
    {I_Jcc, 1, {IMMEDIATE,0,0,0,0}, nasm_bytecodes+18573, IF_8086},
    ITEMPLATE_END
};

static const struct itemplate instrux_SETcc[] = {
    {I_SETcc, 1, {MEMORY,0,0,0,0}, nasm_bytecodes+14462, IF_386|IF_SB},
    {I_SETcc, 1, {REG_GPR|BITS8,0,0,0,0}, nasm_bytecodes+14462, IF_386},
    ITEMPLATE_END
};

const struct itemplate * const nasm_instructions[] = {
    instrux_AAA,
    instrux_AAD,
    instrux_AAM,
    instrux_AAS,
    instrux_ADC,
    instrux_ADD,
    instrux_ADDPD,
    instrux_ADDPS,
    instrux_ADDSD,
    instrux_ADDSS,
    instrux_ADDSUBPD,
    instrux_ADDSUBPS,
    instrux_AESDEC,
    instrux_AESDECLAST,
    instrux_AESENC,
    instrux_AESENCLAST,
    instrux_AESIMC,
    instrux_AESKEYGENASSIST,
    instrux_AND,
    instrux_ANDNPD,
    instrux_ANDNPS,
    instrux_ANDPD,
    instrux_ANDPS,
    instrux_ARPL,
    instrux_BB0_RESET,
    instrux_BB1_RESET,
    instrux_BLENDPD,
    instrux_BLENDPS,
    instrux_BLENDVPD,
    instrux_BLENDVPS,
    instrux_BOUND,
    instrux_BSF,
    instrux_BSR,
    instrux_BSWAP,
    instrux_BT,
    instrux_BTC,
    instrux_BTR,
    instrux_BTS,
    instrux_CALL,
    instrux_CBW,
    instrux_CDQ,
    instrux_CDQE,
    instrux_CLC,
    instrux_CLD,
    instrux_CLFLUSH,
    instrux_CLGI,
    instrux_CLI,
    instrux_CLTS,
    instrux_CMC,
    instrux_CMP,
    instrux_CMPEQPD,
    instrux_CMPEQPS,
    instrux_CMPEQSD,
    instrux_CMPEQSS,
    instrux_CMPLEPD,
    instrux_CMPLEPS,
    instrux_CMPLESD,
    instrux_CMPLESS,
    instrux_CMPLTPD,
    instrux_CMPLTPS,
    instrux_CMPLTSD,
    instrux_CMPLTSS,
    instrux_CMPNEQPD,
    instrux_CMPNEQPS,
    instrux_CMPNEQSD,
    instrux_CMPNEQSS,
    instrux_CMPNLEPD,
    instrux_CMPNLEPS,
    instrux_CMPNLESD,
    instrux_CMPNLESS,
    instrux_CMPNLTPD,
    instrux_CMPNLTPS,
    instrux_CMPNLTSD,
    instrux_CMPNLTSS,
    instrux_CMPORDPD,
    instrux_CMPORDPS,
    instrux_CMPORDSD,
    instrux_CMPORDSS,
    instrux_CMPPD,
    instrux_CMPPS,
    instrux_CMPSB,
    instrux_CMPSD,
    instrux_CMPSQ,
    instrux_CMPSS,
    instrux_CMPSW,
    instrux_CMPUNORDPD,
    instrux_CMPUNORDPS,
    instrux_CMPUNORDSD,
    instrux_CMPUNORDSS,
    instrux_CMPXCHG,
    instrux_CMPXCHG16B,
    instrux_CMPXCHG486,
    instrux_CMPXCHG8B,
    instrux_COMISD,
    instrux_COMISS,
    instrux_CPUID,
    instrux_CPU_READ,
    instrux_CPU_WRITE,
    instrux_CQO,
    instrux_CRC32,
    instrux_CVTDQ2PD,
    instrux_CVTDQ2PS,
    instrux_CVTPD2DQ,
    instrux_CVTPD2PI,
    instrux_CVTPD2PS,
    instrux_CVTPI2PD,
    instrux_CVTPI2PS,
    instrux_CVTPS2DQ,
    instrux_CVTPS2PD,
    instrux_CVTPS2PI,
    instrux_CVTSD2SI,
    instrux_CVTSD2SS,
    instrux_CVTSI2SD,
    instrux_CVTSI2SS,
    instrux_CVTSS2SD,
    instrux_CVTSS2SI,
    instrux_CVTTPD2DQ,
    instrux_CVTTPD2PI,
    instrux_CVTTPS2DQ,
    instrux_CVTTPS2PI,
    instrux_CVTTSD2SI,
    instrux_CVTTSS2SI,
    instrux_CWD,
    instrux_CWDE,
    instrux_DAA,
    instrux_DAS,
    instrux_DB,
    instrux_DD,
    instrux_DEC,
    instrux_DIV,
    instrux_DIVPD,
    instrux_DIVPS,
    instrux_DIVSD,
    instrux_DIVSS,
    instrux_DMINT,
    instrux_DO,
    instrux_DPPD,
    instrux_DPPS,
    instrux_DQ,
    instrux_DT,
    instrux_DW,
    instrux_DY,
    instrux_EMMS,
    instrux_ENTER,
    instrux_EQU,
    instrux_EXTRACTPS,
    instrux_EXTRQ,
    instrux_F2XM1,
    instrux_FABS,
    instrux_FADD,
    instrux_FADDP,
    instrux_FBLD,
    instrux_FBSTP,
    instrux_FCHS,
    instrux_FCLEX,
    instrux_FCMOVB,
    instrux_FCMOVBE,
    instrux_FCMOVE,
    instrux_FCMOVNB,
    instrux_FCMOVNBE,
    instrux_FCMOVNE,
    instrux_FCMOVNU,
    instrux_FCMOVU,
    instrux_FCOM,
    instrux_FCOMI,
    instrux_FCOMIP,
    instrux_FCOMP,
    instrux_FCOMPP,
    instrux_FCOS,
    instrux_FDECSTP,
    instrux_FDISI,
    instrux_FDIV,
    instrux_FDIVP,
    instrux_FDIVR,
    instrux_FDIVRP,
    instrux_FEMMS,
    instrux_FENI,
    instrux_FFREE,
    instrux_FFREEP,
    instrux_FIADD,
    instrux_FICOM,
    instrux_FICOMP,
    instrux_FIDIV,
    instrux_FIDIVR,
    instrux_FILD,
    instrux_FIMUL,
    instrux_FINCSTP,
    instrux_FINIT,
    instrux_FIST,
    instrux_FISTP,
    instrux_FISTTP,
    instrux_FISUB,
    instrux_FISUBR,
    instrux_FLD,
    instrux_FLD1,
    instrux_FLDCW,
    instrux_FLDENV,
    instrux_FLDL2E,
    instrux_FLDL2T,
    instrux_FLDLG2,
    instrux_FLDLN2,
    instrux_FLDPI,
    instrux_FLDZ,
    instrux_FMUL,
    instrux_FMULP,
    instrux_FNCLEX,
    instrux_FNDISI,
    instrux_FNENI,
    instrux_FNINIT,
    instrux_FNOP,
    instrux_FNSAVE,
    instrux_FNSTCW,
    instrux_FNSTENV,
    instrux_FNSTSW,
    instrux_FPATAN,
    instrux_FPREM,
    instrux_FPREM1,
    instrux_FPTAN,
    instrux_FRNDINT,
    instrux_FRSTOR,
    instrux_FSAVE,
    instrux_FSCALE,
    instrux_FSETPM,
    instrux_FSIN,
    instrux_FSINCOS,
    instrux_FSQRT,
    instrux_FST,
    instrux_FSTCW,
    instrux_FSTENV,
    instrux_FSTP,
    instrux_FSTSW,
    instrux_FSUB,
    instrux_FSUBP,
    instrux_FSUBR,
    instrux_FSUBRP,
    instrux_FTST,
    instrux_FUCOM,
    instrux_FUCOMI,
    instrux_FUCOMIP,
    instrux_FUCOMP,
    instrux_FUCOMPP,
    instrux_FWAIT,
    instrux_FXAM,
    instrux_FXCH,
    instrux_FXRSTOR,
    instrux_FXRSTOR64,
    instrux_FXSAVE,
    instrux_FXSAVE64,
    instrux_FXTRACT,
    instrux_FYL2X,
    instrux_FYL2XP1,
    instrux_GETSEC,
    instrux_HADDPD,
    instrux_HADDPS,
    instrux_HINT_NOP0,
    instrux_HINT_NOP1,
    instrux_HINT_NOP10,
    instrux_HINT_NOP11,
    instrux_HINT_NOP12,
    instrux_HINT_NOP13,
    instrux_HINT_NOP14,
    instrux_HINT_NOP15,
    instrux_HINT_NOP16,
    instrux_HINT_NOP17,
    instrux_HINT_NOP18,
    instrux_HINT_NOP19,
    instrux_HINT_NOP2,
    instrux_HINT_NOP20,
    instrux_HINT_NOP21,
    instrux_HINT_NOP22,
    instrux_HINT_NOP23,
    instrux_HINT_NOP24,
    instrux_HINT_NOP25,
    instrux_HINT_NOP26,
    instrux_HINT_NOP27,
    instrux_HINT_NOP28,
    instrux_HINT_NOP29,
    instrux_HINT_NOP3,
    instrux_HINT_NOP30,
    instrux_HINT_NOP31,
    instrux_HINT_NOP32,
    instrux_HINT_NOP33,
    instrux_HINT_NOP34,
    instrux_HINT_NOP35,
    instrux_HINT_NOP36,
    instrux_HINT_NOP37,
    instrux_HINT_NOP38,
    instrux_HINT_NOP39,
    instrux_HINT_NOP4,
    instrux_HINT_NOP40,
    instrux_HINT_NOP41,
    instrux_HINT_NOP42,
    instrux_HINT_NOP43,
    instrux_HINT_NOP44,
    instrux_HINT_NOP45,
    instrux_HINT_NOP46,
    instrux_HINT_NOP47,
    instrux_HINT_NOP48,
    instrux_HINT_NOP49,
    instrux_HINT_NOP5,
    instrux_HINT_NOP50,
    instrux_HINT_NOP51,
    instrux_HINT_NOP52,
    instrux_HINT_NOP53,
    instrux_HINT_NOP54,
    instrux_HINT_NOP55,
    instrux_HINT_NOP56,
    instrux_HINT_NOP57,
    instrux_HINT_NOP58,
    instrux_HINT_NOP59,
    instrux_HINT_NOP6,
    instrux_HINT_NOP60,
    instrux_HINT_NOP61,
    instrux_HINT_NOP62,
    instrux_HINT_NOP63,
    instrux_HINT_NOP7,
    instrux_HINT_NOP8,
    instrux_HINT_NOP9,
    instrux_HLT,
    instrux_HSUBPD,
    instrux_HSUBPS,
    instrux_IBTS,
    instrux_ICEBP,
    instrux_IDIV,
    instrux_IMUL,
    instrux_IN,
    instrux_INC,
    instrux_INCBIN,
    instrux_INSB,
    instrux_INSD,
    instrux_INSERTPS,
    instrux_INSERTQ,
    instrux_INSW,
    instrux_INT,
    instrux_INT01,
    instrux_INT03,
    instrux_INT1,
    instrux_INT3,
    instrux_INTO,
    instrux_INVD,
    instrux_INVEPT,
    instrux_INVLPG,
    instrux_INVLPGA,
    instrux_INVVPID,
    instrux_IRET,
    instrux_IRETD,
    instrux_IRETQ,
    instrux_IRETW,
    instrux_JCXZ,
    instrux_JECXZ,
    instrux_JMP,
    instrux_JMPE,
    instrux_JRCXZ,
    instrux_LAHF,
    instrux_LAR,
    instrux_LDDQU,
    instrux_LDMXCSR,
    instrux_LDS,
    instrux_LEA,
    instrux_LEAVE,
    instrux_LES,
    instrux_LFENCE,
    instrux_LFS,
    instrux_LGDT,
    instrux_LGS,
    instrux_LIDT,
    instrux_LLDT,
    instrux_LLWPCB,
    instrux_LMSW,
    instrux_LOADALL,
    instrux_LOADALL286,
    instrux_LODSB,
    instrux_LODSD,
    instrux_LODSQ,
    instrux_LODSW,
    instrux_LOOP,
    instrux_LOOPE,
    instrux_LOOPNE,
    instrux_LOOPNZ,
    instrux_LOOPZ,
    instrux_LSL,
    instrux_LSS,
    instrux_LTR,
    instrux_LWPINS,
    instrux_LWPVAL,
    instrux_LZCNT,
    instrux_MASKMOVDQU,
    instrux_MASKMOVQ,
    instrux_MAXPD,
    instrux_MAXPS,
    instrux_MAXSD,
    instrux_MAXSS,
    instrux_MFENCE,
    instrux_MINPD,
    instrux_MINPS,
    instrux_MINSD,
    instrux_MINSS,
    instrux_MONITOR,
    instrux_MONTMUL,
    instrux_MOV,
    instrux_MOVAPD,
    instrux_MOVAPS,
    instrux_MOVBE,
    instrux_MOVD,
    instrux_MOVDDUP,
    instrux_MOVDQ2Q,
    instrux_MOVDQA,
    instrux_MOVDQU,
    instrux_MOVHLPS,
    instrux_MOVHPD,
    instrux_MOVHPS,
    instrux_MOVLHPS,
    instrux_MOVLPD,
    instrux_MOVLPS,
    instrux_MOVMSKPD,
    instrux_MOVMSKPS,
    instrux_MOVNTDQ,
    instrux_MOVNTDQA,
    instrux_MOVNTI,
    instrux_MOVNTPD,
    instrux_MOVNTPS,
    instrux_MOVNTQ,
    instrux_MOVNTSD,
    instrux_MOVNTSS,
    instrux_MOVQ,
    instrux_MOVQ2DQ,
    instrux_MOVSB,
    instrux_MOVSD,
    instrux_MOVSHDUP,
    instrux_MOVSLDUP,
    instrux_MOVSQ,
    instrux_MOVSS,
    instrux_MOVSW,
    instrux_MOVSX,
    instrux_MOVSXD,
    instrux_MOVUPD,
    instrux_MOVUPS,
    instrux_MOVZX,
    instrux_MPSADBW,
    instrux_MUL,
    instrux_MULPD,
    instrux_MULPS,
    instrux_MULSD,
    instrux_MULSS,
    instrux_MWAIT,
    instrux_NEG,
    instrux_NOP,
    instrux_NOT,
    instrux_OR,
    instrux_ORPD,
    instrux_ORPS,
    instrux_OUT,
    instrux_OUTSB,
    instrux_OUTSD,
    instrux_OUTSW,
    instrux_PABSB,
    instrux_PABSD,
    instrux_PABSW,
    instrux_PACKSSDW,
    instrux_PACKSSWB,
    instrux_PACKUSDW,
    instrux_PACKUSWB,
    instrux_PADDB,
    instrux_PADDD,
    instrux_PADDQ,
    instrux_PADDSB,
    instrux_PADDSIW,
    instrux_PADDSW,
    instrux_PADDUSB,
    instrux_PADDUSW,
    instrux_PADDW,
    instrux_PALIGNR,
    instrux_PAND,
    instrux_PANDN,
    instrux_PAUSE,
    instrux_PAVEB,
    instrux_PAVGB,
    instrux_PAVGUSB,
    instrux_PAVGW,
    instrux_PBLENDVB,
    instrux_PBLENDW,
    instrux_PCLMULHQHQDQ,
    instrux_PCLMULHQLQDQ,
    instrux_PCLMULLQHQDQ,
    instrux_PCLMULLQLQDQ,
    instrux_PCLMULQDQ,
    instrux_PCMPEQB,
    instrux_PCMPEQD,
    instrux_PCMPEQQ,
    instrux_PCMPEQW,
    instrux_PCMPESTRI,
    instrux_PCMPESTRM,
    instrux_PCMPGTB,
    instrux_PCMPGTD,
    instrux_PCMPGTQ,
    instrux_PCMPGTW,
    instrux_PCMPISTRI,
    instrux_PCMPISTRM,
    instrux_PDISTIB,
    instrux_PEXTRB,
    instrux_PEXTRD,
    instrux_PEXTRQ,
    instrux_PEXTRW,
    instrux_PF2ID,
    instrux_PF2IW,
    instrux_PFACC,
    instrux_PFADD,
    instrux_PFCMPEQ,
    instrux_PFCMPGE,
    instrux_PFCMPGT,
    instrux_PFMAX,
    instrux_PFMIN,
    instrux_PFMUL,
    instrux_PFNACC,
    instrux_PFPNACC,
    instrux_PFRCP,
    instrux_PFRCPIT1,
    instrux_PFRCPIT2,
    instrux_PFRCPV,
    instrux_PFRSQIT1,
    instrux_PFRSQRT,
    instrux_PFRSQRTV,
    instrux_PFSUB,
    instrux_PFSUBR,
    instrux_PHADDD,
    instrux_PHADDSW,
    instrux_PHADDW,
    instrux_PHMINPOSUW,
    instrux_PHSUBD,
    instrux_PHSUBSW,
    instrux_PHSUBW,
    instrux_PI2FD,
    instrux_PI2FW,
    instrux_PINSRB,
    instrux_PINSRD,
    instrux_PINSRQ,
    instrux_PINSRW,
    instrux_PMACHRIW,
    instrux_PMADDUBSW,
    instrux_PMADDWD,
    instrux_PMAGW,
    instrux_PMAXSB,
    instrux_PMAXSD,
    instrux_PMAXSW,
    instrux_PMAXUB,
    instrux_PMAXUD,
    instrux_PMAXUW,
    instrux_PMINSB,
    instrux_PMINSD,
    instrux_PMINSW,
    instrux_PMINUB,
    instrux_PMINUD,
    instrux_PMINUW,
    instrux_PMOVMSKB,
    instrux_PMOVSXBD,
    instrux_PMOVSXBQ,
    instrux_PMOVSXBW,
    instrux_PMOVSXDQ,
    instrux_PMOVSXWD,
    instrux_PMOVSXWQ,
    instrux_PMOVZXBD,
    instrux_PMOVZXBQ,
    instrux_PMOVZXBW,
    instrux_PMOVZXDQ,
    instrux_PMOVZXWD,
    instrux_PMOVZXWQ,
    instrux_PMULDQ,
    instrux_PMULHRIW,
    instrux_PMULHRSW,
    instrux_PMULHRWA,
    instrux_PMULHRWC,
    instrux_PMULHUW,
    instrux_PMULHW,
    instrux_PMULLD,
    instrux_PMULLW,
    instrux_PMULUDQ,
    instrux_PMVGEZB,
    instrux_PMVLZB,
    instrux_PMVNZB,
    instrux_PMVZB,
    instrux_POP,
    instrux_POPA,
    instrux_POPAD,
    instrux_POPAW,
    instrux_POPCNT,
    instrux_POPF,
    instrux_POPFD,
    instrux_POPFQ,
    instrux_POPFW,
    instrux_POR,
    instrux_PREFETCH,
    instrux_PREFETCHNTA,
    instrux_PREFETCHT0,
    instrux_PREFETCHT1,
    instrux_PREFETCHT2,
    instrux_PREFETCHW,
    instrux_PSADBW,
    instrux_PSHUFB,
    instrux_PSHUFD,
    instrux_PSHUFHW,
    instrux_PSHUFLW,
    instrux_PSHUFW,
    instrux_PSIGNB,
    instrux_PSIGND,
    instrux_PSIGNW,
    instrux_PSLLD,
    instrux_PSLLDQ,
    instrux_PSLLQ,
    instrux_PSLLW,
    instrux_PSRAD,
    instrux_PSRAW,
    instrux_PSRLD,
    instrux_PSRLDQ,
    instrux_PSRLQ,
    instrux_PSRLW,
    instrux_PSUBB,
    instrux_PSUBD,
    instrux_PSUBQ,
    instrux_PSUBSB,
    instrux_PSUBSIW,
    instrux_PSUBSW,
    instrux_PSUBUSB,
    instrux_PSUBUSW,
    instrux_PSUBW,
    instrux_PSWAPD,
    instrux_PTEST,
    instrux_PUNPCKHBW,
    instrux_PUNPCKHDQ,
    instrux_PUNPCKHQDQ,
    instrux_PUNPCKHWD,
    instrux_PUNPCKLBW,
    instrux_PUNPCKLDQ,
    instrux_PUNPCKLQDQ,
    instrux_PUNPCKLWD,
    instrux_PUSH,
    instrux_PUSHA,
    instrux_PUSHAD,
    instrux_PUSHAW,
    instrux_PUSHF,
    instrux_PUSHFD,
    instrux_PUSHFQ,
    instrux_PUSHFW,
    instrux_PXOR,
    instrux_RCL,
    instrux_RCPPS,
    instrux_RCPSS,
    instrux_RCR,
    instrux_RDFSBASE,
    instrux_RDGSBASE,
    instrux_RDM,
    instrux_RDMSR,
    instrux_RDPMC,
    instrux_RDRAND,
    instrux_RDSHR,
    instrux_RDTSC,
    instrux_RDTSCP,
    instrux_RESB,
    instrux_RESD,
    instrux_RESO,
    instrux_RESQ,
    instrux_REST,
    instrux_RESW,
    instrux_RESY,
    instrux_RET,
    instrux_RETF,
    instrux_RETN,
    instrux_ROL,
    instrux_ROR,
    instrux_ROUNDPD,
    instrux_ROUNDPS,
    instrux_ROUNDSD,
    instrux_ROUNDSS,
    instrux_RSDC,
    instrux_RSLDT,
    instrux_RSM,
    instrux_RSQRTPS,
    instrux_RSQRTSS,
    instrux_RSTS,
    instrux_SAHF,
    instrux_SAL,
    instrux_SALC,
    instrux_SAR,
    instrux_SBB,
    instrux_SCASB,
    instrux_SCASD,
    instrux_SCASQ,
    instrux_SCASW,
    instrux_SFENCE,
    instrux_SGDT,
    instrux_SHL,
    instrux_SHLD,
    instrux_SHR,
    instrux_SHRD,
    instrux_SHUFPD,
    instrux_SHUFPS,
    instrux_SIDT,
    instrux_SKINIT,
    instrux_SLDT,
    instrux_SLWPCB,
    instrux_SMI,
    instrux_SMINT,
    instrux_SMINTOLD,
    instrux_SMSW,
    instrux_SQRTPD,
    instrux_SQRTPS,
    instrux_SQRTSD,
    instrux_SQRTSS,
    instrux_STC,
    instrux_STD,
    instrux_STGI,
    instrux_STI,
    instrux_STMXCSR,
    instrux_STOSB,
    instrux_STOSD,
    instrux_STOSQ,
    instrux_STOSW,
    instrux_STR,
    instrux_SUB,
    instrux_SUBPD,
    instrux_SUBPS,
    instrux_SUBSD,
    instrux_SUBSS,
    instrux_SVDC,
    instrux_SVLDT,
    instrux_SVTS,
    instrux_SWAPGS,
    instrux_SYSCALL,
    instrux_SYSENTER,
    instrux_SYSEXIT,
    instrux_SYSRET,
    instrux_TEST,
    instrux_UCOMISD,
    instrux_UCOMISS,
    instrux_UD0,
    instrux_UD1,
    instrux_UD2,
    instrux_UD2A,
    instrux_UD2B,
    instrux_UMOV,
    instrux_UNPCKHPD,
    instrux_UNPCKHPS,
    instrux_UNPCKLPD,
    instrux_UNPCKLPS,
    instrux_VADDPD,
    instrux_VADDPS,
    instrux_VADDSD,
    instrux_VADDSS,
    instrux_VADDSUBPD,
    instrux_VADDSUBPS,
    instrux_VAESDEC,
    instrux_VAESDECLAST,
    instrux_VAESENC,
    instrux_VAESENCLAST,
    instrux_VAESIMC,
    instrux_VAESKEYGENASSIST,
    instrux_VANDNPD,
    instrux_VANDNPS,
    instrux_VANDPD,
    instrux_VANDPS,
    instrux_VBLENDPD,
    instrux_VBLENDPS,
    instrux_VBLENDVPD,
    instrux_VBLENDVPS,
    instrux_VBROADCASTF128,
    instrux_VBROADCASTSD,
    instrux_VBROADCASTSS,
    instrux_VCMPEQPD,
    instrux_VCMPEQPS,
    instrux_VCMPEQSD,
    instrux_VCMPEQSS,
    instrux_VCMPEQ_OSPD,
    instrux_VCMPEQ_OSPS,
    instrux_VCMPEQ_OSSD,
    instrux_VCMPEQ_OSSS,
    instrux_VCMPEQ_UQPD,
    instrux_VCMPEQ_UQPS,
    instrux_VCMPEQ_UQSD,
    instrux_VCMPEQ_UQSS,
    instrux_VCMPEQ_USPD,
    instrux_VCMPEQ_USPS,
    instrux_VCMPEQ_USSD,
    instrux_VCMPEQ_USSS,
    instrux_VCMPFALSEPD,
    instrux_VCMPFALSEPS,
    instrux_VCMPFALSESD,
    instrux_VCMPFALSESS,
    instrux_VCMPFALSE_OQPD,
    instrux_VCMPFALSE_OQPS,
    instrux_VCMPFALSE_OQSD,
    instrux_VCMPFALSE_OQSS,
    instrux_VCMPFALSE_OSPD,
    instrux_VCMPFALSE_OSPS,
    instrux_VCMPFALSE_OSSD,
    instrux_VCMPFALSE_OSSS,
    instrux_VCMPGEPD,
    instrux_VCMPGEPS,
    instrux_VCMPGESD,
    instrux_VCMPGESS,
    instrux_VCMPGE_OQPD,
    instrux_VCMPGE_OQPS,
    instrux_VCMPGE_OQSD,
    instrux_VCMPGE_OQSS,
    instrux_VCMPGE_OSPD,
    instrux_VCMPGE_OSPS,
    instrux_VCMPGE_OSSD,
    instrux_VCMPGE_OSSS,
    instrux_VCMPGTPD,
    instrux_VCMPGTPS,
    instrux_VCMPGTSD,
    instrux_VCMPGTSS,
    instrux_VCMPGT_OQPD,
    instrux_VCMPGT_OQPS,
    instrux_VCMPGT_OQSD,
    instrux_VCMPGT_OQSS,
    instrux_VCMPGT_OSPD,
    instrux_VCMPGT_OSPS,
    instrux_VCMPGT_OSSD,
    instrux_VCMPGT_OSSS,
    instrux_VCMPLEPD,
    instrux_VCMPLEPS,
    instrux_VCMPLESD,
    instrux_VCMPLESS,
    instrux_VCMPLE_OQPD,
    instrux_VCMPLE_OQPS,
    instrux_VCMPLE_OQSD,
    instrux_VCMPLE_OQSS,
    instrux_VCMPLE_OSPD,
    instrux_VCMPLE_OSPS,
    instrux_VCMPLE_OSSD,
    instrux_VCMPLE_OSSS,
    instrux_VCMPLTPD,
    instrux_VCMPLTPS,
    instrux_VCMPLTSD,
    instrux_VCMPLTSS,
    instrux_VCMPLT_OQPD,
    instrux_VCMPLT_OQPS,
    instrux_VCMPLT_OQSD,
    instrux_VCMPLT_OQSS,
    instrux_VCMPLT_OSPD,
    instrux_VCMPLT_OSPS,
    instrux_VCMPLT_OSSD,
    instrux_VCMPLT_OSSS,
    instrux_VCMPNEQPD,
    instrux_VCMPNEQPS,
    instrux_VCMPNEQSD,
    instrux_VCMPNEQSS,
    instrux_VCMPNEQ_OQPD,
    instrux_VCMPNEQ_OQPS,
    instrux_VCMPNEQ_OQSD,
    instrux_VCMPNEQ_OQSS,
    instrux_VCMPNEQ_OSPD,
    instrux_VCMPNEQ_OSPS,
    instrux_VCMPNEQ_OSSD,
    instrux_VCMPNEQ_OSSS,
    instrux_VCMPNEQ_UQPD,
    instrux_VCMPNEQ_UQPS,
    instrux_VCMPNEQ_UQSD,
    instrux_VCMPNEQ_UQSS,
    instrux_VCMPNEQ_USPD,
    instrux_VCMPNEQ_USPS,
    instrux_VCMPNEQ_USSD,
    instrux_VCMPNEQ_USSS,
    instrux_VCMPNGEPD,
    instrux_VCMPNGEPS,
    instrux_VCMPNGESD,
    instrux_VCMPNGESS,
    instrux_VCMPNGE_UQPD,
    instrux_VCMPNGE_UQPS,
    instrux_VCMPNGE_UQSD,
    instrux_VCMPNGE_UQSS,
    instrux_VCMPNGE_USPD,
    instrux_VCMPNGE_USPS,
    instrux_VCMPNGE_USSD,
    instrux_VCMPNGE_USSS,
    instrux_VCMPNGTPD,
    instrux_VCMPNGTPS,
    instrux_VCMPNGTSD,
    instrux_VCMPNGTSS,
    instrux_VCMPNGT_UQPD,
    instrux_VCMPNGT_UQPS,
    instrux_VCMPNGT_UQSD,
    instrux_VCMPNGT_UQSS,
    instrux_VCMPNGT_USPD,
    instrux_VCMPNGT_USPS,
    instrux_VCMPNGT_USSD,
    instrux_VCMPNGT_USSS,
    instrux_VCMPNLEPD,
    instrux_VCMPNLEPS,
    instrux_VCMPNLESD,
    instrux_VCMPNLESS,
    instrux_VCMPNLE_UQPD,
    instrux_VCMPNLE_UQPS,
    instrux_VCMPNLE_UQSD,
    instrux_VCMPNLE_UQSS,
    instrux_VCMPNLE_USPD,
    instrux_VCMPNLE_USPS,
    instrux_VCMPNLE_USSD,
    instrux_VCMPNLE_USSS,
    instrux_VCMPNLTPD,
    instrux_VCMPNLTPS,
    instrux_VCMPNLTSD,
    instrux_VCMPNLTSS,
    instrux_VCMPNLT_UQPD,
    instrux_VCMPNLT_UQPS,
    instrux_VCMPNLT_UQSD,
    instrux_VCMPNLT_UQSS,
    instrux_VCMPNLT_USPD,
    instrux_VCMPNLT_USPS,
    instrux_VCMPNLT_USSD,
    instrux_VCMPNLT_USSS,
    instrux_VCMPORDPD,
    instrux_VCMPORDPS,
    instrux_VCMPORDSD,
    instrux_VCMPORDSS,
    instrux_VCMPORD_QPD,
    instrux_VCMPORD_QPS,
    instrux_VCMPORD_QSD,
    instrux_VCMPORD_QSS,
    instrux_VCMPORD_SPD,
    instrux_VCMPORD_SPS,
    instrux_VCMPORD_SSD,
    instrux_VCMPORD_SSS,
    instrux_VCMPPD,
    instrux_VCMPPS,
    instrux_VCMPSD,
    instrux_VCMPSS,
    instrux_VCMPTRUEPD,
    instrux_VCMPTRUEPS,
    instrux_VCMPTRUESD,
    instrux_VCMPTRUESS,
    instrux_VCMPTRUE_UQPD,
    instrux_VCMPTRUE_UQPS,
    instrux_VCMPTRUE_UQSD,
    instrux_VCMPTRUE_UQSS,
    instrux_VCMPTRUE_USPD,
    instrux_VCMPTRUE_USPS,
    instrux_VCMPTRUE_USSD,
    instrux_VCMPTRUE_USSS,
    instrux_VCMPUNORDPD,
    instrux_VCMPUNORDPS,
    instrux_VCMPUNORDSD,
    instrux_VCMPUNORDSS,
    instrux_VCMPUNORD_QPD,
    instrux_VCMPUNORD_QPS,
    instrux_VCMPUNORD_QSD,
    instrux_VCMPUNORD_QSS,
    instrux_VCMPUNORD_SPD,
    instrux_VCMPUNORD_SPS,
    instrux_VCMPUNORD_SSD,
    instrux_VCMPUNORD_SSS,
    instrux_VCOMISD,
    instrux_VCOMISS,
    instrux_VCVTDQ2PD,
    instrux_VCVTDQ2PS,
    instrux_VCVTPD2DQ,
    instrux_VCVTPD2PS,
    instrux_VCVTPH2PS,
    instrux_VCVTPS2DQ,
    instrux_VCVTPS2PD,
    instrux_VCVTPS2PH,
    instrux_VCVTSD2SI,
    instrux_VCVTSD2SS,
    instrux_VCVTSI2SD,
    instrux_VCVTSI2SS,
    instrux_VCVTSS2SD,
    instrux_VCVTSS2SI,
    instrux_VCVTTPD2DQ,
    instrux_VCVTTPS2DQ,
    instrux_VCVTTSD2SI,
    instrux_VCVTTSS2SI,
    instrux_VDIVPD,
    instrux_VDIVPS,
    instrux_VDIVSD,
    instrux_VDIVSS,
    instrux_VDPPD,
    instrux_VDPPS,
    instrux_VERR,
    instrux_VERW,
    instrux_VEXTRACTF128,
    instrux_VEXTRACTPS,
    instrux_VFMADD123PD,
    instrux_VFMADD123PS,
    instrux_VFMADD123SD,
    instrux_VFMADD123SS,
    instrux_VFMADD132PD,
    instrux_VFMADD132PS,
    instrux_VFMADD132SD,
    instrux_VFMADD132SS,
    instrux_VFMADD213PD,
    instrux_VFMADD213PS,
    instrux_VFMADD213SD,
    instrux_VFMADD213SS,
    instrux_VFMADD231PD,
    instrux_VFMADD231PS,
    instrux_VFMADD231SD,
    instrux_VFMADD231SS,
    instrux_VFMADD312PD,
    instrux_VFMADD312PS,
    instrux_VFMADD312SD,
    instrux_VFMADD312SS,
    instrux_VFMADD321PD,
    instrux_VFMADD321PS,
    instrux_VFMADD321SD,
    instrux_VFMADD321SS,
    instrux_VFMADDPD,
    instrux_VFMADDPS,
    instrux_VFMADDSD,
    instrux_VFMADDSS,
    instrux_VFMADDSUB123PD,
    instrux_VFMADDSUB123PS,
    instrux_VFMADDSUB132PD,
    instrux_VFMADDSUB132PS,
    instrux_VFMADDSUB213PD,
    instrux_VFMADDSUB213PS,
    instrux_VFMADDSUB231PD,
    instrux_VFMADDSUB231PS,
    instrux_VFMADDSUB312PD,
    instrux_VFMADDSUB312PS,
    instrux_VFMADDSUB321PD,
    instrux_VFMADDSUB321PS,
    instrux_VFMADDSUBPD,
    instrux_VFMADDSUBPS,
    instrux_VFMSUB123PD,
    instrux_VFMSUB123PS,
    instrux_VFMSUB123SD,
    instrux_VFMSUB123SS,
    instrux_VFMSUB132PD,
    instrux_VFMSUB132PS,
    instrux_VFMSUB132SD,
    instrux_VFMSUB132SS,
    instrux_VFMSUB213PD,
    instrux_VFMSUB213PS,
    instrux_VFMSUB213SD,
    instrux_VFMSUB213SS,
    instrux_VFMSUB231PD,
    instrux_VFMSUB231PS,
    instrux_VFMSUB231SD,
    instrux_VFMSUB231SS,
    instrux_VFMSUB312PD,
    instrux_VFMSUB312PS,
    instrux_VFMSUB312SD,
    instrux_VFMSUB312SS,
    instrux_VFMSUB321PD,
    instrux_VFMSUB321PS,
    instrux_VFMSUB321SD,
    instrux_VFMSUB321SS,
    instrux_VFMSUBADD123PD,
    instrux_VFMSUBADD123PS,
    instrux_VFMSUBADD132PD,
    instrux_VFMSUBADD132PS,
    instrux_VFMSUBADD213PD,
    instrux_VFMSUBADD213PS,
    instrux_VFMSUBADD231PD,
    instrux_VFMSUBADD231PS,
    instrux_VFMSUBADD312PD,
    instrux_VFMSUBADD312PS,
    instrux_VFMSUBADD321PD,
    instrux_VFMSUBADD321PS,
    instrux_VFMSUBADDPD,
    instrux_VFMSUBADDPS,
    instrux_VFMSUBPD,
    instrux_VFMSUBPS,
    instrux_VFMSUBSD,
    instrux_VFMSUBSS,
    instrux_VFNMADD123PD,
    instrux_VFNMADD123PS,
    instrux_VFNMADD123SD,
    instrux_VFNMADD123SS,
    instrux_VFNMADD132PD,
    instrux_VFNMADD132PS,
    instrux_VFNMADD132SD,
    instrux_VFNMADD132SS,
    instrux_VFNMADD213PD,
    instrux_VFNMADD213PS,
    instrux_VFNMADD213SD,
    instrux_VFNMADD213SS,
    instrux_VFNMADD231PD,
    instrux_VFNMADD231PS,
    instrux_VFNMADD231SD,
    instrux_VFNMADD231SS,
    instrux_VFNMADD312PD,
    instrux_VFNMADD312PS,
    instrux_VFNMADD312SD,
    instrux_VFNMADD312SS,
    instrux_VFNMADD321PD,
    instrux_VFNMADD321PS,
    instrux_VFNMADD321SD,
    instrux_VFNMADD321SS,
    instrux_VFNMADDPD,
    instrux_VFNMADDPS,
    instrux_VFNMADDSD,
    instrux_VFNMADDSS,
    instrux_VFNMSUB123PD,
    instrux_VFNMSUB123PS,
    instrux_VFNMSUB123SD,
    instrux_VFNMSUB123SS,
    instrux_VFNMSUB132PD,
    instrux_VFNMSUB132PS,
    instrux_VFNMSUB132SD,
    instrux_VFNMSUB132SS,
    instrux_VFNMSUB213PD,
    instrux_VFNMSUB213PS,
    instrux_VFNMSUB213SD,
    instrux_VFNMSUB213SS,
    instrux_VFNMSUB231PD,
    instrux_VFNMSUB231PS,
    instrux_VFNMSUB231SD,
    instrux_VFNMSUB231SS,
    instrux_VFNMSUB312PD,
    instrux_VFNMSUB312PS,
    instrux_VFNMSUB312SD,
    instrux_VFNMSUB312SS,
    instrux_VFNMSUB321PD,
    instrux_VFNMSUB321PS,
    instrux_VFNMSUB321SD,
    instrux_VFNMSUB321SS,
    instrux_VFNMSUBPD,
    instrux_VFNMSUBPS,
    instrux_VFNMSUBSD,
    instrux_VFNMSUBSS,
    instrux_VFRCZPD,
    instrux_VFRCZPS,
    instrux_VFRCZSD,
    instrux_VFRCZSS,
    instrux_VHADDPD,
    instrux_VHADDPS,
    instrux_VHSUBPD,
    instrux_VHSUBPS,
    instrux_VINSERTF128,
    instrux_VINSERTPS,
    instrux_VLDDQU,
    instrux_VLDMXCSR,
    instrux_VLDQQU,
    instrux_VMASKMOVDQU,
    instrux_VMASKMOVPD,
    instrux_VMASKMOVPS,
    instrux_VMAXPD,
    instrux_VMAXPS,
    instrux_VMAXSD,
    instrux_VMAXSS,
    instrux_VMCALL,
    instrux_VMCLEAR,
    instrux_VMINPD,
    instrux_VMINPS,
    instrux_VMINSD,
    instrux_VMINSS,
    instrux_VMLAUNCH,
    instrux_VMLOAD,
    instrux_VMMCALL,
    instrux_VMOVAPD,
    instrux_VMOVAPS,
    instrux_VMOVD,
    instrux_VMOVDDUP,
    instrux_VMOVDQA,
    instrux_VMOVDQU,
    instrux_VMOVHLPS,
    instrux_VMOVHPD,
    instrux_VMOVHPS,
    instrux_VMOVLHPS,
    instrux_VMOVLPD,
    instrux_VMOVLPS,
    instrux_VMOVMSKPD,
    instrux_VMOVMSKPS,
    instrux_VMOVNTDQ,
    instrux_VMOVNTDQA,
    instrux_VMOVNTPD,
    instrux_VMOVNTPS,
    instrux_VMOVNTQQ,
    instrux_VMOVQ,
    instrux_VMOVQQA,
    instrux_VMOVQQU,
    instrux_VMOVSD,
    instrux_VMOVSHDUP,
    instrux_VMOVSLDUP,
    instrux_VMOVSS,
    instrux_VMOVUPD,
    instrux_VMOVUPS,
    instrux_VMPSADBW,
    instrux_VMPTRLD,
    instrux_VMPTRST,
    instrux_VMREAD,
    instrux_VMRESUME,
    instrux_VMRUN,
    instrux_VMSAVE,
    instrux_VMULPD,
    instrux_VMULPS,
    instrux_VMULSD,
    instrux_VMULSS,
    instrux_VMWRITE,
    instrux_VMXOFF,
    instrux_VMXON,
    instrux_VORPD,
    instrux_VORPS,
    instrux_VPABSB,
    instrux_VPABSD,
    instrux_VPABSW,
    instrux_VPACKSSDW,
    instrux_VPACKSSWB,
    instrux_VPACKUSDW,
    instrux_VPACKUSWB,
    instrux_VPADDB,
    instrux_VPADDD,
    instrux_VPADDQ,
    instrux_VPADDSB,
    instrux_VPADDSW,
    instrux_VPADDUSB,
    instrux_VPADDUSW,
    instrux_VPADDW,
    instrux_VPALIGNR,
    instrux_VPAND,
    instrux_VPANDN,
    instrux_VPAVGB,
    instrux_VPAVGW,
    instrux_VPBLENDVB,
    instrux_VPBLENDW,
    instrux_VPCLMULHQHQDQ,
    instrux_VPCLMULHQLQDQ,
    instrux_VPCLMULLQHQDQ,
    instrux_VPCLMULLQLQDQ,
    instrux_VPCLMULQDQ,
    instrux_VPCMOV,
    instrux_VPCMPEQB,
    instrux_VPCMPEQD,
    instrux_VPCMPEQQ,
    instrux_VPCMPEQW,
    instrux_VPCMPESTRI,
    instrux_VPCMPESTRM,
    instrux_VPCMPGTB,
    instrux_VPCMPGTD,
    instrux_VPCMPGTQ,
    instrux_VPCMPGTW,
    instrux_VPCMPISTRI,
    instrux_VPCMPISTRM,
    instrux_VPCOMB,
    instrux_VPCOMD,
    instrux_VPCOMQ,
    instrux_VPCOMUB,
    instrux_VPCOMUD,
    instrux_VPCOMUQ,
    instrux_VPCOMUW,
    instrux_VPCOMW,
    instrux_VPERM2F128,
    instrux_VPERMILPD,
    instrux_VPERMILPS,
    instrux_VPEXTRB,
    instrux_VPEXTRD,
    instrux_VPEXTRQ,
    instrux_VPEXTRW,
    instrux_VPHADDBD,
    instrux_VPHADDBQ,
    instrux_VPHADDBW,
    instrux_VPHADDD,
    instrux_VPHADDDQ,
    instrux_VPHADDSW,
    instrux_VPHADDUBD,
    instrux_VPHADDUBQ,
    instrux_VPHADDUBW,
    instrux_VPHADDUDQ,
    instrux_VPHADDUWD,
    instrux_VPHADDUWQ,
    instrux_VPHADDW,
    instrux_VPHADDWD,
    instrux_VPHADDWQ,
    instrux_VPHMINPOSUW,
    instrux_VPHSUBBW,
    instrux_VPHSUBD,
    instrux_VPHSUBDQ,
    instrux_VPHSUBSW,
    instrux_VPHSUBW,
    instrux_VPHSUBWD,
    instrux_VPINSRB,
    instrux_VPINSRD,
    instrux_VPINSRQ,
    instrux_VPINSRW,
    instrux_VPMACSDD,
    instrux_VPMACSDQH,
    instrux_VPMACSDQL,
    instrux_VPMACSSDD,
    instrux_VPMACSSDQH,
    instrux_VPMACSSDQL,
    instrux_VPMACSSWD,
    instrux_VPMACSSWW,
    instrux_VPMACSWD,
    instrux_VPMACSWW,
    instrux_VPMADCSSWD,
    instrux_VPMADCSWD,
    instrux_VPMADDUBSW,
    instrux_VPMADDWD,
    instrux_VPMAXSB,
    instrux_VPMAXSD,
    instrux_VPMAXSW,
    instrux_VPMAXUB,
    instrux_VPMAXUD,
    instrux_VPMAXUW,
    instrux_VPMINSB,
    instrux_VPMINSD,
    instrux_VPMINSW,
    instrux_VPMINUB,
    instrux_VPMINUD,
    instrux_VPMINUW,
    instrux_VPMOVMSKB,
    instrux_VPMOVSXBD,
    instrux_VPMOVSXBQ,
    instrux_VPMOVSXBW,
    instrux_VPMOVSXDQ,
    instrux_VPMOVSXWD,
    instrux_VPMOVSXWQ,
    instrux_VPMOVZXBD,
    instrux_VPMOVZXBQ,
    instrux_VPMOVZXBW,
    instrux_VPMOVZXDQ,
    instrux_VPMOVZXWD,
    instrux_VPMOVZXWQ,
    instrux_VPMULDQ,
    instrux_VPMULHRSW,
    instrux_VPMULHUW,
    instrux_VPMULHW,
    instrux_VPMULLD,
    instrux_VPMULLW,
    instrux_VPMULUDQ,
    instrux_VPOR,
    instrux_VPPERM,
    instrux_VPROTB,
    instrux_VPROTD,
    instrux_VPROTQ,
    instrux_VPROTW,
    instrux_VPSADBW,
    instrux_VPSHAB,
    instrux_VPSHAD,
    instrux_VPSHAQ,
    instrux_VPSHAW,
    instrux_VPSHLB,
    instrux_VPSHLD,
    instrux_VPSHLQ,
    instrux_VPSHLW,
    instrux_VPSHUFB,
    instrux_VPSHUFD,
    instrux_VPSHUFHW,
    instrux_VPSHUFLW,
    instrux_VPSIGNB,
    instrux_VPSIGND,
    instrux_VPSIGNW,
    instrux_VPSLLD,
    instrux_VPSLLDQ,
    instrux_VPSLLQ,
    instrux_VPSLLW,
    instrux_VPSRAD,
    instrux_VPSRAW,
    instrux_VPSRLD,
    instrux_VPSRLDQ,
    instrux_VPSRLQ,
    instrux_VPSRLW,
    instrux_VPSUBB,
    instrux_VPSUBD,
    instrux_VPSUBQ,
    instrux_VPSUBSB,
    instrux_VPSUBSW,
    instrux_VPSUBUSB,
    instrux_VPSUBUSW,
    instrux_VPSUBW,
    instrux_VPTEST,
    instrux_VPUNPCKHBW,
    instrux_VPUNPCKHDQ,
    instrux_VPUNPCKHQDQ,
    instrux_VPUNPCKHWD,
    instrux_VPUNPCKLBW,
    instrux_VPUNPCKLDQ,
    instrux_VPUNPCKLQDQ,
    instrux_VPUNPCKLWD,
    instrux_VPXOR,
    instrux_VRCPPS,
    instrux_VRCPSS,
    instrux_VROUNDPD,
    instrux_VROUNDPS,
    instrux_VROUNDSD,
    instrux_VROUNDSS,
    instrux_VRSQRTPS,
    instrux_VRSQRTSS,
    instrux_VSHUFPD,
    instrux_VSHUFPS,
    instrux_VSQRTPD,
    instrux_VSQRTPS,
    instrux_VSQRTSD,
    instrux_VSQRTSS,
    instrux_VSTMXCSR,
    instrux_VSUBPD,
    instrux_VSUBPS,
    instrux_VSUBSD,
    instrux_VSUBSS,
    instrux_VTESTPD,
    instrux_VTESTPS,
    instrux_VUCOMISD,
    instrux_VUCOMISS,
    instrux_VUNPCKHPD,
    instrux_VUNPCKHPS,
    instrux_VUNPCKLPD,
    instrux_VUNPCKLPS,
    instrux_VXORPD,
    instrux_VXORPS,
    instrux_VZEROALL,
    instrux_VZEROUPPER,
    instrux_WBINVD,
    instrux_WRFSBASE,
    instrux_WRGSBASE,
    instrux_WRMSR,
    instrux_WRSHR,
    instrux_XADD,
    instrux_XBTS,
    instrux_XCHG,
    instrux_XCRYPTCBC,
    instrux_XCRYPTCFB,
    instrux_XCRYPTCTR,
    instrux_XCRYPTECB,
    instrux_XCRYPTOFB,
    instrux_XGETBV,
    instrux_XLAT,
    instrux_XLATB,
    instrux_XOR,
    instrux_XORPD,
    instrux_XORPS,
    instrux_XRSTOR,
    instrux_XRSTOR64,
    instrux_XSAVE,
    instrux_XSAVE64,
    instrux_XSAVEOPT,
    instrux_XSAVEOPT64,
    instrux_XSETBV,
    instrux_XSHA1,
    instrux_XSHA256,
    instrux_XSTORE,
    instrux_CMOVcc,
    instrux_Jcc,
    instrux_SETcc,
};
