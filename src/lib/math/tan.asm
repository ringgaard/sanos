;-----------------------------------------------------------------------------
; tan.asm - floating point tangent
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  tan
                global  _tan

tan:
_tan:
                push    ebp
                mov     ebp,esp
                sub     esp,4                   ; Allocate temporary space
                fld     qword [ebp+8]           ; Load real from stack
                fptan                           ; Take the tangent
                fstp    dword [ebp-4]           ; Throw away the constant 1
                mov     esp,ebp                 ; Deallocate temporary space
                pop     ebp
                ret
