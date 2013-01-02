#
# Makefile for Sanos C Compiler
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

all: cc.exe 

install: all
    if not exist $(SDK)\bin mkdir $(SDK)\bin
    copy /Y cc.exe $(SDK)\bin
     
clean:
    del /Q *.exe *.obj

#
# C compiler
#

TCC_SRCFILES=asm386.c asm.c cc.c codegen386.c codegen.c compiler.c elf.c pe.c preproc.c symbol.c type.c util.c

TCC_HDRFILES=cc.h config.h elf.h opcodes.h tokens.h

TCC_LIBS=$(SANOS)/lib/os.lib $(SANOS)/lib/libc.lib

cc.exe: $(TCC_SRCFILES) $(TCC_HDRFILES) $(TCC_LIBS)
    $(CC) $(CFLAGS) /Fe$@ $(TCC_SRCFILES) $(TCC_LIBS) /link /NODEFAULTLIB /FIXED:NO /HEAP:33554432,131072
