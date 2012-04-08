#
# Makefile for mkboot
#

all: mkboot.exe

mkboot.exe: mkboot.c
    $(CC) mkboot.c

install: mkboot.exe
    sh cp mkboot.exe /bin/mkboot.exe

clean:
    sh rm mkboot.exe

