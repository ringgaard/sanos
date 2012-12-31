#
# Makefile for setup
#

all: setup.exe

setup.exe: setup.c
    $(CC) setup.c

install: setup.exe
    cp setup.exe /bin/setup.exe

clean:
    rm setup.exe
