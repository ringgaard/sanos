//
// trap.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Interrupt and trap handling
//

#ifndef TRAP_H
#define TRAP_H

#define IRQBASE       0x20

#define IRQ2INTR(irq) (IRQBASE + (irq))

// PC ISA interrupt requests

#define IRQ_TMR			0
#define IRQ_KBD			1
#define IRQ_SERALT		3
#define IRQ_SERPRI		4
#define IRQ_FD			6
#define IRQ_PARA		7
#define IRQ_RTC			8
#define IRQ_AUX			12
#define IRQ_MATHCOPRERR		13
#define IRQ_HD			14

// x86 exceptions (0-31)

#define INTR_DIV		0
#define INTR_DEBUG		1
#define INTR_NMI		2
#define INTR_BPT		3
#define INTR_OVFL		4
#define INTR_BOUND		5
#define INTR_INSTR		6
#define INTR_FPU		7
#define INTR_DFAULT		8
#define INTR_CPSOVER		9
#define INTR_INVTSS		10
#define INTR_SEG		11
#define INTR_STACK		12
#define INTR_GENPRO		13
#define INTR_PGFLT		14
#define INTR_RESVD1		15
#define INTR_NPX		16

// PC ISA interrupts (32-47)

#define INTR_TMR		IRQ2INTR(IRQ_TMR)
#define INTR_KBD		IRQ2INTR(IRQ_KBD)
#define INTR_SERALT		IRQ2INTR(IRQ_SERALT)
#define INTR_SERPRI		IRQ2INTR(IRQ_SERPRI)
#define INTR_FD			IRQ2INTR(IRQ_FD)
#define INTR_PARA		IRQ2INTR(IRQ_PARA)
#define INTR_RTC		IRQ2INTR(IRQ_RTC)
#define INTR_AUX		IRQ2INTR(IRQ_AUX)
#define INTR_FPUERR	        IRQ2INTR(IRQ_FPUERR)
#define INTR_HD			IRQ2INTR(IRQ_HD)

// System call interrupt

#define INTR_SYSCALL            48

// Trap context

struct context
{
  unsigned long es, ds;
  unsigned long edi, esi, ebp, ebx, edx, ecx, eax;
  unsigned long traptype;
  unsigned long errcode;

  unsigned long eip, ecs;
  unsigned long eflags;
  unsigned long esp, ess;
};

typedef void (*intrproc_t)(struct context *ctxt, void *arg);

struct interrupt_handler
{
  intrproc_t handler;
  void *arg;
};

#ifdef KERNEL

void init_trap();
krnlapi void set_interrupt_handler(int intrno, intrproc_t f, void *arg);

void __inline cli() { __asm cli };
void __inline sti() { __asm sti };
void __inline halt() { __asm hlt };

#endif

#endif
