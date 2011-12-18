#
# Makefile for login
#

all: login.exe

login.exe: login.c
    $(CC) login.c

install: login.exe
    sh cp login.exe /bin/login.exe

clean:
    sh rm login.exe
