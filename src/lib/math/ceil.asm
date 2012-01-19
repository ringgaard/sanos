;-----------------------------------------------------------------------------
; ceil.asm - floating point ceiling
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  ceil
                global  _ceil
                
ceil:
_ceil:
                push    ebp
                mov     ebp,esp
                sub     esp,4                   ; Allocate temporary space
                fld     qword [ebp+8]           ; Load real from stack
                fstcw   [ebp-2]                 ; Save control word
                fclex                           ; Clear exceptions
                mov     word [ebp-4],0b63h      ; Rounding control word
                fldcw   [ebp-4]                 ; Set new rounding control
                frndint                         ; Round to integer
                fclex                           ; Clear exceptions
                fldcw   [ebp-2]                 ; Restore control word
                mov     esp,ebp                 ; Deallocate temporary space
                pop     ebp
                ret
