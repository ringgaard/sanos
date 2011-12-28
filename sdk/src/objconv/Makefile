#
# Makefile for objconv
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

all: objconv.exe 

install: all
    if not exist $(SDK)\bin mkdir $(SDK)\bin
    copy /Y objconv.exe $(SDK)\bin
     
clean:
    del *.exe *.obj

SRCFILES=cmdline.cpp cof2asm.cpp cof2cof.cpp cof2elf.cpp cof2omf.cpp \
         coff.cpp containers.cpp disasm1.cpp disasm2.cpp elf.cpp \
         elf2asm.cpp elf2cof.cpp elf2elf.cpp elf2mac.cpp error.cpp \
         library.cpp mac2asm.cpp mac2elf.cpp mac2mac.cpp macho.cpp \
         main.cpp omf.cpp omf2asm.cpp omf2cof.cpp omfhash.cpp \
         opcodes.cpp stdafx.cpp

objconv.exe: $(SRCFILES) $(SANOS)/lib/os.lib $(SANOS)/lib/libc.lib
    $(CC) $(CFLAGS) /Fe$@ $** /link /NODEFAULTLIB /FIXED:NO /HEAP:33554432,131072
