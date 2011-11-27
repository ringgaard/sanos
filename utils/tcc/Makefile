#
# Makefile for Sanos Tiny C Compiler
#

SANOS=..\..
LIBSRC=$(SANOS)\src\lib
INCLUDE=$(SANOS)\src\include
MATHOBJSRC=$(SANOS)\obj\libc

!IFNDEF INSTALLDIR
INSTALLDIR=..\..\install
!ENDIF

TOOLS=$(SANOS)\tools
AR=$(TOOLS)\ar.exe
OBJCOPY=$(TOOLS)\objcopy.exe
IMPDEF=$(TOOLS)\tiny_impdef.exe
TCC=cc.exe

OBJ=tcccrt.o assert.o bsearch.o conio.o crt0.o ctype.o dirent.o fcvt.o fork.o getopt.o glob.o hash.o inifile.o input.o \
     math.o opts.o output.o qsort.o random.o readline.o rmap.o rtttl.o sched.o semaphore.o stdio.o stdlib.o strftime.o \
     string.o strtod.o strtol.o time.o xtoa.o \
     barrier.o condvar.o mutex.o pthread.o rwlock.o spinlock.o \
     setjmp.o chkstk.o \
     acos.o asin.o atan.o atan2.o ceil.o cos.o cosh.o exp.o fabs.o floor.o fmod.o fpconst.o fpreset.o frexp.o \
     ftol.o ldexp.o log.o log10.o modf.o pow.o sin.o sinh.o sqrt.o tan.o tanh.o

CFLAGS=/nologo /O2 /Og /Ob1 /Oi /Ot /Oy /X /GF /Gy /W1 /I $(SANOS)/src/include /D SANOS
OBJCOPYFLAGS=-g --remove-leading-char -I pe-i386 -O elf32-i386

all: cc.exe os.dll libc.a os.def

install: all
     if not exist $(INSTALLDIR)\usr\bin mkdir $(INSTALLDIR)\usr\bin
     if not exist $(INSTALLDIR)\usr\include mkdir $(INSTALLDIR)\usr\include
     if not exist $(INSTALLDIR)\usr\include\arpa mkdir $(INSTALLDIR)\usr\include\arpa
     if not exist $(INSTALLDIR)\usr\include\java mkdir $(INSTALLDIR)\usr\include\java
     if not exist $(INSTALLDIR)\usr\include\net mkdir $(INSTALLDIR)\usr\include\net
     if not exist $(INSTALLDIR)\usr\include\netinet mkdir $(INSTALLDIR)\usr\include\netinet
     if not exist $(INSTALLDIR)\usr\include\os mkdir $(INSTALLDIR)\usr\include\os
     if not exist $(INSTALLDIR)\usr\include\sys mkdir $(INSTALLDIR)\usr\include\sys
     if not exist $(INSTALLDIR)\usr\lib mkdir $(INSTALLDIR)\usr\lib
     if not exist $(INSTALLDIR)\usr\src\cc mkdir $(INSTALLDIR)\usr\src\cc
     copy /Y cc.exe $(INSTALLDIR)\usr\bin
     copy $(INCLUDE)\* $(INSTALLDIR)\usr\include
     copy $(INCLUDE)\arpa\* $(INSTALLDIR)\usr\include\arpa
     copy $(INCLUDE)\java\* $(INSTALLDIR)\usr\include\java
     copy $(INCLUDE)\net\* $(INSTALLDIR)\usr\include\net
     copy $(INCLUDE)\netinet\* $(INSTALLDIR)\usr\include\netinet
     copy $(INCLUDE)\os\* $(INSTALLDIR)\usr\include\os
     copy $(INCLUDE)\sys\* $(INSTALLDIR)\usr\include\sys
     copy /Y libc.a $(INSTALLDIR)\usr\lib
     copy /Y os.def $(INSTALLDIR)\usr\lib
     copy /Y *.c $(INSTALLDIR)\usr\src\cc
     copy /Y *.h $(INSTALLDIR)\usr\src\cc
     copy /Y stab.def $(INSTALLDIR)\usr\src\cc
     copy /Y $(SANOS)\utils\samples\* $(INSTALLDIR)\usr\src

clean:
  del *.exe *.obj *.dll *.o os.def libc.a *.nbc *.suo

tcc.c: config.h elf.h i386-asm.c i386-asm.h i386-gen.c libtcc.h stab.def stab.h tccasm.c tccelf.c tccpe.c tcctok.h

cc.exe: tcc.c $(SANOS)/lib/os.lib $(SANOS)/lib/libc.lib
    $(CC) $(CFLAGS) /D TCC_TARGET_PE /Fe$@ $** /link /NODEFAULTLIB /FIXED:NO
    
os.def: $(SANOS)/bin/os.dll
    $(IMPDEF) $** -o $@

os.dll: $(SANOS)/tools/os.dll
    copy $(SANOS)\tools\os.dll os.dll

libc.a: $(OBJ) 
    $(AR) rcs $@ $**

tcccrt.o: $(LIBSRC)/tcccrt.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

assert.o: $(LIBSRC)/assert.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

bsearch.o: $(LIBSRC)/bsearch.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

conio.o: $(LIBSRC)/conio.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

crt0.o: $(LIBSRC)/crt0.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

ctype.o: $(LIBSRC)/ctype.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

dirent.o: $(LIBSRC)/dirent.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

fcvt.o: $(LIBSRC)/fcvt.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

fork.o: $(LIBSRC)/fork.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

getopt.o: $(LIBSRC)/getopt.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

glob.o: $(LIBSRC)/glob.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

hash.o: $(LIBSRC)/hash.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

inifile.o: $(LIBSRC)/inifile.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

input.o: $(LIBSRC)/input.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

math.o: $(LIBSRC)/math.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

opts.o: $(LIBSRC)/opts.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

output.o: $(LIBSRC)/output.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

qsort.o: $(LIBSRC)/qsort.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

random.o: $(LIBSRC)/random.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

readline.o: $(LIBSRC)/readline.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

rmap.o: $(LIBSRC)/rmap.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

rtttl.o: $(LIBSRC)/rtttl.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

sched.o: $(LIBSRC)/sched.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

semaphore.o: $(LIBSRC)/semaphore.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

stdio.o: $(LIBSRC)/stdio.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

stdlib.o: $(LIBSRC)/stdlib.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

strftime.o: $(LIBSRC)/strftime.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

string.o: $(LIBSRC)/string.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

strtod.o: $(LIBSRC)/strtod.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

strtol.o: $(LIBSRC)/strtol.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

time.o: $(LIBSRC)/time.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

xtoa.o: $(LIBSRC)/xtoa.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

barrier.o: $(LIBSRC)/pthread/barrier.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

condvar.o: $(LIBSRC)/pthread/condvar.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

mutex.o: $(LIBSRC)/pthread/mutex.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

pthread.o: $(LIBSRC)/pthread/pthread.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

rwlock.o: $(LIBSRC)/pthread/rwlock.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

spinlock.o: $(LIBSRC)/pthread/spinlock.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

setjmp.o: $(LIBSRC)/setjmp.s
    $(TCC) -c $** -o $@ -I$(INCLUDE)

chkstk.o: $(LIBSRC)/chkstk.s
    $(TCC) -c $** -o $@ -I$(INCLUDE)

acos.o: $(MATHOBJSRC)\acos.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

asin.o: $(MATHOBJSRC)\asin.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

atan.o: $(MATHOBJSRC)\atan.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

atan2.o: $(MATHOBJSRC)\atan2.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

ceil.o: $(MATHOBJSRC)\ceil.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

cos.o: $(MATHOBJSRC)\cos.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

cosh.o: $(MATHOBJSRC)\cosh.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

exp.o: $(MATHOBJSRC)\exp.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

fabs.o: $(MATHOBJSRC)\fabs.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

floor.o: $(MATHOBJSRC)\floor.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

fmod.o: $(MATHOBJSRC)\fmod.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

fpconst.o: $(MATHOBJSRC)\fpconst.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

fpreset.o: $(MATHOBJSRC)\fpreset.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

frexp.o: $(MATHOBJSRC)\frexp.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

ftol.o: $(MATHOBJSRC)\ftol.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

ldexp.o: $(MATHOBJSRC)\ldexp.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

log.o: $(MATHOBJSRC)\log.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

log10.o: $(MATHOBJSRC)\log10.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

modf.o: $(MATHOBJSRC)\modf.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

pow.o: $(MATHOBJSRC)\pow.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

sin.o: $(MATHOBJSRC)\sin.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

sinh.o: $(MATHOBJSRC)\sinh.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

sqrt.o: $(MATHOBJSRC)\sqrt.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

tan.o: $(MATHOBJSRC)\tan.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@

tanh.o: $(MATHOBJSRC)\tanh.obj
    $(OBJCOPY) $(OBJCOPYFLAGS) $** $@
