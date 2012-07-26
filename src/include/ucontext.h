//
// ucontext.h
//
// User context
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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef UCONTEXT_H
#define UCONTEXT_H

#include <sys/types.h>

//
// General registers in context
//

#define REG_ES     0
#define REG_DS     1
#define REG_EDI    2
#define REG_ESI    3
#define REG_EBP    4
#define REG_EBX    5
#define REG_EDX    6
#define REG_ECX    7
#define REG_EAX    8
#define REG_TRAPNO 9
#define REG_ERR    10
#define REG_EIP    11
#define REG_ECS    12
#define REG_EFLAGS 13
#define REG_ESP    14
#define REG_ESS    15

#define NGREG      16

//
// Machine-dependent context
//

struct mcontext {
  int gregs[NGREG];
};

typedef struct mcontext mcontext_t;

//
// Stack
//

struct stack {
  void *ss_sp;       // Stack base or pointer
  size_t ss_size;    // Stack size
  int ss_flags;      // Flags
};

typedef struct stack stack_t;

//
// User context
//

struct ucontext {
  struct ucontext *uc_link;
  sigset_t uc_sigmask;
  stack_t uc_stack;
  mcontext_t uc_mcontext;
}; 

typedef struct ucontext ucontext_t;

#ifdef  __cplusplus
extern "C" {
#endif


#ifdef  __cplusplus
}
#endif

#endif
