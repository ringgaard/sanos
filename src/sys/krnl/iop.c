//
// iop.c
//
// Port input/output
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

#include <os/krnl.h>

void insw(int port, void *buf, int count)
{
  __asm
  {
    mov edx, port
    mov edi, buf
    mov ecx, count
    rep insw
  }
}

void outsw(int port, void *buf, int count)
{
  __asm
  {
    mov edx, port
    mov esi, buf
    mov ecx, count
    rep outsw
  }
}

void insd(int port, void *buf, int count)
{
  __asm
  {
    mov edx, port
    mov edi, buf
    mov ecx, count
    rep insd
  }
}

void outsd(int port, void *buf, int count)
{
  __asm
  {
    mov edx, port
    mov esi, buf
    mov ecx, count
    rep outsd
  }
}

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
