;-----------------------------------------------------------------------------
; cos.asm - floating point cosine
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  _cos
                
_cos            proc    near
                assume  cs:_TEXT
                push    ebp
                mov     ebp,esp                 ; Point to the stack frame
                fld     qword ptr [ebp+8]       ; Load real from stack
                fcos                            ; Take the cosine
                pop     ebp
                ret
_cos            endp

_TEXT           ends
                end
