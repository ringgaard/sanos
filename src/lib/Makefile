#
# Makefile for C runtime library
#

OBJ=tcccrt.o assert.o bsearch.o conio.o crt0.o ctype.o dirent.o fcvt.o fork.o getopt.o glob.o hash.o inifile.o input.o \
    math.o opts.o output.o qsort.o random.o readline.o rmap.o rtttl.o sched.o semaphore.o stdio.o stdlib.o strftime.o \
    string.o strtod.o strtol.o time.o xtoa.o \
    regcomp.o regexec.o regerror.o regfree.o \
    barrier.o condvar.o mutex.o pthread.o rwlock.o spinlock.o \
    setjmp.o chkstk.o

all: libc.a

libc.a: $(OBJ) /usr/lib/libm.a
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

install: libc.a
    sh cp libc.a /usr/lib/libc.a

clean:
    sh rm libc.a $(OBJ)
