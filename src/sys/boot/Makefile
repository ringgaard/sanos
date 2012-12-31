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
    cp boot /boot/boot
    cp cdboot /boot/cdboot
    cp cdemboot /boot/cdemboot
    cp netboot /boot/netboot
    mkboot -b /boot/boot

clean:
    rm boot cdboot cdemboot netboot

