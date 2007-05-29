;-----------------------------------------------------------------------------
; log10.asm - floating point logarithm base 10
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  _log10

_log10          proc    near
                assume  cs:_TEXT
                push    ebp
                mov     ebp,esp
                fld     qword ptr [ebp+8]       ; Load real from stack
                fldlg2                          ; Load log base 10 of 2
                fxch    st(1)                   ; Exchange st, st(1)
                fyl2x                           ; Compute the log base 10(x)
                pop     ebp
                ret
_log10          endp

_TEXT           ends
                end
