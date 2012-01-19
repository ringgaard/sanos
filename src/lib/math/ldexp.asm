;-----------------------------------------------------------------------------
; ldexp.asm - floating point x * 2 to the n
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  ldexp
                global  _ldexp
                
ldexp:
_ldexp:
                push    ebp
                mov     ebp,esp
                sub     esp,8                   ; Allocate temporary space
                fild    dword [ebp+16]          ; Load n as integer
                fld     qword [ebp+8]           ; Load real from stack
                fscale                          ; Compute 2 to the n
                fstp    st1                     ; Set new top of stack
                fst     qword [ebp-8]           ; Store result
                mov     esp,ebp                 ; Deallocate temporary space
                pop     ebp
                ret
