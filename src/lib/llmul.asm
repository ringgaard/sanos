;-----------------------------------------------------------------------------
; llmul.asm - long multiply routine
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  __allmul

__allmul        proc    near
                assume  cs:_TEXT

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

__allmul        endp

_TEXT           ends
                end
