//
// stdlib.c
//
// Standard library functions
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inifile.h>

#ifdef KERNEL
#include <os/krnl.h>
#endif

#pragma function(abs)

int _fltused = 0x9875;

__declspec(naked) void _allmul()
{
  __asm
  {

    mov     eax,[esp+8]     ; HIWORD(A)
    mov     ecx,[esp+16]    ; HIWORD(B)
    or      ecx,eax         ; test for both hiwords zero.
    mov     ecx,[esp+12]    ; LOWORD(B)
    jnz     short hard      ; both are zero, just mult ALO and BLO

    mov     eax,[esp+4]     ; LOWORD(A)
    mul     ecx

    ret     16              ; callee restores the stack

  hard:
    push    ebx

    mul     ecx             ; eax has AHI, ecx has BLO, so AHI * BLO
    mov     ebx,eax         ; save result

    mov     eax,[esp+8]     ; LOWORD(A2)
    mul     dword ptr [esp+20] ; ALO * BHI
    add     ebx,eax         ; ebx = ((ALO * BHI) + (AHI * BLO))

    mov     eax,[esp+8]     ; ecx = BLO
    mul     ecx             ; so edx:eax = ALO*BLO
    add     edx,ebx         ; now edx has all the LO*HI stuff

    pop     ebx

    ret     16              ; callee restores the stack
  }
}

__declspec(naked) void _alldiv()
{
  __asm
  {
        push    edi
        push    esi
        push    ebx

	; Determine sign of the result (edi = 0 if result is positive, non-zero
	; otherwise) and make operands positive.

        xor     edi,edi         ; result sign assumed positive

        mov     eax,[esp+20]    ; hi word of a
        or      eax,eax         ; test to see if signed
        jge     short L1        ; skip rest if a is already positive
        inc     edi             ; complement result sign flag
        mov     edx,[esp+16]    ; lo word of a
        neg     eax             ; make a positive
        neg     edx
        sbb     eax,0
        mov     [esp+20],eax    ; save positive value
        mov     [esp+16],edx
L1:
        mov     eax,[esp+28]    ; hi word of b
        or      eax,eax         ; test to see if signed
        jge     short L2        ; skip rest if b is already positive
        inc     edi             ; complement the result sign flag
        mov     edx,[esp+24]    ; lo word of a
        neg     eax             ; make b positive
        neg     edx
        sbb     eax,0
        mov     [esp+28],eax    ; save positive value
        mov     [esp+24],edx
L2:

	;
	; Now do the divide.  First look to see if the divisor is less than 4194304K.
	; If so, then we can use a simple algorithm with word divides, otherwise
	; things get a little more complex.
	;
	; NOTE - eax currently contains the high order word of DVSR
	;

        or      eax,eax         ; check to see if divisor < 4194304K
        jnz     short L3        ; nope, gotta do this the hard way
        mov     ecx,[esp+24]    ; load divisor
        mov     eax,[esp+20]    ; load high word of dividend
        xor     edx,edx
        div     ecx             ; eax <- high order bits of quotient
        mov     ebx,eax         ; save high bits of quotient
        mov     eax,[esp+16]    ; edx:eax <- remainder:lo word of dividend
        div     ecx             ; eax <- low order bits of quotient
        mov     edx,ebx         ; edx:eax <- quotient
        jmp     short L4        ; set sign, restore stack and return

	;
	; Here we do it the hard way.  Remember, eax contains the high word of DVSR
	;

L3:
        mov     ebx,eax         ; ebx:ecx <- divisor
        mov     ecx,[esp+24]
        mov     edx,[esp+20]    ; edx:eax <- dividend
        mov     eax,[esp+16]
L5:
        shr     ebx,1           ; shift divisor right one bit
        rcr     ecx,1
        shr     edx,1           ; shift dividend right one bit
        rcr     eax,1
        or      ebx,ebx
        jnz     short L5        ; loop until divisor < 4194304K
        div     ecx             ; now divide, ignore remainder
        mov     esi,eax         ; save quotient

	;
	; We may be off by one, so to check, we will multiply the quotient
	; by the divisor and check the result against the orignal dividend
	; Note that we must also check for overflow, which can occur if the
	; dividend is close to 2**64 and the quotient is off by 1.
	;

        mul     dword ptr [esp+28] ; QUOT * HIWORD(DVSR)
        mov     ecx,eax
        mov     eax,[esp+24]
        mul     esi             ; QUOT * LOWORD(DVSR)
        add     edx,ecx         ; EDX:EAX = QUOT * DVSR
        jc      short L6        ; carry means Quotient is off by 1

	;
	; do long compare here between original dividend and the result of the
	; multiply in edx:eax.  If original is larger or equal, we are ok, otherwise
	; subtract one (1) from the quotient.
	;

        cmp     edx,[esp+20]    ; compare hi words of result and original
        ja      short L6        ; if result > original, do subtract
        jb      short L7        ; if result < original, we are ok
        cmp     eax,[esp+16]    ; hi words are equal, compare lo words
        jbe     short L7        ; if less or equal we are ok, else subtract
L6:
        dec     esi             ; subtract 1 from quotient
L7:
        xor     edx,edx         ; edx:eax <- quotient
        mov     eax,esi

	;
	; Just the cleanup left to do.  edx:eax contains the quotient.  Set the sign
	; according to the save value, cleanup the stack, and return.
	;

L4:
        dec     edi             ; check to see if result is negative
        jnz     short L8        ; if EDI == 0, result should be negative
        neg     edx             ; otherwise, negate the result
        neg     eax
        sbb     edx,0

	;
	; Restore the saved registers and return.
	;

L8:
        pop     ebx
        pop     esi
        pop     edi

        ret     16
  }
}

__declspec(naked) void _chkstk()
{
  __asm
  {
        push    ecx                     ; save ecx
        cmp     eax,PAGESIZE            ; more than one page requested?
        lea     ecx,[esp] + 8           ;   compute new stack pointer in ecx
                                        ;   correct for return address and
                                        ;   saved ecx
        jb      short lastpage          ; no

probepages:
        sub     ecx,PAGESIZE            ; yes, move down a page
        sub     eax,PAGESIZE            ; adjust request and...

        test    dword ptr [ecx],eax     ; ...probe it

        cmp     eax,PAGESIZE            ; more than one page requested?
        jae     short probepages        ; no

lastpage:
        sub     ecx,eax                 ; move stack down by eax
        mov     eax,esp                 ; save current tos and do a...

        test    dword ptr [ecx],eax     ; ...probe in case a page was crossed

        mov     esp,ecx                 ; set the new stack pointer

        mov     ecx,dword ptr [eax]     ; recover ecx
        mov     eax,dword ptr [eax + 4] ; recover return address

        push    eax                     ; prepare return address
                                        ; ...probe in case a page was crossed
        ret
  }
}

char *strdup(const char *s)
{
  char *t;
  int len;

  if (!s) return NULL;
  len = strlen(s);
  t = (char *) malloc(len + 1);
  memcpy(t, s, len + 1);
  return t;
}

int parse_args(char *args, char **argv)
{
  char *p;
  int argc;
  char *start;
  char *end;
  char *buf;
  int delim;

  p = args;
  argc = 0;
  while (*p)
  {
    while (*p == ' ') p++;
    if (!*p) break;

    if (*p == '"' || *p == '\'')
    {
      delim = *p++;
      start = p;
      while (*p && *p != delim) p++;
      end = p;
      if (*p == delim) p++;
    }
    else
    {
      start = p;
      while (*p && *p != ' ') p++;
      end = p;
    }

    if (argv)
    {
      buf = (char *) malloc(end - start + 1);
      if (!buf) break;
      memcpy(buf, start, end - start);
      buf[end - start] = 0;
      argv[argc] = buf;
    }
    
    argc++;
  }

  return argc;
}

void free_args(int argc, char **argv)
{
  int i;

  for (i = 0; i < argc; i++) free(argv[i]);
  if (argv) free(argv);
}

int abs(int number)
{
  return number >= 0 ? number : -number;
}

#ifndef KERNEL

char *getenv(const char *option)
{
  return get_property(config, "env", (char *) option, NULL);
}

void abort()
{
  exit(3);
}

#ifdef DEBUG

void _assert(void *expr, void *filename, unsigned lineno)
{
  printf("Assertion failed: %s, file %s, line %d\n", expr, filename, lineno);
  exit(3);
}

#endif

#endif
