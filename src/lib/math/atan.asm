;-----------------------------------------------------------------------------
; atan.asm - floating point arc tangent
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  atan
                global  _atan
                
atan:
_atan:
                push    ebp
                mov     ebp,esp
                fld     qword [ebp+8]           ; Load real from stack
                fld1                            ; Load constant 1
                fpatan                          ; Take the arctangent
                pop     ebp
                ret
