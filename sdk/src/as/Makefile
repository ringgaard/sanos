#
# Makefile for Sanos TASM assembler
#

SANOS=..\..\..
INCLUDE=$(SANOS)\src\include
SDK=..\..

!IFNDEF MSVC
MSVC=7
!ENDIF

OUTDEFS=/D OF_ONLY /D OF_ELF32 /D OF_WIN32 /D OF_COFF /D OF_OBJ /D OF_BIN /D OF_DBG /D OF_DEFAULT=of_elf32

COMMONCFLAGS=/I $(INCLUDE) /I. /D HAVE_SNPRINTF /D HAVE_VSNPRINTF /D SANOS /D USE_LOCAL_HEAP $(OUTDEFS)

!IF $(MSVC) == 8 || $(MSVC) == 9 || $(MSVC) == 10
CFLAGS=/nologo /O2 /Ob1 /Oi /Ot /Oy /GS- /GR- /X /GF /Gy /W1 $(COMMONCFLAGS)
!ELSE
CFLAGS=/nologo /O2 /Og /Ob1 /Oi /Ot /Oy /X /GF /Gy /W1 $(COMMONCFLAGS)
!ENDIF

all: as.exe 

install: all
    if not exist $(SDK)\bin mkdir $(SDK)\bin
    copy /Y as.exe $(SDK)\bin
     
clean:
    del /Q *.exe *.obj

#
# Assembler
#

NASM=nasm.c nasmlib.c ver.c raa.c saa.c rbtree.c float.c insnsa.c insnsb.c \
     directiv.c assemble.c labels.c hashtbl.c crc64.c parser.c \
     preproc.c quote.c pptok.c macros.c listing.c eval.c exprlib.c stdscan.c \
     strfunc.c tokhash.c regvals.c regflags.c ilog2.c strlcpy.c \
     output\outform.c output\outlib.c output\nulldbg.c output\nullout.c output\outbin.c  \
     output\outcoff.c output\outelf.c output\outelf32.c output\outobj.c output\outdbg.c \

as.exe: $(NASM) $(SANOS)/lib/os.lib $(SANOS)/lib/libc.lib
    $(CC) $(CFLAGS) /Fe$@ $** /link /NODEFAULTLIB /FIXED:NO /HEAP:33554432,131072
