#
# Makefile for C runtime library
#

OBJ=tcccrt.o assert.o bsearch.o conio.o crt0.o ctype.o dirent.o fcvt.o \
    fork.o getopt.o glob.o hash.o inifile.o input.o math.o opts.o output.o \
    qsort.o random.o readline.o rmap.o rtttl.o sched.o semaphore.o \
    stdio.o scanf.o printf.o tmpfile.o popen.o stdlib.o strftime.o \
    string.o strtod.o strtol.o time.o xtoa.o \
    regcomp.o regexec.o regerror.o regfree.o \
    barrier.o condvar.o mutex.o pthread.o rwlock.o spinlock.o \
    setjmp.o chkstk.o

MATHOBJ=acos.o asin.o atan.o atan2.o ceil.o cos.o cosh.o exp.o fabs.o floor.o \
        fmod.o fpconst.o fpreset.o frexp.o ftol.o ldexp.o log.o log10.o \
        modf.o pow.o sin.o sinh.o sqrt.o tan.o tanh.o

all: libc.a os.def

libc.a: $(OBJ) $(MATHOBJ)
    ar -s -m $@ $^

tcccrt.o: tcccrt.c
    $(CC) -c $<

assert.o: assert.c
    $(CC) -c $<

bsearch.o: bsearch.c
    $(CC) -c $<

conio.o: conio.c
    $(CC) -c $<

crt0.o: crt0.c
    $(CC) -c $<

ctype.o: ctype.c
    $(CC) -c $<

dirent.o: dirent.c
    $(CC) -c $<

fcvt.o: fcvt.c
    $(CC) -c $<

fork.o: fork.c
    $(CC) -c $<

getopt.o: getopt.c
    $(CC) -c $<

glob.o: glob.c
    $(CC) -c $<

hash.o: hash.c
    $(CC) -c $<

inifile.o: inifile.c
    $(CC) -c $<

input.o: input.c
    $(CC) -c $<

math.o: math.c
    $(CC) -c $<

opts.o: opts.c
    $(CC) -c $<

output.o: output.c
    $(CC) -c $<

qsort.o: qsort.c
    $(CC) -c $<

random.o: random.c
    $(CC) -c $<

readline.o: readline.c
    $(CC) -c $<

rmap.o: rmap.c
    $(CC) -c $<

rtttl.o: rtttl.c
    $(CC) -c $<

sched.o: sched.c
    $(CC) -c $<

semaphore.o: semaphore.c
    $(CC) -c $<

stdio.o: stdio.c
    $(CC) -c $<

printf.o: printf.c
    $(CC) -c $<

scanf.o: scanf.c
    $(CC) -c $<

tmpfile.o: tmpfile.c
    $(CC) -c $<

popen.o: popen.c
    $(CC) -c $<

stdlib.o: stdlib.c
    $(CC) -c $<

strftime.o: strftime.c
    $(CC) -c $<

string.o: string.c
    $(CC) -c $<

strtod.o: strtod.c
    $(CC) -c $<

strtol.o: strtol.c
    $(CC) -c $<

time.o: time.c
    $(CC) -c $<

xtoa.o: xtoa.c
    $(CC) -c $<

regcomp.o: regex/regcomp.c
    $(CC) -c $<

regexec.o: regex/regexec.c
    $(CC) -c $<

regerror.o: regex/regerror.c
    $(CC) -c $<

regfree.o: regex/regfree.c
    $(CC) -c $<

barrier.o: pthread/barrier.c
    $(CC) -c $<

condvar.o: pthread/condvar.c
    $(CC) -c $<

mutex.o: pthread/mutex.c
    $(CC) -c $<

pthread.o: pthread/pthread.c
    $(CC) -c $<

rwlock.o: pthread/rwlock.c
    $(CC) -c $<

spinlock.o: pthread/spinlock.c
    $(CC) -c $<

setjmp.o: setjmp.s
    $(CC) -c $<

chkstk.o: chkstk.s
    $(CC) -c $<

acos.o: math/acos.asm
    $(AS) $** -o $@

asin.o: math/asin.asm
    $(AS) $** -o $@

atan.o: math/atan.asm
    $(AS) $** -o $@

atan2.o: math/atan2.asm
    $(AS) $** -o $@

ceil.o: math/ceil.asm
    $(AS) $** -o $@

cos.o: math/cos.asm
    $(AS) $** -o $@

cosh.o: math/cosh.asm
    $(AS) $** -o $@

exp.o: math/exp.asm
    $(AS) $** -o $@

fabs.o: math/fabs.asm
    $(AS) $** -o $@

floor.o: math/floor.asm
    $(AS) $** -o $@

fmod.o: math/fmod.asm
    $(AS) $** -o $@

fpconst.o: math/fpconst.asm
    $(AS) $** -o $@

fpreset.o: math/fpreset.asm
    $(AS) $** -o $@

frexp.o: math/frexp.asm
    $(AS) $** -o $@

ftol.o: math/ftol.asm
    $(AS) $** -o $@

ldexp.o: math/ldexp.asm
    $(AS) $** -o $@

log.o: math/log.asm
    $(AS) $** -o $@

log10.o: math/log10.asm
    $(AS) $** -o $@

modf.o: math/modf.asm
    $(AS) $** -o $@

pow.o: math/pow.asm
    $(AS) $** -o $@

sin.o: math/sin.asm
    $(AS) $** -o $@

sinh.o: math/sinh.asm
    $(AS) $** -o $@

sqrt.o: math/sqrt.asm
    $(AS) $** -o $@

tan.o: math/tan.asm
    $(AS) $** -o $@

tanh.o: math/tanh.asm
    $(AS) $** -o $@

os.def: /boot/os.dll
    impdef $^ $@

install: libc.a os.def
    sh cp libc.a /usr/lib/libc.a
    sh cp os.def /usr/lib/os.def

clean:
    sh rm libc.a os.def $(OBJ) $(MATHOBJ)
