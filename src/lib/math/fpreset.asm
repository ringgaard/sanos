;-----------------------------------------------------------------------------
; fpreset.asm - floating point unit reset
; Ported from Al Maromaty's free C Runtime Library
;-----------------------------------------------------------------------------

                SECTION .text

                global  _fpreset
                global  __fpreset
                
_fpreset:
__fpreset:
                finit                           ; Initialize the FPU
                ret
