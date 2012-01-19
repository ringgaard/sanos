;-----------------------------------------------------------------------------
; atan2.asm - floating point arc tangent (2 argument)
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  atan2
                global  _atan2
                
atan2:
_atan2:
                push    ebp
                mov     ebp,esp
                fld     qword [ebp+8]           ; Load real from stack
                fld     qword [ebp+16]          ; Load real from stack
                fpatan                          ; Take the arctangent
                mov     esp,ebp
                pop     ebp
                ret
