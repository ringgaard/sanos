//
// iop.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Port input/output
//

#include <os/krnl.h>

#ifdef DEBUG

int __declspec(naked) _outp(port_t port, int databyte)
{
  __asm
  {
    xor     eax,eax
    mov     dx,word ptr [esp + 4]
    mov     al,byte ptr [esp + 8]
    out     dx, al
    ret
  }
}

unsigned short __declspec(naked) _outpw(port_t port, unsigned short dataword)
{
  __asm
  {
    mov     dx,word ptr [esp + 4]
    mov     ax,word ptr [esp + 8]
    out     dx,ax
    ret
  }
}

unsigned long __declspec(naked) _outpd(port_t port, unsigned long dataword)
{
  __asm
  {
    mov     dx,word ptr [esp + 4]
    mov     eax,[esp + 8]
    out     dx,eax
    ret
  }
}

int __declspec(naked) _inp(port_t port)
{
  __asm
  {
    xor     eax,eax
    mov     dx,word ptr [esp + 4]
    in      al,dx
    ret
  }
}

unsigned short __declspec(naked) _inpw(port_t port)
{
  __asm
  {
    mov     dx,word ptr [esp + 4]
    in      ax,dx
    ret
  }
}

unsigned long __declspec(naked) _inpd(port_t port)
{
  __asm
  {
    mov     dx,word ptr [esp + 4]
    in      eax,dx
    ret
  }
}

#endif
