;-----------------------------------------------------------------------------
; tan.asm - floating point tangent
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  _tan

_tan            proc    near
                assume  cs:_TEXT
                push    ebp
                mov     ebp,esp
                sub     esp,4                   ; Allocate temporary space
                fld     qword ptr [ebp+8]       ; Load real from stack
                fptan                           ; Take the tangent
                fstp    dword ptr [ebp-4]       ; Throw away the constant 1
                mov     esp,ebp                 ; Deallocate temporary space
                pop     ebp
                ret
_tan            endp

_TEXT           ends
                end
