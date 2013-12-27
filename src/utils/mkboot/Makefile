#
# Makefile for mkboot
#

all: mkboot.exe

mkboot.exe: mkboot.c
    $(CC) mkboot.c

install: mkboot.exe
    cp mkboot.exe $(ROOT)/bin/mkboot.exe

cross: install

clean:
    rm mkboot.exe

