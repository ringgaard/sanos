//
// bitops.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Bitmap manipulation routines
//

#ifndef BITOPS_H
#define BITOPS_H

__inline void set_bit(void *bitmap, int pos)
{
  __asm 
  { 
    mov eax, pos
    mov ebx, bitmap
    bts dword ptr [ebx], eax
  }
}

__inline void clear_bit(void *bitmap, int pos)
{
  __asm 
  { 
    mov eax, pos
    mov ebx, bitmap
    btr dword ptr [ebx], eax
  }
}

__inline int test_bit(void *bitmap, int pos)
{
  int result;

  __asm 
  { 
    mov eax, pos
    mov ebx, bitmap
    bt dword ptr [ebx], eax
    sbb eax, eax
    mov result, eax
  }

  return result;
}

__inline void set_bits(void *bitmap, int pos, int len)
{
  while (len-- > 0) set_bit(bitmap, pos++);
}

int find_first_zero_bit(void *bitmap, int len);
int find_next_zero_bit(void *bitmap, int len, int start);

#endif
