#
# Makefile for makedepend
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

all: makedepend.exe 

install: all
    if not exist $(SDK)\bin mkdir $(SDK)\bin
    copy /Y makedepend.exe $(SDK)\bin
     
clean:
    del *.exe *.obj

SRCFILES=cppsetup.c ifparser.c include.c main.c parse.c pr.c

makedepend.exe: $(SRCFILES) $(SANOS)/lib/os.lib $(SANOS)/lib/libc.lib
    $(CC) $(CFLAGS) /Fe$@ $** /link /NODEFAULTLIB /FIXED:NO /HEAP:33554432,131072
