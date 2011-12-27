#
# Makefile for ctohtml utility
#

all: ctohtml.exe

ctohtml.exe: ctohtml.c
    $(CC) ctohtml.c

install: ctohtml.exe
    sh cp ctohtml.exe /bin/ctohtml.exe

clean:
    sh rm ctohtml.exe
