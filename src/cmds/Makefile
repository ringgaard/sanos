#
# Makefile for shell commands
#

all: cp.exe ping.exe test.exe

cp.exe: cp.c
    $(CC) cp.c

ping.exe: ping.c
    $(CC) ping.c

test.exe: test.c
    $(CC) test.c

install: cp.exe ping.exe test.exe
    cp cp.exe /bin/cp.exe
    cp ping.exe /bin/ping.exe
    cp test.exe /bin/test.exe

clean:
    rm ping.exe test.exe

