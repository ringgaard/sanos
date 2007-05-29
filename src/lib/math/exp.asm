;-----------------------------------------------------------------------------
; exp.asm - floating point exponent
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  _exp
                
_exp            proc    near
                assume  cs:_TEXT
                push    ebp
                mov     ebp,esp
                sub     esp,8                   ; Allocate temporary space
                fld     qword ptr [ebp+8]       ; Load real from stack
                fldl2e                          ; Load log base 2(e)
                fmulp   st(1),st                ; Multiply x * log base 2(e)
                fst     st(1)                   ; Push result
                frndint                         ; Round to integer
                fsub    st(1),st                ; Subtract
                fxch                            ; Exchange st, st(1)
                f2xm1                           ; Compute 2 to the (x - 1)
                fld1                            ; Load real number 1
                fadd                            ; 2 to the x
                fscale                          ; Scale by power of 2
                fstp    st(1)                   ; Set new stack top and pop
                fst     qword ptr [ebp-8]       ; Throw away scale factor
                mov     esp,ebp                 ; Deallocate temporary space
                pop     ebp
                ret
_exp            endp

_TEXT           ends
                end
