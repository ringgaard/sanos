;-----------------------------------------------------------------------------
; frexp.asm - get normalized fraction and exponent
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global frexp
                global _frexp

frexp:
_frexp:
                push    ebp
                mov     ebp,esp
                push    edi                     ; Save register edi
                fld     qword [ebp+8]           ; Load real from stack
                mov     edi,dword [ebp+16]      ; Put exponent address in edi
                ftst                            ; Test st for zero
                fstsw   ax                      ; Put test result in ax
                sahf                            ; Set flags based on test
                jnz     __frexp1                ; Re-direct if not zero
                fld     st0                     ; Set exponent to zero
                jmp     __frexp2                ; End of case
__frexp1:       fxtract                         ; Get exponent and significand
                fld1                            ; Load constant 1
                fld1                            ; Load constant 1
                fadd                            ; Constant 2
                fdiv                            ; Significand / 2
                fxch                            ; Swap st, st(1)
                fld1                            ; Load constant 1
                fadd                            ; Increment exponent
                fistp   dword [edi]             ; Store result exponent and pop
__frexp2:       pop     edi                     ; Restore register edi
                mov     esp,ebp
                pop     ebp
                ret
