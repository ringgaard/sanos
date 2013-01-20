#
# Makefile for C runtime library
#

OBJ=tcccrt.o assert.o bsearch.o conio.o crt0.o ctype.o dirent.o fcvt.o fnmatch.o \
    fork.o getopt.o glob.o hash.o inifile.o input.o mman.o math.o opts.o output.o \
    qsort.o random.o readline.o rmap.o rtttl.o sched.o semaphore.o \
    stdio.o scanf.o shlib.o printf.o tmpfile.o popen.o stdlib.o strftime.o \
    string.o strtod.o strtol.o termios.o time.o xtoa.o \
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
    $(CC) -c $< -g

assert.o: assert.c
    $(CC) -c $< -g

bsearch.o: bsearch.c
    $(CC) -c $< -g

conio.o: conio.c
    $(CC) -c $< -g

crt0.o: crt0.c
    $(CC) -c $< -g

ctype.o: ctype.c
    $(CC) -c $< -g

dirent.o: dirent.c
    $(CC) -c $< -g

fcvt.o: fcvt.c
    $(CC) -c $< -g

fnmatch.o: fnmatch.c
    $(CC) -c $< -g

fork.o: fork.c
    $(CC) -c $< -g

getopt.o: getopt.c
    $(CC) -c $< -g

glob.o: glob.c
    $(CC) -c $< -g

hash.o: hash.c
    $(CC) -c $< -g

inifile.o: inifile.c
    $(CC) -c $< -g

input.o: input.c
    $(CC) -c $< -g

math.o: math.c
    $(CC) -c $< -g

mman.o: mman.c
    $(CC) -c $< -g

opts.o: opts.c
    $(CC) -c $< -g

output.o: output.c
    $(CC) -c $< -g

qsort.o: qsort.c
    $(CC) -c $< -g

random.o: random.c
    $(CC) -c $< -g

readline.o: readline.c
    $(CC) -c $< -g

rmap.o: rmap.c
    $(CC) -c $< -g

rtttl.o: rtttl.c
    $(CC) -c $< -g

sched.o: sched.c
    $(CC) -c $< -g

semaphore.o: semaphore.c
    $(CC) -c $< -g

stdio.o: stdio.c
    $(CC) -c $< -g

shlib.o: shlib.c
    $(CC) -c $< -g

printf.o: printf.c
    $(CC) -c $< -g

scanf.o: scanf.c
    $(CC) -c $< -g

tmpfile.o: tmpfile.c
    $(CC) -c $< -g

popen.o: popen.c
    $(CC) -c $< -g

setjmp.o: setjmp.c
    $(CC) -c $< -g

stdlib.o: stdlib.c
    $(CC) -c $< -g

strftime.o: strftime.c
    $(CC) -c $< -g

string.o: string.c
    $(CC) -c $< -g

strtod.o: strtod.c
    $(CC) -c $< -g

strtol.o: strtol.c
    $(CC) -c $< -g

termios.o: termios.c
    $(CC) -c $< -g

time.o: time.c
    $(CC) -c $< -g

xtoa.o: xtoa.c
    $(CC) -c $< -g

regcomp.o: regex/regcomp.c
    $(CC) -c $< -g

regexec.o: regex/regexec.c
    $(CC) -c $< -g

regerror.o: regex/regerror.c
    $(CC) -c $< -g

regfree.o: regex/regfree.c
    $(CC) -c $< -g

barrier.o: pthread/barrier.c
    $(CC) -c $< -g

condvar.o: pthread/condvar.c
    $(CC) -c $< -g

mutex.o: pthread/mutex.c
    $(CC) -c $< -g

pthread.o: pthread/pthread.c
    $(CC) -c $< -g

rwlock.o: pthread/rwlock.c
    $(CC) -c $< -g

spinlock.o: pthread/spinlock.c
    $(CC) -c $< -g

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
    cp libc.a /usr/lib/libc.a
    cp os.def /usr/lib/os.def

clean:
    rm libc.a os.def $(OBJ) $(MATHOBJ)
