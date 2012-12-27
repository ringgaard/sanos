#
# Makefile for shell
#

all: sh.exe

sh.exe: sh.c
    $(CC) sh.c

install: sh.exe
    sh cp sh.exe /bin/sh.exe

clean:
    sh rm sh.exe
