#include "bitops.h"

#ifdef USE_I386_BITOPS

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

#else

static __inline unsigned long ffz(unsigned long word)
{
  unsigned long i;
  for (i = 0; i < 32; i++)
  {
    if (!(word & (1 << i))) return i;
  }

  return 0;
}

int find_next_zero_bit(void *bitmap, int len, int start)
{
  unsigned long *p = ((unsigned long *) bitmap) + (start >> 5);
  unsigned long result = start & ~31UL;
  unsigned long tmp;

  if (start >= len) return len;
  len -= result;
  start &= 31UL;
  if (start) 
  {
    tmp = *(p++);
    tmp |= ((unsigned long) ~0UL) >> (32 - start);
    if (len < 32) goto found_first;
    if (~tmp) goto found_middle;
    len -= 32;
    result += 32;
  }
  while (len & ~31UL) 
  {
    if (~(tmp = *(p++))) goto found_middle;
    result += 32;
    len -= 32;
  }
  if (!len) return result;
  tmp = *p;

found_first:
  tmp |= ~0UL << len;

found_middle:
  return result + ffz(tmp);
}

int find_first_zero_bit(void *bitmap, int len) 
{
  return find_next_zero_bit(bitmap, len, 0);
}

#endif


#if 0
#define BUFLEN 4096

void main(int argc, char **argv)
{
  unsigned char buffer[BUFLEN];
  int b;
  int i;
  int n;
  unsigned int start;

  while (1)
  {
    start = GetTickCount();

    for (n = 0; n < 10; n++)
    {
      memset(buffer, 0, BUFLEN);
      b = 0;
      for (i = 0; i < BUFLEN * 8; i++)
      {
	//b = find_first_zero_bit(buffer, BUFLEN * 8);
	b = find_next_zero_bit(buffer, BUFLEN * 8, i);
	set_bit(buffer, b);
        //printf("bit pos %d (%d,%d)\n", b, test_bit(buffer, b), test_bit(buffer, b + 1));
      }
    }
    printf("bit pos sum is %d (%d ms)\n", buffer[0], GetTickCount() - start);
  }
}
#endif
