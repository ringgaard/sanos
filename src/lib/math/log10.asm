;-----------------------------------------------------------------------------
; log10.asm - floating point logarithm base 10
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  log10
                global  _log10

log10:
_log10:
                push    ebp
                mov     ebp,esp
                fld     qword [ebp+8]           ; Load real from stack
                fldlg2                          ; Load log base 10 of 2
                fxch    st1                     ; Exchange st0, st1
                fyl2x                           ; Compute the log base 10(x)
                pop     ebp
                ret
