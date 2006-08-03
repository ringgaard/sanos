;-----------------------------------------------------------------------------
; fpconst.asm - floating point constants
;-----------------------------------------------------------------------------
                .386
_DATA           segment DWORD public USE32 'DATA'
                assume  DS:FLAT

                public  __fltused
                public  __infinity
                public  __nan

__fltused       dd      9875h                   ; Floating point used flag
__infinity      db      6 dup(0), 0f0h, 07fh    ; Floating point infinity
__nan           db      6 dup(0ffh), 0f8h, 07fh ; Floating point NaN

_DATA           ends

                end
