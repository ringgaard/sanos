#
# Makefile for pkg
#

all: pkg.exe

pkg.exe: pkg.c
    $(CC) pkg.c

install: pkg.exe
    cp pkg.exe $(ROOT)/bin/pkg.exe

cross: install

clean:
    rm pkg.exe
