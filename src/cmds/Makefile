#
# Makefile for shell commands
#

all: ping.exe test.exe

ping.exe: ping.c
    $(CC) ping.c

test.exe: test.c
    $(CC) test.c

install: ping.exe test.exe
    sh cp ping.exe /bin/ping.exe
    sh cp test.exe /bin/test.exe

clean:
    sh rm ping.exe test.exe

