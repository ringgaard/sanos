//
// bitops.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Bitmap manipulation routines
//

#include "bitops.h"

int find_first_zero_bit(void *bitmap, int len)
{
  int result;

  if (!len) return 0;

  __asm
  {
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

int find_next_zero_bit(void *bitmap, int len, int start)
{
  unsigned long *p = ((unsigned long *) bitmap) + (start >> 5);
  int set = 0;
  int bit = start & 31;
  int result;

  if (start >= len) return len;
  if (bit) 
  {
    unsigned long mask = ~(*p >> bit);

    __asm
    {
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

