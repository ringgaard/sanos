//
// bitops.c
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

#include <bitops.h>

int find_first_zero_bit(void *bitmap, int len) {
  int result;

  if (!len) return 0;

  __asm {
    mov ebx, bitmap
    mov edi, ebx
    mov ecx, len
    add ecx, 31
    shr ecx, 5
    xor edx, edx
    mov eax, -1
    repe scasd
    je ffzb1
    
    sub edi, 4
    xor eax, [edi]
    bsf edx, eax

ffzb1:
    sub edi, ebx
    shl edi, 3
    add edx, edi
    mov result, edx
  }

  return result;
}

int find_next_zero_bit(void *bitmap, int len, int start) {
  unsigned long *p = ((unsigned long *) bitmap) + (start >> 5);
  int set = 0;
  int bit = start & 31;
  int result;

  if (start >= len) return len;
  if (bit) {
    unsigned long mask = ~(*p >> bit);

    __asm {
      mov eax, mask
      bsf edx, eax
      jne fnzb1
      mov edx, 32
  fnzb1:
      mov set, edx
    }
    
    if (set < (32 - bit)) return set + start;
    set = 32 - bit;
    p++;
  }

  result = find_first_zero_bit (p, len - 32 * (p - (unsigned long *) bitmap));
  return start + set + result;
}

