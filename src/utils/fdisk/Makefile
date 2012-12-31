#
# Makefile for fdisk
#

all: fdisk.exe

fdisk.exe: fdisk.c
    $(CC) fdisk.c

install: fdisk.exe
    cp fdisk.exe /bin/fdisk.exe

clean:
    rm fdisk.exe
