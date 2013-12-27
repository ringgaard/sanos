#
# Makefile for login
#

all: login.exe

login.exe: login.c
    $(CC) login.c

install: login.exe
    cp login.exe $(ROOT)/bin/login.exe

cross: install

clean:
    rm login.exe
