;-----------------------------------------------------------------------------
; fpconst.asm - floating point constants
;-----------------------------------------------------------------------------

                SECTION .data

                global  _fltused
                global  __fltused
                global  _infinity
                global  __infinity
                global  _nan
                global  __nan

; Floating point used flag
_fltused:
__fltused:      dd      9875h

; Floating point infinity
_infinity:
__infinity:     db      000h, 000h, 000h, 000h, 000h, 000h, 0f0h, 07fh    

; Floating point NaN
_nan:
__nan:          db      0ffh, 0ffh, 0ffh, 0ffh, 0ffh, 0ffh, 0f8h, 0ffh
