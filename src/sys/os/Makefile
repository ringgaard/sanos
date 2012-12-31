#
# Makefile for os.dll
#

SRCS=\
  critsect.c \
  environ.c \
  heap.c \
  netdb.c \
  os.c \
  resolv.c \
  signal.c \
  sntp.c \
  sysapi.c \
  syserr.c \
  syslog.c \
  thread.c \
  tls.c \
  userdb.c

LIB_SRCS=\
  /usr/src/lib/bitops.c \
  /usr/src/lib/crypt.c \
  /usr/src/lib/ctype.c \
  /usr/src/lib/fcvt.c \
  /usr/src/lib/inifile.c \
  /usr/src/lib/moddb.c \
  /usr/src/lib/opts.c \
  /usr/src/lib/strftime.c \
  /usr/src/lib/string.c \
  /usr/src/lib/strtol.c \
  /usr/src/lib/tcccrt.c \
  /usr/src/lib/time.c \
  /usr/src/lib/verinfo.c \
  /usr/src/lib/vsprintf.c

LIB_OBJS=modf.o

all: os.dll

os.dll: $(SRCS) $(LIB_SRCS) $(LIB_OBJS)
    $(CC) -shared -entry _start@12 -fixed 0x7FF00000 -nostdlib -o os.dll -def os.def $(SRCS) $(LIB_SRCS) $(LIB_OBJS) -D OS_LIB

modf.o: /usr/src/lib/math/modf.asm
    $(AS) $** -o $@

install: os.dll
    cp os.dll /boot/os.dll
    cp os.def /usr/lib/os.def

clean:
    rm $(LIB_OBJS) os.dll os.def

