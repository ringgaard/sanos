;-----------------------------------------------------------------------------
; tanh.asm - floating point hyperbolic tangent
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  tanh
                global  _tanh
                global  __CItanh

tanh:
_tanh:
                push    ebp
                mov     ebp,esp
                fld     qword [ebp+8]           ; Load real from stack
                fld     st0                     ; Duplicate stack top
                fadd                            ; Compute 2 * x
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
                fld1                            ; Load constant 1
                fadd                            ; Compute exp(2*x)+1
                fld1                            ; Load the constant 1
                fld1                            ; Load the constant 1
                fadd                            ; Set divisor to 2
                fdivr                           ; Compute 2/(exp(2*x)+1)
                fld1                            ; Load constant 1
                fsubr                           ; Compute the hyperbolic tangent
                pop     ebp                     ; Restore register bp
                ret

__CItanh:
                sub     esp,8                   ; Allocate stack space for x
                fstp    qword [esp]             ; Copy x onto stack
                call    _tanh                   ; Call tanh
                add     esp,8                   ; Remove x from stack
                ret
