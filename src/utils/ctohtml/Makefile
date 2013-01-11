#
# Makefile for ctohtml utility
#

all: ctohtml.exe

ctohtml.exe: ctohtml.c
    $(CC) ctohtml.c

install: ctohtml.exe
    cp ctohtml.exe /usr/bin/ctohtml.exe

clean:
    rm ctohtml.exe
