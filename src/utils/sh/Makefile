#
# Makefile for shell
#

all: sh.exe

sh.exe: sh.c ping.c
    $(CC) sh.c ping.c

install: sh.exe
    sh cp sh.exe /bin/sh.exe

clean:
    sh rm sh.exe
