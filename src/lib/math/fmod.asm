;-----------------------------------------------------------------------------
; fmod.asm - floating point remainder of x/y
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  fmod
                global  _fmod
                global  __CIfmod
                
fmod:
_fmod:
                push    ebp
                mov     ebp,esp
                fld     qword [ebp+16]          ; Load real from stack
                fld     qword [ebp+8]           ; Load real from stack
__fmod1:        fprem                           ; Get the partial remainder
                fstsw   ax                      ; Get coprocessor status
                test    ax,0400h                ; Complete remainder ?
                jnz     __fmod1                 ; No, go get next remainder
                fstp    st1                     ; Set new top of stack
                pop     ebp
                ret

__CIfmod:
                fxch    st1                     ; Swap arguments
__CIfmod1:      fprem                           ; Get the partial remainder
                fstsw   ax                      ; Get coprocessor status
                test    ax,0400h                ; Complete remainder ?
                jnz     __CIfmod1               ; No, go get next remainder
                fstp    st1                     ; Set new top of stack
                ret
