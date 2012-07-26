//
// trap.h
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

#ifndef TRAP_H
#define TRAP_H

#define IRQBASE       0x20

#define IRQ2INTR(irq) (IRQBASE + (irq))

//
// PC ISA interrupt requests
//

#define IRQ_TMR                 0
#define IRQ_KBD                 1
#define IRQ_SERALT              3
#define IRQ_SERPRI              4
#define IRQ_FD                  6
#define IRQ_PARA                7
#define IRQ_RTC                 8
#define IRQ_AUX                 12
#define IRQ_MATHCOPRERR         13
#define IRQ_HD                  14

//
// x86 exceptions (0-31)
//

#define INTR_DIV                0
#define INTR_DEBUG              1
#define INTR_NMI                2
#define INTR_BPT                3
#define INTR_OVFL               4
#define INTR_BOUND              5
#define INTR_INSTR              6
#define INTR_FPU                7
#define INTR_DFAULT             8
#define INTR_CPSOVER            9
#define INTR_INVTSS             10
#define INTR_SEG                11
#define INTR_STACK              12
#define INTR_GENPRO             13
#define INTR_PGFLT              14
#define INTR_RESVD1             15
#define INTR_NPX                16
#define INTR_ALIGN              17

//
// PC ISA interrupts (32-47)
//

#define INTR_TMR                IRQ2INTR(IRQ_TMR)
#define INTR_KBD                IRQ2INTR(IRQ_KBD)
#define INTR_SERALT             IRQ2INTR(IRQ_SERALT)
#define INTR_SERPRI             IRQ2INTR(IRQ_SERPRI)
#define INTR_FD                 IRQ2INTR(IRQ_FD)
#define INTR_PARA               IRQ2INTR(IRQ_PARA)
#define INTR_RTC                IRQ2INTR(IRQ_RTC)
#define INTR_AUX                IRQ2INTR(IRQ_AUX)
#define INTR_FPUERR             IRQ2INTR(IRQ_FPUERR)
#define INTR_HD                 IRQ2INTR(IRQ_HD)

//
// System call interrupt
//

#define INTR_SYSCALL            48
#define INTR_SIGEXIT            49
#define INTR_SYSENTER           0xFFFF

typedef int (*intrproc_t)(struct context *ctxt, void *arg);

struct interrupt {
  struct interrupt *next;
  int flags;
  intrproc_t handler;
  void *arg;
};

#ifdef KERNEL

void init_trap();
krnlapi void register_interrupt(struct interrupt *intr, int intrno, intrproc_t f, void *arg);
krnlapi void unregister_interrupt(struct interrupt *intr, int intrno);

krnlapi int send_user_signal(struct thread *t, int signum);
int deliver_pending_signals(int retcode);
int set_signal_mask(int how, sigset_t *set, sigset_t *oldset);
int get_pending_signals(sigset_t *set);

int get_context(struct thread *t, struct context *ctxt);
int set_context(struct thread *t, struct context *ctxt);

#endif

#endif
