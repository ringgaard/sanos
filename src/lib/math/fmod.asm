;-----------------------------------------------------------------------------
; fmod.asm - floating point remainder of x/y
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  _fmod
                public  __CIfmod
                
_fmod           proc    near
                assume  cs:_TEXT
                push    ebp
                mov     ebp,esp
                fld     qword ptr [ebp+16]      ; Load real from stack
                fld     qword ptr [ebp+8]       ; Load real from stack
__fmod1:        fprem                           ; Get the partial remainder
                fstsw   ax                      ; Get coprocessor status
                test    ax,0400h                ; Complete remainder ?
                jnz     __fmod1                 ; No, go get next remainder
                fstp    st(1)                   ; Set new top of stack
                pop     ebp
                ret
_fmod           endp

__CIfmod        proc    near
                assume  cs:_TEXT
                fxch    st(1)                   ; Swap arguments
__CIfmod1:      fprem                           ; Get the partial remainder
                fstsw   ax                      ; Get coprocessor status
                test    ax,0400h                ; Complete remainder ?
                jnz     __CIfmod1               ; No, go get next remainder
                fstp    st(1)                   ; Set new top of stack
                ret
__CIfmod        endp

_TEXT           ends
                end
