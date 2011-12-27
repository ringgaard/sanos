#
# Makefile for boot sectors
#

all: boot cdboot cdemboot netboot ldrinit.exe

boot: boot.asm
    $(AS) -f bin $< -o $@

cdboot: cdboot.asm
    $(AS) -f bin $< -o $@

cdemboot: cdemboot.asm
    $(AS) -f bin $< -o $@

netboot: netboot.asm
    $(AS) -f bin $< -o $@

ldrinit.exe: ldrinit.asm
    $(AS) -f bin $< -o $@

clean:
    sh rm boot cdboot cdemboot netboot ldrinit.exe
