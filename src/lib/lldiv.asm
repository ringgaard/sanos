;-----------------------------------------------------------------------------
; lldiv.asm - signed long divide
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  __alldiv

__alldiv        proc    near
                assume  cs:_TEXT

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

__alldiv        endp

_TEXT           ends
                end
