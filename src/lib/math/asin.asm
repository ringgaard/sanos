;-----------------------------------------------------------------------------
; asin.asm - floating point arc sine
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  _asin
                public  __CIasin
                
_asin           proc    near
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
                fpatan                          ; This gives the arc sine !
                pop     ebp
                ret
_asin           endp

__CIasin        proc    near
                assume  cs:_TEXT
                fld     st(0)                   ; Load x
                fld     st(0)                   ; Load x
                fmul                            ; Multiply (x squared)
                fld1                            ; Load 1
                fsubr                           ; 1 - (x squared)
                fsqrt                           ; Square root of (1 - x squared)
                fpatan                          ; This gives the arc sine !
                ret
__CIasin        endp

_TEXT           ends
                end
