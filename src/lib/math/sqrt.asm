;-----------------------------------------------------------------------------
; sqrt.asm - floating point square root
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  sqrt
                global  _sqrt
                global  __CIsqrt
sqrt:
_sqrt:
                push    ebp
                mov     ebp,esp
                fld     qword [ebp+8]           ; Load real from stack
                fsqrt                           ; Take the square root
                pop     ebp
                ret

__CIsqrt:
                fsqrt                           ; Take the square root
                ret
