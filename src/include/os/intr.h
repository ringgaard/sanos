//
// intr.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Interrupt and trap handling
//

#ifndef INTR_H
#define INTR_H

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

// x86 EFLAGS

#define	EFLAG_CF	0x00000001	// Carry
#define	EFLAG_PF	0x00000004	// Parity
#define	EFLAG_AF	0x00000010	// BCD stuff
#define	EFLAG_ZF	0x00000040	// Zero
#define	EFLAG_SF	0x00000080	// Sign
#define	EFLAG_TF	0x00000100	// Single step
#define	EFLAG_IF	0x00000200	// Interrupts 
#define	EFLAG_DF	0x00000400	// Direction
#define	EFLAG_OF	0x00000800	// Overflow
#define	EFLAG_IOPL	0x00003000	// I/O privilege level
#define	EFLAG_NT	0x00004000	// Nested task
#define	EFLAG_RF	0x00010000	// Resume flag
#define	EFLAG_VM	0x00020000	// Virtual 8086

// x86 CR0 flags

#define CR0_PE (1 << 0)	       // Protection enable
#define CR0_MP (1 << 1)	       // Math processor present
#define CR0_EM (1 << 2)	       // Emulate FP - trap on FP instruction
#define CR0_TS (1 << 3)	       // Task switched flag
#define CR0_ET (1 << 4)	       // Extension type - 387 DX presence */
#define CR0_NE (1 << 5)	       // Numeric Error - allow traps on numeric errors
#define CR0_WP (1 << 16)       // Write protect - ring 0 honors RO PTE's
#define CR0_AM (1 << 18)       // Alignment - trap on unaligned refs
#define CR0_NW (1 << 29)       // Not write-through - inhibit write-through
#define CR0_CD (1 << 30)       // Cache disable
#define CR0_PG (1 << 31)       // Paging - use PTEs/CR3

// CPU context

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

void init_intr();
krnlapi void set_interrupt_handler(int intrno, intrproc_t f, void *arg);

#endif

#endif
