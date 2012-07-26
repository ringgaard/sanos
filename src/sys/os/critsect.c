//
// critsect.c
//
// Critical sections
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

#include <os.h>
#include <atomic.h>

__inline tid_t threadid() {
  struct tib *tib;

  __asm {
    mov eax, fs:[TIB_SELF_OFFSET]
    mov [tib], eax
  }

  return tib->tid;
}

void mkcs(critsect_t cs) {
  cs->count = -1;
  cs->recursion = 0;
  cs->owner = NOHANDLE;
  cs->event = mkevent(0, 0);
}

void csfree(critsect_t cs) {
  close(cs->event);
}

void enter(critsect_t cs) {
  tid_t tid = threadid();

  if (cs->owner == tid) {
    cs->recursion++;
  } else {    
    if (atomic_add(&cs->count, 1) > 0) while (waitone(cs->event, INFINITE) != 0);
    cs->owner = tid;
  }
}

void leave(critsect_t cs) {
  if (cs->owner != threadid()) return;
  if (cs->recursion > 0) {
    cs->recursion--;
  } else {
    cs->owner = NOHANDLE;
    if (atomic_add(&cs->count, -1) >= 0) eset(cs->event);
  }
}
