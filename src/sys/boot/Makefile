#
# Makefile for boot sectors
#

all: boot cdboot cdemboot netboot

boot: boot.asm
    $(AS) -f bin $< -o $@

cdboot: cdboot.asm
    $(AS) -f bin $< -o $@

cdemboot: cdemboot.asm
    $(AS) -f bin $< -o $@

netboot: netboot.asm
    $(AS) -f bin $< -o $@

install: boot cdboot cdemboot netboot
    sh cp boot /boot/boot
    sh cp cdboot /boot/cdboot
    sh cp cdemboot /boot/cdemboot
    sh cp netboot /boot/netboot
    mkboot -b /boot/boot

clean:
    sh rm boot cdboot cdemboot netboot

