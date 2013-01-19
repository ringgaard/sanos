#
# Main makefile for Sanos operating system
#

all: boot osldr krnl dev os

boot:
    make -C boot

osldr:
    make -C osldr

krnl:
    make -C krnl

dev: krnl
    make -C dev

os:
    make -C os

clean:
    make -C boot clean
    make -C osldr clean
    make -C krnl clean
    make -C dev clean
    make -C os clean

install: boot osldr krnl dev os
    make -C boot install
    make -C osldr install
    make -C krnl install
    make -C dev install
    make -C os install

.PHONY: boot osldr krnl dev os

