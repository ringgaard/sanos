;-----------------------------------------------------------------------------
; chkstk.asm - check stack upon procedure entry
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  __chkstk
         
PAGESIZE        equ     4096

__chkstk        proc    near
                assume  cs:_TEXT

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

__chkstk        endp

_TEXT           ends
                end
