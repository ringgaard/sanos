#
# Makefile for fdisk
#

all: fdisk.exe

fdisk.exe: fdisk.c
    $(CC) fdisk.c

install: fdisk.exe
    cp fdisk.exe $(ROOT)/bin/fdisk.exe

cross: install

clean:
    rm fdisk.exe
