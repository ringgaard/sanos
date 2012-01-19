;-----------------------------------------------------------------------------
; acos.asm - floating point arc cosine
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  acos
                global  _acos
                global  __CIacos
                
acos:
_acos:
                push    ebp
                mov     ebp,esp
                fld     qword [ebp+8]           ; Load real from stack
                fld     st0                     ; Load x
                fld     st0                     ; Load x
                fmul                            ; Multiply (x squared)
                fld1                            ; Load 1
                fsubr                           ; 1 - (x squared)
                fsqrt                           ; Square root of (1 - x squared)
                fxch                            ; Exchange st, st(1)
                fpatan                          ; This gives the arc cosine !
                pop     ebp
                ret

__CIacos:
                fld     st0                     ; Load x
                fld     st0                     ; Load x
                fmul                            ; Multiply (x squared)
                fld1                            ; Load 1
                fsubr                           ; 1 - (x squared)
                fsqrt                           ; Square root of (1 - x squared)
                fxch                            ; Exchange st, st(1)
                fpatan                          ; This gives the arc cosine !
                ret
