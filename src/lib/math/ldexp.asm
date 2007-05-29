;-----------------------------------------------------------------------------
; ldexp.asm - floating point x * 2 to the n
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  _ldexp
                
_ldexp          proc    near
                assume  cs:_TEXT
                push    ebp
                mov     ebp,esp
                sub     esp,8                   ; Allocate temporary space
                fild    dword ptr [ebp+16]      ; Load n as integer
                fld     qword ptr [ebp+8]       ; Load real from stack
                fscale                          ; Compute 2 to the n
                fstp    st(1)                   ; Set new top of stack
                fst     qword ptr [ebp-8]       ; Store result
                mov     esp,ebp                 ; Deallocate temporary space
                pop     ebp
                ret
_ldexp          endp

_TEXT           ends
                end
