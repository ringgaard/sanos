//
// mach.h
//
// Interface to physical/virtual machine
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

#ifndef MACH_H
#define MACH_H

//
// Machine interface
//

struct mach {
  int kring;

  void (*sti)();
  void (*cli)();
  void (*hlt)();
  void (*iretd)();
  void (*sysret)();
  int (__fastcall *in)(port_t port);
  unsigned short (__fastcall *inw)(port_t port);
  unsigned long (__fastcall *ind)(port_t port);
  void (*insw)(port_t port, void *buf, int count);
  void (*insd)(port_t port, void *buf, int count);
  int (__fastcall *out)(port_t port, int val);
  unsigned short (__fastcall *outw)(port_t port, unsigned short val);
  unsigned long (__fastcall *outd)(port_t port, unsigned long val);
  void (*outsw)(port_t port, void *buf, int count);
  void (*outsd)(port_t port, void *buf, int count);
  void (*cpuid)(unsigned long reg, unsigned long values[4]);
  unsigned long (*get_cr0)();
  void (*set_cr0)(unsigned long val);
  unsigned long (*get_cr2)();
  unsigned __int64 (*rdtsc)();
  void (*wrmsr)(unsigned long reg, unsigned long valuelow, unsigned long valuehigh);
  void (*set_gdt_entry)(int entry, unsigned long addr, unsigned long size, int access, int granularity);
  void (*set_idt_gate)(int intrno, void *handler);
  void (*set_idt_trap)(int intrno, void *handler);
  void (*switch_kernel_stack)();
  void (*flushtlb)();
  void (*invlpage)(void *addr);
  void (*register_page_dir)(unsigned long pfn);
  void (*register_page_table)(unsigned long pfn);
  void (*set_page_dir_entry)(pte_t *pde, unsigned long value);
  void (*set_page_table_entry)(pte_t *pte, unsigned long value);
  void (*poweroff)();
  void (*reboot)();
};

extern struct mach mach;

#ifdef VMACH

//
// Interface functions for virtual machine layer
//

void __inline sti() { 
  mach.sti(); 
}

void __inline cli() {
  mach.cli(); 
}

void __inline halt() {
  mach.hlt(); 
}

void __inline insw(port_t port, void *buf, int count) {
  mach.insw(port, buf, count); 
}

void __inline insd(port_t port, void *buf, int count) {
  mach.insd(port, buf, count); 
}

void __inline outsw(port_t port, void *buf, int count) {
  mach.outsw(port, buf, count); 
}

void __inline outsd(port_t port, void *buf, int count) {
  mach.outsd(port, buf, count); 
}

unsigned long __inline get_cr0() {
  return mach.get_cr0();
}

void __inline set_cr0(unsigned long val) {
  mach.set_cr0(val);
}

unsigned long __inline get_cr2() {
  return mach.get_cr2();
}

unsigned __int64 __inline rdtsc() {
  return mach.rdtsc();
}

void __inline wrmsr(unsigned long reg, unsigned long valuelow, unsigned long valuehigh) {
  mach.wrmsr(reg, valuelow, valuehigh);
}

void __inline switch_kernel_stack() {
  mach.switch_kernel_stack();
}

void __inline register_page_dir(unsigned long pfn) {
  mach.register_page_dir(pfn);
}

void __inline register_page_table(unsigned long pfn) {
  mach.register_page_table(pfn);
}

void __inline set_page_dir_entry(pte_t *pde, unsigned long value) {
  mach.set_page_dir_entry(pde, value);
}

void __inline set_page_table_entry(pte_t *pte, unsigned long value) {
  mach.set_page_table_entry(pte, value);
}

#define inp(port)  (mach.in(port))
#define inpw(port) (mach.inw(port))
#define inpd(port) (mach.ind(port))

#define outp(port, val)  (mach.out((port), (val)))
#define outpw(port, val) (mach.outw((port), (val)))
#define outpd(port, val) (mach.outd((port), (val)))

#define CLI call cs:mach.cli
#define STI call cs:mach.sti
#define IRETD call cs:mach.iretd
#define SYSEXIT call cs:mach.sysret

#else

//
// Interface functions for direct hardware access
//

void __inline sti() {
  __asm sti
}

void __inline cli() {
  __asm cli
}

void __inline halt() {
  __asm hlt
}

unsigned long __inline get_cr0() {
  unsigned long val;

  __asm {
    mov eax, cr0
    mov val, eax
  }

  return val;
}

void __inline set_cr0(unsigned long val) {
  __asm {
    mov eax, val
    mov cr0, eax
  }
}

unsigned long __inline get_cr2() {
  unsigned long val;

  __asm {
    mov eax, cr2
    mov val, eax
  }

  return val;
}

__declspec(naked) unsigned __int64 __inline rdtsc() {
  __asm {
    rdtsc
    ret 
  }
}

void __inline wrmsr(unsigned long reg, unsigned long valuelow, unsigned long valuehigh) {
  __asm {
    mov ecx, reg
    mov eax, valuelow
    mov edx, valuehigh
    wrmsr
  }
}

void __inline register_page_dir(unsigned long pfn) {
  // Do nothing
}

void __inline register_page_table(unsigned long pfn) {
  // Do nothing
}

void __inline set_page_dir_entry(pte_t *pde, unsigned long value) {
  *pde = value;
}

void __inline set_page_table_entry(pte_t *pte, unsigned long value) {
  *pte = value;
}

#define inp(port)  (inb(port))
#define inpw(port) (inw(port))
#define inpd(port) (ind(port))

#define outp(port, val)  (outb((port), (val)))
#define outpw(port, val) (outw((port), (val)))
#define outpd(port, val) (outd((port), (val)))

krnlapi unsigned char inb(port_t port);
krnlapi unsigned short inpw(port_t port);
krnlapi unsigned long inpd(port_t port);

krnlapi void insw(port_t port, void *buf, int count);
krnlapi void insd(port_t port, void *buf, int count);

krnlapi unsigned char outb(port_t port, unsigned char val);
krnlapi unsigned short outw(port_t port, unsigned short val);
krnlapi unsigned long outd(port_t port, unsigned long val);

krnlapi void outsw(port_t port, void *buf, int count);
krnlapi void outsd(port_t port, void *buf, int count);

#define CLI cli
#define STI sti
#define IRETD iretd
#define SYSEXIT sysexit

#endif

//
// Common interface functions
//

void __inline cpuid(unsigned long reg, unsigned long values[4]) {
  mach.cpuid(reg, values); 
}

void __inline set_gdt_entry(int entry, unsigned long addr, unsigned long size, int access, int granularity) {
  mach.set_gdt_entry(entry, addr, size, access, granularity);
}

void __inline set_idt_gate(int intrno, void *handler) {
  mach.set_idt_gate(intrno, handler);
}

void __inline set_idt_trap(int intrno, void *handler) {
  mach.set_idt_trap(intrno, handler);
}

void __inline flushtlb() {
  mach.flushtlb();
}

void __inline invlpage(void *addr) {
  mach.invlpage(addr);
}

void __inline poweroff() {
  mach.poweroff();
}

void __inline reboot() {
  mach.reboot();
}

void init_mach();

#endif
