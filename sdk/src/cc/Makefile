#
# Makefile for Sanos Tiny C Compiler
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
    del *.exe *.obj

#
# C compiler
#

tcc.c: config.h elf.h i386-asm.c i386-asm.h i386-gen.c libtcc.h stab.def stab.h tccasm.c tccelf.c tccpe.c tcctok.h

cc.exe: tcc.c $(SANOS)/lib/os.lib $(SANOS)/lib/libc.lib
    $(CC) $(CFLAGS) /Fe$@ $** /link /NODEFAULTLIB /FIXED:NO /HEAP:33554432,131072
