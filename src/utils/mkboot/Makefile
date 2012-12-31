#
# Makefile for mkboot
#

all: mkboot.exe

mkboot.exe: mkboot.c
    $(CC) mkboot.c

install: mkboot.exe
    cp mkboot.exe /bin/mkboot.exe

clean:
    rm mkboot.exe

