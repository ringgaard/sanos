#
# Makefile for os.dll
#

OBJS=environ.o heap.o netdb.o os.o resolv.o sntp.o syserr.o syslog.o userdb.o moddb.o verinfo.o

# critsect.o signal.o sysapi.o thread.o tls.o

all: os.dll

os.dll: $(OBJS)
    $(CC) -shared -o os.dll $(OBJS)

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

verinfo.o: /usr/src/lib/verinfo.c
    $(CC) -c $** -o $@ -D OS_LIB

install: os.dll
    sh cp os.dll /boot/os.dll

clean:
    sh rm os.dll
