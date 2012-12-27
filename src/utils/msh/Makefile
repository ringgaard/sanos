#
# Makefile for mini shell
#

all: msh.exe

sh.exe: msh.c
    $(CC) msh.c

install: msh.exe
    sh cp msh.exe /bin/msh.exe

clean:
    sh rm msh.exe
