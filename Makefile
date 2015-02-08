#
# Makefile for building Sanos under Windows
#

!IFNDEF TOPDIR
TOPDIR=.
!ENDIF

OUTPUT=$(TOPDIR)\win

!IFNDEF INSTALL
INSTALL=$(OUTPUT)\install
!ENDIF

BUILD=$(TOPDIR)\build
SRC=$(TOPDIR)\src
TOOLS=$(TOPDIR)\tools
TOOLSRC=$(TOPDIR)\utils
BIN=$(TOPDIR)\bin

IMG=$(OUTPUT)\img
TOOLBIN=$(OUTPUT)\tools
LIBS=$(OUTPUT)\lib
OBJ=$(OUTPUT)\obj
TOOLOBJ=$(OUTPUT)\obj

MKISOFS=$(TOOLS)\mkisofs.exe

ASM=$(TOOLBIN)\nasm.exe
MKDFS=$(TOOLBIN)\mkdfs.exe
MKFLOPPY=$(TOOLBIN)\mkfloppy.exe
MKPART=$(TOOLBIN)\mkpart.exe
TCC=$(TOOLBIN)\cc.exe
NASM=$(TOOLBIN)\as.exe
AR=$(TOOLBIN)\ar.exe
IMPDEF=$(TOOLBIN)\impdef.exe
CTOHTML=$(TOOLBIN)\ctohtml.exe
SOW=$(TOOLBIN)\os.dll
DBGGW=$(TOOLBIN)\dbggw.exe
LIBRARIAN=lib

SDK=$(TOPDIR)\sdk
SDKSRC=$(SDK)\src

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
# MSVC=11 for Microsoft Visual Visual Studio 2012

!IFNDEF MSVC
MSVC=7
!ENDIF

AFLAGS=/nologo

!IF $(MSVC) == 11
CFLAGS=/nologo /O2 /Ob1 /Oi /Ot /Oy /GS- /GR- /X /GF /Gy /W3 /arch:IA32 /I $(SRC)/include $(DEFS)
!ELSEIF $(MSVC) == 8 || $(MSVC) == 9 || $(MSVC) == 10
CFLAGS=/nologo /O2 /Ob1 /Oi /Ot /Oy /GS- /GR- /X /GF /Gy /W3 /I $(SRC)/include $(DEFS)
!ELSE
CFLAGS=/nologo /O2 /Og /Ob1 /Oi /Ot /Oy /X /GF /Gy /W3 /I $(SRC)/include $(DEFS)
!ENDIF

!IF $(MSVC) > 8
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
# /arch:IA32            Generate code for x87
#

all: dirs tools sanos bootdisk boothd netbootimg bootcd

!INCLUDE $(BUILD)/sanos.dep

sanos: dirs kernel drivers libc win32 utils

#
# dirs
#

dirs: $(OUTPUT)/ok

$(OUTPUT)/ok:
    -@if not exist $(OUTPUT) mkdir $(OUTPUT)
    -@if not exist $(TOOLBIN) mkdir $(TOOLBIN)
    -@if not exist $(TOOLOBJ) mkdir $(TOOLOBJ)
    -@if not exist $(TOOLOBJ)\nasm mkdir $(TOOLOBJ)\nasm
    -@if not exist $(TOOLOBJ)\mkfloppy mkdir $(TOOLOBJ)\mkfloppy
    -@if not exist $(TOOLOBJ)\mkpart mkdir $(TOOLOBJ)\mkpart
    -@if not exist $(TOOLOBJ)\dbggw mkdir $(TOOLOBJ)\dbggw
    -@if not exist $(TOOLOBJ)\sow mkdir $(TOOLOBJ)\sow
    -@if not exist $(TOOLOBJ)\dfs mkdir $(TOOLOBJ)\dfs
    -@if not exist $(LIBS) mkdir $(LIBS)
    -@if not exist $(IMG) mkdir $(IMG)
    -@if not exist $(OBJ) mkdir $(OBJ)
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
    -@if not exist $(OBJ)\genvmdk mkdir $(OBJ)\genvmdk
    -@if not exist $(OBJ)\pkg mkdir $(OBJ)\pkg
    -@if not exist $(OBJ)\grep mkdir $(OBJ)\grep
    -@if not exist $(OBJ)\find mkdir $(OBJ)\find
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
    -@if not exist $(OBJ)\cc mkdir $(OBJ)\cc
    -@if not exist $(OBJ)\as mkdir $(OBJ)\as
    -@if not exist $(OBJ)\crt mkdir $(OBJ)\crt
    -@if not exist $(INSTALL) mkdir $(INSTALL)
    -@if not exist $(INSTALL)\bin mkdir $(INSTALL)\bin
    -@if not exist $(INSTALL)\boot mkdir $(INSTALL)\boot
    -@if not exist $(INSTALL)\dev mkdir $(INSTALL)\dev
    -@if not exist $(INSTALL)\proc mkdir $(INSTALL)\proc
    -@if not exist $(INSTALL)\etc mkdir $(INSTALL)\etc
    -@if not exist $(INSTALL)\var mkdir $(INSTALL)\var
    -@if not exist $(INSTALL)\usr mkdir $(INSTALL)\usr
    -@if not exist $(INSTALL)\usr\bin mkdir $(INSTALL)\usr\bin
    -@if not exist $(INSTALL)\usr\lib mkdir $(INSTALL)\usr\lib
    -@if not exist $(BIN) mkdir $(BIN)
    -@echo OK > $(OUTPUT)\ok

#
# clean
#

clean:
    #-del /Q $(OUTPUT)

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

tools: $(ASM) $(MKDFS) $(MKFLOPPY) $(MKPART) $(DBGGW) $(SOW)

NASMFLAGS=/W1 /I $(SDKSRC)\as \
          /D OF_ONLY /D OF_ELF32 /D OF_WIN32 /D OF_COFF /D OF_OBJ /D OF_BIN /D OF_DBG /D OF_DEFAULT=of_elf32

NASMSRC=\
  $(SDKSRC)/as/nasm.c \
  $(SDKSRC)/as/nasmlib.c \
  $(SDKSRC)/as/ver.c \
  $(SDKSRC)/as/raa.c \
  $(SDKSRC)/as/saa.c \
  $(SDKSRC)/as/rbtree.c \
  $(SDKSRC)/as/float.c \
  $(SDKSRC)/as/insnsa.c \
  $(SDKSRC)/as/insnsb.c \
  $(SDKSRC)/as/directiv.c \
  $(SDKSRC)/as/assemble.c \
  $(SDKSRC)/as/labels.c \
  $(SDKSRC)/as/hashtbl.c \
  $(SDKSRC)/as/crc64.c \
  $(SDKSRC)/as/parser.c \
  $(SDKSRC)/as/preproc.c \
  $(SDKSRC)/as/quote.c \
  $(SDKSRC)/as/pptok.c \
  $(SDKSRC)/as/macros.c \
  $(SDKSRC)/as/listing.c \
  $(SDKSRC)/as/eval.c \
  $(SDKSRC)/as/exprlib.c \
  $(SDKSRC)/as/stdscan.c \
  $(SDKSRC)/as/strfunc.c \
  $(SDKSRC)/as/tokhash.c \
  $(SDKSRC)/as/regvals.c \
  $(SDKSRC)/as/regflags.c \
  $(SDKSRC)/as/ilog2.c \
  $(SDKSRC)/as/strlcpy.c \
  $(SDKSRC)/as/output/outform.c \
  $(SDKSRC)/as/output/outlib.c \
  $(SDKSRC)/as/output/nulldbg.c \
  $(SDKSRC)/as/output/nullout.c \
  $(SDKSRC)/as/output/outbin.c  \
  $(SDKSRC)/as/output/outcoff.c \
  $(SDKSRC)/as/output/outelf.c \
  $(SDKSRC)/as/output/outelf32.c \
  $(SDKSRC)/as/output/outobj.c \
  $(SDKSRC)/as/output/outdbg.c

NASMWINSRC=\
  $(SDKSRC)/as/snprintf.c \
  $(SDKSRC)/as/vsnprintf.c \

$(ASM): $(NASMSRC) $(NASMWINSRC)
    $(CC) $(WIN32CFLAGS) $(NASMFLAGS) /Fe$@ /Fo$(TOOLOBJ)/nasm/ $**

$(MKFLOPPY): $(TOOLSRC)/mkfloppy/mkfloppy.c
    $(CC) $(WIN32CFLAGS) /Fe$@ /Fo$(TOOLOBJ)/mkfloppy/ $**

$(MKPART): $(TOOLSRC)/mkpart/mkpart.c
    $(CC) $(WIN32CFLAGS) /Fe$@ /Fo$(TOOLOBJ)/mkpart/ $**

$(DBGGW): $(TOOLSRC)/dbggw/dbggw.c $(TOOLSRC)/dbggw/rdp.c
    $(CC) $(WIN32CFLAGS) /I$(SRC)/include/os /Fe$@ /Fo$(TOOLOBJ)/dbggw/ $** /link wsock32.lib

$(SOW): $(TOOLSRC)/sow/sow.c $(SRC)/lib/vsprintf.c $(SRC)/sys/os/syserr.c
    $(CC) $(WIN32CFLAGS) /I$(SRC)/include/os /Fe$@ /Fo$(TOOLOBJ)/sow/ $** /D NOFLOAT /D SOW /link /ENTRY:dllmain /DLL /NODEFAULTLIB kernel32.lib /IMPLIB:$(LIBS)/sow.lib

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
    $(CC) $(WIN32CFLAGS) /Fe$@ /Fo$(TOOLOBJ)/dfs/ $**

#
# kernel
#

kernel: dirs tools boot $(INSTALL)/boot/krnl.dll $(INSTALL)/boot/os.dll

boot: dirs tools $(INSTALL)/boot/boot $(INSTALL)/boot/cdboot $(INSTALL)/boot/cdemboot $(INSTALL)/boot/netboot $(INSTALL)/boot/osldr.dll

$(INSTALL)/boot/boot: $(SRC)/sys/boot/boot.asm
    $(ASM) -f bin $** -o $@

$(INSTALL)/boot/cdboot: $(SRC)/sys/boot/cdboot.asm
    $(ASM) -f bin $** -o $@

$(INSTALL)/boot/cdemboot: $(SRC)/sys/boot/cdemboot.asm
    $(ASM) -f bin $** -o $@

$(INSTALL)/boot/netboot: $(SRC)/sys/boot/netboot.asm
    $(ASM) -f bin $** -o $@

$(OBJ)/osldr/ldrinit.exe: $(SRC)/sys/osldr/ldrinit.asm
    $(ASM) -f bin $** -o $@ -l $(OBJ)/osldr/ldrinit.lst

$(OBJ)/osldr/bioscall.obj: $(SRC)/sys/osldr/bioscall.asm
    $(ASM) -f win32 $** -o $@ -l $(OBJ)/osldr/bioscall.lst

OSLDRSRC=$(SRC)\sys\osldr\osldr.c $(SRC)\sys\osldr\loadkrnl.c $(SRC)\sys\osldr\unzip.c $(SRC)\lib\vsprintf.c $(SRC)\lib\string.c 

$(INSTALL)/boot/osldr.dll: $(OSLDRSRC) $(OBJ)\osldr\ldrinit.exe $(OBJ)/osldr/bioscall.obj
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

$(LIBS)/krnl.lib $(INSTALL)/boot/krnl.dll: \
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
  $(SRC)\sys\dev\vga.c \
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
    $(CC) $(CFLAGS) /Fe$(INSTALL)/boot/krnl.dll /Fo$(OBJ)/krnl/ $** /D KERNEL /D KRNL_LIB \
      /link /DLL /LARGEADDRESSAWARE /NODEFAULTLIB $(RAWIMGFLAGS) /ENTRY:start \
      /BASE:0x80000000 /FIXED /IMPLIB:$(LIBS)/krnl.lib

$(OBJ)/os/modf.obj: $(SRC)/lib/math/modf.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/os/ftol.obj: $(SRC)/lib/math/ftol.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/os/fpconst.obj: $(SRC)/lib/math/fpconst.asm
    $(ASM) -f win32 $** -o $@

$(LIBS)/os.lib $(INSTALL)/boot/os.dll: \
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
    $(CC) $(CFLAGS) /Fe$(INSTALL)/boot/os.dll /Fo$(OBJ)/os/ $** /D OS_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /BASE:0x7FF00000 /HEAP:33554432,131072 /FIXED /IMPLIB:$(LIBS)/os.lib

#
# drivers
#

drivers: dirs \
  $(INSTALL)/boot/3c905c.sys \
  $(INSTALL)/boot/eepro100.sys \
  $(INSTALL)/boot/ne2000.sys \
  $(INSTALL)/boot/pcnet32.sys \
  $(INSTALL)/boot/rtl8139.sys \
  $(INSTALL)/boot/sis900.sys \
  $(INSTALL)/boot/tulip.sys \
  $(INSTALL)/boot/virtionet.sys

$(INSTALL)/boot/3c905c.sys: \
  $(SRC)/sys/dev/3c905c.c \
  $(SRC)/lib/string.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/3c905c/ $** /D KERNEL /D 3C905C_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/3c905c.lib

$(INSTALL)/boot/eepro100.sys: \
  $(SRC)/sys/dev/eepro100.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/eepro100/ $** /D KERNEL /D EEPRO100_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/eepro100.lib

$(INSTALL)/boot/ne2000.sys: \
  $(SRC)/sys/dev/ne2000.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/ne2000/ $** /D KERNEL /D NE2000_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/ne2000.lib

$(INSTALL)/boot/pcnet32.sys: \
  $(SRC)/sys/dev/pcnet32.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/pcnet32/ $** /D KERNEL /D PCNET32_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/pcnet32.lib

$(INSTALL)/boot/rtl8139.sys: \
  $(SRC)/sys/dev/rtl8139.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/rtl8139/ $** /D KERNEL /D RTL8139_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/rtl8139.lib

$(INSTALL)/boot/sis900.sys: \
  $(SRC)/sys/dev/sis900.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/sis900/ $** /D KERNEL /D SIS900_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/sis900.lib

$(INSTALL)/boot/tulip.sys: \
  $(SRC)/sys/dev/tulip.c \
  $(SRC)/lib/opts.c \
  $(SRC)/lib/strtol.c \
  $(SRC)/lib/string.c \
  $(SRC)/lib/ctype.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/tulip/ $** /D KERNEL /D TULIP_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/tulip.lib

$(INSTALL)/boot/virtionet.sys: \
  $(SRC)/sys/dev/virtionet.c \
  $(SRC)/lib/string.c \
  $(LIBS)/krnl.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/tulip/ $** /D KERNEL /D VIRTIONET_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:start /IMPLIB:$(LIBS)/virtionet.lib

#
# libc
#

libc: dirs tools $(LIBS)/libc.lib

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
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/tan.obj: $(SRC)/lib/math/tan.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/sqrt.obj: $(SRC)/lib/math/sqrt.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/sinh.obj: $(SRC)/lib/math/sinh.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/sin.obj: $(SRC)/lib/math/sin.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/pow.obj: $(SRC)/lib/math/pow.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/modf.obj: $(SRC)/lib/math/modf.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/log10.obj: $(SRC)/lib/math/log10.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/log.obj: $(SRC)/lib/math/log.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/ldexp.obj: $(SRC)/lib/math/ldexp.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/ftol.obj: $(SRC)/lib/math/ftol.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/frexp.obj: $(SRC)/lib/math/frexp.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/fpreset.obj: $(SRC)/lib/math/fpreset.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/fpconst.obj: $(SRC)/lib/math/fpconst.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/fmod.obj: $(SRC)/lib/math/fmod.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/floor.obj: $(SRC)/lib/math/floor.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/fabs.obj: $(SRC)/lib/math/fabs.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/exp.obj: $(SRC)/lib/math/exp.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/cosh.obj: $(SRC)/lib/math/cosh.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/cos.obj: $(SRC)/lib/math/cos.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/ceil.obj: $(SRC)/lib/math/ceil.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/atan2.obj: $(SRC)/lib/math/atan2.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/atan.obj: $(SRC)/lib/math/atan.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/asin.obj: $(SRC)/lib/math/asin.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/libc/acos.obj: $(SRC)/lib/math/acos.asm
    $(ASM) -f win32 $** -o $@

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
    $(LIBRARIAN) /NOLOGO /NODEFAULTLIB /OUT:$(LIBS)/libc.lib $**

#
# win32
#

win32: dirs tools \
  $(INSTALL)/bin/kernel32.dll \
  $(INSTALL)/bin/user32.dll \
  $(INSTALL)/bin/advapi32.dll \
  $(INSTALL)/bin/wsock32.dll \
  $(INSTALL)/bin/winmm.dll \
  $(INSTALL)/bin/msvcrt.dll

$(LIBS)/kernel32.lib $(INSTALL)/bin/kernel32.dll: \
  $(SRC)/win32/kernel32/kernel32.c \
  $(SRC)/win32/kernel32/kernel32.def \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$(INSTALL)/bin/kernel32.dll /Fo$(OBJ)/kernel32/ $** /D KERNEL32_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:DllMain /IMPLIB:$(LIBS)/kernel32.lib

$(INSTALL)/bin/user32.dll: \
  $(SRC)/win32/user32/user32.c \
  $(SRC)/win32/user32/user32.def \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/user32/ $** /D USER32_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:DllMain /IMPLIB:$(LIBS)/user32.lib

$(INSTALL)/bin/advapi32.dll: \
  $(SRC)/win32/advapi32/advapi32.c \
  $(SRC)/win32/advapi32/advapi32.def \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/advapi32/ $** /D ADVAPI32_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:DllMain /IMPLIB:$(LIBS)/advapi32.lib

$(INSTALL)/bin/wsock32.dll: \
  $(SRC)/win32/wsock32/wsock32.c \
  $(SRC)/win32/wsock32/wsock32.def \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/wsock32/ $** /D WSOCK32_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:DllMain /IMPLIB:$(LIBS)/wsock32.lib

$(INSTALL)/bin/winmm.dll: \
  $(SRC)/win32/winmm/winmm.c \
  $(SRC)/win32/winmm/winmm.def \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/winmm/ $** /D WINMM_LIB \
      /link /DLL /NODEFAULTLIB /ENTRY:DllMain /IMPLIB:$(LIBS)/winmm.lib

$(OBJ)/msvcrt/fpconst.obj: $(SRC)/lib/math/fpconst.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/msvcrt/floor.obj: $(SRC)/lib/math/floor.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/msvcrt/fmod.obj: $(SRC)/lib/math/fmod.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/msvcrt/frexp.obj: $(SRC)/lib/math/frexp.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/msvcrt/ftol.obj: $(SRC)/lib/math/ftol.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/msvcrt/ldexp.obj: $(SRC)/lib/math/ldexp.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/msvcrt/modf.obj: $(SRC)/lib/math/modf.asm
    $(ASM) -f win32 $** -o $@

$(OBJ)/msvcrt/llmul.obj: $(SRC)/lib/llmul.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(OBJ)/msvcrt/lldvrm.obj: $(SRC)/lib/lldvrm.asm
    $(AS) $(AFLAGS) /c /Fo$@ $**

$(INSTALL)/bin/msvcrt.dll: \
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

utils: dirs \
  $(INSTALL)/bin/sh.exe \
  $(INSTALL)/bin/msh.exe \
  $(INSTALL)/bin/edit.exe \
  $(INSTALL)/bin/less.exe \
  $(INSTALL)/bin/fdisk.exe \
  $(INSTALL)/bin/setup.exe \
  $(INSTALL)/bin/jinit.exe \
  $(INSTALL)/bin/ftpd.exe \
  $(INSTALL)/bin/telnetd.exe \
  $(INSTALL)/bin/login.exe \
  $(INSTALL)/bin/mkboot.exe \
  $(INSTALL)/bin/ping.exe \
  $(INSTALL)/bin/grep.exe \
  $(INSTALL)/bin/find.exe \
  $(INSTALL)/bin/pkg.exe \
  $(INSTALL)/bin/genvmdk.exe \
  $(INSTALL)/bin/httpd.dll \
  $(INSTALL)/usr/bin/make.exe \
  $(INSTALL)/usr/bin/ar.exe \
  $(INSTALL)/usr/bin/ctohtml.exe \
  $(INSTALL)/usr/bin/impdef.exe

$(INSTALL)/bin/sh.exe: \
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
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/sh/ $** /D SHELL /link /NODEFAULTLIB /FIXED:NO /IMPLIB:$(LIBS)/sh.lib

$(INSTALL)/bin/msh.exe: \
  $(SRC)/utils/msh/msh.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/msh/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/edit.exe: \
  $(SRC)/utils/edit/edit.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/edit/ $** /D SANOS /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/less.exe: \
  $(SRC)/utils/edit/edit.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/edit/ $** /D SANOS /D LESS /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/fdisk.exe: \
  $(SRC)/utils/fdisk/fdisk.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/fdisk/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/setup.exe: \
  $(SRC)/utils/setup/setup.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/setup/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/jinit.exe: \
  $(SRC)/utils/jinit/jinit.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/jinit/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/ftpd.exe: \
  $(SRC)/utils/ftpd/ftpd.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/ftpd/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/telnetd.exe: \
  $(SRC)/utils/telnetd/telnetd.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/telnetd/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/login.exe: \
  $(SRC)/utils/login/login.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/login/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/mkboot.exe: \
  $(SRC)/utils/mkboot/mkboot.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/mkboot/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/genvmdk.exe: \
  $(SRC)/utils/genvmdk/genvmdk.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/genvmdk/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/pkg.exe: \
  $(SRC)/utils/pkg/pkg.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/pkg/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/grep.exe: \
  $(SRC)/cmds/grep.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/grep/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/find.exe: \
  $(SRC)/cmds/find.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/find/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/bin/ping.exe: \
  $(SRC)/cmds/ping.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/ping/ $** /link /NODEFAULTLIB /FIXED:NO

$(OBJ)/httpd/httpd.res: $(SRC)/utils/httpd/httpd.rc
  $(RC) /d "NDEBUG" /l 0x406 /fo$@ $**

$(INSTALL)/bin/httpd.dll: \
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

$(INSTALL)/usr/bin/make.exe: \
  $(SRC)/utils/make/make.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/make/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/usr/bin/ar.exe: \
  $(SRC)/utils/ar/ar.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/ar/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/usr/bin/impdef.exe: \
  $(SRC)/utils/impdef/impdef.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/impdef/ $** /link /NODEFAULTLIB /FIXED:NO

$(INSTALL)/usr/bin/ctohtml.exe: \
  $(SRC)/utils/ctohtml/ctohtml.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /Fe$@ /Fo$(OBJ)/ctohtml/ $** /link /NODEFAULTLIB /FIXED:NO

#
# config
#

config: $(INSTALL)/etc/os.ini $(INSTALL)/boot/krnl.ini $(INSTALL)/etc/setup.ini

$(INSTALL)/etc/os.ini: $(BUILD)/os.ini
    copy /Y $(BUILD)\os.ini $(INSTALL)\etc\os.ini

$(INSTALL)/boot/krnl.ini: $(BUILD)/krnl.ini
    copy /Y $(BUILD)\krnl.ini $(INSTALL)\boot\krnl.ini

$(INSTALL)/etc/setup.ini: $(BUILD)/setup.ini
    copy /Y $(BUILD)\setup.ini $(INSTALL)\etc\setup.ini

#
# bootdisk
#

bootdisk: $(IMG)/sanos.flp

$(IMG)/sanos.flp: dirs sanos config $(MKDFS) $(BUILD)/bootdisk.lst
    $(MKDFS) -d $(IMG)/sanos.flp -b $(INSTALL)\boot\boot -l $(INSTALL)\boot\osldr.dll -k $(INSTALL)\boot\krnl.dll -c 1440 -i -f -S $(INSTALL) -F $(BUILD)\bootdisk.lst

#
# boothd
#

boothd: $(IMG)/sanos.vmdk

$(IMG)/sanos.vmdk: dirs sanos config sdk $(MKDFS)
    $(MKDFS) -d $(IMG)/sanos.vmdk -t vmdk -b $(INSTALL)\boot\boot -l $(INSTALL)\boot\osldr.dll -k $(INSTALL)\boot\krnl.dll -c 100M -i -f -S $(INSTALL) -T /

#
# netbootimg
#

netbootimg: $(IMG)/sanos.0

$(IMG)/sanos.0: dirs sanos $(MKDFS) $(BUILD)/bootnet.lst
    $(MKDFS) -d $(IMG)/sanos.0 -b $(INSTALL)\boot\netboot -l $(INSTALL)\boot\osldr.dll -k $(INSTALL)\boot\krnl.dll -c 512 -I 8192 -i -f -S $(TOPDIR) -F $(BUILD)\bootnet.lst

#
# bootcd
#

bootcd: $(IMG)/sanos.iso

$(IMG)/sanos.iso: dirs sanos tools
    if exist $(IMG)\sanos.iso del $(IMG)\sanos.iso
    $(MKDFS) -d $(INSTALL)\BOOTIMG.BIN -b $(INSTALL)\boot\cdemboot -l $(INSTALL)\boot\osldr.dll -k $(INSTALL)\boot\krnl.dll -c 512 -C 1440 -I 8192 -i -f -K rootdev=cd0,rootfs=cdfs
    $(MKISOFS) -J -c BOOTCAT.BIN -b BOOTIMG.BIN -o $(IMG)\sanos.iso $(INSTALL)
    del $(INSTALL)\BOOTIMG.BIN

#
# minimal
#

minimal: $(IMG)/minimal.flp

$(IMG)/minimal.flp: dirs sanos $(MKDFS) $(BUILD)/minbootdisk.lst
    $(MKDFS) -d $(IMG)/minimal.flp -b $(INSTALL)\boot\boot -l $(INSTALL)\boot\osldr.dll -k $(INSTALL)\boot\krnl.dll -c 1440 -i -f -S linux/install -F $(BUILD)\minbootdisk.lst

#
# sdk
#

sdk: $(TCC) $(NASM) $(AR) $(IMPDEF) $(CTOHTML) crt

$(AR): $(INSTALL)/usr/bin/ar.exe $(SOW)
    copy /Y $(INSTALL)\usr\bin\ar.exe $(AR)

$(IMPDEF): $(INSTALL)/usr/bin/impdef.exe $(SOW)
    copy /Y $(INSTALL)\usr\bin\impdef.exe $(IMPDEF)

$(CTOHTML): $(INSTALL)/usr/bin/ctohtml.exe $(SOW)
    copy /Y $(INSTALL)\usr\bin\ctohtml.exe $(CTOHTML)

$(TCC): $(INSTALL)/usr/bin/cc.exe $(SOW)
    copy /Y $(INSTALL)\usr\bin\cc.exe $(TCC)

$(NASM): $(INSTALL)/usr/bin/as.exe $(SOW)
    copy /Y $(INSTALL)\usr\bin\as.exe $(NASM)

$(INSTALL)/usr/bin/cc.exe: \
  $(SDKSRC)/cc/asm386.c \
  $(SDKSRC)/cc/asm.c \
  $(SDKSRC)/cc/cc.c \
  $(SDKSRC)/cc/codegen386.c \
  $(SDKSRC)/cc/codegen.c \
  $(SDKSRC)/cc/compiler.c \
  $(SDKSRC)/cc/elf.c \
  $(SDKSRC)/cc/pe.c \
  $(SDKSRC)/cc/preproc.c \
  $(SDKSRC)/cc/symbol.c \
  $(SDKSRC)/cc/type.c \
  $(SDKSRC)/cc/util.c \
  $(LIBS)/os.lib \
  $(LIBS)/libc.lib
    $(CC) $(CFLAGS) /W1 /Fe$@ /Fo$(OBJ)/cc/ $** /D USE_LOCAL_HEAP /link /NODEFAULTLIB /FIXED:NO /HEAP:33554432,131072

$(INSTALL)/usr/bin/as.exe: $(NASMSRC) $(LIBS)/os.lib $(LIBS)/libc.lib
    $(CC) $(CFLAGS) $(NASMFLAGS) /D HAVE_SNPRINTF /D HAVE_VSNPRINTF /D SANOS /D USE_LOCAL_HEAP /Fe$@ /Fo$(OBJ)/as/ $** /link /NODEFAULTLIB /FIXED:NO /HEAP:33554432,131072

#
# crt
#

crt: $(INSTALL)/usr/lib/libc.a $(INSTALL)/usr/lib/os.def $(INSTALL)/usr/lib/krnl.def

$(INSTALL)/usr/lib/os.def: $(INSTALL)/boot/os.dll $(IMPDEF)
    $(IMPDEF) $(INSTALL)/boot/os.dll $(INSTALL)/usr/lib/os.def

$(INSTALL)/usr/lib/krnl.def: $(INSTALL)/boot/krnl.dll $(IMPDEF)
    $(IMPDEF) $(INSTALL)/boot/krnl.dll $(INSTALL)/usr/lib/krnl.def

$(INSTALL)/usr/lib/libc.a: \
  $(OBJ)/crt/tcccrt.o \
  $(OBJ)/crt/assert.o \
  $(OBJ)/crt/bsearch.o \
  $(OBJ)/crt/conio.o \
  $(OBJ)/crt/crt0.o \
  $(OBJ)/crt/ctype.o \
  $(OBJ)/crt/dirent.o \
  $(OBJ)/crt/fcvt.o \
  $(OBJ)/crt/fnmatch.o \
  $(OBJ)/crt/fork.o \
  $(OBJ)/crt/getopt.o \
  $(OBJ)/crt/glob.o \
  $(OBJ)/crt/hash.o \
  $(OBJ)/crt/inifile.o \
  $(OBJ)/crt/input.o \
  $(OBJ)/crt/mman.o \
  $(OBJ)/crt/math.o \
  $(OBJ)/crt/opts.o \
  $(OBJ)/crt/output.o \
  $(OBJ)/crt/qsort.o \
  $(OBJ)/crt/random.o \
  $(OBJ)/crt/readline.o \
  $(OBJ)/crt/rmap.o \
  $(OBJ)/crt/rtttl.o \
  $(OBJ)/crt/sched.o \
  $(OBJ)/crt/semaphore.o \
  $(OBJ)/crt/stdio.o \
  $(OBJ)/crt/shlib.o \
  $(OBJ)/crt/scanf.o \
  $(OBJ)/crt/printf.o \
  $(OBJ)/crt/tmpfile.o \
  $(OBJ)/crt/popen.o \
  $(OBJ)/crt/stdlib.o \
  $(OBJ)/crt/strftime.o \
  $(OBJ)/crt/string.o \
  $(OBJ)/crt/strtod.o \
  $(OBJ)/crt/strtol.o \
  $(OBJ)/crt/termios.o \
  $(OBJ)/crt/time.o \
  $(OBJ)/crt/xtoa.o \
  $(OBJ)/crt/regcomp.o \
  $(OBJ)/crt/regexec.o \
  $(OBJ)/crt/regerror.o \
  $(OBJ)/crt/regfree.o \
  $(OBJ)/crt/barrier.o \
  $(OBJ)/crt/condvar.o \
  $(OBJ)/crt/mutex.o \
  $(OBJ)/crt/pthread.o \
  $(OBJ)/crt/rwlock.o \
  $(OBJ)/crt/spinlock.o \
  $(OBJ)/crt/setjmp.o \
  $(OBJ)/crt/chkstk.o \
  $(OBJ)/crt/acos.o \
  $(OBJ)/crt/asin.o \
  $(OBJ)/crt/atan.o \
  $(OBJ)/crt/atan2.o \
  $(OBJ)/crt/ceil.o \
  $(OBJ)/crt/cos.o \
  $(OBJ)/crt/cosh.o \
  $(OBJ)/crt/exp.o \
  $(OBJ)/crt/fabs.o \
  $(OBJ)/crt/floor.o \
  $(OBJ)/crt/fmod.o \
  $(OBJ)/crt/fpconst.o \
  $(OBJ)/crt/fpreset.o \
  $(OBJ)/crt/frexp.o \
  $(OBJ)/crt/ftol.o \
  $(OBJ)/crt/ldexp.o \
  $(OBJ)/crt/log.o \
  $(OBJ)/crt/log10.o \
  $(OBJ)/crt/modf.o \
  $(OBJ)/crt/pow.o \
  $(OBJ)/crt/sin.o \
  $(OBJ)/crt/sinh.o \
  $(OBJ)/crt/sqrt.o \
  $(OBJ)/crt/tan.o \
  $(OBJ)/crt/tanh.o
    $(AR) -s $@ $**

$(OBJ)/crt/tcccrt.o: $(SRC)/lib/tcccrt.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/assert.o: $(SRC)/lib/assert.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/bsearch.o: $(SRC)/lib/bsearch.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/conio.o: $(SRC)/lib/conio.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/crt0.o: $(SRC)/lib/crt0.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/ctype.o: $(SRC)/lib/ctype.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/dirent.o: $(SRC)/lib/dirent.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/fcvt.o: $(SRC)/lib/fcvt.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/fnmatch.o: $(SRC)/lib/fnmatch.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/fork.o: $(SRC)/lib/fork.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/getopt.o: $(SRC)/lib/getopt.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/glob.o: $(SRC)/lib/glob.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/hash.o: $(SRC)/lib/hash.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/inifile.o: $(SRC)/lib/inifile.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/input.o: $(SRC)/lib/input.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/math.o: $(SRC)/lib/math.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/mman.o: $(SRC)/lib/mman.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/opts.o: $(SRC)/lib/opts.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/output.o: $(SRC)/lib/output.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/qsort.o: $(SRC)/lib/qsort.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/random.o: $(SRC)/lib/random.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/readline.o: $(SRC)/lib/readline.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/rmap.o: $(SRC)/lib/rmap.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/rtttl.o: $(SRC)/lib/rtttl.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/sched.o: $(SRC)/lib/sched.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/semaphore.o: $(SRC)/lib/semaphore.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/stdio.o: $(SRC)/lib/stdio.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/shlib.o: $(SRC)/lib/shlib.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/scanf.o: $(SRC)/lib/scanf.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/printf.o: $(SRC)/lib/printf.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/tmpfile.o: $(SRC)/lib/tmpfile.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/popen.o: $(SRC)/lib/popen.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/setjmp.o: $(SRC)/lib/setjmp.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/stdlib.o: $(SRC)/lib/stdlib.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/strftime.o: $(SRC)/lib/strftime.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/string.o: $(SRC)/lib/string.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/strtod.o: $(SRC)/lib/strtod.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/strtol.o: $(SRC)/lib/strtol.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/termios.o: $(SRC)/lib/termios.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/time.o: $(SRC)/lib/time.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/xtoa.o: $(SRC)/lib/xtoa.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/regcomp.o: $(SRC)/lib/regex/regcomp.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/regexec.o: $(SRC)/lib/regex/regexec.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/regerror.o: $(SRC)/lib/regex/regerror.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/regfree.o: $(SRC)/lib/regex/regfree.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/barrier.o: $(SRC)/lib/pthread/barrier.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/condvar.o: $(SRC)/lib/pthread/condvar.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/mutex.o: $(SRC)/lib/pthread/mutex.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/pthread.o: $(SRC)/lib/pthread/pthread.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/rwlock.o: $(SRC)/lib/pthread/rwlock.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/spinlock.o: $(SRC)/lib/pthread/spinlock.c
    $(TCC) -c $** -o $@ -I$(SRC)/include -g

$(OBJ)/crt/chkstk.o: $(SRC)/lib/chkstk.s
    $(TCC) -c $** -o $@ -I$(SRC)/include

$(OBJ)/crt/acos.o: $(SRC)/lib/math/acos.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/asin.o: $(SRC)/lib/math/asin.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/atan.o: $(SRC)/lib/math/atan.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/atan2.o: $(SRC)/lib/math/atan2.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/ceil.o: $(SRC)/lib/math/ceil.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/cos.o: $(SRC)/lib/math/cos.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/cosh.o: $(SRC)/lib/math/cosh.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/exp.o: $(SRC)/lib/math/exp.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/fabs.o: $(SRC)/lib/math/fabs.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/floor.o: $(SRC)/lib/math/floor.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/fmod.o: $(SRC)/lib/math/fmod.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/fpconst.o: $(SRC)/lib/math/fpconst.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/fpreset.o: $(SRC)/lib/math/fpreset.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/frexp.o: $(SRC)/lib/math/frexp.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/ftol.o: $(SRC)/lib/math/ftol.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/ldexp.o: $(SRC)/lib/math/ldexp.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/log.o: $(SRC)/lib/math/log.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/log10.o: $(SRC)/lib/math/log10.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/modf.o: $(SRC)/lib/math/modf.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/pow.o: $(SRC)/lib/math/pow.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/sin.o: $(SRC)/lib/math/sin.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/sinh.o: $(SRC)/lib/math/sinh.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/sqrt.o: $(SRC)/lib/math/sqrt.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/tan.o: $(SRC)/lib/math/tan.asm
    $(NASM) -f elf $** -o $@

$(OBJ)/crt/tanh.o: $(SRC)/lib/math/tanh.asm
    $(NASM) -f elf $** -o $@

sdkdisk: sanos sdk source boothd

#
# source
#

source: dirs
    -@if not exist $(INSTALL)\usr mkdir $(INSTALL)\usr
    -@if not exist $(INSTALL)\usr\include mkdir $(INSTALL)\usr\include
    -@if not exist $(INSTALL)\usr\src mkdir $(INSTALL)\usr\src
    -@if not exist $(INSTALL)\usr\src\lib mkdir $(INSTALL)\usr\src\lib
    -@if not exist $(INSTALL)\usr\src\sys mkdir $(INSTALL)\usr\src\sys
    -@if not exist $(INSTALL)\usr\src\utils mkdir $(INSTALL)\usr\src\utils
    -@if not exist $(INSTALL)\usr\src\cmds mkdir $(INSTALL)\usr\src\cmds
    -@if not exist $(INSTALL)\usr\src\win32 mkdir $(INSTALL)\usr\src\win32
    copy /Y $(SRC)\Makefile $(INSTALL)\usr\src
    xcopy /S /I /Y /Q $(SRC)\include $(INSTALL)\usr\include
    xcopy /S /I /Y /Q $(SRC)\lib $(INSTALL)\usr\src\lib
    xcopy /S /I /Y /Q $(SRC)\sys $(INSTALL)\usr\src\sys
    xcopy /S /I /Y /Q $(SRC)\utils $(INSTALL)\usr\src\utils
    xcopy /S /I /Y /Q $(SRC)\cmds $(INSTALL)\usr\src\cmds
    xcopy /S /I /Y /Q $(SRC)\win32 $(INSTALL)\usr\src\win32
    xcopy /S /I /Y /Q $(SDKSRC)\cc $(INSTALL)\usr\src\utils\cc
    xcopy /S /I /Y /Q $(SDKSRC)\as $(INSTALL)\usr\src\utils\as

#
# crosstools
#

crosstools: $(BIN)/as.exe $(BIN)/cc.exe $(BIN)/sh.exe $(BIN)/make.exe $(BIN)/ar.exe $(BIN)/os.dll $(BIN)/mkdfs.exe

$(BIN)/as.exe: $(INSTALL)/usr/bin/as.exe
    copy /Y $(INSTALL)\usr\bin\as.exe $(BIN)\as.exe

$(BIN)/cc.exe: $(INSTALL)/usr/bin/cc.exe
    copy /Y $(INSTALL)\usr\bin\cc.exe $(BIN)\cc.exe

$(BIN)/sh.exe: $(INSTALL)/bin/sh.exe
    copy /Y $(INSTALL)\bin\sh.exe $(BIN)\sh.exe

$(BIN)/make.exe: $(INSTALL)/usr/bin/make.exe
    copy /Y $(INSTALL)\usr\bin\make.exe $(BIN)\make.exe

$(BIN)/ar.exe: $(INSTALL)/usr/bin/ar.exe
    copy /Y $(INSTALL)\usr\bin\ar.exe $(BIN)\ar.exe

$(BIN)/os.dll: $(SOW)
    copy /Y $(SOW) $(BIN)\os.dll

$(BIN)/mkdfs.exe: $(MKDFS)
    copy /Y $(MKDFS) $(BIN)\mkdfs.exe
