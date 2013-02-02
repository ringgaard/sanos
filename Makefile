#
# Makefile for sanos
#

!IFNDEF TOPDIR
TOPDIR=.
!ENDIF

BIN=$(TOPDIR)\bin
BUILD=$(TOPDIR)\build
IMG=$(TOPDIR)\img
LIBS=$(TOPDIR)\lib
OBJ=$(TOPDIR)\obj
SRC=$(TOPDIR)\src
TOOLS=$(TOPDIR)\tools
TOOLSRC=$(TOPDIR)\utils
!IFNDEF INSTALL
INSTALL=$(TOPDIR)\install
!ENDIF

NASM=$(TOOLS)\nasm.exe
MKISOFS=$(TOOLS)\mkisofs.exe

MKDFS=$(TOOLS)\mkdfs.exe
MKFLOPPY=$(TOOLS)\mkfloppy.exe
MKPART=$(TOOLS)\mkpart.exe
SOW=$(TOOLS)\os.dll
DBGGW=$(TOOLS)\dbggw.exe
AR=lib

!IFDEF PRERELEASEBUILD
DEFS=/D PRERELEASE
!ELSEIFDEF RELEASEBUILD
DEFS=/D RELEASE
!ELSE
DEFS=
!ENDIF

# MSVC=7  for Microsoft Visual Studio.NET (2002)
# MSVC=8  for Microsoft Visual Visual Studio 2005
# MSVC=9  for Microsoft Visual Visual Studio 2008
# MSVC=10 for Microsoft Visual Visual Studio 2010

!IFNDEF MSVC
MSVC=7
!ENDIF

AFLAGS=/nologo

!IF $(MSVC) == 8 || $(MSVC) == 9 || $(MSVC) == 10
CFLAGS=/nologo /O2 /Ob1 /Oi /Ot /Oy /GS- /GR- /X /GF /Gy /W3 /I $(SRC)/include $(DEFS)
!ELSE
CFLAGS=/nologo /O2 /Og /Ob1 /Oi /Ot /Oy /X /GF /Gy /W3 /I $(SRC)/include $(DEFS)
!ENDIF

!IF $(MSVC) == 9 || $(MSVC) == 10
RAWIMGFLAGS=/FILEALIGN:4096
!ELSE
RAWIMGFLAGS=/OPT:WIN98
!ENDIF

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
# /GS-                  Disable security checks
# /GR-                  Disable runtime typeinfo
#

all: dirs tools sanos bootdisk boothd netbootimg bootcd

!INCLUDE $(BUILD)/sanos.dep

sanos: dirs kernel drivers libc win32 utils

#
# dirs
#

dirs:
    -@if not exist $(BIN) mkdir $(BIN)
    -@if not exist $(OBJ) mkdir $(OBJ)
    -@if not exist $(IMG) mkdir $(IMG)
    -@if not exist $(LIBS) mkdir $(LIBS)
    -@if not exist $(INSTALL) mkdir $(INSTALL)
    -@if not exist $(OBJ)\3c905c mkdir $(OBJ)\3c905c
    -@if not exist $(OBJ)\advapi32 mkdir $(OBJ)\advapi32
    -@if not exist $(OBJ)\eepro100 mkdir $(OBJ)\eepro100
    -@if not exist $(OBJ)\edit mkdir $(OBJ)\edit
    -@if not exist $(OBJ)\fdisk mkdir $(OBJ)\fdisk
    -@if not exist $(OBJ)\make mkdir $(OBJ)\make
    -@if not exist $(OBJ)\ar mkdir $(OBJ)\ar
    -@if not exist $(OBJ)\ctohtml mkdir $(OBJ)\ctohtml
    -@if not exist $(OBJ)\impdef mkdir $(OBJ)\impdef
    -@if not exist $(OBJ)\mkboot mkdir $(OBJ)\mkboot
    -@if not exist $(OBJ)\pkg mkdir $(OBJ)\pkg
    -@if not exist $(OBJ)\grep mkdir $(OBJ)\grep
    -@if not exist $(OBJ)\ping mkdir $(OBJ)\ping
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
    -@if not exist $(OBJ)\virtionet mkdir $(OBJ)\virtionet
    -@if not exist $(OBJ)\setup mkdir $(OBJ)\setup
    -@if not exist $(OBJ)\sh mkdir $(OBJ)\sh
    -@if not exist $(OBJ)\msh mkdir $(OBJ)\msh
    -@if not exist $(OBJ)\sis900 mkdir $(OBJ)\sis900
    -@if not exist $(OBJ)\tulip mkdir $(OBJ)\tulip
    -@if not exist $(OBJ)\edit mkdir $(OBJ)\edit
    -@if not exist $(OBJ)\less mkdir $(OBJ)\less
    -@if not exist $(OBJ)\telnetd mkdir $(OBJ)\telnetd
    -@if not exist $(OBJ)\ftpd mkdir $(OBJ)\ftpd
    -@if not exist $(OBJ)\login mkdir $(OBJ)\login
    -@if not exist $(OBJ)\user32 mkdir $(OBJ)\user32
    -@if not exist $(OBJ)\winmm mkdir $(OBJ)\winmm
    -@if not exist $(OBJ)\wsock32 mkdir $(OBJ)\wsock32
    -@if not exist $(TOOLS) mkdir $(TOOLS)
    -@if not exist $(TOOLSRC)\mkfloppy\release mkdir $(TOOLSRC)\mkfloppy\release
    -@if not exist $(TOOLSRC)\mkpart\release mkdir $(TOOLSRC)\mkpart\release
    -@if not exist $(TOOLSRC)\dbggw\release mkdir $(TOOLSRC)\dbggw\release
    -@if not exist $(TOOLSRC)\sow\release mkdir $(TOOLSRC)\sow\release
    -@if not exist $(TOOLSRC)\dfs\release mkdir $(TOOLSRC)\dfs\release

#
# clean
#

clean:
    -del /Q $(BIN)
    -del /Q /S $(OBJ)
    -del /Q /S $(INSTALL)
    -del /Q $(MKDFS)
    -del /Q $(MKFLOPPY)
    -del /Q $(MKPART)
    -del /Q $(DBGGW)
    -del /Q $(SOW)
    -del /Q $(TOOLSRC)\mkfloppy\release
    -del /Q $(TOOLSRC)\mkpart\release
    -del /Q $(TOOLSRC)\dbggw\release
    -del /Q $(TOOLSRC)\sow\release
    -del /Q $(TOOLSRC)\dfs\release
    -del /Q /S $(IMG)

#
# tools
#

#
# /nologo               Suppress copyright message
# /O2                   Maximize speed
# /Ob1                  Inline expansion (1 level)
# /Oy                   Enable frame pointer omission
# /Oi                   Enable intrinsic functions 
# /GF                   Enable read-only string pooling
# /GS-                  Disable security checks
# /GR-                  Disable runtime typeinfo
# /MT                   Link with multithreaded CRT
# /Gy                   Separate functions for linker
# /W3                   Set warning level 3
# /TC                   Compile all files as .c
# /I $(SRC)/include     Include search path
#

!IF $(MSVC) == 9 || $(MSVC) == 10
WIN32CFLAGS=/nologo /O2 /Ob1 /Oy /Oi /GF /GS- /GR- /MT /Gy /W3 /TC /D WIN32 /D NDEBUG /D _CONSOLE /D _MBCS /D _CRT_SECURE_NO_DEPRECATE /D _CRT_NONSTDC_NO_DEPRECATE /D _USE_32BIT_TIME_T
!ELSEIF $(MSVC) == 8
WIN32CFLAGS=/nologo /O2 /Ob1 /Oy /Oi /GF /GS- /GR- /MT /Gy /W3 /TC /D WIN32 /D NDEBUG /D _CONSOLE /D _MBCS /D _CRT_SECURE_NO_DEPRECATE /D _CRT_NONSTDC_NO_DEPRECATE /D _USE_32BIT_TIME_T
!ELSE
WIN32CFLAGS=/nologo /O2 /Ob1 /Oy /GF /ML /Gy /W3 /TC /D WIN32 /D NDEBUG /D _CONSOLE /D _MBCS
!ENDIF

tools: $(MKDFS) $(MKFLOPPY) $(MKPART) $(DBGGW) $(SOW)

$(MKFLOPPY): $(TOOLSRC)/mkfloppy/mkfloppy.c
    $(CC) $(WIN32CFLAGS) /Fe$@ /Fo$(TOOLSRC)/mkfloppy/release/ $**

$(MKPART): $(TOOLSRC)/mkpart/mkpart.c
    $(CC) $(WIN32CFLAGS) /Fe$@ /Fo$(TOOLSRC)/mkpart/release/ $**

$(DBGGW): $(TOOLSRC)/dbggw/dbggw.c $(TOOLSRC)/dbggw/rdp.c
    $(CC) $(WIN32CFLAGS) /I$(SRC)/include/os /Fe$@ /Fo$(TOOLSRC)/dbggw/release/ $** /link wsock32.lib

$(SOW): $(TOOLSRC)/sow/sow.c $(SRC)/lib/vsprintf.c
    $(CC) $(WIN32CFLAGS) /I$(SRC)/include/os /Fe$@ /Fo$(TOOLSRC)/sow/release/ $** /D NOFLOAT /link /ENTRY:dllmain /DLL /NODEFAULTLIB kernel32.lib

$(MKDFS): \
  $(TOOLSRC)/dfs/blockdev.c \
  $(TOOLSRC)/dfs/vmdk.c \
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

$(OBJ)/osldr/ldrinit.exe: $(SRC)/sys/osldr/ldrinit.asm
    $(NASM) -f bin $** -o $@ -l $(OBJ)/osldr/ldrinit.lst

$(OBJ)/osldr/bioscall.obj: $(SRC)/sys/osldr/bioscall.asm
    $(NASM) -f win32 $** -o $@ -l $(OBJ)/osldr/bioscall.lst

OSLDRSRC=$(SRC)\sys\osldr\osldr.c $(SRC)\sys\osldr\loadkrnl.c $(SRC)\sys\osldr\unzip.c $(SRC)\lib\vsprintf.c $(SRC)\lib\string.c 

$(BIN)/osldr.dll: $(OSLDRSRC) $(OBJ)\osldr\ldrinit.exe $(OBJ)/osldr/bioscall.obj
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/osldr/ $(OSLDRSRC) $(OBJ)/osldr/bioscall.obj /D KERNEL /D OSLDR /link /DLL /NODEFAULTLIB /ENTRY:start /BASE:0x00090000 /FIXED /STUB:$(OBJ)\osldr\ldrinit.exe $(RAWIMGFLAGS)

$(OBJ)/krnl/lldiv.obj: $(SRC)/lib/lldiv.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/krnl/llmul.obj: $(SRC)/lib/llmul.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/krnl/lldvrm.obj: $(SRC)/lib/lldvrm.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/krnl/llrem.obj: $(SRC)/lib/llrem.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/krnl/krnl.res: $(SRC)/sys/krnl/krnl.rc
  $(RC) /d "NDEBUG" /l 0x406 /I $(SRC)/include $(DEFS) /fo$@ $**

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
  $(SRC)\sys\krnl\user.c \
  $(SRC)\sys\krnl\mach.c \
  $(SRC)\sys\krnl\vmi.c \
  $(SRC)\sys\krnl\virtio.c \
  $(SRC)\sys\dev\video.c \
  $(SRC)\sys\dev\serial.c \
  $(SRC)\sys\dev\rnd.c \
  $(SRC)\sys\dev\ramdisk.c \
  $(SRC)\sys\dev\smbios.c \
  $(SRC)\sys\dev\nvram.c \
  $(SRC)\sys\dev\null.c \
  $(SRC)\sys\dev\klog.c \
  $(SRC)\sys\dev\kbd.c \
  $(SRC)\sys\dev\hd.c \
  $(SRC)\sys\dev\fd.c \
  $(SRC)\sys\dev\virtioblk.c \
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
  $(SRC)\lib\verinfo.c \
  $(OBJ)/krnl/lldiv.obj \
  $(OBJ)/krnl/llmul.obj \
  $(OBJ)/krnl/llrem.obj \
  $(OBJ)/krnl/lldvrm.obj \
  $(OBJ)/krnl/krnl.res
    $(CC) $(CFLAGS) /Fe$(BIN)/krnl.dll /Fo$(OBJ)/krnl/ $** /D KERNEL /D KRNL_LIB \
      /link /DLL /LARGEADDRESSAWARE /NODEFAULTLIB $(RAWIMGFLAGS) /ENTRY:start \
      /BASE:0x80000000 /FIXED /IMPLIB:$(LIBS)/krnl.lib

$(OBJ)/os/modf.obj: $(SRC)/lib/math/modf.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/os/ftol.obj: $(SRC)/lib/math/ftol.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/os/fpconst.obj: $(SRC)/lib/math/fpconst.asm
    $(NASM) -f win32 $** -o $@

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
  $(SRC)/sys/os/userdb.c \
  $(SRC)/sys/os/environ.c \
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
  $(SRC)/lib/verinfo.c \
  $(SRC)/lib/crypt.c \
  $(OBJ)/libc/llmul.obj \
  $(OBJ)/libc/lldvrm.obj \
  $(OBJ)/os/modf.obj \
  $(OBJ)/os/ftol.obj \
  $(OBJ)/os/fpconst.obj
    $(CC) $(CFLAGS) /Fe$(BIN)/os.dll /Fo$(OBJ)/os/ $** /D OS_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /BASE:0x7FF00000 /HEAP:33554432,131072 /FIXED /IMPLIB:$(LIBS)/os.lib

#
# drivers
#

drivers: dirs $(BIN)/3c905c.sys $(BIN)/eepro100.sys $(BIN)/ne2000.sys $(BIN)/pcnet32.sys $(BIN)/rtl8139.sys $(BIN)/sis900.sys $(BIN)/tulip.sys $(BIN)/virtionet.sys

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
  $(SRC)/lib/opts.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/ne2000/ $** /D KERNEL /D NE2000_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/ne2000.lib

$(BIN)/pcnet32.sys: \
  $(SRC)/sys/dev/pcnet32.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/pcnet32/ $** /D KERNEL /D PCNET32_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/pcnet32.lib

$(BIN)/rtl8139.sys: \
  $(SRC)/sys/dev/rtl8139.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/rtl8139/ $** /D KERNEL /D RTL8139_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/rtl8139.lib

$(BIN)/sis900.sys: \
  $(SRC)/sys/dev/sis900.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/sis900/ $** /D KERNEL /D SIS900_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/sis900.lib

$(BIN)/tulip.sys: \
  $(SRC)/sys/dev/tulip.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/tulip/ $** /D KERNEL /D TULIP_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/tulip.lib

$(BIN)/virtionet.sys: \
  $(SRC)/sys/dev/virtionet.c \
  $(SRC)/lib/string.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/tulip/ $** /D KERNEL /D VIRTIONET_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/virtionet.lib

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
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/tan.obj: $(SRC)/lib/math/tan.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/sqrt.obj: $(SRC)/lib/math/sqrt.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/sinh.obj: $(SRC)/lib/math/sinh.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/sin.obj: $(SRC)/lib/math/sin.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/pow.obj: $(SRC)/lib/math/pow.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/modf.obj: $(SRC)/lib/math/modf.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/log10.obj: $(SRC)/lib/math/log10.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/log.obj: $(SRC)/lib/math/log.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/ldexp.obj: $(SRC)/lib/math/ldexp.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/ftol.obj: $(SRC)/lib/math/ftol.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/frexp.obj: $(SRC)/lib/math/frexp.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/fpreset.obj: $(SRC)/lib/math/fpreset.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/fpconst.obj: $(SRC)/lib/math/fpconst.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/fmod.obj: $(SRC)/lib/math/fmod.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/floor.obj: $(SRC)/lib/math/floor.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/fabs.obj: $(SRC)/lib/math/fabs.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/exp.obj: $(SRC)/lib/math/exp.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/cosh.obj: $(SRC)/lib/math/cosh.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/cos.obj: $(SRC)/lib/math/cos.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/ceil.obj: $(SRC)/lib/math/ceil.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/atan2.obj: $(SRC)/lib/math/atan2.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/atan.obj: $(SRC)/lib/math/atan.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/asin.obj: $(SRC)/lib/math/asin.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/acos.obj: $(SRC)/lib/math/acos.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/libc/xtoa.obj: $(SRC)/lib/xtoa.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/termios.obj: $(SRC)/lib/termios.c
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

$(OBJ)/libc/shlib.obj: $(SRC)/lib/shlib.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/scanf.obj: $(SRC)/lib/scanf.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/tmpfile.obj: $(SRC)/lib/tmpfile.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/printf.obj: $(SRC)/lib/printf.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/popen.obj: $(SRC)/lib/popen.c
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

$(OBJ)/libc/math.obj: $(SRC)/lib/math.c
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

$(OBJ)/libc/fork.obj: $(SRC)/lib/fork.c
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

$(OBJ)/libc/dirent.obj: $(SRC)/lib/dirent.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/getopt.obj: $(SRC)/lib/getopt.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/glob.obj: $(SRC)/lib/glob.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/fnmatch.obj: $(SRC)/lib/fnmatch.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/mman.obj: $(SRC)/lib/mman.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/regcomp.obj: $(SRC)/lib/regex/regcomp.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/regexec.obj: $(SRC)/lib/regex/regexec.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/regerror.obj: $(SRC)/lib/regex/regerror.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(OBJ)/libc/regfree.obj: $(SRC)/lib/regex/regfree.c
    $(CC) $(CFLAGS) /Fo$(OBJ)/libc/ /D LIBC /c $**

$(LIBS)/libc.lib: \
  $(OBJ)/libc/spinlock.obj \
  $(OBJ)/libc/rwlock.obj \
  $(OBJ)/libc/pthread.obj \
  $(OBJ)/libc/mutex.obj \
  $(OBJ)/libc/condvar.obj \
  $(OBJ)/libc/barrier.obj \
  $(OBJ)/libc/xtoa.obj \
  $(OBJ)/libc/termios.obj \
  $(OBJ)/libc/time.obj \
  $(OBJ)/libc/strtol.obj \
  $(OBJ)/libc/strtod.obj \
  $(OBJ)/libc/string.obj \
  $(OBJ)/libc/strftime.obj \
  $(OBJ)/libc/stdlib.obj \
  $(OBJ)/libc/stdio.obj \
  $(OBJ)/libc/shlib.obj \
  $(OBJ)/libc/scanf.obj \
  $(OBJ)/libc/printf.obj \
  $(OBJ)/libc/tmpfile.obj \
  $(OBJ)/libc/popen.obj \
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
  $(OBJ)/libc/math.obj \
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
  $(OBJ)/libc/acos.obj \
  $(OBJ)/libc/dirent.obj \
  $(OBJ)/libc/getopt.obj \
  $(OBJ)/libc/glob.obj \
  $(OBJ)/libc/fnmatch.obj \
  $(OBJ)/libc/mman.obj \
  $(OBJ)/libc/fork.obj \
  $(OBJ)/libc/regcomp.obj \
  $(OBJ)/libc/regexec.obj \
  $(OBJ)/libc/regerror.obj \
  $(OBJ)/libc/regfree.obj
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

$(OBJ)/msvcrt/fpconst.obj: $(SRC)/lib/math/fpconst.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/msvcrt/floor.obj: $(SRC)/lib/math/floor.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/msvcrt/fmod.obj: $(SRC)/lib/math/fmod.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/msvcrt/frexp.obj: $(SRC)/lib/math/frexp.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/msvcrt/ftol.obj: $(SRC)/lib/math/ftol.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/msvcrt/ldexp.obj: $(SRC)/lib/math/ldexp.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/msvcrt/modf.obj: $(SRC)/lib/math/modf.asm
    $(NASM) -f win32 $** -o $@

$(OBJ)/msvcrt/llmul.obj: $(SRC)/lib/llmul.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/msvcrt/lldvrm.obj: $(SRC)/lib/lldvrm.asm
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
  $(SRC)/lib/strtod.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/strftime.c \
  $(SRC)/lib/setjmp.c \
  $(SRC)/lib/qsort.c \
  $(SRC)/lib/inifile.c \
  $(SRC)/lib/fcvt.c \
  $(SRC)/lib/ctype.c \
  $(SRC)/lib/bsearch.c \
  $(OBJ)/msvcrt/fpconst.obj \
  $(OBJ)/msvcrt/floor.obj \
  $(OBJ)/msvcrt/fmod.obj \
  $(OBJ)/msvcrt/frexp.obj \
  $(OBJ)/msvcrt/ftol.obj \
  $(OBJ)/msvcrt/ldexp.obj \
  $(OBJ)/msvcrt/modf.obj \
  $(OBJ)/msvcrt/llmul.obj \
  $(OBJ)/msvcrt/lldvrm.obj \
  $(LIBS)/os.lib \
  $(LIBS)/kernel32.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/msvcrt/ $** /D MSVCRT_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:dllmain /IMPLIB:$(LIBS)/msvcrt.lib

#
# utils
#

utils: dirs $(BIN)/sh.exe $(BIN)/msh.exe $(BIN)/edit.exe $(BIN)/less.exe $(BIN)/fdisk.exe $(BIN)/setup.exe $(BIN)/make.exe $(BIN)/ar.exe $(BIN)/impdef.exe $(BIN)/jinit.exe $(BIN)/ftpd.exe $(BIN)/telnetd.exe $(BIN)/login.exe $(BIN)/ctohtml.exe $(BIN)/mkboot.exe $(BIN)/ping.exe $(BIN)/grep.exe $(BIN)/pkg.exe $(BIN)/httpd.dll

$(BIN)/sh.exe: \
  $(SRC)/utils/sh/sh.c \
  $(SRC)/utils/sh/input.c \
  $(SRC)/utils/sh/parser.c \
  $(SRC)/utils/sh/stmalloc.c \
  $(SRC)/utils/sh/node.c \
  $(SRC)/utils/sh/chartype.c \
  $(SRC)/utils/sh/job.c \
  $(SRC)/utils/sh/interp.c \
  $(SRC)/utils/sh/cmds.c \
  $(SRC)/utils/sh/builtins.c \
  $(SRC)/cmds/chgrp.c \
  $(SRC)/cmds/chmod.c \
  $(SRC)/cmds/chown.c \
  $(SRC)/cmds/cp.c \
  $(SRC)/cmds/du.c \
  $(SRC)/cmds/ls.c \
  $(SRC)/cmds/mkdir.c \
  $(SRC)/cmds/mv.c \
  $(SRC)/cmds/rm.c \
  $(SRC)/cmds/test.c \
  $(SRC)/cmds/touch.c \
  $(SRC)/cmds/wc.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/sh/ $** /D SHELL /link /NODEFAULTLIB /FIXED:NO

$(BIN)/msh.exe: \
  $(SRC)/utils/msh/msh.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/msh/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/edit.exe: \
  $(SRC)/utils/edit/edit.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/edit/ $** /D SANOS /link /NODEFAULTLIB /FIXED:NO

$(BIN)/less.exe: \
  $(SRC)/utils/edit/edit.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/edit/ $** /D SANOS /D LESS /link /NODEFAULTLIB /FIXED:NO

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

$(BIN)/make.exe: \
  $(SRC)/utils/make/make.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/make/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/ar.exe: \
  $(SRC)/utils/ar/ar.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/ar/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/impdef.exe: \
  $(SRC)/utils/impdef/impdef.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/impdef/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/jinit.exe: \
  $(SRC)/utils/jinit/jinit.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/jinit/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/ftpd.exe: \
  $(SRC)/utils/ftpd/ftpd.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/ftpd/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/telnetd.exe: \
  $(SRC)/utils/telnetd/telnetd.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/telnetd/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/login.exe: \
  $(SRC)/utils/login/login.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/login/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/ctohtml.exe: \
  $(SRC)/utils/ctohtml/ctohtml.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/ctohtml/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/mkboot.exe: \
  $(SRC)/utils/mkboot/mkboot.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/mkboot/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/pkg.exe: \
  $(SRC)/utils/pkg/pkg.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/pkg/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/grep.exe: \
  $(SRC)/cmds/grep.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/grep/ $** /link /NODEFAULTLIB /FIXED:NO

$(BIN)/ping.exe: \
  $(SRC)/cmds/ping.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/ping/ $** /link /NODEFAULTLIB /FIXED:NO

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

bootdisk: $(IMG)/sanos.flp

$(IMG)/sanos.flp: dirs sanos $(MKDFS) $(BUILD)/bootdisk.lst
    $(MKDFS) -d $(IMG)/sanos.flp -b $(BIN)\boot -l $(BIN)\osldr.dll -k $(BIN)\krnl.dll -c 1440 -i -f -S $(TOPDIR) -F $(BUILD)\bootdisk.lst

#
# boothd
#

boothd: $(IMG)/sanos.vmdk

$(IMG)/sanos.vmdk: dirs sanos $(MKDFS) $(BUILD)/boothd.lst
    $(MKDFS) -d $(IMG)/sanos.vmdk -t vmdk -b $(BIN)\boot -l $(BIN)\osldr.dll -k $(BIN)\krnl.dll -c 100M -i -f -S $(TOPDIR) -F $(BUILD)\boothd.lst

#
# netbootimg
#

netbootimg: $(IMG)/sanos.0

$(IMG)/sanos.0: dirs sanos $(MKDFS) $(BUILD)/bootnet.lst
    $(MKDFS) -d $(IMG)/sanos.0 -b $(BIN)\netboot -l $(BIN)\osldr.dll -k $(BIN)\krnl.dll -c 512 -I 8192 -i -f -S $(TOPDIR) -F $(BUILD)\bootnet.lst

#
# bootcd
#

bootcd: install $(IMG)/sanos.iso

$(IMG)/sanos.iso: dirs sanos tools
    if not exist $(IMG) mkdir $(IMG)
    if exist $(IMG)\sanos.iso del $(IMG)\sanos.iso
    $(MKDFS) -d $(INSTALL)\BOOTIMG.BIN -b $(BIN)\cdemboot -l $(BIN)\osldr.dll -k $(BIN)\krnl.dll -c 512 -C 1440 -I 8192 -i -f -K rootdev=cd0,rootfs=cdfs
    $(MKISOFS) -J -c BOOTCAT.BIN -b BOOTIMG.BIN -o $(IMG)\sanos.iso $(INSTALL)
#    $(MKDFS) -d $(INSTALL)\BOOTIMG.BIN -b $(BIN)\cdboot -l $(BIN)\osldr.dll -k $(BIN)\krnl.dll -c 512 -I 8192 -i -f -K rootdev=cd0,rootfs=cdfs
#    $(MKISOFS) -no-emul-boot -J -c BOOTCAT.BIN -b BOOTIMG.BIN -o $(IMG)\sanos.iso $(INSTALL)
    del $(INSTALL)\BOOTIMG.BIN

#
# minimal
#

minimal: $(IMG)/minimal.flp

$(IMG)/minimal.flp: dirs sanos $(MKDFS) $(BUILD)/minbootdisk.lst
    $(MKDFS) -d $(IMG)/minimal.flp -b $(BIN)\boot -l $(BIN)\osldr.dll -k $(BIN)\krnl.dll -c 1440 -i -f -S . -F $(BUILD)\minbootdisk.lst

#
# sdk
#

SDK=$(TOPDIR)\sdk
SDKBIN=$(SDK)\bin
SDKLIB=$(SDK)\lib
SDKSRC=$(SDK)\src

$(SDKBIN)/os.dll: $(SOW)
    if not exist $(SDKBIN) mkdir $(SDKBIN)
    copy /Y $(SOW) $(SDKBIN)\os.dll

$(SDKBIN)/make.exe: $(BIN)/make.exe
    if not exist $(SDKBIN) mkdir $(SDKBIN)
    copy /Y $(BIN)\make.exe $(SDKBIN)\make.exe

$(SDKBIN)/ar.exe: $(BIN)/ar.exe
    if not exist $(SDKBIN) mkdir $(SDKBIN)
    copy /Y $(BIN)\ar.exe $(SDKBIN)\ar.exe

$(SDKBIN)/impdef.exe: $(BIN)/impdef.exe
    if not exist $(SDKBIN) mkdir $(SDKBIN)
    copy /Y $(BIN)\impdef.exe $(SDKBIN)\impdef.exe

$(SDKBIN)/ctohtml.exe: $(BIN)/ctohtml.exe
    if not exist $(SDKBIN) mkdir $(SDKBIN)
    copy /Y $(BIN)\ctohtml.exe $(SDKBIN)\ctohtml.exe

sdk: $(SDKBIN)/os.dll $(SDKBIN)/make.exe $(SDKBIN)/ar.exe $(SDKBIN)/impdef.exe $(SDKBIN)/ctohtml.exe
    cd $(SDKSRC)\as && nmake install
    cd $(SDKSRC)\cc && nmake install
    cd $(SDKSRC)\libc && nmake install

sdk-clean:
    del /Q $(SDKBIN)
    cd $(SDKSRC)\as && nmake clean
    cd $(SDKSRC)\cc && nmake clean
    cd $(SDKSRC)\libc && nmake clean

sdkdisk: sanos sdk install install-source install-sdk boothd

#
# install
#

install: sanos
    if not exist $(INSTALL)\bin mkdir $(INSTALL)\bin
    if not exist $(INSTALL)\boot mkdir $(INSTALL)\boot
    if not exist $(INSTALL)\dev mkdir $(INSTALL)\dev
    if not exist $(INSTALL)\etc mkdir $(INSTALL)\etc
    if not exist $(INSTALL)\mnt mkdir $(INSTALL)\mnt
    if not exist $(INSTALL)\proc mkdir $(INSTALL)\proc
    if not exist $(INSTALL)\tmp mkdir $(INSTALL)\tmp
    if not exist $(INSTALL)\usr mkdir $(INSTALL)\usr
    if not exist $(INSTALL)\var mkdir $(INSTALL)\var
    copy /Y $(BIN)\sh.exe        $(INSTALL)\bin\sh.exe
    copy /Y $(BIN)\msh.exe       $(INSTALL)\bin\msh.exe
    copy /Y $(BIN)\httpd.dll     $(INSTALL)\bin\httpd.dll
    copy /Y $(BIN)\setup.exe     $(INSTALL)\bin\setup.exe
    copy /Y $(BIN)\ctohtml.exe   $(INSTALL)\bin\ctohtml.exe
    copy /Y $(BIN)\mkboot.exe    $(INSTALL)\bin\mkboot.exe
    copy /Y $(BIN)\edit.exe      $(INSTALL)\bin\edit.exe
    copy /Y $(BIN)\less.exe      $(INSTALL)\bin\less.exe
    copy /Y $(BIN)\fdisk.exe     $(INSTALL)\bin\fdisk.exe
    copy /Y $(BIN)\pkg.exe       $(INSTALL)\bin\pkg.exe
    copy /Y $(BIN)\jinit.exe     $(INSTALL)\bin\jinit.exe
    copy /Y $(BIN)\telnetd.exe   $(INSTALL)\bin\telnetd.exe
    copy /Y $(BIN)\ftpd.exe      $(INSTALL)\bin\ftpd.exe
    copy /Y $(BIN)\login.exe     $(INSTALL)\bin\login.exe
    copy /Y $(BIN)\ping.exe      $(INSTALL)\bin\ping.exe
    copy /Y $(BIN)\grep.exe      $(INSTALL)\bin\grep.exe
    copy /Y $(BIN)\msvcrt.dll    $(INSTALL)\bin\msvcrt.dll
    copy /Y $(BIN)\kernel32.dll  $(INSTALL)\bin\kernel32.dll
    copy /Y $(BIN)\user32.dll    $(INSTALL)\bin\user32.dll
    copy /Y $(BIN)\advapi32.dll  $(INSTALL)\bin\advapi32.dll
    copy /Y $(BIN)\winmm.dll     $(INSTALL)\bin\winmm.dll
    copy /Y $(BIN)\wsock32.dll   $(INSTALL)\bin\wsock32.dll
    copy /Y $(BUILD)\os.ini      $(INSTALL)\etc\os.ini
    copy /Y $(BUILD)\setup.ini   $(INSTALL)\etc\setup.ini
    copy /Y $(BUILD)\krnl.ini    $(INSTALL)\boot\krnl.ini
    copy /Y $(BIN)\boot          $(INSTALL)\boot\boot
    copy /Y $(BIN)\cdboot        $(INSTALL)\boot\cdboot
    copy /Y $(BIN)\cdemboot      $(INSTALL)\boot\cdemboot
    copy /Y $(BIN)\netboot       $(INSTALL)\boot\netboot
    copy /Y $(BIN)\osldr.dll     $(INSTALL)\boot\osldr.dll
    copy /Y $(BIN)\os.dll        $(INSTALL)\boot\os.dll
    copy /Y $(BIN)\3c905c.sys    $(INSTALL)\boot\3c905c.sys
    copy /Y $(BIN)\eepro100.sys  $(INSTALL)\boot\eepro100.sys
    copy /Y $(BIN)\ne2000.sys    $(INSTALL)\boot\ne2000.sys
    copy /Y $(BIN)\pcnet32.sys   $(INSTALL)\boot\pcnet32.sys
    copy /Y $(BIN)\rtl8139.sys   $(INSTALL)\boot\rtl8139.sys
    copy /Y $(BIN)\sis900.sys    $(INSTALL)\boot\sis900.sys
    copy /Y $(BIN)\tulip.sys     $(INSTALL)\boot\tulip.sys
    copy /Y $(BIN)\virtionet.sys $(INSTALL)\boot\virtionet.sys

install-source: dirs
    -@if not exist $(INSTALL)\usr mkdir $(INSTALL)\usr
    -@if not exist $(INSTALL)\usr\include mkdir $(INSTALL)\usr\include
    -@if not exist $(INSTALL)\usr\src mkdir $(INSTALL)\usr\src
    -@if not exist $(INSTALL)\usr\src\lib mkdir $(INSTALL)\usr\src\lib
    -@if not exist $(INSTALL)\usr\src\sys mkdir $(INSTALL)\usr\src\sys
    -@if not exist $(INSTALL)\usr\src\utils mkdir $(INSTALL)\usr\src\utils
    -@if not exist $(INSTALL)\usr\src\cmds mkdir $(INSTALL)\usr\src\cmds
    -@if not exist $(INSTALL)\usr\src\win32 mkdir $(INSTALL)\usr\src\win32
    xcopy /S /I /Y $(SRC)\include $(INSTALL)\usr\include
    xcopy /S /I /Y $(SRC)\lib $(INSTALL)\usr\src\lib
    xcopy /S /I /Y $(SRC)\sys $(INSTALL)\usr\src\sys
    xcopy /S /I /Y $(SRC)\utils $(INSTALL)\usr\src\utils
    xcopy /S /I /Y $(SRC)\cmds $(INSTALL)\usr\src\cmds
    xcopy /S /I /Y $(SRC)\win32 $(INSTALL)\usr\src\win32
    copy /Y $(SRC)\Makefile $(INSTALL)\usr\src

install-sdk: install-source sdk
    -@if not exist $(INSTALL)\usr\bin mkdir $(INSTALL)\usr\bin
    -@if not exist $(INSTALL)\usr\lib mkdir $(INSTALL)\usr\lib
    -@if not exist $(INSTALL)\usr\src\utils\as mkdir $(INSTALL)\usr\src\utils\as
    -@if not exist $(INSTALL)\usr\src\utils\as\output mkdir $(INSTALL)\usr\src\utils\as\output
    -@if not exist $(INSTALL)\usr\src\utils\cc mkdir $(INSTALL)\usr\src\utils\cc
    -@if not exist $(INSTALL)\usr\src\utils\ar mkdir $(INSTALL)\usr\src\utils\ar
    copy /Y $(SDKBIN)\as.exe              $(INSTALL)\usr\bin
    copy /Y $(SDKBIN)\cc.exe              $(INSTALL)\usr\bin
    copy /Y $(SDKBIN)\make.exe            $(INSTALL)\usr\bin
    copy /Y $(SDKBIN)\ar.exe              $(INSTALL)\usr\bin
    copy /Y $(SDKBIN)\impdef.exe          $(INSTALL)\usr\bin
    copy /Y $(SDKLIB)\libc.a              $(INSTALL)\usr\lib
    copy /Y $(SDKLIB)\os.def              $(INSTALL)\usr\lib\os.def
    copy /Y $(SDKLIB)\krnl.def            $(INSTALL)\usr\lib\krnl.def
    copy /Y $(SDKSRC)\as\*.c              $(INSTALL)\usr\src\utils\as
    copy /Y $(SDKSRC)\as\*.h              $(INSTALL)\usr\src\utils\as
    copy /Y $(SDKSRC)\as\output\*.h       $(INSTALL)\usr\src\utils\as\output
    copy /Y $(SDKSRC)\as\output\*.c       $(INSTALL)\usr\src\utils\as\output
    copy /Y $(SDKSRC)\as\Makefile.sanos   $(INSTALL)\usr\src\utils\as\Makefile
    copy /Y $(SDKSRC)\cc\*.c              $(INSTALL)\usr\src\utils\cc
    copy /Y $(SDKSRC)\cc\*.h              $(INSTALL)\usr\src\utils\cc
    copy /Y $(SDKSRC)\cc\Makefile.sanos   $(INSTALL)\usr\src\utils\cc\Makefile

