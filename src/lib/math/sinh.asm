;-----------------------------------------------------------------------------
; sinh.asm - floating point hyperbolic sine
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  _sinh
                public  __CIsinh

_sinh           proc    near
                assume  cs:_TEXT
                push    ebp
                mov     ebp,esp
                sub     esp,8                   ; Allocate temporary space
                fld     qword ptr [ebp+8]       ; Load real from stack
                fchs                            ; Set x = -x
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
                fstp    qword ptr [ebp-8]       ; Save exp(-x)
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
                fscale                          ; Compute exp(-x)
                fstp    st(1)                   ; Set new stack top and pop
                fld     qword ptr [ebp-8]       ; Get exp(x)
                fsub                            ; Compute exp(x) - exp(-x)
                fld1                            ; Load the constant 1
                fld1                            ; Load the constant 1
                fadd                            ; Set divisor to 2
                fdiv                            ; Compute the hyperbolic sine
                mov     esp,ebp                 ; Deallocate temporary space
                pop     ebp
                ret
_sinh           endp

__CIsinh        proc    near
                assume  cs:_TEXT
                sub     esp,8                   ; Allocate stack space for x
                fstp    qword ptr [esp]         ; Copy x onto stack
                call    _sinh                   ; Call sinh
                add     esp,8                   ; Remove x from stack
                ret
__CIsinh        endp

_TEXT           ends
                end
