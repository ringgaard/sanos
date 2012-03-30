#
# Makefile for os.dll
#

OBJS= critsect.o environ.o heap.o netdb.o os.o resolv.o signal.o sntp.o sysapi.o syserr.o syslog.o thread.o tls.o userdb.o \
      string.o inifile.o opts.o moddb.o verinfo.o bitops.o crypt.o time.o strtol.o ctype.o vsprintf.o fcvt.o modf.o tcccrt.o

all: os.dll

os.dll: $(OBJS)
    $(CC) -shared -entry _start@12 -fixed 0x7FF00000 -nostdlib -o os.dll $(OBJS)

critsect.o: critsect.c
    $(CC) -c $** -o $@ -D OS_LIB

environ.o: environ.c
    $(CC) -c $** -o $@ -D OS_LIB

heap.o: heap.c
    $(CC) -c $** -o $@ -D OS_LIB

netdb.o: netdb.c
    $(CC) -c $** -o $@ -D OS_LIB

os.o: os.c
    $(CC) -c $** -o $@ -D OS_LIB

resolv.o: resolv.c
    $(CC) -c $** -o $@ -D OS_LIB

signal.o: signal.c
    $(CC) -c $** -o $@ -D OS_LIB

sntp.o: sntp.c
    $(CC) -c $** -o $@ -D OS_LIB

sysapi.o: sysapi.c
    $(CC) -c $** -o $@ -D OS_LIB

syserr.o: syserr.c
    $(CC) -c $** -o $@ -D OS_LIB

syslog.o: syslog.c
    $(CC) -c $** -o $@ -D OS_LIB

thread.o: thread.c
    $(CC) -c $** -o $@ -D OS_LIB

tls.o: tls.c
    $(CC) -c $** -o $@ -D OS_LIB

userdb.o: userdb.c
    $(CC) -c $** -o $@ -D OS_LIB

moddb.o: /usr/src/lib/moddb.c
    $(CC) -c $** -o $@ -D OS_LIB

string.o: /usr/src/lib/string.c
    $(CC) -c $** -o $@ -D OS_LIB

inifile.o: /usr/src/lib/inifile.c
    $(CC) -c $** -o $@ -D OS_LIB

opts.o: /usr/src/lib/opts.c
    $(CC) -c $** -o $@ -D OS_LIB

verinfo.o: /usr/src/lib/verinfo.c
    $(CC) -c $** -o $@ -D OS_LIB

bitops.o: /usr/src/lib/bitops.c
    $(CC) -c $** -o $@ -D OS_LIB

crypt.o: /usr/src/lib/crypt.c
    $(CC) -c $** -o $@ -D OS_LIB

time.o: /usr/src/lib/time.c
    $(CC) -c $** -o $@ -D OS_LIB

strtol.o: /usr/src/lib/strtol.c
    $(CC) -c $** -o $@ -D OS_LIB

ctype.o: /usr/src/lib/ctype.c
    $(CC) -c $** -o $@ -D OS_LIB

vsprintf.o: /usr/src/lib/vsprintf.c
    $(CC) -c $** -o $@ -D OS_LIB

fcvt.o: /usr/src/lib/fcvt.c
    $(CC) -c $** -o $@ -D OS_LIB

modf.o: /usr/src/lib/math/modf.asm
    $(AS) $** -o $@

tcccrt.o: /usr/src/lib/tcccrt.c
    $(CC) -c $** -o $@ -D OS_LIB

install: os.dll
    sh cp os.dll /boot/os.dll

clean:
    sh rm $(OBJS) os.dll os.def
