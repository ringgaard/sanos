#
# Makefile for pkg
#

all: pkg.exe

pkg.exe: pkg.c
    $(CC) pkg.c

install: pkg.exe
    cp pkg.exe /bin/pkg.exe

clean:
    rm pkg.exe
