;
; boot.asm
;
; Copyright (c) 2001 Michael Ringgaard. All rights reserved.
;
; Boot sector
;

OSLDRSEG    equ 0x1000
OSLDRSTART  equ 8
OSLDRSIZE   equ 32

OSLDRBASE   equ 0x10000

STACKTOP    equ	0x90000

	ORG	0x7c00
	BITS	16
	SECTION	.text

	; Entry point for initial bootstap code
boot:
	jmp 	start
	nop
	
	db	'SANOS   '

ldrsize  dw	OSLDRSIZE
ldrstrt  dd	OSLDRSTART

start:
	; Setup initial environment
	mov	ax, cs
	mov	ds, ax

	; Save boot drive
	mov	[bootdrv], dl

	; Display boot message
	mov	si, bootmsg
	call	print

readldr:
	; Get boot drive geometry
	mov	dl, [bootdrv]
	mov	ah, 8
	xor	di, di
	mov	es, di
	int	0x13

	and	cl, 0x3F
	mov	byte [sectors], cl
	inc	dh
	mov	byte [heads], dh

	; Booting from floppy or harddisk
	mov	dl, [bootdrv]
	cmp	dl, 0x80
	jb	floppy

floppy:

	; Load os loader from boot drive

loadnext:
	mov	al, '.'
	call	printchar

	xor	eax, eax
	mov	ax, [sectno]	    ; eax = os loader sector number

	mov	bx, ax
	shl	bx, 5		    ; 512/16 segments per sector		    
	add	bx, OSLDRSEG
	mov	es, bx		    ; es = segment for next os loader sector

	mov	di, [ldrsize]
	sub	di, ax		    ; di = number of os loader sectors left to read

	mov	cx, [sectno]	    ; make sure we do not cross a 64KB boundary
	and	cx, 0x7F
	neg	cx
	add	cx, 0x80
	cmp	cx, di
	jg	loadnext1
	mov	di, cx
loadnext1:
	mov	ebx, [ldrstrt]      ; eax = LBA of next os loader sector
	add	eax, ebx
	call	readsectors

	mov	ax, [sectno]	    ; update next os loader sector to read
	add	ax, di
	mov	[sectno], ax

	cmp	ax, [ldrsize]
	jnz	loadnext

killmotor:
	mov	dx,0x3f2	    ; kill motor
	mov	al,0
	out	dx, al

	mov	al, 10
	call	printchar
	mov	al, 13
	call	printchar

	; Move system into 32-bit protected mode
	cli			    ; no interrupts allowed

	; enable A20
	call	empty8042
	mov	al, 0xd1	    ; command write
	out	0x64, al
	call	empty8042
	mov	al,0xdf	  	    ; A20 on
	out	0x60, al
	call	empty8042

	; Load idt, gdt and ldt
	lidt	[idtsel]	    ; load idt with 0,0
	lgdt	[gdtsel]	    ; load gdt with whatever appropriate

	; Switch to protected mode
	mov	ax, 0x0001
	lmsw	ax

	; Initialize segment registers
	mov	ax, flat_data - gdt
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax

	; Set code segment and clear prefetch
	jmp	dword (flat_code - gdt):start32

start32:
	BITS	32
	; Setup stack
	mov	esp, STACKTOP

	; Clear flags
	push	dword 2
	popfd

	; Calculate entrypoint
	mov	eax, [OSLDRBASE + 0x3c]
	mov	eax, [eax + OSLDRBASE + 0x28]
	add	eax, OSLDRBASE

	; Get boot drive
	xor	ebx, ebx
	mov	bl, [bootdrv]

	; Push os loader load address, boot drive and a dummy parameter
	push	dword 0
	push	ebx
	push	dword OSLDRBASE

	; Call startup code in os loader
	call	eax

	; We never return here
	cli
	hlt

	BITS	16

readsectors:

	; Read sectors from boot drive
	; input:
	;   eax = LBA
	;   di = maximum sector count
	;   es = segment for buffer
	; output:
	;   di = sectors read

	; Convert LBA to CHS
	cdq			    ; edx = 0
	movzx	ebx, word [sectors]
	div	ebx		    ; eax = track, edx = sector - 1
	mov	cx, dx		    ; cl = sector - 1, ch = 0
	inc	cx		    ; cl = sector number
	xor	dx, dx
	mov	bl, [heads]
	div	ebx

	mov	dh, dl		    ; head
	mov	dl, [bootdrv]	    ; boot drive
	xchg	ch, al		    ; ch = low 8 bits of cylinder number, al = 0
	shr	ax, 2		    ; al[6:7] = high two bits of cylinder, ah = 0
	or	cl, al		    ; cx = cylinder and sector

	; Determine number of sectors to read
	mov	al, cl
	mov	ah, 0
	add	ax, di		    ; ax = last sector to xfer
	cmp	ax, [sectors]
	jbe	readall

	mov	ax, [sectors]	    ; read to end of track
	inc	ax
	sub	al, cl
	push	ax
	jmp	read

readall:
	mov	ax, di		    ; we can read all sectors
	push	ax
	jmp	read

readagain:
	pusha
	mov	al, '#'		    ; print # to mark errror
	call	printchar
	mov	ax, 0		    ; reset disk system
	mov	dx, 0
	int	0x13
	popa

read:
	mov	ah, 2		    ; read sector using bios
	xor	bx, bx
	int	0x13
	jc	readagain

	pop	di

	ret
		
	; Empty keyboard command queue

empty8042:
	call	delay
	in	al,0x64		; 8042 status port
	test	al,2		; is input buffer full?
	jnz	empty8042	; yes - loop
	ret

	; delay is needed after doing i/o
delay:
	dw	0x00eb, 0x00eb
	ret


	; Print string to console
	; si = ptr to first character of a null terminated string
print:
	push	ax
	cld
nextchar:
	mov	al, [si]
	cmp	al, 0
	je	printdone
	call	printchar
	inc	si
	jmp 	nextchar
printdone:
	pop	ax
	ret

	; Print a single character to the console
	; al = character to be printed
printchar:
	mov	ah, 0x0e
	int	0x10
	ret

	; Global descriptor table

D_DATA		equ	0x1000
D_CODE		equ	0x1800

D_WRITE		equ	0x200
D_READ		equ	0x200

D_BIG		equ	0x40
D_BIG_LIM	equ	0x80

D_PRESENT	equ	0x8000

%macro segdesc 3
	dw	%2
	dw	%1
	db	(%1) >> 16
	db	((%3) | D_PRESENT) >> 8
	db	((%3) & 0xff) | ((%2) >> 16)
	db	(%1) >> 24
%endmacro

idtsel:
	dw	0		; idt limit = 0
	dw	0,0		; idt base = 0L

gdtsel:

	dw	gdtlen
	dd	gdt

	;align	8
gdt:

null_desc	segdesc	0,0,0
flat_code	segdesc	0, 0xFFFFF, D_CODE | D_READ | D_BIG | D_BIG_LIM
flat_data	segdesc	0, 0xFFFFF, D_DATA | D_WRITE | D_BIG | D_BIG_LIM

gdtlen	  	equ $ - gdt - 1

	; Variables

sectno	dw	0
sectors	dw	0
heads	dw	0
bootdrv	db	0

	; Message strings
bootmsg:
	db	10, 13, 'Loading system', 0

	; Boot signature
	times 	510-($-$$) db 0
	dw 	0xAA55
