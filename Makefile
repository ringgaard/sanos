#
# Makefile for sanos
#

TOPDIR=.

BIN=$(TOPDIR)\bin
BUILD=$(TOPDIR)\build
IMG=$(TOPDIR)\img
LIBS=$(TOPDIR)\lib
OBJ=$(TOPDIR)\obj
SRC=$(TOPDIR)\src
TOOLS=$(TOPDIR)\tools
TOOLSRC=$(TOPDIR)\utils

NASM=$(TOOLS)\nasmw.exe
MKISOFS=$(TOOLS)\mkisofs.exe

MKDFS=$(TOOLS)\mkdfs.exe
MKFLOPPY=$(TOOLS)\mkfloppy.exe
MKPART=$(TOOLS)\mkpart.exe
DBGGW=$(TOOLS)\dbggw.exe

AR=lib

AFLAGS=/nologo
CFLAGS=/nologo /O2 /Og /Ob1 /Oi /Ot /Oy /X /GF /Gy /W3 /I $(SRC)/include

#
# /nologo               Suppress copyright message
# /O2                   Maximize speed
# /Og                   Enable global optimization
# /Ob1                  Inline expansion (1 level)
# /Oi                   Enable intrinsic functions 
# /Ot                   Favor code speed
# /Oy                   Enable frame pointer omission
# /X                    Ignore "standard places"
# /GF                   Enable read-only string pooling
# /Gy                   Separate functions for linker
# /W3                   Set warning level 3
# /I $(SRC)/include     Include search path
#

!INCLUDE $(BUILD)/sanos.dep

all: dirs tools sanos bootdisk netbootimg bootcd

sanos: dirs kernel drivers libc win32 utils

#
# dirs
#

dirs:
    -@if not exist $(BIN) mkdir $(BIN)
    -@if not exist $(OBJ) mkdir $(OBJ)
    -@if not exist $(IMG) mkdir $(IMG)
    -@if not exist $(LIBS) mkdir $(LIBS)
    -@if not exist $(OBJ)\3c905c mkdir $(OBJ)\3c905c
    -@if not exist $(OBJ)\advapi32 mkdir $(OBJ)\advapi32
    -@if not exist $(OBJ)\boot mkdir $(OBJ)\boot
    -@if not exist $(OBJ)\eepro100 mkdir $(OBJ)\eepro100
    -@if not exist $(OBJ)\edit mkdir $(OBJ)\edit
    -@if not exist $(OBJ)\fdisk mkdir $(OBJ)\fdisk
    -@if not exist $(OBJ)\httpd mkdir $(OBJ)\httpd
    -@if not exist $(OBJ)\jinit mkdir $(OBJ)\jinit
    -@if not exist $(OBJ)\kernel32 mkdir $(OBJ)\kernel32
    -@if not exist $(OBJ)\krnl mkdir $(OBJ)\krnl
    -@if not exist $(OBJ)\libc mkdir $(OBJ)\libc
    -@if not exist $(OBJ)\msvcrt mkdir $(OBJ)\msvcrt
    -@if not exist $(OBJ)\ne2000 mkdir $(OBJ)\ne2000
    -@if not exist $(OBJ)\os mkdir $(OBJ)\os 
    -@if not exist $(OBJ)\osldr mkdir $(OBJ)\osldr
    -@if not exist $(OBJ)\pcnet32 mkdir $(OBJ)\pcnet32
    -@if not exist $(OBJ)\rtl8139 mkdir $(OBJ)\rtl8139
    -@if not exist $(OBJ)\setup mkdir $(OBJ)\setup
    -@if not exist $(OBJ)\sh mkdir $(OBJ)\sh
    -@if not exist $(OBJ)\sis900 mkdir $(OBJ)\sis900
    -@if not exist $(OBJ)\telnetd mkdir $(OBJ)\telnetd
    -@if not exist $(OBJ)\user32 mkdir $(OBJ)\user32
    -@if not exist $(OBJ)\winmm mkdir $(OBJ)\winmm
    -@if not exist $(OBJ)\wsock32 mkdir $(OBJ)\wsock32
    -@if not exist $(TOOLS) mkdir $(TOOLS)
    -@if not exist $(TOOLSRC)\mkfloppy\release mkdir $(TOOLSRC)\mkfloppy\release
    -@if not exist $(TOOLSRC)\mkpart\release mkdir $(TOOLSRC)\mkpart\release
    -@if not exist $(TOOLSRC)\dbggw\release mkdir $(TOOLSRC)\dbggw\release
    -@if not exist $(TOOLSRC)\dfs\release mkdir $(TOOLSRC)\dfs\release

#
# clean
#

clean:
    del /Q $(BIN)
    del /Q /S $(OBJ)
    del /Q $(MKDFS)
    del /Q $(MKFLOPPY)
    del /Q $(MKPART)
    del /Q $(DBGGW)
    del /Q $(TOOLSRC)\mkfloppy\release
    del /Q $(TOOLSRC)\mkpart\release
    del /Q $(TOOLSRC)\dbggw\release
    del /Q $(TOOLSRC)\dfs\release
    del /Q $(IMG)\bootdisk.img
    del /Q $(IMG)\sanos.0
    del /Q $(IMG)\sanos.iso

#
# tools
#

WIN32CFLAGS=/nologo /O2 /Ob1 /Oy /GF /ML /Gy /W3 /TC /D WIN32 /D NDEBUG /D _CONSOLE /D _MBCS

tools: $(MKDFS) $(MKFLOPPY) $(MKPART) $(DBGGW)

$(MKFLOPPY): $(TOOLSRC)/mkfloppy/mkfloppy.c
    $(CC) $(WIN32CFLAGS) /Fe$@ /Fo$(TOOLSRC)/mkfloppy/release/ $**

$(MKPART): $(TOOLSRC)/mkpart/mkpart.c
    $(CC) $(WIN32CFLAGS) /Fe$@ /Fo$(TOOLSRC)/mkpart/release/ $**

$(DBGGW): $(TOOLSRC)/dbggw/dbggw.c $(TOOLSRC)/dbggw/rdp.c
    $(CC) $(WIN32CFLAGS) /I$(SRC)/include/os /Fe$@ /Fo$(TOOLSRC)/dbggw/release/ $** /link wsock32.lib

$(MKDFS): \
  $(TOOLSRC)/dfs/bitops.c \
  $(TOOLSRC)/dfs/buf.c \
  $(TOOLSRC)/dfs/dfs.c \
  $(TOOLSRC)/dfs/dir.c \
  $(TOOLSRC)/dfs/file.c \
  $(TOOLSRC)/dfs/getopt.c \
  $(TOOLSRC)/dfs/group.c \
  $(TOOLSRC)/dfs/inode.c \
  $(TOOLSRC)/dfs/mkdfs.c \
  $(TOOLSRC)/dfs/super.c \
  $(TOOLSRC)/dfs/vfs.c
    $(CC) $(WIN32CFLAGS) /Fe$@ /Fo$(TOOLSRC)/dfs/release/ $**

#
# kernel
#

kernel: dirs boot $(BIN)/krnl.dll $(BIN)/os.dll

boot: dirs $(BIN)/boot $(BIN)/cdboot $(BIN)/cdemboot $(BIN)/netboot $(BIN)/osldr.dll

$(BIN)/boot: $(SRC)/sys/boot/boot.asm
    $(NASM) -f bin $** -o $@

$(BIN)/cdboot: $(SRC)/sys/boot/cdboot.asm
    $(NASM) -f bin $** -o $@

$(BIN)/cdemboot: $(SRC)/sys/boot/cdemboot.asm
    $(NASM) -f bin $** -o $@

$(BIN)/netboot: $(SRC)/sys/boot/netboot.asm
    $(NASM) -f bin $** -o $@

$(OBJ)/boot/ldrinit.exe: $(SRC)/sys/boot/ldrinit.asm
    $(NASM) -f bin $** -o $@

OSLDRSRC=$(SRC)\sys\osldr\video.c $(SRC)\sys\osldr\osldr.c $(SRC)\sys\osldr\loadkrnl.c $(SRC)\sys\osldr\boothd.c $(SRC)\sys\osldr\bootfd.c $(SRC)\sys\osldr\unzip.c $(SRC)\lib\vsprintf.c 

$(BIN)/osldr.dll: $(OSLDRSRC) $(OBJ)\boot\ldrinit.exe
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/osldr/ $(OSLDRSRC) /D KERNEL /D OSLDR /link /DLL /NODEFAULTLIB /OPT:WIN98 /ENTRY:start /BASE:0x00090000 /FIXED /STUB:$(OBJ)\boot\ldrinit.exe

$(OBJ)/krnl/lldiv.obj: $(SRC)/lib/lldiv.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/krnl/llmul.obj: $(SRC)/lib/llmul.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(LIBS)/krnl.lib $(BIN)/krnl.dll: \
  $(SRC)\sys\krnl\vmm.c \
  $(SRC)\sys\krnl\vfs.c \
  $(SRC)\sys\krnl\trap.c \
  $(SRC)\sys\krnl\timer.c \
  $(SRC)\sys\krnl\syscall.c \
  $(SRC)\sys\krnl\start.c \
  $(SRC)\sys\krnl\sched.c \
  $(SRC)\sys\krnl\queue.c \
  $(SRC)\sys\krnl\pnpbios.c \
  $(SRC)\sys\krnl\pit.c \
  $(SRC)\sys\krnl\pic.c \
  $(SRC)\sys\krnl\pframe.c \
  $(SRC)\sys\krnl\pdir.c \
  $(SRC)\sys\krnl\pci.c \
  $(SRC)\sys\krnl\object.c \
  $(SRC)\sys\krnl\ldr.c \
  $(SRC)\sys\krnl\kmem.c \
  $(SRC)\sys\krnl\kmalloc.c \
  $(SRC)\sys\krnl\iovec.c \
  $(SRC)\sys\krnl\iop.c \
  $(SRC)\sys\krnl\iomux.c \
  $(SRC)\sys\krnl\hndl.c \
  $(SRC)\sys\krnl\fpu.c \
  $(SRC)\sys\krnl\dev.c \
  $(SRC)\sys\krnl\dbg.c \
  $(SRC)\sys\krnl\cpu.c \
  $(SRC)\sys\krnl\buf.c \
  $(SRC)\sys\krnl\apm.c \
  $(SRC)\sys\dev\video.c \
  $(SRC)\sys\dev\serial.c \
  $(SRC)\sys\dev\rnd.c \
  $(SRC)\sys\dev\ramdisk.c \
  $(SRC)\sys\dev\nvram.c \
  $(SRC)\sys\dev\null.c \
  $(SRC)\sys\dev\klog.c \
  $(SRC)\sys\dev\kbd.c \
  $(SRC)\sys\dev\hd.c \
  $(SRC)\sys\dev\fd.c \
  $(SRC)\sys\dev\cons.c \
  $(SRC)\sys\net\udpsock.c \
  $(SRC)\sys\net\udp.c \
  $(SRC)\sys\net\rawsock.c \
  $(SRC)\sys\net\raw.c \
  $(SRC)\sys\net\tcpsock.c \
  $(SRC)\sys\net\tcp_output.c \
  $(SRC)\sys\net\tcp_input.c \
  $(SRC)\sys\net\tcp.c \
  $(SRC)\sys\net\stats.c \
  $(SRC)\sys\net\socket.c \
  $(SRC)\sys\net\pbuf.c \
  $(SRC)\sys\net\netif.c \
  $(SRC)\sys\net\loopif.c \
  $(SRC)\sys\net\ipaddr.c \
  $(SRC)\sys\net\ip.c \
  $(SRC)\sys\net\inet.c \
  $(SRC)\sys\net\icmp.c \
  $(SRC)\sys\net\ether.c \
  $(SRC)\sys\net\dhcp.c \
  $(SRC)\sys\net\arp.c \
  $(SRC)\sys\fs\cdfs\cdfs.c \
  $(SRC)\sys\fs\pipefs\pipefs.c \
  $(SRC)\sys\fs\smbfs\smbutil.c \
  $(SRC)\sys\fs\smbfs\smbproto.c \
  $(SRC)\sys\fs\smbfs\smbfs.c \
  $(SRC)\sys\fs\smbfs\smbcache.c \
  $(SRC)\sys\fs\procfs\procfs.c \
  $(SRC)\sys\fs\devfs\devfs.c \
  $(SRC)\sys\fs\dfs\super.c \
  $(SRC)\sys\fs\dfs\inode.c \
  $(SRC)\sys\fs\dfs\group.c \
  $(SRC)\sys\fs\dfs\file.c \
  $(SRC)\sys\fs\dfs\dir.c \
  $(SRC)\sys\fs\dfs\dfs.c \
  $(SRC)\lib\vsprintf.c \
  $(SRC)\lib\time.c \
  $(SRC)\lib\strtol.c \
  $(SRC)\lib\string.c \
  $(SRC)\lib\rmap.c \
  $(SRC)\lib\opts.c \
  $(SRC)\lib\moddb.c \
  $(SRC)\lib\inifile.c \
  $(SRC)\lib\ctype.c \
  $(SRC)\lib\bitops.c \
  $(OBJ)/krnl/lldiv.obj \
  $(OBJ)/krnl/llmul.obj
    $(CC) $(CFLAGS) /Fe$(BIN)/krnl.dll /Fo$(OBJ)/krnl/ $** /D KERNEL /D KRNL_LIB \
      /link /DLL /LARGEADDRESSAWARE /NODEFAULTLIB /OPT:WIN98 /ENTRY:start \
      /BASE:0x80000000 /FIXED /IMPLIB:$(LIBS)/krnl.lib

$(OBJ)/os/modf.obj: $(SRC)/lib/math/modf.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/os/ftol.obj: $(SRC)/lib/math/ftol.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/os/fpconst.obj: $(SRC)/lib/math/fpconst.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(LIBS)/os.lib $(BIN)/os.dll: \
  $(SRC)/sys/os/tls.c \
  $(SRC)/sys/os/thread.c \
  $(SRC)/sys/os/sysapi.c \
  $(SRC)/sys/os/syslog.c \
  $(SRC)/sys/os/syserr.c \
  $(SRC)/sys/os/sntp.c \
  $(SRC)/sys/os/signal.c \
  $(SRC)/sys/os/resolv.c \
  $(SRC)/sys/os/os.c \
  $(SRC)/sys/os/netdb.c \
  $(SRC)/sys/os/heap.c \
  $(SRC)/sys/os/critsect.c \
  $(SRC)/lib/vsprintf.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/stdlib.c \
  $(SRC)/lib/time.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/moddb.c \
  $(SRC)/lib/inifile.c \
  $(SRC)/lib/fcvt.c \
  $(SRC)/lib/ctype.c \
  $(SRC)/lib/bitops.c \
  $(OBJ)/os/modf.obj \
  $(OBJ)/os/ftol.obj \
  $(OBJ)/os/fpconst.obj
    $(CC) $(CFLAGS) /Fe$(BIN)/os.dll /Fo$(OBJ)/os/ $** /D OS_LIB \
      /link /DLL /NODEFAULTLIB /OPT:WIN98 /ENTRY:start /BASE:0x7FF00000 /FIXED /IMPLIB:$(LIBS)/os.lib

#
# drivers
#

drivers: dirs $(BIN)/3c905c.sys $(BIN)/eepro100.sys $(BIN)/ne2000.sys $(BIN)/pcnet32.sys $(BIN)/rtl8139.sys $(BIN)/sis900.sys

$(BIN)/3c905c.sys: \
  $(SRC)/sys/dev/3c905c.c \
  $(SRC)/lib/string.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/3c905c/ $** /D KERNEL /D 3C905C_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/3c905c.lib

$(BIN)/eepro100.sys: \
  $(SRC)/sys/dev/eepro100.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/eepro100/ $** /D KERNEL /D EEPRO100_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/eepro100.lib

$(BIN)/ne2000.sys: \
  $(SRC)/sys/dev/ne2000.c \
  $(SRC)/sys/krnl/iop.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/ne2000/ $** /D KERNEL /D NE2000_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/ne2000.lib

$(BIN)/pcnet32.sys: \
  $(SRC)/sys/dev/pcnet32.c \
  $(SRC)/sys/krnl/iop.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/pcnet32/ $** /D KERNEL /D PCNET32_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/pcnet32.lib

$(BIN)/rtl8139.sys: \
  $(SRC)/sys/dev/rtl8139.c \
  $(SRC)/sys/krnl/iop.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/pcnet32/ $** /D KERNEL /D RTL8139_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/rtl8139.lib

$(BIN)/sis900.sys: \
  $(SRC)/sys/dev/sis900.c \
  $(SRC)/sys/krnl/iop.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/pcnet32/ $** /D KERNEL /D SIS900_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/sis900.lib

#
# libc
#

libc: dirs $(LIBS)/libc.lib

$(OBJ)/libc/ullshr.obj: $(SRC)/lib/ullshr.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/ullrem.obj: $(SRC)/lib/ullrem.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/ulldvrm.obj: $(SRC)/lib/ulldvrm.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/ulldiv.obj: $(SRC)/lib/ulldiv.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/llshr.obj: $(SRC)/lib/llshr.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/llshl.obj: $(SRC)/lib/llshl.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/llrem.obj: $(SRC)/lib/llrem.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/llmul.obj: $(SRC)/lib/llmul.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/lldvrm.obj: $(SRC)/lib/lldvrm.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/lldiv.obj: $(SRC)/lib/lldiv.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/chkstk.obj: $(SRC)/lib/chkstk.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/tanh.obj: $(SRC)/lib/math/tanh.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/tan.obj: $(SRC)/lib/math/tan.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/sqrt.obj: $(SRC)/lib/math/sqrt.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/sinh.obj: $(SRC)/lib/math/sinh.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/sin.obj: $(SRC)/lib/math/sin.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/pow.obj: $(SRC)/lib/math/pow.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/modf.obj: $(SRC)/lib/math/modf.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/log10.obj: $(SRC)/lib/math/log10.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/log.obj: $(SRC)/lib/math/log.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/ldexp.obj: $(SRC)/lib/math/ldexp.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/ftol.obj: $(SRC)/lib/math/ftol.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/frexp.obj: $(SRC)/lib/math/frexp.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/fpreset.obj: $(SRC)/lib/math/fpreset.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/fpconst.obj: $(SRC)/lib/math/fpconst.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/fmod.obj: $(SRC)/lib/math/fmod.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/floor.obj: $(SRC)/lib/math/floor.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/fabs.obj: $(SRC)/lib/math/fabs.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/exp.obj: $(SRC)/lib/math/exp.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/cosh.obj: $(SRC)/lib/math/cosh.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/cos.obj: $(SRC)/lib/math/cos.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/ceil.obj: $(SRC)/lib/math/ceil.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/atan2.obj: $(SRC)/lib/math/atan2.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/atan.obj: $(SRC)/lib/math/atan.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/asin.obj: $(SRC)/lib/math/asin.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/acos.obj: $(SRC)/lib/math/acos.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/libc/xtoa.obj: $(SRC)/lib/xtoa.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/time.obj: $(SRC)/lib/time.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/strtol.obj: $(SRC)/lib/strtol.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/strtod.obj: $(SRC)/lib/strtod.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/string.obj: $(SRC)/lib/string.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/strftime.obj: $(SRC)/lib/strftime.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/stdlib.obj: $(SRC)/lib/stdlib.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/stdio.obj: $(SRC)/lib/stdio.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/setjmp.obj: $(SRC)/lib/setjmp.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/rtttl.obj: $(SRC)/lib/rtttl.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/rmap.obj: $(SRC)/lib/rmap.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/readline.obj: $(SRC)/lib/readline.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/random.obj: $(SRC)/lib/random.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/qsort.obj: $(SRC)/lib/qsort.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/output.obj: $(SRC)/lib/output.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/opts.obj: $(SRC)/lib/opts.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/new.obj: $(SRC)/lib/new.cpp
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/input.obj: $(SRC)/lib/input.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/inifile.obj: $(SRC)/lib/inifile.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/hash.obj: $(SRC)/lib/hash.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/fcvt.obj: $(SRC)/lib/fcvt.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/ctype.obj: $(SRC)/lib/ctype.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/crt0.obj: $(SRC)/lib/crt0.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/conio.obj: $(SRC)/lib/conio.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/bsearch.obj: $(SRC)/lib/bsearch.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/bitops.obj: $(SRC)/lib/bitops.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/assert.obj: $(SRC)/lib/assert.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/sched.obj: $(SRC)/lib/sched.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/semaphore.obj: $(SRC)/lib/semaphore.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/barrier.obj: $(SRC)/lib/pthread/barrier.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/condvar.obj: $(SRC)/lib/pthread/condvar.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/mutex.obj: $(SRC)/lib/pthread/mutex.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/pthread.obj: $(SRC)/lib/pthread/pthread.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/rwlock.obj: $(SRC)/lib/pthread/rwlock.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/spinlock.obj: $(SRC)/lib/pthread/spinlock.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(LIBS)/libc.lib: \
  $(OBJ)/libc/spinlock.obj \
  $(OBJ)/libc/rwlock.obj \
  $(OBJ)/libc/pthread.obj \
  $(OBJ)/libc/mutex.obj \
  $(OBJ)/libc/condvar.obj \
  $(OBJ)/libc/barrier.obj \
  $(OBJ)/libc/xtoa.obj \
  $(OBJ)/libc/time.obj \
  $(OBJ)/libc/strtol.obj \
  $(OBJ)/libc/strtod.obj \
  $(OBJ)/libc/string.obj \
  $(OBJ)/libc/strftime.obj \
  $(OBJ)/libc/stdlib.obj \
  $(OBJ)/libc/stdio.obj \
  $(OBJ)/libc/setjmp.obj \
  $(OBJ)/libc/semaphore.obj \
  $(OBJ)/libc/sched.obj \
  $(OBJ)/libc/rtttl.obj \
  $(OBJ)/libc/rmap.obj \
  $(OBJ)/libc/readline.obj \
  $(OBJ)/libc/random.obj \
  $(OBJ)/libc/qsort.obj \
  $(OBJ)/libc/output.obj \
  $(OBJ)/libc/opts.obj \
  $(OBJ)/libc/input.obj \
  $(OBJ)/libc/inifile.obj \
  $(OBJ)/libc/hash.obj \
  $(OBJ)/libc/fcvt.obj \
  $(OBJ)/libc/ctype.obj \
  $(OBJ)/libc/crt0.obj \
  $(OBJ)/libc/conio.obj \
  $(OBJ)/libc/bsearch.obj \
  $(OBJ)/libc/bitops.obj \
  $(OBJ)/libc/assert.obj \
  $(OBJ)/libc/ullshr.obj \
  $(OBJ)/libc/ullrem.obj \
  $(OBJ)/libc/ulldvrm.obj \
  $(OBJ)/libc/ulldiv.obj \
  $(OBJ)/libc/llshr.obj \
  $(OBJ)/libc/llshl.obj \
  $(OBJ)/libc/llrem.obj \
  $(OBJ)/libc/llmul.obj \
  $(OBJ)/libc/lldvrm.obj \
  $(OBJ)/libc/lldiv.obj \
  $(OBJ)/libc/chkstk.obj \
  $(OBJ)/libc/tanh.obj \
  $(OBJ)/libc/tan.obj \
  $(OBJ)/libc/sqrt.obj \
  $(OBJ)/libc/sinh.obj \
  $(OBJ)/libc/sin.obj \
  $(OBJ)/libc/pow.obj \
  $(OBJ)/libc/modf.obj \
  $(OBJ)/libc/new.obj \
  $(OBJ)/libc/log10.obj \
  $(OBJ)/libc/log.obj \
  $(OBJ)/libc/ldexp.obj \
  $(OBJ)/libc/ftol.obj \
  $(OBJ)/libc/frexp.obj \
  $(OBJ)/libc/fpreset.obj \
  $(OBJ)/libc/fpconst.obj \
  $(OBJ)/libc/fmod.obj \
  $(OBJ)/libc/floor.obj \
  $(OBJ)/libc/fabs.obj \
  $(OBJ)/libc/exp.obj \
  $(OBJ)/libc/cosh.obj \
  $(OBJ)/libc/cos.obj \
  $(OBJ)/libc/ceil.obj \
  $(OBJ)/libc/atan2.obj \
  $(OBJ)/libc/atan.obj \
  $(OBJ)/libc/asin.obj \
  $(OBJ)/libc/acos.obj
    $(AR) /NOLOGO /NODEFAULTLIB /OUT:$(LIBS)/libc.lib $**

#
# win32
#

win32: dirs $(BIN)/kernel32.dll $(BIN)/user32.dll $(BIN)/advapi32.dll $(BIN)/wsock32.dll $(BIN)/winmm.dll $(BIN)/msvcrt.dll

$(LIBS)/kernel32.lib $(BIN)/kernel32.dll: \
  $(SRC)/win32/kernel32/kernel32.c \
  $(SRC)/win32/kernel32/kernel32.def \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$(BIN)/kernel32.dll /Fo$(OBJ)/kernel32/ $** /D KERNEL32_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:DllMain /IMPLIB:$(LIBS)/kernel32.lib

$(BIN)/user32.dll: \
  $(SRC)/win32/user32/user32.c \
  $(SRC)/win32/user32/user32.def \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/user32/ $** /D USER32_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:DllMain /IMPLIB:$(LIBS)/user32.lib

$(BIN)/advapi32.dll: \
  $(SRC)/win32/advapi32/advapi32.c \
  $(SRC)/win32/advapi32/advapi32.def \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/advapi32/ $** /D ADVAPI32_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:DllMain /IMPLIB:$(LIBS)/advapi32.lib

$(BIN)/wsock32.dll: \
  $(SRC)/win32/wsock32/wsock32.c \
  $(SRC)/win32/wsock32/wsock32.def \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/wsock32/ $** /D WSOCK32_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:DllMain /IMPLIB:$(LIBS)/wsock32.lib

$(BIN)/winmm.dll: \
  $(SRC)/win32/winmm/winmm.c \
  $(SRC)/win32/winmm/winmm.def \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/winmm/ $** /D WINMM_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:DllMain /IMPLIB:$(LIBS)/winmm.lib

$(OBJ)/msvcrt/modf.obj: $(SRC)/lib/math/modf.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/msvcrt/ftol.obj: $(SRC)/lib/math/ftol.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/msvcrt/fmod.obj: $(SRC)/lib/math/fmod.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/msvcrt/floor.obj: $(SRC)/lib/math/floor.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(BIN)/msvcrt.dll: \
  $(SRC)/win32/msvcrt/msvcrt.def \
  $(SRC)/win32/msvcrt/msvcrt.c \
  $(SRC)/win32/msvcrt/malloc.c \
  $(SRC)/win32/msvcrt/float.c \
  $(SRC)/win32/msvcrt/file.c \
  $(SRC)/win32/msvcrt/except.c \
  $(SRC)/lib/new.cpp \
  $(SRC)/lib/vsprintf.c \
  $(SRC)/lib/time.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/strftime.c \
  $(SRC)/lib/setjmp.c \
  $(SRC)/lib/qsort.c \
  $(SRC)/lib/inifile.c \
  $(SRC)/lib/fcvt.c \
  $(SRC)/lib/ctype.c \
  $(SRC)/lib/bsearch.c \
  $(OBJ)/msvcrt/modf.obj \
  $(OBJ)/msvcrt/ftol.obj \
  $(OBJ)/msvcrt/fmod.obj \
  $(OBJ)/msvcrt/floor.obj \
  $(LIBS)/os.lib \
  $(LIBS)/kernel32.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/msvcrt/ $** /D MSVCRT_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:dllmain /IMPLIB:$(LIBS)/msvcrt.lib

#
# utils
#

utils: dirs $(BIN)/sh.exe $(BIN)/edit.exe $(BIN)/fdisk.exe $(BIN)/setup.exe $(BIN)/jinit.exe $(BIN)/telnetd.exe $(BIN)/httpd.dll

$(BIN)/sh.exe: \
  $(SRC)/utils/sh/sh.c \
  $(SRC)/utils/sh/ping.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/sh/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/edit.exe: \
  $(SRC)/utils/edit/edit.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/fdisk/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/fdisk.exe: \
  $(SRC)/utils/fdisk/fdisk.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/fdisk/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/setup.exe: \
  $(SRC)/utils/setup/setup.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/setup/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/jinit.exe: \
  $(SRC)/utils/jinit/jinit.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/jinit/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/telnetd.exe: \
  $(SRC)/utils/telnetd/telnetd.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/fdisk/ $** /link /NODEFAULTLIB /FIXED:NO

$(OBJ)/httpd/httpd.res: $(SRC)/utils/httpd/httpd.rc
  $(RC) /d "NDEBUG" /l 0x406 /fo$@ $**

$(BIN)/httpd.dll: \
  $(SRC)/utils/httpd/httpd.c \
  $(SRC)/utils/httpd/hbuf.c \
  $(SRC)/utils/httpd/hfile.c \
  $(SRC)/utils/httpd/hlog.c \
  $(SRC)/utils/httpd/hutils.c \
  $(OBJ)/httpd/httpd.res \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/httpd/ $** /D HTTPD_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:DllMain /IMPLIB:$(LIBS)/httpd.lib

#
# bootdisk
#

bootdisk: dirs sanos $(MKDFS) $(BUILD)/bootdisk.lst
    $(MKDFS) -d $(IMG)/bootdisk.img -b $(BIN)\boot -l $(BIN)\osldr.dll -k $(BIN)\krnl.dll -c 1440 -i -f -S $(TOPDIR) -F $(BUILD)\bootdisk.lst

#
# netbootimg
#

netbootimg: dirs sanos $(MKDFS) $(BUILD)/bootnet.lst
    $(MKDFS) -d $(IMG)/sanos.0 -b $(BIN)\netboot -l $(BIN)\osldr.dll -k $(BIN)\krnl.dll -c 512 -I 8192 -i -f -S $(TOPDIR) -F $(BUILD)\bootnet.lst

#
# bootcd
#

bootcd: $(IMG)/sanos.iso

$(IMG)/sanos.iso: dirs sanos tools $(BUILD)/bootcd.lst
    if not exist $(IMG) mkdir $(IMG)
    if not exist $(IMG)\usr mkdir $(IMG)\usr
    if not exist $(IMG)\usr\bin mkdir $(IMG)\usr\bin
    if exist $(IMG)\sanos.iso del $(IMG)\sanos.iso
    copy $(BIN)\sh.exe       $(IMG)\usr\bin\sh.exe
    copy $(BIN)\httpd.dll    $(IMG)\usr\bin\httpd.dll
    copy $(BIN)\setup.exe    $(IMG)\usr\bin\setup.exe
    copy $(BIN)\edit.exe     $(IMG)\usr\bin\edit.exe
    copy $(BIN)\fdisk.exe    $(IMG)\usr\bin\fdisk.exe
    copy $(BIN)\jinit.exe    $(IMG)\usr\bin\jinit.exe
    copy $(BIN)\msvcrt.dll   $(IMG)\usr\bin\msvcrt.dll
    copy $(BIN)\kernel32.dll $(IMG)\usr\bin\kernel32.dll
    copy $(BIN)\user32.dll   $(IMG)\usr\bin\user32.dll
    copy $(BIN)\advapi32.dll $(IMG)\usr\bin\advapi32.dll
    copy $(BIN)\winmm.dll    $(IMG)\usr\bin\winmm.dll
    copy $(BIN)\wsock32.dll  $(IMG)\usr\bin\wsock32.dll
    $(MKDFS) -d $(IMG)\usr\BOOTIMG.BIN -b $(BIN)\cdboot -l $(BIN)\osldr.dll -k $(BIN)\krnl.dll -c 512 -I 8192 -i -f -S $(TOPDIR) -F $(BUILD)\bootcd.lst
    $(MKISOFS) -no-emul-boot -J -c BOOTCAT.BIN -b BOOTIMG.BIN -o $(IMG)\sanos.iso $(IMG)\usr
