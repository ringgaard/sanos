;-----------------------------------------------------------------------------
; tanh.asm - floating point hyperbolic tangent
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  _tanh
                public  __CItanh

_tanh           proc    near
                assume  cs:_TEXT
                push    ebp
                mov     ebp,esp
                fld     qword ptr [ebp+8]       ; Load real from stack
                fld     st                      ; Duplicate stack top
                fadd                            ; Compute 2 * x
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
_tanh           endp

__CItanh        proc    near
                assume  cs:_TEXT
                sub     esp,8                   ; Allocate stack space for x
                fstp    qword ptr [esp]         ; Copy x onto stack
                call    _tanh                   ; Call tanh
                add     esp,8                   ; Remove x from stack
                ret
__CItanh        endp

_TEXT           ends                            ;End of segment
                end                             ;End of module
