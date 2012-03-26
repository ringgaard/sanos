#
# Makefile for new shell
#

all: nsh.exe

nsh.exe: node.h \
         sh.c sh.h \
         parser.c parser.h \
         stmalloc.c stmalloc.h \
         input.c input.h \
         chartype.c chartype.h
    $(CC) -o nsh.exe sh.c parser.c stmalloc.c input.c chartype.c

clean:
    sh rm nsh.exe
