//
// tls.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Thread local storage
//

#include <os.h>
#include <bitops.h>

char tlsbitmap[MAX_TLS / 8];

tls_t tlsalloc()
{
  tls_t index = find_first_zero_bit(tlsbitmap, MAX_TLS);
  if (index == MAX_TLS) return INVALID_TLS_INDEX;
  set_bit(tlsbitmap, index);

  return index;
}

void tlsfree(tls_t index)
{
  if (index >= 0 && index < MAX_TLS)
  {
    clear_bit(tlsbitmap, index);
  }
}

void *tlsget(tls_t index)
{
  struct tib *tib;

  __asm
  {
    mov eax, fs:[TIB_SELF_OFFSET]
    mov [tib], eax
  }

  return tib->tls[index];
}

void tlsset(tls_t index, void *value)
{
  struct tib *tib;

  __asm
  {
    mov eax, fs:[TIB_SELF_OFFSET]
    mov [tib], eax
  }

  tib->tls[index] = value;
}
