#ifndef BITOPS_H
#define BITOPS_H

#define BITS_PER_BYTE 8

#define USE_I386_BITOPS

#ifdef USE_I386_BITOPS

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

#else

__inline void set_bit(void *bitmap, int pos)
{
  *(((unsigned char *) bitmap) + (pos / BITS_PER_BYTE)) |= (1 << (pos % BITS_PER_BYTE));
}

__inline void clear_bit(void *bitmap, int pos)
{
  *(((unsigned char *) bitmap) + (pos / BITS_PER_BYTE)) &= ~(1 << (pos % BITS_PER_BYTE));
}

__inline int test_bit(void *bitmap, int pos)
{
  return *(((unsigned char *) bitmap) + (pos / BITS_PER_BYTE)) & (1 << (pos % BITS_PER_BYTE));
}

#endif

__inline void set_bits(void *bitmap, int pos, int len)
{
  while (len-- > 0) set_bit(bitmap, pos++);
}

int find_first_zero_bit(void *bitmap, int len);
int find_next_zero_bit(void *bitmap, int len, int start);

#endif
