;
; netboot.asm
;
; PXE Network Boot sector
;
; Copyright (C) 2002 Michael Ringgaard. All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
; 
; 1. Redistributions of source code must retain the above copyright 
;    notice, this list of conditions and the following disclaimer.  
; 2. Redistributions in binary form must reproduce the above copyright
;    notice, this list of conditions and the following disclaimer in the
;    documentation and/or other materials provided with the distribution.  
; 3. Neither the name of the project nor the names of its contributors
;    may be used to endorse or promote products derived from this software
;    without specific prior written permission. 
; 
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
; OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
; HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
; LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
; OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
; SUCH DAMAGE.
; 

OSLDRSEG    equ 0x9000
OSLDRBASE   equ (OSLDRSEG * 16)

OSLDRSTART  equ 8
OSLDRSIZE   equ 32

        ORG     0x7c00
        BITS    16
        SECTION .text

        ; Entry point for initial bootstap code
boot:
        jmp     short start
        nop
        nop
        
        db      'SANOS   '

ldrsize  dw     OSLDRSIZE
ldrstrt  dd     OSLDRSTART

start:
        ; Setup initial environment
        jmp     0:start1
start1:
        mov     ax, cs
        mov     ds, ax
        
        mov     ax, 0x9000
        mov     ss, ax
        mov     sp, 0xF800

        ; Display boot message
        mov     si, bootmsg
        call    print
        
        ; Copy os loader from ram boot image 
        push    ds

        mov     cx, [ldrsize]       ; cx = number of bytes to copy
        shl     cx, 9               ; convert from sectors to bytes

        mov     eax, [ldrstrt]      ; ds = loader start segment
        shl     eax, 5              ; convert from sectors to segments
        add     eax, 0x7c0
        mov     ds, ax

        mov     ax, OSLDRSEG        ; es = loader segment
        mov     es, ax

        xor     di, di
        xor     si, si

        cld                         ; copy os loader from boot image
        rep movsb
        pop     ds

        ; Call real mode entry point in os loader
        mov     ax, OSLDRSEG
        mov     ds, ax
        add     ax, [0x16]          ; cs
        push    ax
        push    word [0x14]         ; ip

        mov     dl, 0xFE            ; boot drive (0xFE for PXE) 
        mov     ebx, 0x7C00         ; RAM disk image
        retf

        ; Print string to console
        ; si = ptr to first character of a null terminated string
print:
        push    ax
        cld
nextchar:
        mov     al, [si]
        cmp     al, 0
        je      printdone
        call    printchar
        inc     si
        jmp     nextchar
printdone:
        pop     ax
        ret

        ; Print a single character to the console
        ; al = character to be printed
printchar:
        mov     ah, 0x0e
        int     0x10
        ret

        ; Message strings
bootmsg:
        db      'Booting system from network... ', 0

        ; Boot signature
        times   510-($-$$) db 0
        dw      0xAA55
