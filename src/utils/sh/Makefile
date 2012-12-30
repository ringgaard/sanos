#
# Makefile for shell
#

all: sh.exe

SH_SRCFILES=node.c stmalloc.c input.c chartype.c parser.c job.c interp.c sh.c builtins.c cmds.c
SH_HDRFILES=node.h stmalloc.h input.h chartype.h parser.h job.h interp.h sh.h
CMDS=../../cmds/ping.c ../../cmds/test.c

sh.exe: $(SH_SRCFILES) $(SH_HDRFILES) $(CMDS)
    $(CC) -o sh.exe $(SH_SRCFILES) $(CMDS) -D SHELL

install: sh.exe
    sh cp sh.exe /bin/sh.exe

clean:
    sh rm sh.exe

