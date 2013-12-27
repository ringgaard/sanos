#
# Makefile for genvmdk
#

all: genvmdk.exe

genvmdk.exe: genvmdk.c
    $(CC) genvmdk.c

install: genvmdk.exe
    cp genvmdk.exe $(ROOT)/bin/genvmdk.exe

cross: install

clean:
    rm genvmdk.exe

