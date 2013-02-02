#
# Makefile for shell
#

all: sh.exe

SH_SRCFILES=node.c stmalloc.c input.c chartype.c parser.c job.c interp.c sh.c builtins.c cmds.c
SH_HDRFILES=node.h stmalloc.h input.h chartype.h parser.h job.h interp.h sh.h
CMDS= \
  ../../cmds/chgrp.c \
  ../../cmds/chmod.c \
  ../../cmds/chown.c \
  ../../cmds/cp.c \
  ../../cmds/du.c \
  ../../cmds/ls.c \
  ../../cmds/mkdir.c \
  ../../cmds/mv.c \
  ../../cmds/ping.c \
  ../../cmds/rm.c \
  ../../cmds/test.c \
  ../../cmds/touch.c \
  ../../cmds/wc.c

sh.exe: $(SH_SRCFILES) $(SH_HDRFILES) $(CMDS)
    $(CC) -o sh.exe $(SH_SRCFILES) $(CMDS) -D SHELL

install: sh.exe
    cp sh.exe /bin/sh.exe

clean:
    rm sh.exe

