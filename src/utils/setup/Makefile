#
# Makefile for setup
#

all: setup.exe

setup.exe: setup.c
    $(CC) setup.c

install: setup.exe
    cp setup.exe $(ROOT)/bin/setup.exe

cross: install

clean:
    rm setup.exe
