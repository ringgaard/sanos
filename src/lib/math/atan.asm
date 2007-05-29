;-----------------------------------------------------------------------------
; atan.asm - floating point arc tangent
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  _atan
                
_atan           proc    near
                assume  cs:_TEXT
                push    ebp
                mov     ebp,esp
                fld     qword ptr [ebp+8]       ; Load real from stack
                fld1                            ; Load constant 1
                fpatan                          ; Take the arctangent
                pop     ebp
                ret
_atan           endp

_TEXT           ends
                end
