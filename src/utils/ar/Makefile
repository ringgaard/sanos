#
# Makefile for ar utility
#

all: ar.exe

ar.exe: ar.c
    $(CC) ar.c

install: ar.exe
    cp ar.exe $(ROOT)/usr/bin/ar.exe

cross: install

clean:
    rm ar.exe
