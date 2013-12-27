#
# Makefile for make
#

all: make.exe

make.exe: make.c
    $(CC) make.c

install: make.exe
    cp make.exe $(ROOT)/usr/bin/make.exe

cross: install

clean:
    rm make.exe
