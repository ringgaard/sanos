//
// intr.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Interrupt and trap handling
//

#include <os/krnl.h>

#define INTRS MAXIDT

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
// Common entry points (slow and fast) for all system routines
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

static void __declspec(naked) sysentry(void)
{
  __asm
  {
    mov     esp, [esp]
    sti

    push    ecx
    push    ebp
    
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

struct syspage *syspage = (struct syspage *) SYSPAGE_ADDRESS;
struct interrupt_handler intrhndlr[INTRS];

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
// Default interrupt handler
//

void default_interrupt_handler(struct context *ctxt, void *arg)
{
  dbg_enter(ctxt, NULL);
}

//
// Page fault handler
//

void pagefault_handler(struct context *ctxt, void *arg)
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
}

//
// Tests if context is a user mode context
//

__inline static int usermode(struct context *ctxt)
{
  return (ctxt->eip < OSBASE);
}

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
// Initialize interrupts
//

void init_intr()
{
  int i;

  // Initialize interrupt dispatch table
  for (i = 0; i < INTRS; i++)
  {
    intrhndlr[i].handler = default_interrupt_handler;
    intrhndlr[i].arg = NULL;
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
  set_interrupt_handler(INTR_PGFLT, pagefault_handler, NULL);

  // Initialize fast syscall
  if (cpu.features & CPU_FEATURE_SEP)
  {
    wrmsr(MSR_SYSENTER_CS, SEL_KDATA, 0);
    wrmsr(MSR_SYSENTER_ESP, TSS_ESP0, 0);
    wrmsr(MSR_SYSENTER_EIP, (unsigned long) sysentry, 0);
  }
}

//
// Trap dispatcher
//

static void __cdecl trap(unsigned long args)
{
  struct context *ctxt = (struct context *) &args;
  struct interrupt_handler *h;

  // Call interrupt handler
  h = &intrhndlr[ctxt->traptype];
  h->handler(ctxt, h->arg);

  // If we interrupted a user mode context, dispatch DPC's now
  if (usermode(ctxt)) dispatch_dpc_queue();
}

//
// Set interrupt handler for interrupt
//

void set_interrupt_handler(int intrno, intrproc_t f, void *arg)
{
  intrhndlr[intrno].handler = f;
  intrhndlr[intrno].arg = arg;
}
