//
// fpu.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Floating point unit
//

#include <os/krnl.h>

void fpu_enable(struct fpu *state)
{
  // Turn on access to FPU
  __asm
  {
    mov eax, cr0
    and eax, ~(CR0_EM | CR0_TS)
    mov cr0, eax
  }

  if (state)
  {
    // Restore FPU state
    __asm
    {
      mov eax, state
      frstor [eax]
    }
  }
  else
  {
    // Initialize FPU
    __asm
    {
      fnclex
      fninit
    }
  }

}

void fpu_disable(struct fpu *state)
{
  // Save FPU state 
  if (state)
  {
    __asm
    {
      mov eax, state
      fnsave [eax]
    }
  }

  // Disable acces to FPU
  __asm
  {
    mov eax, cr0
    or eax, CR0_EM
    mov cr0, eax
  }
}

void fpu_trap_handler(struct context *ctxt, void *arg)
{
  struct thread *t = current_thread();

  if (t->flags & THREAD_FPU_USED)
  {
    fpu_enable(t->fpustate);
    t->flags |= THREAD_FPU_ENABLED;
  }
  else
  {
    fpu_enable(NULL);
    t->flags |= THREAD_FPU_ENABLED | THREAD_FPU_USED;
  }
}

void init_fpu()
{
  set_interrupt_handler(INTR_FPU, fpu_trap_handler, NULL);

  _asm
  {
    mov eax, cr0
    or eax, CR0_EM | CR0_NE
    mov cr0, eax
  }
}
