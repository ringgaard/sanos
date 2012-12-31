#
# Makefile for osldr.dll
#

SRCS=osldr.c loadkrnl.c unzip.c
OBJS=bioscall.o 
LIBSRCS=/usr/src/lib/vsprintf.c /usr/src/lib/string.c

all: osldr.dll

osldr.dll: $(SRCS) $(OBJS) $(LIBSRCS) ldrinit.exe
    $(CC) -shared -entry _start@12 -fixed 0x00090000 -filealign 4096 -stub ldrinit.exe -nostdlib -o osldr.dll $(SRCS) $(OBJS) $(LIBSRCS) -D OSLDR -D KERNEL

bioscall.o: bioscall.asm
    $(AS) $** -o $@

ldrinit.exe: ldrinit.asm
    $(AS) -f bin $** -o $@

install: osldr.dll
    cp osldr.dll /boot/osldr.dll
    mkboot -l /boot/osldr.dll

clean:
    rm $(OBJS) osldr.dll ldrinit.exe

