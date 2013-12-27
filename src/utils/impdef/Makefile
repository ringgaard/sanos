#
# Makefile for impdef utility
#

all: impdef.exe

impdef.exe: impdef.c
    $(CC) impdef.c

install: impdef.exe
    cp impdef.exe $(ROOT)/usr/bin/impdef.exe

cross: install

clean:
    rm impdef.exe
