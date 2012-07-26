//
// mach.c
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

#include <os/krnl.h>

int probe_vmi(); // from vmi.c
int init_vmi(); // from vmi.c

static void hw_sti() {
  __asm sti;
}

static void hw_cli() {
  __asm cli;
}

static void hw_hlt() {
  __asm hlt;
}

static void hw_iretd() {
  __asm add esp,4;
  __asm iretd;
}

static void hw_sysret() {
  __asm add esp,4;
  __asm sysexit;
}

static __declspec(naked) int __fastcall hw_in(port_t port) {
  __asm {
    mov     dx,cx
    xor     eax,eax
    in      al,dx
    ret
  }
}

static __declspec(naked) unsigned short __fastcall hw_inw(port_t port) {
  __asm {
    mov     dx,cx
    in      ax,dx
    ret
  }
}

static __declspec(naked) unsigned long __fastcall hw_ind(port_t port) {
  __asm {
    mov     dx,cx
    in      eax,dx
    ret
  }
}

static void hw_insw(port_t port, void *buf, int count) {
  __asm {
    mov edx, port
    mov edi, buf
    mov ecx, count
    rep insw
  }
}

static void hw_insd(port_t port, void *buf, int count) {
  __asm {
    mov edx, port
    mov edi, buf
    mov ecx, count
    rep insd
  }
}

static __declspec(naked) int __fastcall hw_out(port_t port, int val) {
  __asm {
    mov     al,dl
    mov     dx,cx
    out     dx, al
    ret
  }
}

static __declspec(naked) unsigned short __fastcall hw_outw(port_t port, unsigned short val) {
  __asm {
    mov     ax,dx
    mov     dx,cx
    out     dx, ax
    ret
  }
}

static __declspec(naked) unsigned long __fastcall hw_outd(port_t port, unsigned long val) {
  __asm {
    mov     eax,edx
    mov     dx,cx
    out     dx, eax
    ret
  }
}

static void hw_outsw(port_t port, void *buf, int count) {
  __asm {
    mov edx, port
    mov esi, buf
    mov ecx, count
    rep outsw
  }
}

static void hw_outsd(port_t port, void *buf, int count) {
  __asm {
    mov edx, port
    mov esi, buf
    mov ecx, count
    rep outsd
  }
}

static void hw_cpuid(unsigned long reg, unsigned long values[4]) {
  __asm {
    mov    eax, reg
    cpuid
    mov    esi, values
    mov    [esi], eax
    mov    [esi+4], ebx
    mov    [esi+8], ecx
    mov    [esi+12], edx
  }
}

static unsigned long hw_get_cr0() {
  unsigned long val;

  __asm  {
    mov eax, cr0
    mov val, eax
  }

  return val;
}

static void hw_set_cr0(unsigned long val) {
  __asm {
    mov eax, val
    mov cr0, eax
  }
}

static unsigned long hw_get_cr2() {
  unsigned long val;

  __asm  {
    mov eax, cr2
    mov val, eax
  }

  return val;
}

static __declspec(naked) unsigned __int64 hw_rdtsc() {
  __asm  { 
    rdtsc
    ret 
  }
}

static void hw_wrmsr(unsigned long reg, unsigned long valuelow, unsigned long valuehigh) {
  __asm {
    mov ecx, reg
    mov eax, valuelow
    mov edx, valuehigh
    wrmsr
  }
}

static void hw_set_gdt_entry(int entry, unsigned long addr, unsigned long size, int access, int granularity) {
  seginit(&syspage->gdt[entry], addr, size, access, granularity);
}

static void hw_set_idt_gate(int intrno, void *handler) {
  syspage->idt[intrno].offset_low = (unsigned short) (((unsigned long) handler) & 0xFFFF);
  syspage->idt[intrno].selector = SEL_KTEXT | mach.kring;
  syspage->idt[intrno].access = D_PRESENT | D_INT | D_DPL0;
  syspage->idt[intrno].offset_high = (unsigned short) (((unsigned long) handler) >> 16);
}

static void hw_set_idt_trap(int intrno, void *handler) {
  syspage->idt[intrno].offset_low = (unsigned short) (((unsigned long) handler) & 0xFFFF);
  syspage->idt[intrno].selector = SEL_KTEXT | mach.kring;
  syspage->idt[intrno].access = D_PRESENT | D_TRAP | D_DPL3;
  syspage->idt[intrno].offset_high = (unsigned short) (((unsigned long) handler) >> 16);
}

static void hw_switch_kernel_stack() {
}

static void hw_flushtlb() {
  __asm { mov eax, cr3 }
  __asm { mov cr3, eax }
}

static void hw_invlpage(void *addr) {
  if (cpu.family < CPU_FAMILY_486) {
    __asm { mov eax, cr3 }
    __asm { mov cr3, eax }
  } else   {
    __asm { mov eax, addr }
    __asm { invlpg [eax] }
  }
}

static void hw_register_page_dir(unsigned long pfn) {
  // Do nothing
}

static void hw_register_page_table(unsigned long pfn) {
  // Do nothing
}

static void hw_set_page_dir_entry(pte_t *pde, unsigned long value) {
  *pde = value;
}

static void hw_set_page_table_entry(pte_t *pte, unsigned long value) {
  *pte = value;
}

static void hw_poweroff() {
  if (apm_enabled) {
    apm_power_off();
  } else {
    kprintf("kernel: power management not enabled, system stopped...\n");
  }

  while (1) {
    __asm {
      cli
      hlt
    }
  }
}

static void hw_reboot() {
  kbd_reboot();
}

struct mach mach = {
  0, // kernel ring

  hw_sti,
  hw_cli,
  hw_hlt,
  hw_iretd,
  hw_sysret,
  hw_in,
  hw_inw,
  hw_ind,
  hw_insw,
  hw_insd,
  hw_out,
  hw_outw,
  hw_outd,
  hw_outsw,
  hw_outsd,
  hw_cpuid,
  hw_get_cr0,
  hw_set_cr0,
  hw_get_cr2,
  hw_rdtsc,
  hw_wrmsr,
  hw_set_gdt_entry,
  hw_set_idt_gate,
  hw_set_idt_trap,
  hw_switch_kernel_stack,
  hw_flushtlb,
  hw_invlpage,
  hw_register_page_dir,
  hw_register_page_table,
  hw_set_page_dir_entry,
  hw_set_page_table_entry,
  hw_poweroff,
  hw_reboot
};

#ifdef VMACH

void init_mach() {
  kprintf("mach: running in machine virtualization mode\n");
  if (probe_vmi()) init_vmi();
}

#else

void init_mach() {
}

#endif
