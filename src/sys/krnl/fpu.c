//
// fpu.c
//
// Floating point unit
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

struct interrupt fpuintr;
struct interrupt fpuxcpt;

void fpu_enable(struct fpu *state) {
  // Turn on access to FPU
  set_cr0(get_cr0() &  ~(CR0_EM | CR0_TS));

  if (state) {
    // Restore FPU state
    __asm {
      mov eax, state
      frstor [eax]
    }
  } else {
    // Initialize FPU
    __asm {
      fnclex
      fninit
    }
  }
}

void fpu_disable(struct fpu *state) {
  // Save FPU state 
  if (state) {
    __asm {
      mov eax, state
      fnsave [eax]
    }
  }

  // Disable acces to FPU
  set_cr0(get_cr0() | CR0_EM);
}

int fpu_trap_handler(struct context *ctxt, void *arg) {
  struct thread *t = self();

  if (t->flags & THREAD_FPU_USED) {
    fpu_enable(&t->fpustate);
    t->flags |= THREAD_FPU_ENABLED;
  } else {
    fpu_enable(NULL);
    t->flags |= THREAD_FPU_ENABLED | THREAD_FPU_USED;
  }

  return 0;
}

int fpu_npx_handler(struct context *ctxt, void *arg) {
  panic("floating point processor fault");
  return 0;
}

void init_fpu() {
  register_interrupt(&fpuintr, INTR_FPU, fpu_trap_handler, NULL);
  register_interrupt(&fpuxcpt, INTR_NPX, fpu_npx_handler, NULL);
  set_cr0(get_cr0() | CR0_EM | CR0_NE);
}
