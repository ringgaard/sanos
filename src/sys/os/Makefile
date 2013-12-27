#
# Makefile for os.dll
#

LIB=$(ROOT)/usr/src/lib

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
  $(LIB)/bitops.c \
  $(LIB)/crypt.c \
  $(LIB)/ctype.c \
  $(LIB)/fcvt.c \
  $(LIB)/inifile.c \
  $(LIB)/moddb.c \
  $(LIB)/opts.c \
  $(LIB)/strftime.c \
  $(LIB)/string.c \
  $(LIB)/strtol.c \
  $(LIB)/tcccrt.c \
  $(LIB)/time.c \
  $(LIB)/verinfo.c \
  $(LIB)/vsprintf.c

LIB_OBJS=modf.o

all: os.dll

os.dll: $(SRCS) $(LIB_SRCS) $(LIB_OBJS)
    $(CC) -shared -entry _start@12 -fixed 0x7FF00000 -nostdlib -o os.dll -def os.def $(SRCS) $(LIB_SRCS) $(LIB_OBJS) -D OS_LIB

modf.o: $(LIB)/math/modf.asm
    $(AS) $** -o $@

install: os.dll
    cp os.dll $(ROOT)/boot/os.dll
    cp os.def $(ROOT)/usr/lib/os.def

cross: install

clean:
    rm $(LIB_OBJS) os.dll os.def

