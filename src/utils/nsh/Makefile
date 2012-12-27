#
# Makefile for new shell
#

all: nsh.exe

SH_SRCFILES=node.c stmalloc.c input.c chartype.c parser.c job.c interp.c sh.c builtins.c cmds.c
SH_HDRFILES=node.h stmalloc.h input.h chartype.h parser.h job.h interp.h sh.h

nsh.exe: $(SH_SRCFILES) $(SH_HDRFILES)
    $(CC) -o nsh.exe $(SH_SRCFILES) -D SHELL

install:
    sh cp nsh.exe /bin/nsh.exe

clean:
    sh rm nsh.exe

