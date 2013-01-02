#
# Makefile for yacc
#

SANOS=..\..\..
INCLUDE=$(SANOS)\src\include
SDK=..\..

!IFNDEF MSVC
MSVC=7
!ENDIF

COMMONCFLAGS=/I $(INCLUDE) /D SANOS /D USE_LOCAL_HEAP

!IF $(MSVC) == 8 || $(MSVC) == 9 || $(MSVC) == 10
CFLAGS=/nologo /O2 /Ob1 /Oi /Ot /Oy /GS- /GR- /X /GF /Gy /W1 $(COMMONCFLAGS)
!ELSE
CFLAGS=/nologo /O2 /Og /Ob1 /Oi /Ot /Oy /X /GF /Gy /W1 $(COMMONCFLAGS)
!ENDIF

all: yacc.exe 

install: all
    if not exist $(SDK)\bin mkdir $(SDK)\bin
    copy /Y yacc.exe $(SDK)\bin

clean:
    del /Q *.exe *.obj

YACC_SRCFILES=closure.c error.c graph.c lalr.c lr0.c main.c mkpar.c output.c reader.c skeleton.c symtab.c verbose.c warshall.c
YACC_HDRFILES=defs.h
YACC_LIBS=$(SANOS)/lib/os.lib $(SANOS)/lib/libc.lib

cc.exe: $(YACC_SRCFILES) $(YACC_HDRFILES) $(YACC_LIBS)
    $(CC) $(CFLAGS) /Fe$@ $(TCC_SRCFILES) $(TCC_LIBS) /link /NODEFAULTLIB /FIXED:NO
