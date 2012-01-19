;-----------------------------------------------------------------------------
; asin.asm - floating point arc sine
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  asin
                global  _asin
                global  __CIasin
                
asin:
_asin:
                push    ebp
                mov     ebp,esp
                fld     qword [ebp+8]           ; Load real from stack
                fld     st0                     ; Load x
                fld     st0                     ; Load x
                fmul                            ; Multiply (x squared)
                fld1                            ; Load 1
                fsubr                           ; 1 - (x squared)
                fsqrt                           ; Square root of (1 - x squared)
                fpatan                          ; This gives the arc sine !
                pop     ebp
                ret

__CIasin:
                fld     st0                     ; Load x
                fld     st0                     ; Load x
                fmul                            ; Multiply (x squared)
                fld1                            ; Load 1
                fsubr                           ; 1 - (x squared)
                fsqrt                           ; Square root of (1 - x squared)
                fpatan                          ; This gives the arc sine !
                ret
