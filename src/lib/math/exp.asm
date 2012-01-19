;-----------------------------------------------------------------------------
; exp.asm - floating point exponent
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  exp
                global  _exp
                
exp:
_exp:
                push    ebp
                mov     ebp,esp
                sub     esp,8                   ; Allocate temporary space
                fld     qword [ebp+8]           ; Load real from stack
                fldl2e                          ; Load log base 2(e)
                fmulp   st1,st0                 ; Multiply x * log base 2(e)
                fst     st1                     ; Push result
                frndint                         ; Round to integer
                fsub    st1,st0                 ; Subtract
                fxch                            ; Exchange st, st(1)
                f2xm1                           ; Compute 2 to the (x - 1)
                fld1                            ; Load real number 1
                fadd                            ; 2 to the x
                fscale                          ; Scale by power of 2
                fstp    st1                     ; Set new stack top and pop
                fst     qword [ebp-8]           ; Throw away scale factor
                mov     esp,ebp                 ; Deallocate temporary space
                pop     ebp
                ret
