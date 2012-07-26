//
// vmi.h
//
// VMWare Virtual Machine Interface (VMI)
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Copyright (C) 2005 VMWare, Inc.
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

#ifndef VMI_H
#define VMI_H

//
// VMI call numbers
//

#define VMI_CALL_RESERVED0                 0
#define VMI_CALL_RESERVED1                 1
#define VMI_CALL_RESERVED2                 2
#define VMI_CALL_RESERVED3                 3
#define VMI_CALL_Init                      4
#define VMI_CALL_CPUID                     5
#define VMI_CALL_WRMSR                     6
#define VMI_CALL_RDMSR                     7
#define VMI_CALL_SetGDT                    8
#define VMI_CALL_SetLDT                    9
#define VMI_CALL_SetIDT                    10
#define VMI_CALL_SetTR                     11
#define VMI_CALL_GetGDT                    12
#define VMI_CALL_GetLDT                    13
#define VMI_CALL_GetIDT                    14
#define VMI_CALL_GetTR                     15
#define VMI_CALL_WriteGDTEntry             16
#define VMI_CALL_WriteLDTEntry             17
#define VMI_CALL_WriteIDTEntry             18
#define VMI_CALL_UpdateKernelStack         19
#define VMI_CALL_SetCR0                    20
#define VMI_CALL_SetCR2                    21
#define VMI_CALL_SetCR3                    22
#define VMI_CALL_SetCR4                    23
#define VMI_CALL_GetCR0                    24
#define VMI_CALL_GetCR2                    25
#define VMI_CALL_GetCR3                    26
#define VMI_CALL_GetCR4                    27
#define VMI_CALL_INVD                      28
#define VMI_CALL_WBINVD                    29
#define VMI_CALL_SetDR                     30
#define VMI_CALL_GetDR                     31
#define VMI_CALL_RDPMC                     32
#define VMI_CALL_RDTSC                     33
#define VMI_CALL_CLTS                      34
#define VMI_CALL_EnableInterrupts          35
#define VMI_CALL_DisableInterrupts         36
#define VMI_CALL_GetInterruptMask          37
#define VMI_CALL_SetInterruptMask          38
#define VMI_CALL_IRET                      39
#define VMI_CALL_SYSEXIT                   40
#define VMI_CALL_Pause                     41
#define VMI_CALL_Halt                      42
#define VMI_CALL_Reboot                    43
#define VMI_CALL_Shutdown                  44
#define VMI_CALL_SetPxE0                   45
#define VMI_CALL_GetPxE                    46
#define VMI_CALL_SwapPxE                   47
#define VMI_CALL_SetPxELong0               48
#define VMI_CALL_GetPxELong                49
#define VMI_CALL_SwapPxELongAtomic         50
#define VMI_CALL_TestAndSetPxEBit          51
#define VMI_CALL_TestAndClearPxEBit        52
#define VMI_CALL_AllocatePage              53
#define VMI_CALL_ReleasePage               54
#define VMI_CALL_InvalPage                 55
#define VMI_CALL_FlushTLB                  56
#define VMI_CALL_FlushDeferredCalls        57
#define VMI_CALL_SetLinearMapping          58
#define VMI_CALL_IN                        59
#define VMI_CALL_INB                       60
#define VMI_CALL_INW                       61
#define VMI_CALL_INS                       62
#define VMI_CALL_INSB                      63
#define VMI_CALL_INSW                      64
#define VMI_CALL_OUT                       65
#define VMI_CALL_OUTB                      66
#define VMI_CALL_OUTW                      67
#define VMI_CALL_OUTS                      68
#define VMI_CALL_OUTSB                     69
#define VMI_CALL_OUTSW                     70
#define VMI_CALL_SetIOPLMask               71
#define VMI_CALL_DeactivatePxELongAtomic   72
#define VMI_CALL_TestAndSetPxELongBit      73
#define VMI_CALL_TestAndClearPxELongBit    74
#define VMI_CALL_SetInitialAPState         75
#define VMI_CALL_APICWrite                 76
#define VMI_CALL_APICRead                  77
#define VMI_CALL_IODelay                   78
#define VMI_CALL_GetCycleFrequency         79
#define VMI_CALL_GetCycleCounter           80
#define VMI_CALL_SetAlarm                  81
#define VMI_CALL_CancelAlarm               82
#define VMI_CALL_GetWallclockTime          83
#define VMI_CALL_WallclockUpdated          84
#define VMI_CALL_GetRelocationInfo         85
#define VMI_CALL_SetPxE                    86
#define VMI_CALL_SetPxELong                87

#define NUM_VMI_CALLS                      88

//
// VMI Option ROM API
// 

#define VMI_SIGNATURE 0x696d5663   // "cVmi"

#define VMI_API_REV_MAJOR       13
#define VMI_API_REV_MINOR       3

// VMI Relocation types
#define VMI_RELOCATION_NONE     0
#define VMI_RELOCATION_CALL_REL 1
#define VMI_RELOCATION_JUMP_REL 2

struct vmi_relocation_info {
  unsigned long eip;
  unsigned char type;   
  unsigned char reserved[3];
};

// Flags used by VMI_Reboot call
#define VMI_REBOOT_SOFT          0x0
#define VMI_REBOOT_HARD          0x1

// Flags used by MMU calls
#define VMI_PAGE_PT              0x01
#define VMI_PAGE_PD              0x02
#define VMI_PAGE_PAE             0x04
#define VMI_PAGE_PDP             0x04
#define VMI_PAGE_PML4            0x08

// VMI_PAGE_CURRENT_AS implies VMI_PAGE_VA_MASK field is valid
#define VMI_PAGE_CURRENT_AS      0x10
#define VMI_PAGE_DEFER           0x20
#define VMI_PAGE_VA_MASK         0xfffff000

// Flags used by VMI_FlushTLB call
#define VMI_FLUSH_TLB            0x01
#define VMI_FLUSH_GLOBAL         0x02

// Flags used by VMI_FlushSync call
#define VMI_FLUSH_PT_UPDATES     0x80
#define VMI_FLUSH_CPU_STATE      0x40

// The number of VMI address translation slot
#define VMI_LINEAR_MAP_SLOTS    4

// The cycle counters
#define VMI_CYCLES_REAL         0
#define VMI_CYCLES_AVAILABLE    1
#define VMI_CYCLES_STOLEN       2

// The alarm interface 'flags' bits
#define VMI_ALARM_COUNTERS      2

#define VMI_ALARM_COUNTER_MASK  0x000000ff

#define VMI_ALARM_WIRED_IRQ0    0x00000000
#define VMI_ALARM_WIRED_LVTT    0x00010000

#define VMI_ALARM_IS_ONESHOT    0x00000000
#define VMI_ALARM_IS_PERIODIC   0x00000100

//
// VROM call table definitions
//

#pragma pack(push, 1)

#define VROM_CALL_LEN 32

struct vrom_call_slot {
  char f[VROM_CALL_LEN];
};

struct vrom_header {
  unsigned short romSignature;    // option ROM signature
  unsigned char romLength;        // ROM length in 512 byte chunks
  unsigned char romEntry[4];      // 16-bit code entry point
  unsigned char romPad0;          // 4-byte align pad
  unsigned long vRomSignature;    // VROM identification signature
  unsigned char APIVersionMinor;  // Minor version of API
  unsigned char APIVersionMajor;  // Major version of API
  unsigned char reserved0;        // Reserved for expansion
  unsigned char reserved1;        // Reserved for expansion
  unsigned long reserved2;        // Reserved for expansion
  unsigned long reserved3;        // Reserved for private use
  unsigned short pciHeaderOffset; // Offset to PCI OPROM header
  unsigned short pnpHeaderOffset; // Offset to PnP OPROM header
  unsigned long romPad3;          // PnP reserverd / VMI reserved
};

//
// State needed to start an application processor in an SMP system
//

struct vmi_ap_state {
  unsigned long cr0;
  unsigned long cr2;
  unsigned long cr3;
  unsigned long cr4;

  unsigned __int64 efer;

  unsigned long eip;
  unsigned long eflags;
  unsigned long eax;
  unsigned long ebx;
  unsigned long ecx;
  unsigned long edx;
  unsigned long esp;
  unsigned long ebp;
  unsigned long esi;
  unsigned long edi;
  unsigned short cs;
  unsigned short ss;
  unsigned short ds;
  unsigned short es;
  unsigned short fs;
  unsigned short gs;
  unsigned short ldtr;

  unsigned short gdtr_limit;
  unsigned long gdtr_base;
  unsigned long idtr_base;
  unsigned short idtr_limit;
};

#pragma pack(pop)

#endif
