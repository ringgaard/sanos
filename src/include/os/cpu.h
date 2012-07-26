//
// cpu.h
//
// CPU information
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
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

#ifndef CPU_H
#define CPU_H

//
// ASM instructions
//

#define sysenter __asm _emit 0x0F __asm _emit 0x34
#define sysexit  __asm _emit 0x0F __asm _emit 0x35

//
// x86 EFLAGS
//

#define EFLAG_CF        0x00000001          // Carry
#define EFLAG_PF        0x00000004          // Parity
#define EFLAG_AF        0x00000010          // BCD stuff
#define EFLAG_ZF        0x00000040          // Zero
#define EFLAG_SF        0x00000080          // Sign
#define EFLAG_TF        0x00000100          // Single step
#define EFLAG_IF        0x00000200          // Interrupts 
#define EFLAG_DF        0x00000400          // Direction
#define EFLAG_OF        0x00000800          // Overflow
#define EFLAG_IOPL      0x00003000          // I/O privilege level
#define EFLAG_NT        0x00004000          // Nested task
#define EFLAG_RF        0x00010000          // Resume flag
#define EFLAG_VM        0x00020000          // Virtual 8086
#define EFLAGS_AC       0x00040000          // Alignment Check
#define EFLAGS_VIF      0x00080000          // Virtual Interrupt Flag
#define EFLAGS_VIP      0x00100000          // Virtual Interrupt Pending
#define EFLAGS_ID       0x00200000          // CPUID detection flag

//
// x86 CR0 flags
//

#define CR0_PE          (1 << 0)            // Protection enable
#define CR0_MP          (1 << 1)            // Math processor present
#define CR0_EM          (1 << 2)            // Emulate FP - trap on FP instruction
#define CR0_TS          (1 << 3)            // Task switched flag
#define CR0_ET          (1 << 4)            // Extension type - 387 DX presence
#define CR0_NE          (1 << 5)            // Numeric Error - allow traps on numeric errors
#define CR0_WP          (1 << 16)           // Write protect - ring 0 honors RO PTE's
#define CR0_AM          (1 << 18)           // Alignment - trap on unaligned refs
#define CR0_NW          (1 << 29)           // Not write-through - inhibit write-through
#define CR0_CD          (1 << 30)           // Cache disable
#define CR0_PG          (1 << 31)           // Paging - use PTEs/CR3

//
// x86 CR4 feature flags
//
#define CR4_VME         0x0001              // Enable vm86 extensions
#define CR4_PVI         0x0002              // Virtual interrupts flag enable
#define CR4_TSD         0x0004              // Disable time stamp at ipl 3
#define CR4_DE          0x0008              // Enable debugging extensions
#define CR4_PSE         0x0010              // Enable page size extensions
#define CR4_PAE         0x0020              // Enable physical address extensions
#define CR4_MCE         0x0040              // Machine check enable
#define CR4_PGE         0x0080              // Enable global pages
#define CR4_PCE         0x0100              // Enable performance counters at ipl 3
#define CR4_OSFXSR      0x0200              // Enable fast FPU save and restore
#define CR4_OSXMMEXCPT  0x0400              // Enable unmasked SSE exceptions

//
// CPU feature flags (CPUID level 0x00000001, edx)
//

#define CPU_FEATURE_FPU         (1 << 0)    // Onboard FPU
#define CPU_FEATURE_VME         (1 << 1)    // Virtual Mode Extensions
#define CPU_FEATURE_DE          (1 << 2)    // Debugging Extensions
#define CPU_FEATURE_PSE         (1 << 3)    // Page Size Extensions
#define CPU_FEATURE_TSC         (1 << 4)    // Time Stamp Counter
#define CPU_FEATURE_MSR         (1 << 5)    // Model-Specific Registers, RDMSR, WRMSR
#define CPU_FEATURE_PAE         (1 << 6)    // Physical Address Extensions
#define CPU_FEATURE_MCE         (1 << 7)    // Machine Check Architecture
#define CPU_FEATURE_CX8         (1 << 8)    // CMPXCHG8 instruction
#define CPU_FEATURE_APIC        (1 << 9)    // Onboard APIC
#define CPU_FEATURE_SEP         (1 << 11)   // SYSENTER/SYSEXIT
#define CPU_FEATURE_MTRR        (1 << 12)   // Memory Type Range Registers
#define CPU_FEATURE_PGE         (1 << 13)   // Page Global Enable
#define CPU_FEATURE_MCA         (1 << 14)   // Machine Check Architecture
#define CPU_FEATURE_CMOV        (1 << 15)   // CMOV instruction
#define CPU_FEATURE_PAT         (1 << 16)   // Page Attribute Table
#define CPU_FEATURE_PSE36       (1 << 17)   // 36-bit PSEs
#define CPU_FEATURE_PN          (1 << 18)   // Processor serial number
#define CPU_FEATURE_CLFLSH      (1 << 19)   // Supports the CLFLUSH instruction
#define CPU_FEATURE_DTES        (1 << 21)   // Debug Trace Store
#define CPU_FEATURE_ACPI        (1 << 22)   // ACPI via MSR
#define CPU_FEATURE_MMX         (1 << 23)   // Multimedia Extensions
#define CPU_FEATURE_FXSR        (1 << 24)   // FXSAVE and FXRSTOR instructions
#define CPU_FEATURE_XMM         (1 << 25)   // Streaming SIMD Extensions
#define CPU_FEATURE_XMM2        (1 << 26)   // Streaming SIMD Extensions-2
#define CPU_FEATURE_SELFSNOOP   (1 << 27)   // CPU self snoop
#define CPU_FEATURE_ACC         (1 << 29)   // Automatic clock control
#define CPU_FEATURE_IA64        (1 << 30)   // IA-64 processor

//
// Model Specific Registers
//

#define MSR_SYSENTER_CS         0x174       // CS register target for CPL 0 code
#define MSR_SYSENTER_ESP        0x175       // Stack pointer for CPL 0 code
#define MSR_SYSENTER_EIP        0x176       // CPL 0 code entry point

//
// CPU vendors
//

#define CPU_VENDOR_UNKNOWN   0
#define CPU_VENDOR_INTEL     1
#define CPU_VENDOR_CYRIX     2
#define CPU_VENDOR_AMD       3
#define CPU_VENDOR_UMC       4
#define CPU_VENDOR_NEXGEN    5
#define CPU_VENDOR_CENTAUR   6
#define CPU_VENDOR_RISE      7
#define CPU_VENDOR_TRANSMETA 8

//
// CPU family
//

#define CPU_FAMILY_386       3
#define CPU_FAMILY_486       4
#define CPU_FAMILY_P5        5
#define CPU_FAMILY_P6        6

//
// CPU information
//

struct cpu {
  int family;
  int vendor;
  int model;
  int stepping;
  int mhz;
  unsigned long features;
  unsigned long cpuid_level;
  char vendorid[16];
  char modelid[64];
};

#ifdef KERNEL

extern struct cpu cpu;

void init_cpu();
int cpu_sysinfo(struct cpuinfo *info);
unsigned long eflags();

#endif

#endif
