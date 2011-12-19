#
# Makefile for impdef utility
#

all: impdef.exe

impdef.exe: impdef.c
    $(CC) impdef.c

install: impdef.exe
    sh cp impdef.exe /usr/bin/impdef.exe

clean:
    sh rm impdef.exe
