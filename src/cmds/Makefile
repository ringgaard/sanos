#
# Makefile for shell commands
#

all: ping.exe

ping.exe: ping.c
    $(CC) ping.c

install: ping.exe
    sh cp ping.exe /bin/ping.exe

clean:
    sh rm ping.exe

