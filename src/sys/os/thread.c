//
// thread.c
//
// Thread routines
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
// 3. Neither the name of Michael Ringgaard nor the names of its contributors
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

#include <os.h>

#include <os/syscall.h>
#include <os/seg.h>
#include <os/tss.h>
#include <os/syspage.h>

handle_t mkthread(void (__stdcall *startaddr)(struct tib *), unsigned long stacksize, struct tib **ptib)
{
  return syscall(SYSCALL_MKTHREAD, &startaddr);
}

void __stdcall threadstart(struct tib *tib)
{
  // Set usermode segment selectors
  __asm
  {
    mov	ax, SEL_UDATA + SEL_RPL3
    mov	ds, ax
    mov	es, ax
  }

  // Call thread routine
  ((void (__stdcall *)(void *)) (tib->startaddr))(tib->startarg);
  endthread(0);
}

handle_t beginthread(void (__stdcall *startaddr)(void *), unsigned stacksize, void *arg, int suspended, tid_t *tid)
{
  struct tib *tib;
  handle_t h;

  h = mkthread(threadstart, stacksize, &tib);
  if (h < 0) return h;

  tib->startaddr = (void *) startaddr;
  tib->startarg = arg;

  if (!suspended) resume(h);

  if (tid) *tid = tib->tid;
  return h;
}

int suspend(handle_t thread)
{
  return syscall(SYSCALL_SUSPEND, &thread);
}

int resume(handle_t thread)
{
  return syscall(SYSCALL_RESUME, &thread);
}

void endthread(int retval)
{
  syscall(SYSCALL_ENDTHREAD, &retval);
}

tid_t gettid()
{
  return gettib()->tid;
}

int getcontext(handle_t thread, void *context)
{
  return syscall(SYSCALL_GETCONTEXT, &thread);
}

int setcontext(handle_t thread, void *context)
{
  return syscall(SYSCALL_SETCONTEXT, &thread);
}

int getprio(handle_t thread)
{
  return syscall(SYSCALL_GETPRIO, &thread);
}

int setprio(handle_t thread, int priority)
{
  return syscall(SYSCALL_SETPRIO, &thread);
}

void sleep(int millisecs)
{
  syscall(SYSCALL_SLEEP, &millisecs);
}

struct tib *gettib()
{
  struct tib *tib;

  __asm
  {
    mov eax, fs:[TIB_SELF_OFFSET]
    mov [tib], eax
  }

  return tib;
}

