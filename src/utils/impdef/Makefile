#
# Makefile for impdef utility
#

all: impdef.exe

impdef.exe: impdef.c
    $(CC) impdef.c

install: impdef.exe
    cp impdef.exe /usr/bin/impdef.exe

clean:
    rm impdef.exe
