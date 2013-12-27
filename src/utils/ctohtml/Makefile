#
# Makefile for ctohtml utility
#

all: ctohtml.exe

ctohtml.exe: ctohtml.c
    $(CC) ctohtml.c

install: ctohtml.exe
    cp ctohtml.exe $(ROOT)/usr/bin/ctohtml.exe

cross: install

clean:
    rm ctohtml.exe
