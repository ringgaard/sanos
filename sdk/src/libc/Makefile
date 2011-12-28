#
# Makefile for Sanos native C library
#

SANOS=..\..\..
SDK=..\..
LIBSRC=$(SANOS)\src\lib
INCLUDE=$(SANOS)\src\include

AR=$(SDK)\bin\ar.exe
IMPDEF=$(SDK)\bin\impdef.exe
TCC=$(SDK)\bin\cc.exe
NASM=$(SDK)\bin\as.exe

OBJ=tcccrt.o assert.o bsearch.o conio.o crt0.o ctype.o dirent.o fcvt.o fork.o getopt.o glob.o hash.o \
    inifile.o input.o math.o opts.o output.o qsort.o random.o readline.o rmap.o rtttl.o sched.o \
    semaphore.o stdio.o scanf.o printf.o tmpfile.o popen.o stdlib.o strftime.o string.o strtod.o \
    strtol.o time.o xtoa.o \
    regcomp.o regexec.o regerror.o regfree.o \
    barrier.o condvar.o mutex.o pthread.o rwlock.o spinlock.o \
    setjmp.o chkstk.o

MATHOBJ=acos.o asin.o atan.o atan2.o ceil.o cos.o cosh.o exp.o fabs.o floor.o fmod.o fpconst.o fpreset.o frexp.o \
        ftol.o ldexp.o log.o log10.o modf.o pow.o sin.o sinh.o sqrt.o tan.o tanh.o

all: libc.a os.def

install: all
     if not exist $(SDK)\lib mkdir $(SDK)\lib
     copy /Y libc.a $(SDK)\lib
     copy /Y os.def $(SDK)\lib

clean:
  del *.o *.a os.def *.dll

#
# OS lib
#

os.def: $(SANOS)/bin/os.dll
    $(IMPDEF) $** $@

os.dll: $(SDK)/bin/os.dll
    copy $(SDK)/bin/os.dll os.dll

#
# C lib
#

libc.a: $(OBJ) $(MATHOBJ)
    $(AR) -s -m $@ $**

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

scanf.o: $(LIBSRC)/scanf.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

printf.o: $(LIBSRC)/printf.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

tmpfile.o: $(LIBSRC)/tmpfile.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

popen.o: $(LIBSRC)/popen.c
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

regcomp.o: $(LIBSRC)/regex/regcomp.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

regexec.o: $(LIBSRC)/regex/regexec.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

regerror.o: $(LIBSRC)/regex/regerror.c
    $(TCC) -c $** -o $@ -I$(INCLUDE)

regfree.o: $(LIBSRC)/regex/regfree.c
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

acos.o: $(LIBSRC)\math\acos.asm
    $(NASM) -f elf $** -o $@

asin.o: $(LIBSRC)\math\asin.asm
    $(NASM) -f elf $** -o $@

atan.o: $(LIBSRC)\math\atan.asm
    $(NASM) -f elf $** -o $@

atan2.o: $(LIBSRC)\math\atan2.asm
    $(NASM) -f elf $** -o $@

ceil.o: $(LIBSRC)\math\ceil.asm
    $(NASM) -f elf $** -o $@

cos.o: $(LIBSRC)\math\cos.asm
    $(NASM) -f elf $** -o $@

cosh.o: $(LIBSRC)\math\cosh.asm
    $(NASM) -f elf $** -o $@

exp.o: $(LIBSRC)\math\exp.asm
    $(NASM) -f elf $** -o $@

fabs.o: $(LIBSRC)\math\fabs.asm
    $(NASM) -f elf $** -o $@

floor.o: $(LIBSRC)\math\floor.asm
    $(NASM) -f elf $** -o $@

fmod.o: $(LIBSRC)\math\fmod.asm
    $(NASM) -f elf $** -o $@

fpconst.o: $(LIBSRC)\math\fpconst.asm
    $(NASM) -f elf $** -o $@

fpreset.o: $(LIBSRC)\math\fpreset.asm
    $(NASM) -f elf $** -o $@

frexp.o: $(LIBSRC)\math\frexp.asm
    $(NASM) -f elf $** -o $@

ftol.o: $(LIBSRC)\math\ftol.asm
    $(NASM) -f elf $** -o $@

ldexp.o: $(LIBSRC)\math\ldexp.asm
    $(NASM) -f elf $** -o $@

log.o: $(LIBSRC)\math\log.asm
    $(NASM) -f elf $** -o $@

log10.o: $(LIBSRC)\math\log10.asm
    $(NASM) -f elf $** -o $@

modf.o: $(LIBSRC)\math\modf.asm
    $(NASM) -f elf $** -o $@

pow.o: $(LIBSRC)\math\pow.asm
    $(NASM) -f elf $** -o $@

sin.o: $(LIBSRC)\math\sin.asm
    $(NASM) -f elf $** -o $@

sinh.o: $(LIBSRC)\math\sinh.asm
    $(NASM) -f elf $** -o $@

sqrt.o: $(LIBSRC)\math\sqrt.asm
    $(NASM) -f elf $** -o $@

tan.o: $(LIBSRC)\math\tan.asm
    $(NASM) -f elf $** -o $@

tanh.o: $(LIBSRC)\math\tanh.asm
    $(NASM) -f elf $** -o $@
