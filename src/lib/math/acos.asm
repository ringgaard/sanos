;-----------------------------------------------------------------------------
; acos.asm - floating point arc cosine
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  _acos
                
_acos           proc    near
                assume  cs:_TEXT
                push    ebp
                mov     ebp,esp
                fld     qword ptr [ebp+8]       ; Load real from stack
                fld     st(0)                   ; Load x
                fld     st(0)                   ; Load x
                fmul                            ; Multiply (x squared)
                fld1                            ; Load 1
                fsubr                           ; 1 - (x squared)
                fsqrt                           ; Square root of (1 - x squared)
                fxch                            ; Exchange st, st(1)
                fpatan                          ; This gives the arc cosine !
                pop     ebp
                ret
_acos           endp

_TEXT           ends
                end
