//
// trap.c
//
// Interrupt and trap handling
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

#define INTRS MAXIDT

struct syspage *syspage = (struct syspage *) SYSPAGE_ADDRESS;
struct interrupt *intrhndlr[INTRS];
int intrcount[INTRS];
struct interrupt pgfintr;

char *trapnames[INTRS] = 
{
  "Divide error",
  "Debug exception",
  "Non-maskable interrupt",
  "Breakpoint",
  "Overflow",
  "Bounds check",
  "Invalid opcode",
  "FPU not available",
  "Double fault",
  "FPU segment overrun",
  "Invalid task state segment",
  "Segment not present",
  "Stack fault",
  "General protection fault",
  "Page fault",
  "(reserved)",
  "FPU error",
  "Alignment check",
  "Machine check",
  "SIMD FP exception",
  "(reserved)",
  "(reserved)",
  "(reserved)",
  "(reserved)",
  "(reserved)",
  "(reserved)",
  "(reserved)",
  "(reserved)",
  "(reserved)",
  "(reserved)",
  "(reserved)",
  "(reserved)",

  "IRQ0 (timer)",
  "IRQ1 (keyboard)",
  "IRQ2",
  "IRQ3 (com2)",
  "IRQ4 (com1)",
  "IRQ5",
  "IRQ6 (fdc)",
  "IRQ7 (par)",
  "IRQ8 (rtc)",
  "IRQ9",
  "IRQ10",
  "IRQ11",
  "IRQ12",
  "IRQ13",
  "IRQ14 (hdc1)",
  "IRQ15 (hdc2)",

  "System call",
  "(unused)",
  "(unused)",
  "(unused)",
  "(unused)",
  "(unused)",
  "(unused)",
  "(unused)",
  "(unused)",
  "(unused)",
  "(unused)",
  "(unused)",
  "(unused)",
  "(unused)",
  "(unused)",
  "(unused)"
};

//
// trap
//
// Common entry point for all interrupt service routines
//

static void __cdecl trap(unsigned long args);

static void __declspec(naked) isr()
{
  __asm
  {
    cld
    push    eax
    push    ecx
    push    edx
    push    ebx
    push    ebp
    push    esi
    push    edi
    push    ds
    push    es
    mov	    ax, SEL_KDATA
    mov	    ds, ax
    mov	    es, ax
    call    trap
    pop	    es
    pop	    ds
    pop	    edi
    pop	    esi
    pop	    ebp
    pop	    ebx
    pop	    edx
    pop	    ecx
    pop	    eax
    add	    esp, 8
    iretd
  }
}

//
// syscall
//
// Kernel entry point for int 48 syscall
//

int syscall(int syscallno, char *params);

static void __declspec(naked) systrap(void)
{
  __asm
  {
    push    ds
    push    es
    push    edx
    push    eax
    mov	    ax, SEL_KDATA
    mov	    ds, ax
    mov	    es, ax
    call    syscall
    add	    esp, 8
    pop	    es
    pop	    ds
    iretd
  }
}

//
// sysentry
//
// Kernel entry point for sysenter syscall
//

static void __declspec(naked) sysentry(void)
{
  __asm
  {
    mov     esp, ss:[esp]
    sti

    push    ecx
    push    ebp

    push    INTR_SYSENTER

    push    ds
    push    es

    push    edx
    push    eax

    mov	    ax, SEL_KDATA
    mov	    ds, ax
    mov	    es, ax

    call    syscall
    add	    esp, 8

    pop	    es
    pop	    ds

    add     esp, 4

    pop     ecx
    pop     edx

    sysexit
  }
}

//
// Generate interrupt service routines
//

#define ISR(n)                                \
static void __declspec(naked) isr##n(void)    \
{                                             \
  __asm { push 0 }                            \
  __asm { push n }                            \
  __asm { jmp isr }                           \
}

#define ISRE(n)                               \
static void __declspec(naked) isr##n(void)    \
{                                             \
  __asm { push n }                            \
  __asm { jmp isr }                           \
}


ISR(0)  ISR(1)  ISR(2)   ISR(3)   ISR(4)   ISR(5)   ISR(6)   ISR(7)
ISRE(8) ISR(9)  ISRE(10) ISRE(11) ISRE(12) ISRE(13) ISRE(14) ISR(15)
ISR(16) ISR(17) ISR(18)  ISR(19)  ISR(20)  ISR(21)  ISR(22)  ISR(23)
ISR(24) ISR(25) ISR(26)  ISR(27)  ISR(28)  ISR(29)  ISR(30)  ISR(31)
ISR(32) ISR(33) ISR(34)  ISR(35)  ISR(36)  ISR(37)  ISR(38)  ISR(39)
ISR(40) ISR(41) ISR(42)  ISR(43)  ISR(44)  ISR(45)  ISR(46)  ISR(47)
        ISR(49) ISR(50)  ISR(51)  ISR(52)  ISR(53)  ISR(54)  ISR(55)
ISR(56) ISR(57) ISR(58)  ISR(59)  ISR(60)  ISR(61)  ISR(62)  ISR(63)

//
// init_idt_gate
//
// Initialize one idt entry
//

static void init_idt_gate(int intrno, void *handler)
{
  syspage->idt[intrno].offset_low = (unsigned short) (((unsigned long) handler) & 0xFFFF);
  syspage->idt[intrno].selector = SEL_KTEXT;
  syspage->idt[intrno].access = D_PRESENT | D_INT | D_DPL0;
  syspage->idt[intrno].offset_high = (unsigned short) (((unsigned long) handler) >> 16);
}

//
// init_trap_gate
//
// Initialize one trap entry
//

static void init_trap_gate(int intrno, void *handler)
{
  syspage->idt[intrno].offset_low = (unsigned short) (((unsigned long) handler) & 0xFFFF);
  syspage->idt[intrno].selector = SEL_KTEXT;
  syspage->idt[intrno].access = D_PRESENT | D_TRAP | D_DPL3;
  syspage->idt[intrno].offset_high = (unsigned short) (((unsigned long) handler) >> 16);
}

//
// pagefault_handler
//
// Page fault handler
//

int pagefault_handler(struct context *ctxt, void *arg)
{
  unsigned long addr;
  void *pageaddr;

  __asm { mov eax, cr2 };
  __asm { mov addr, eax };

  pageaddr = (void *) (addr & ~(PAGESIZE - 1));

  if (page_guarded(pageaddr))
  {
    if (guard_page_handler(pageaddr) < 0) dbg_enter(ctxt, (void *) addr);
  }
  else
    dbg_enter(ctxt, (void *) addr);

  return 0;
}

//
// usermode
//
// Tests if context is a user mode context
//

__inline static int usermode(struct context *ctxt)
{
  return USERSPACE(ctxt->eip);
}

//
// wrmsr
//
// Write MSR
//

void wrmsr(unsigned long reg, unsigned long valuelow, unsigned long valuehigh)
{
  __asm
  {
    mov ecx, reg
    mov eax, valuelow
    mov edx, valuehigh
    wrmsr
  }
}

//
// traps_proc
//
// Performance counters for traps
//

static int traps_proc(struct proc_file *pf, void *arg)
{
  int i;

  pprintf(pf, "no trap                                  count\n");
  pprintf(pf, "-- -------------------------------- ----------\n");

  for (i = 0; i < INTRS; i++) 
  {
    if (intrcount[i] != 0)
    {
      pprintf(pf,"%2d %-32s %10d\n", i, trapnames[i], intrcount[i]);
    }
  }

  return 0;
}

//
// init_trap
//
// Initialize traps and interrupts
//

void init_trap()
{
  int i;

  // Initialize interrupt dispatch table
  for (i = 0; i < INTRS; i++) 
  {
    intrhndlr[i] = NULL;
    intrcount[i] = 0;
  }

  // Setup idt
  init_idt_gate(0, isr0);
  init_idt_gate(1, isr1);
  init_idt_gate(2, isr2);
  init_trap_gate(3, isr3);
  init_idt_gate(4, isr4);
  init_idt_gate(5, isr5);
  init_idt_gate(6, isr6);
  init_idt_gate(7, isr7);
  init_idt_gate(8, isr8);
  init_idt_gate(9, isr9);
  init_idt_gate(10, isr10);
  init_idt_gate(11, isr11);
  init_idt_gate(12, isr12);
  init_idt_gate(13, isr13);
  init_idt_gate(14, isr14);
  init_idt_gate(15, isr15);
  init_idt_gate(16, isr16);
  init_idt_gate(17, isr17);
  init_idt_gate(18, isr18);
  init_idt_gate(19, isr19);
  init_idt_gate(20, isr20);
  init_idt_gate(21, isr21);
  init_idt_gate(22, isr22);
  init_idt_gate(23, isr23);
  init_idt_gate(24, isr24);
  init_idt_gate(25, isr25);
  init_idt_gate(26, isr26);
  init_idt_gate(27, isr27);
  init_idt_gate(28, isr28);
  init_idt_gate(29, isr29);
  init_idt_gate(30, isr30);
  init_idt_gate(31, isr31);
  init_idt_gate(32, isr32);
  init_idt_gate(33, isr33);
  init_idt_gate(34, isr34);
  init_idt_gate(35, isr35);
  init_idt_gate(36, isr36);
  init_idt_gate(37, isr37);
  init_idt_gate(38, isr38);
  init_idt_gate(39, isr39);
  init_idt_gate(40, isr40);
  init_idt_gate(41, isr41);
  init_idt_gate(42, isr42);
  init_idt_gate(43, isr43);
  init_idt_gate(44, isr44);
  init_idt_gate(45, isr45);
  init_idt_gate(46, isr46);
  init_idt_gate(47, isr47);
  init_trap_gate(48, systrap);
  init_idt_gate(49, isr49);
  init_idt_gate(50, isr50);
  init_idt_gate(51, isr51);
  init_idt_gate(52, isr52);
  init_idt_gate(53, isr53);
  init_idt_gate(54, isr54);
  init_idt_gate(55, isr55);
  init_idt_gate(56, isr56);
  init_idt_gate(57, isr57);
  init_idt_gate(58, isr58);
  init_idt_gate(59, isr59);
  init_idt_gate(60, isr60);
  init_idt_gate(61, isr61);
  init_idt_gate(62, isr62);
  init_idt_gate(63, isr63);

  // Set system page fault handler
  register_interrupt(&pgfintr, INTR_PGFLT, pagefault_handler, NULL);

  // Initialize fast syscall
  if (cpu.features & CPU_FEATURE_SEP)
  {
    wrmsr(MSR_SYSENTER_CS, SEL_KTEXT, 0);
    wrmsr(MSR_SYSENTER_ESP, TSS_ESP0, 0);
    wrmsr(MSR_SYSENTER_EIP, (unsigned long) sysentry, 0);
  }

  // Register /proc/traps
  register_proc_inode("traps", traps_proc, NULL);
}

//
// trap
//
// Trap dispatcher
//

static void __cdecl trap(unsigned long args)
{
  struct context *ctxt = (struct context *) &args;
  struct thread *t = self();
  struct context *prevctxt;
  struct interrupt *intr;
  int rc;

  // Save context
  prevctxt = t->ctxt;
  t->ctxt = ctxt;
  if (usermode(ctxt)) t->uctxt = (char *) ctxt + offsetof(struct context, traptype);

  // Statistics
  intrcount[ctxt->traptype]++;

  // Call interrupt handlers
  intr = intrhndlr[ctxt->traptype];
  while (1)
  {
    if (!intr)
    {
      dbg_enter(ctxt, NULL);
      break;
    }

    rc = intr->handler(ctxt, intr->arg);
    if (rc == 0) break;
    intr = intr->next;
  }

  // If we interrupted a user mode context, dispatch DPC's now 
  // and check for quantum expiry.
  if (usermode(ctxt)) 
  {
    check_dpc_queue();
    check_preempt();
  }

  // Restore context
  t->ctxt = prevctxt;
  if (usermode(ctxt)) t->uctxt = NULL;
}

//
// register_interrupt
//
// Register interrupt handler for interrupt
//

void register_interrupt(struct interrupt *intr, int intrno, intrproc_t f, void *arg)
{
  cli();
  intr->handler = f;
  intr->arg = arg;
  intr->flags = 0;
  intr->next = intrhndlr[intrno];
  intrhndlr[intrno] = intr;
  sti();
}

//
// unregister_interrupt
//
// Remove interrupt handler for interrupt
//

void unregister_interrupt(struct interrupt *intr, int intrno)
{
  struct interrupt *i;

  cli();

  if (intrhndlr[intrno] == intr)
  {
    intrhndlr[intrno] = intr->next;
  }
  else 
  {
    for (i = intrhndlr[intrno]; i != NULL; i = i->next)
    {
      if (i->next == intr)
      {
	i->next = intr->next;
	break;
      }
    }
  }
  intr->next = NULL;

  sti();
}

//
// get_context
//
// Retrieves the processor context for a thread
//

int get_context(struct thread *t, struct context *ctxt)
{
  unsigned long *uctxt = (unsigned long *) t->uctxt;

  if (!ctxt) return -EINVAL;
  if (uctxt == NULL) return -EPERM;

  if (*uctxt == INTR_SYSCALL)
  {
    struct syscall_context *sysctxt = (struct syscall_context *) ((char *) uctxt - offsetof(struct syscall_context, traptype));

    memset(ctxt, 0, sizeof(struct context));
    ctxt->eip = sysctxt->softint.eip;
    ctxt->ecs = sysctxt->softint.ecs;
    ctxt->eflags = sysctxt->softint.eflags;
    ctxt->esp = sysctxt->softint.esp;
    ctxt->ess = sysctxt->softint.ess;

    ctxt->ds = sysctxt->ds;
    ctxt->es = sysctxt->es;
    ctxt->ebp = ctxt->esp;
    ctxt->traptype = INTR_SYSCALL;
  }
  else if (*uctxt == INTR_SYSENTER)
  {
    struct syscall_context *sysctxt = (struct syscall_context *) ((char *) uctxt - offsetof(struct syscall_context, traptype));

    memset(ctxt, 0, sizeof(struct context));
    ctxt->eip = sysctxt->sysentry.eip;
    ctxt->esp = sysctxt->sysentry.esp;
    ctxt->ds = sysctxt->ds;
    ctxt->es = sysctxt->es;
    ctxt->ebp = ctxt->esp;

    ctxt->traptype = INTR_SYSENTER;
    ctxt->ecs = SEL_UTEXT + SEL_RPL3;
    ctxt->ess = SEL_UDATA + SEL_RPL3;
  }
  else
  {
    memcpy(ctxt, (char *) uctxt - offsetof(struct context, traptype), sizeof(struct context));
  }

  return 0;
}

//
// set_context
//
// Sets the processor context for a thread
//

int set_context(struct thread *t, struct context *ctxt)
{
  return -ENOSYS;
}
