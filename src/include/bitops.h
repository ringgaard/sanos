//
// bitops.h
//
// Bitmap manipulation routines
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

#ifndef BITOPS_H
#define BITOPS_H

#ifdef  __cplusplus
extern "C" {
#endif

__inline void set_bit(void *bitmap, int pos) {
  __asm { 
    mov eax, pos
    mov ebx, bitmap
    bts dword ptr [ebx], eax
  }
}

__inline void clear_bit(void *bitmap, int pos) {
  __asm { 
    mov eax, pos
    mov ebx, bitmap
    btr dword ptr [ebx], eax
  }
}

__inline int test_bit(void *bitmap, int pos) {
  int result;

  __asm {
    mov eax, pos
    mov ebx, bitmap
    bt dword ptr [ebx], eax
    sbb eax, eax
    mov result, eax
  }

  return result;
}

static __inline int find_lowest_bit(unsigned mask) {
  int n;

  __asm {
    bsf eax, mask
    mov n, eax
  }

  return n;
}

#if 0
static __inline int find_highest_bit(unsigned mask) {
  int n;

  __asm {
    bsr eax, mask
    mov n, eax
  }

  return n;
}
#else
static __inline int find_highest_bit(unsigned mask) {
  int n = 31;

  while (n > 0 && !(mask & 0x80000000)) {
    mask <<= 1;
    n--;
  }

  return n;
}
#endif

__inline void set_bits(void *bitmap, int pos, int len) {
  while (len-- > 0) set_bit(bitmap, pos++);
}

int find_first_zero_bit(void *bitmap, int len);
int find_next_zero_bit(void *bitmap, int len, int start);

#ifdef  __cplusplus
}
#endif

#endif
