;
; ldrinit.asm
;
; OS loader real mode startup
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

;
; The ldrinit is called by the bootstrap (bootsector) after 
; the os loader has been loaded. The ldrinit uses a simplified 
; DOS .EXE format. It is attached to osldr.dll as the DOS stub
; for the DLL.
;

; The os loader is loaded at the fixed address 0x90000. 
; This allows for 56K for the loader, data area and stack.

OSLDRSEG    equ 0x9000
OSLDRBASE   equ (OSLDRSEG * 16)

; Put the stack 8K below the 640K border to allow room for
; the Extended BIOS Data Area

STACKTOP    equ 0x9e000

        BITS    16
        SECTION .text

;
; EXE file header
;

textstart:

header_start:
        db      0x4d, 0x5a          ; EXE file signature
        dw      textsize % 512
        dw      (textsize + 511) / 512
        dw      0                   ; Relocation information: none
        dw      (header_end - header_start) / 16 ; Header size in paragraphs
        dw      0                   ; Min extra mem
        dw      0xffff              ; Max extra mem
        dw      0                   ; Initial SS (before fixup)
        dw      0                   ; Initial SP 
        dw      0                   ; (no) Checksum
        dw      start               ; Initial IP
        dw      0                   ; Initial CS (before fixup)
        dw      header_end          ; File offset to relocation table: none
        dw      krnlopts            ; Overlay number (offset for kernel options)
        align   64, db 0
header_end:

;
; Entry point from bootstrap
;

start:
        ; Setup initial environment
        mov     ax, cs
        mov     ds, ax
        mov     es, ax

        ; Save boot drive and boot image
        mov     [bootdrv], dl
        mov     [bootimg], ebx

        ; Display boot message
        mov     si, osldrmsg
        call    print

        ; Try to get system memory map from BIOS
        call    getmemmap

        ; Check for APM BIOS
        call    apmbioscheck

        ; Move system into 32-bit protected mode
        cli                         ; no interrupts allowed

        ; Enable A20
        call    empty8042
        mov     al, 0xd1            ; command write
        out     0x64, al
        call    empty8042
        mov     al,0xdf             ; A20 on
        out     0x60, al
        call    empty8042

        ; Load idt and gdt
        lidt    [idtsel]            ; load idt with 0,0
        lgdt    [gdtsel]            ; load gdt with whatever appropriate

        ; Switch to protected mode
        mov     ax, 0x0001
        lmsw    ax

        ; Initialize segment registers
        mov     ax, flat_data - gdt
        mov     ds, ax
        mov     es, ax
        mov     fs, ax
        mov     gs, ax
        mov     ss, ax

        ; Set code segment and clear prefetch
        jmp     dword (flat_code - gdt):start32 + OSLDRBASE

start32:
        BITS    32
        ; Setup stack
        mov     esp, STACKTOP
        
        ; Add room for protected mode stack pointer
        push    esp

        ; Clear flags
        push    dword 2
        popfd

        ; Calculate entrypoint
        mov     eax, [OSLDRBASE + 0x3c]
        mov     eax, [eax + OSLDRBASE + 0x28]
        add     eax, OSLDRBASE

        ; Push os loader load address and boot parameter block
        push    dword 0
        push    dword OSLDRBASE + bootparams
        push    dword OSLDRBASE

        ; Call startup code in os loader
        call    eax

        ; We never return here
        cli
        hlt

        BITS    16

;
; Print string to console
; si = ptr to first character of a null terminated string
;

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

;
; Print a single character to the console
;   al = character to be printed
;

printchar:
        mov     ah, 0x0e
        int     0x10
        ret

;
; Empty keyboard command queue
;

empty8042:
        call    delay
        in      al,0x64         ; 8042 status port
        test    al,2            ; is input buffer full?
        jnz     empty8042       ; yes - loop
        ret

        ; Delay is needed after doing i/o
delay:
        dw      0x00eb, 0x00eb
        ret

;
; Check for APM BIOS
;

apmbioscheck:
        mov     ax, 0x5300          ; APM BIOS installation check
        xor     bx, bx
        int     0x15
        jc      apmdone

        cmp     bx, 0x504d          ; Check for "PM" signature
        jne     apmdone

        and     cx, 0x02            ; Is 32-bit supported?
        jz      apmdone
        
        mov     [apmversion], ax    ; Record the APM BIOS version
        mov     [apmflags], cx      ; and flags

        mov     ax, 0x5304          ; Disconnect first just in case
        xor     bx, bx
        int     0x15                ; Ignore return code

        mov     ax, 0x5303          ; APM BIOS 32-bit Connect
        xor     ebx, ebx
        xor     cx, cx
        xor     dx, dx
        xor     esi, esi
        xor     di, di
        int     0x15
        jc      no32apmbios
        
        mov     [apmcseg32], ax
        mov     [apmentry], ebx
        mov     [apmcseg16], cx
        mov     [apmdseg], dx
        mov     [apmcseg32len], si
        shr     esi, 16
        mov     [apmcseg16len], si
        mov     [apmdseglen], di
        
        mov     ax, 0x5300          ; Redo APM BIOS installation check
        xor     bx, bx
        xor     cx, cx
        int     0x15
        jc      apmdisconnect

;       cmp     bx, 0x504d          ; Check for "PM" signature
;       jne     apmdisconnect

;       mov     [apmversion], ax    ; Record the APM BIOS version
;       mov     [apmflags], cx      ; and flags

        jmp     apmdone

apmdisconnect:
        mov     ax, 0x5304          ; Disconnect
        xor     bx, bx
        int     0x15
        jmp     apmdone
        
no32apmbios:
        mov     cx, 0xfffd          ; Remove 32 bit support bit
        and     [apmflags], cx

apmdone:
        ret

;
; Get memory map from BIOS
;

SMAP       equ 0x534d4150
MAXMEMRECS equ 32
MEMRECSIZ  equ 20
   
getmemmap:
        ; Get memory map using int 15 ax=0xe80
        xor     ebx, ebx            ; Continuation value
        mov     di, memrecs         ; Pointer to memmap

e820loop:
        ; Get next memory map entry
        mov     eax, 0x0000e820     ; eax = E820
        mov     edx, SMAP           ; edx = ASCII 'SMAP'
        mov     ecx, MEMRECSIZ      ; ecx = Size of the mementry
        push    ds                  ; es:di = Address of memory map
        pop     es
        int     0x15
        jc      e820fail

        cmp     eax, SMAP           ; Check the return is `SMAP'
        jne     e820fail

        ; Save new memory entry
        mov     eax, [nrmemrecs]    ; Check for max number of memory record
        cmp     eax, MAXMEMRECS
        jnl     e820fail

        inc     eax                 ; Move forward to next record in memory map
        mov     [nrmemrecs], eax
        add     di, MEMRECSIZ
        
        ; Check for more memory records
        cmp     ebx, 0
        jne     e820loop

        ret

e820fail:
        xor     eax, eax            ; Reset memory map
        mov     [nrmemrecs], eax
        ret

;
; Global descriptor table
;

D_DATA          equ     0x1000
D_CODE          equ     0x1800

D_WRITE         equ     0x200
D_READ          equ     0x200
D_CONFORM       equ     0x400

D_BIG           equ     0x40
D_BIG_LIM       equ     0x80

D_PRESENT       equ     0x8000

%macro segdesc 3
        dw      (%2 & 0xffff)
        dw      (%1 & 0xffff)
        db      (%1) >> 16
        db      ((%3) | D_PRESENT) >> 8
        db      ((%3) & 0xff) | ((%2) >> 16)
        db      (%1) >> 24
%endmacro

idtsel:
        dw      0               ; idt limit = 0
        dw      0,0             ; idt base = 0L

gdtsel:
        dw      gdtlen
        dd      gdt + OSLDRBASE

        align   8
gdt:

null_desc       segdesc 0,0,0
flat_code       segdesc 0, 0x0fffff, D_CODE | D_READ | D_BIG | D_BIG_LIM
flat_data       segdesc 0, 0x0fffff, D_DATA | D_WRITE | D_BIG | D_BIG_LIM
real_code       segdesc OSLDRBASE, 0x0ffff, D_CODE | D_READ | D_CONFORM
real_data       segdesc OSLDRBASE, 0x0ffff, D_DATA | D_WRITE

gdtlen          equ $ - gdt - 1

;
; Boot Parameter Block
;

bootparams:

bootdrv      dd    0        ; Boot drive
bootimg      dd    0        ; RAM disk image address

krnlopts     times 128 db 0 ; Kernel options

apmversion   dw    0        ; APM version (BCD format)
apmflags     dw    0        ; APM flags from install check
apmcseg32    dw    0        ; APM 32-bit code segment (real mode segment base address)
apmentry     dw    0        ; Offset of the entry point into the APM BIOS
apmcseg16    dw    0        ; APM 16-bit code segment (real mode segment base address)
apmdseg      dw    0        ; APM data segment (real mode segment base address)
apmcseg32len dw    0        ; APM BIOS 32-bit code segment length
apmcseg16len dw    0        ; APM BIOS 16-bit code segment length
apmdseglen   dw    0        ; APM BIOS data segment length

nrmemrecs    dd    0        ; System memory map
memrecs      times (MAXMEMRECS * MEMRECSIZ) db 0

;
; Strings
;

osldrmsg:    db    ', ldrinit', 0

textend:

textsize equ (textend - textstart)
