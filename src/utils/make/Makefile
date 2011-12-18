#
# Makefile for make
#

all: make.exe

make.exe: make.c
    $(CC) make.c

install: make.exe
    sh cp make.exe /usr/bin/make.exe

clean:
    sh rm make.exe
