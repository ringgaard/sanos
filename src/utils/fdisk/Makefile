#
# Makefile for fdisk
#

all: fdisk.exe

fdisk.exe: fdisk.c
    $(CC) fdisk.c

install: fdisk.exe
    sh cp fdisk.exe /bin/fdisk.exe

clean:
    sh rm fdisk.exe
