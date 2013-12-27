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
    cp boot cdboot cdemboot netboot /boot
    mkboot -b /boot/boot

cross: boot cdboot cdemboot netboot
    cp boot cdboot cdemboot netboot $(ROOT)/boot

clean:
    rm boot cdboot cdemboot netboot

