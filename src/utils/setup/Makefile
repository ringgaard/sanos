#
# Makefile for setup
#

all: setup.exe

setup.exe: setup.c
    $(CC) setup.c

install: setup.exe
    sh cp setup.exe /bin/setup.exe

clean:
    sh rm setup.exe
