;-----------------------------------------------------------------------------
; cosh.asm - floating point hyperbolic cosine
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  cosh
                global  _cosh
                global  __CIcosh
                
cosh:
_cosh:
                push    ebp
                mov     ebp,esp
                sub     esp,8                   ; Allocate temporary space
                fld     qword [ebp+8]           ; Load real from stack
                fchs                            ; Set x = -x
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
                fstp    qword [ebp-8]           ; Save exp(-x)
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
                fscale                          ; Compute exp(-x)
                fstp    st1                     ; Set new stack top and pop
                fld     qword [ebp-8]           ; Get exp(x)
                fadd                            ; Compute exp(x) + exp(-x)
                fld1                            ; Load the constant 1
                fld1                            ; Load the constant 1
                fadd                            ; Set divisor to 2
                fdiv                            ; Compute the hyperbolic cosine
                mov     esp,ebp                 ; Deallocate temporary space
                pop     ebp
                ret

__CIcosh:
                sub     esp,8                   ; Allocate stack space for x
                fstp    qword [esp]             ; Copy x onto stack
                call    _cosh                   ; Call cosh
                add     esp,8                   ; Remove x from stack
                ret
