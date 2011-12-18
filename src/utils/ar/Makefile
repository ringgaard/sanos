#
# Makefile for ar utility
#

all: ar.exe

ar.exe: ar.c
    $(CC) ar.c

install: ar.exe
    sh cp ar.exe /usr/bin/ar.exe

clean:
    sh rm ar.exe
