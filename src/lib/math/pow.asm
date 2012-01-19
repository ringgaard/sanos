;-----------------------------------------------------------------------------
; pow.asm - floating point power
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  pow
                global  _pow
                global  __CIpow

fzero:          dq      0.0e0                   ; Floating point zero

pow:
_pow:
                push    ebp
                mov     ebp,esp
                sub     esp,12                  ; Allocate temporary space
                push    edi                     ; Save register edi
                push    eax                     ; Save register eax
                mov     dword [ebp-12],0        ; Set negation flag to zero
                fld     qword [ebp+16]          ; Load real from stack
                fld     qword [ebp+8]           ; Load real from stack
                mov     edi, fzero              ; Point to real zero
                fcom    qword [edi]             ; Compare x with zero
                fstsw   ax                      ; Get the FPU status word
                mov     al,ah                   ; Move condition flags to AL
                lahf                            ; Load Flags into AH
                and     al, 01000101B           ; Isolate  C0, C2 and C3
                and     ah,~01000101B           ; Turn off CF, PF and ZF
                or      ah,al                   ; Set new  CF, PF and ZF
                sahf                            ; Store AH into Flags
                jb      __fpow1                 ; Re-direct if x < 0
                ja      __fpow3                 ; Re-direct if x > 0
                fxch                            ; Swap st, st(1)
                fcom    qword [edi]             ; Compare y with zero
                fxch                            ; Restore x as top of stack
                fstsw   ax                      ; Get the FPU status word
                mov     al,ah                   ; Move condition flags to AL
                lahf                            ; Load Flags into AH
                and     al, 01000101B           ; Isolate  C0, C2 and C3
                and     ah,~01000101B           ; Turn off CF, PF and ZF
                or      ah,al                   ; Set new  CF, PF and ZF
                sahf                            ; Store AH into Flags
                ja      __fpow3                 ; Re-direct if y > 0
                fstp    st1                     ; Set new stack top and pop
                mov     eax,1                   ; Set domain error (EDOM)
                jmp     __fpow5                 ; End of case
__fpow1:        fxch                            ; Put y on top of stack
                fld     st0                     ; Duplicate y as st1
                frndint                         ; Round to integer
                fxch                            ; Put y on top of stack
                fcomp                           ; y = int(y) ?
                fstsw   ax                      ; Get the FPU status word
                mov     al,ah                   ; Move condition flags to AL
                lahf                            ; Load Flags into AH
                and     al, 01000101B           ; Isolate  C0, C2 and C3
                and     ah,~01000101B           ; Turn off CF, PF and ZF
                or      ah,al                   ; Set new  CF, PF and ZF
                sahf                            ; Store AH into Flags
                je      __fpow2                 ; Proceed if y = int(y)
                fstp    st1                     ; Set new stack top and pop
                fldz                            ; Set result to zero
                fstp    st1                     ; Set new stack top and pop
                mov     eax,1                   ; Set domain error (EDOM)
                jmp     __fpow5                 ; End of case
__fpow2:        fist    dword [ebp-12]          ; Store y as integer
                and     dword [ebp-12],1        ; Set bit if y is odd
                fxch                            ; Put x on top of stack
                fabs                            ; x = |x|
__fpow3:        fldln2                          ; Load log base e of 2
                fxch    st1                     ; Exchange st0, st1
                fyl2x                           ; Compute the natural log(x)
                fmul                            ; Compute y * ln(x)
                fldl2e                          ; Load log base 2(e)
                fmulp   st1,st0                 ; Multiply x * log base 2(e)
                fst     st1                     ; Push result
                frndint                         ; Round to integer
                fsub    st1,st0                 ; Subtract
                fxch                            ; Exchange st0, st1
                f2xm1                           ; Compute 2 to the (x - 1)
                fld1                            ; Load real number 1
                fadd                            ; 2 to the x
                fscale                          ; Scale by power of 2
                fstp    st1                     ; Set new stack top and pop
                test    dword [ebp-12],1        ; Negation required ?
                jz      __fpow4                 ; No, re-direct
                fchs                            ; Negate the result
__fpow4:        fstp    qword [ebp-8]           ; Save (double)pow(x, y)
                fld     qword [ebp-8]           ; Load (double)pow(x, y)
                fxam                            ; Examine st
                fstsw   ax                      ; Get the FPU status word
                cmp     ah,5                    ; Infinity ?
                jne     __fpow6                 ; No, end of case
                mov     eax,2                   ; Set range error (ERANGE)
                                                ; Get errno pointer offset
__fpow5:        int     3
                mov     edi,0                   ; TODO: offset flat:__crt_errno
                mov     edi,[edi]               ; Get C errno variable pointer
                mov     dword [edi],eax         ; Set errno
__fpow6:        pop     eax                     ; Restore register eax
                pop     edi                     ; Restore register edi
                mov     esp,ebp                 ; Deallocate temporary space
                pop     ebp
                ret

__CIpow:
                sub     esp,16                  ; Allocate stack space for args
                fstp    qword [esp+8]           ; Copy y onto stack
                fstp    qword [esp]             ; Copy x onto stack
                call    _pow                    ; Call pow
                add     esp,16                  ; Remove args from stack
                ret
