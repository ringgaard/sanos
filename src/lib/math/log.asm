;-----------------------------------------------------------------------------
; log.asm - floating point logarithm
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  log
                global  _log

log:
_log:
                push    ebp
                mov     ebp,esp
                fld     qword [ebp+8]           ; Load real from stack
                fldln2                          ; Load log base e of 2
                fxch    st1                     ; Exchange st0, st1
                fyl2x                           ; Compute the natural log(x)
                pop     ebp
                ret
