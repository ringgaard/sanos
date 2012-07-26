//
// vmi.c
//
// VMWare Virtual Machine Interface (VMI)
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
#include <os/vmi.h>

#ifdef VMACH

#define SET_MACH_FUNC(func, addr) *((unsigned long *) &mach.func) = addr

static struct vrom_header *vmi = NULL;
static unsigned long vmicall[NUM_VMI_CALLS];

#define VMCALL(func) call vmicall[func*4]
#define VMJUMP(func) jmp vmicall[func*4]

//
// VMI machine functions
//

static __declspec(naked) int __fastcall vmi_in(port_t port) {
  __asm {
    mov edx, ecx
    xor eax, eax
    VMJUMP(VMI_CALL_INB)
  }
}

static __declspec(naked) unsigned short __fastcall vmi_inw(port_t port) {
  __asm {
    mov edx, ecx
    VMJUMP(VMI_CALL_INW)
  }
}

static __declspec(naked) unsigned long __fastcall vmi_ind(port_t port) {
  __asm {
    mov edx, ecx
    VMJUMP(VMI_CALL_IN)
  }
}

static void vmi_insw(port_t port, void *buf, int count) {
  __asm {
    mov edx, port
    mov edi, buf
    mov ecx, count
    VMCALL(VMI_CALL_INSW);
  }
}

static void vmi_insd(port_t port, void *buf, int count) {
  __asm {
    mov edx, port
    mov edi, buf
    mov ecx, count
    VMCALL(VMI_CALL_INS)
  }
}

static __declspec(naked) int __fastcall vmi_out(port_t port, int val) {
  __asm {
    mov eax,edx
    mov edx,ecx
    VMJUMP(VMI_CALL_OUTB)
  }
}

static __declspec(naked) unsigned short __fastcall vmi_outw(port_t port, unsigned short val) {
  __asm {
    mov eax,edx
    mov edx,ecx
    VMJUMP(VMI_CALL_OUTW)
  }
}

static __declspec(naked) unsigned long __fastcall vmi_outd(port_t port, unsigned long val) {
  __asm {
    mov eax,edx
    mov edx,ecx
    VMJUMP(VMI_CALL_OUT)
  }
}

static void vmi_outsw(port_t port, void *buf, int count) {
  __asm {
    mov edx, port
    mov esi, buf
    mov ecx, count
    VMCALL(VMI_CALL_OUTSW)
  }
}

static void vmi_outsd(port_t port, void *buf, int count) {
  __asm {
    mov edx, port
    mov esi, buf
    mov ecx, count
    VMCALL(VMI_CALL_OUTS)
  }
}

static void vmi_cpuid(unsigned long reg, unsigned long values[4]) {
  __asm {
    mov eax, reg
    VMCALL(VMI_CALL_CPUID)
    mov esi, values
    mov [esi], eax
    mov [esi+4], ebx
    mov [esi+8], ecx
    mov [esi+12], edx
  }
}

static unsigned long vmi_get_cr0() {
  unsigned long val;

  __asm  {
    VMCALL(VMI_CALL_GetCR0);
    mov val, eax
  }

  return val;
}

static void vmi_set_cr0(unsigned long val) {
  __asm {
    mov eax, val
    VMCALL(VMI_CALL_SetCR0);
  }
}

static unsigned long vmi_get_cr2() {
  unsigned long val;

  __asm {
    VMCALL(VMI_CALL_GetCR2);
    mov val, eax
  }

  return val;
}

static void vmi_wrmsr(unsigned long reg, unsigned long valuelow, unsigned long valuehigh) {
  __asm {
    mov ecx, reg
    mov eax, valuelow
    mov edx, valuehigh
    VMCALL(VMI_CALL_WRMSR);
  }
}

static void vmi_set_gdt_entry(int entry, unsigned long addr, unsigned long size, int access, int granularity) {
  union dte dte;

  seginit(&dte.segment, addr, size, access, granularity);

  __asm {
    mov eax, offset syspage.gdt
    mov edx, entry
    mov ecx, dte.desc.low
    push dte.desc.high
    VMCALL(VMI_CALL_WriteGDTEntry)
    add esp, 4
  }
}

static void vmi_set_idt_gate(int intrno, void *handler) {
  union dte dte;

  dte.gate.offset_low = (unsigned short) (((unsigned long) handler) & 0xFFFF);
  dte.gate.selector = SEL_KTEXT | mach.kring;
  dte.gate.access = D_PRESENT | D_INT | D_DPL0;
  dte.gate.offset_high = (unsigned short) (((unsigned long) handler) >> 16);

  __asm {
    mov eax, offset syspage.idt
    mov edx, intrno
    mov ecx, dte.desc.low
    push dte.desc.high
    VMCALL(VMI_CALL_WriteIDTEntry)
    add esp, 4
  }
}

static void vmi_set_idt_trap(int intrno, void *handler) {
  union dte dte;

  dte.gate.offset_low = (unsigned short) (((unsigned long) handler) & 0xFFFF);
  dte.gate.selector = SEL_KTEXT | mach.kring;
  dte.gate.access = D_PRESENT | D_TRAP | D_DPL3;
  dte.gate.offset_high = (unsigned short) (((unsigned long) handler) >> 16);

  __asm {
    mov eax, offset syspage.idt
    mov edx, intrno
    mov ecx, dte.desc.low
    push dte.desc.high
    VMCALL(VMI_CALL_WriteIDTEntry)
    add esp, 4
  }
}

static void vmi_switch_kernel_stack() {
  __asm {
    mov eax, offset syspage.tss
    mov edx, syspage.tss.esp0
    VMCALL(VMI_CALL_UpdateKernelStack)
  }
}

static void vmi_flushtlb() {
  __asm {
    mov eax, VMI_FLUSH_TLB
    VMCALL(VMI_CALL_FlushTLB)
  }
}

static void vmi_invlpage(void *addr) {
  __asm {
    mov eax, addr
    VMCALL(VMI_CALL_InvalPage)
  }
}

static void vmi_flush_deferred_calls() {
  __asm {
    mov eax, VMI_FLUSH_PT_UPDATES | VMI_FLUSH_CPU_STATE
    VMCALL(VMI_CALL_FlushDeferredCalls)
  }
}

static void vmi_register_page_dir(unsigned long pfn) {
  __asm {
    mov eax, pfn
    mov edx, VMI_PAGE_PD | VMI_PAGE_PT
    xor ecx, ecx
    push ecx
    push ecx
    VMCALL(VMI_CALL_AllocatePage)
    add esp, 8
  }
}

static void vmi_register_page_table(unsigned long pfn) {
  __asm {
    mov eax, pfn
    mov edx, VMI_PAGE_PT
    xor ecx, ecx
    push ecx
    push ecx
    VMCALL(VMI_CALL_AllocatePage)
    add esp, 8
  }
}

static void vmi_set_linear_mapping(int slot, unsigned long vaddr, unsigned long pages, unsigned long pfn) {
  __asm {
    mov eax, slot
    mov edx, vaddr
    mov ecx, pages
    push pfn
    VMCALL(VMI_CALL_SetLinearMapping)
    add esp, 4
  }
}

static void vmi_set_page_dir_entry(pte_t *pde, unsigned long value) {
  kprintf("vmi: vmi_set_linear_mapping %p %d\n", PAGEADDR(pde), virt2pfn(pde));

  vmi_set_linear_mapping(0, PAGEADDR(pde), 1, virt2pfn(pde));

  kprintf("vmi: vmi_set_page_dir_entry\n");

  __asm {
    mov eax, value
    mov edx, pde
    mov ecx, VMI_PAGE_PD
    VMCALL(VMI_CALL_SetPxE)
  }

  kprintf("vmi: vmi_set_page_dir_entry done\n");
  vmi_flush_deferred_calls();
}

static void vmi_set_page_table_entry(pte_t *pte, unsigned long value) {
  vmi_set_linear_mapping(0, PAGEADDR(pte), 1, virt2pfn(pte));

  __asm {
    mov eax, value
    mov edx, pte
    mov ecx, VMI_PAGE_PT
    VMCALL(VMI_CALL_SetPxE)
  }

  vmi_flush_deferred_calls();
}

static void vmi_reboot() {
  __asm {
    mov eax, VMI_REBOOT_HARD
    VMCALL(VMI_CALL_Reboot)
  }
}

//
// Initialize VMI
//

static int vmi_init() {
  int rc;

  __asm {
    mov ecx, vmi
    add ecx, VMI_CALL_Init * VROM_CALL_LEN
    call ecx
    mov rc, eax
  }

  kprintf("vmi: VMI_CALL_Init retuned %d\n", rc);
  return rc;
}

static void vmi_get_reloc_info(int callno, struct vmi_relocation_info *reloc) {
  __asm {
    mov eax, callno
    mov ecx, vmi
    add ecx, VMI_CALL_GetRelocationInfo * VROM_CALL_LEN
    call ecx
    mov ecx, reloc
    mov dword ptr [ecx], eax
    mov dword ptr [ecx + 4], edx
  }
}

int probe_vmi() {
  unsigned long base;

  // VMI ROM is in option ROM area, check signature
  for (base = 0xC0000; base < 0xE0000; base += 2048) {
    struct vrom_header *rom;

    rom = (struct vrom_header *) base;

    if (rom->romSignature != 0xAA55) continue;
    //kprintf("vrom: sig ok at 0x%p, vrom sig 0x%08X\n", rom, rom->vRomSignature);

    if (rom->vRomSignature == VMI_SIGNATURE) {
      kprintf("mach: VMWare VMI ROM version %d.%d detected\n", rom->APIVersionMajor, rom->APIVersionMinor);
      if (rom->APIVersionMajor == VMI_API_REV_MAJOR) {
        vmi = rom;
        break;
      }
    }
  }

  //kprintf("vmi: ROM at 0x%p\n", vmi);
  return vmi != NULL;
}

int init_vmi() {
  int i;
  int cssel;

  kprintf("mach: entering VMI paravirt mode\n");
  if (vmi_init() < 0) return -ENXIO;

  kprintf("mach: now in VMI mode\n");
  for (i = VMI_CALL_Init; i < NUM_VMI_CALLS; i++) {
    struct vmi_relocation_info reloc;

    vmi_get_reloc_info(i, &reloc);
    if (reloc.type != VMI_RELOCATION_CALL_REL) panic("vmi: unsupported reloc type");
    vmicall[i] = reloc.eip;
  }

  // Determine kernel ring
  __asm {
    push cs
    pop cssel
  }
  mach.kring = cssel & 3;
  kprintf("vmi: kernel ring %d\n", mach.kring);

  // Setup direct VMI calls
  SET_MACH_FUNC(sti, vmicall[VMI_CALL_EnableInterrupts]);
  SET_MACH_FUNC(cli, vmicall[VMI_CALL_DisableInterrupts]);
  SET_MACH_FUNC(hlt, vmicall[VMI_CALL_Halt]);
  SET_MACH_FUNC(iretd, vmicall[VMI_CALL_IRET]);
  SET_MACH_FUNC(sysret, vmicall[VMI_CALL_SYSEXIT]);
  SET_MACH_FUNC(rdtsc, vmicall[VMI_CALL_RDTSC]);
  SET_MACH_FUNC(poweroff, vmicall[VMI_CALL_Shutdown]);

  // Setup VMI calls
  mach.in = vmi_in;
  mach.inw = vmi_inw;
  mach.ind = vmi_ind;
  mach.insw = vmi_insw;
  mach.insd = vmi_insd;
  mach.out = vmi_out;
  mach.outw = vmi_outw;
  mach.outd = vmi_outd;
  mach.outsw = vmi_outsw;
  mach.outsd = vmi_outsd;
  mach.cpuid = vmi_cpuid;
  mach.get_cr0 = vmi_get_cr0;
  mach.set_cr0 = vmi_set_cr0;
  mach.get_cr2 = vmi_get_cr2;
  mach.wrmsr = vmi_wrmsr;
  mach.set_gdt_entry = vmi_set_gdt_entry;
  mach.set_idt_gate = vmi_set_idt_gate;
  mach.set_idt_trap = vmi_set_idt_trap;
  mach.switch_kernel_stack = vmi_switch_kernel_stack;
  mach.flushtlb = vmi_flushtlb;
  mach.invlpage = vmi_invlpage;
  mach.register_page_dir = vmi_register_page_dir;
  mach.register_page_table = vmi_register_page_table;
  mach.set_page_dir_entry = vmi_set_page_dir_entry;
  mach.set_page_table_entry = vmi_set_page_table_entry;
  mach.reboot = vmi_reboot;

  return 0;
}

#endif
