# Microsoft Developer Studio Project File - Name="krnl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=krnl - Win32 SanOS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "krnl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "krnl.mak" CFG="krnl - Win32 SanOS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "krnl - Win32 SanOS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "krnl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "krnl"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "krnl - Win32 SanOS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SanOS"
# PROP BASE Intermediate_Dir "SanOS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj\krnl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KRNL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /X /I "..\src\include" /u /D "KERNEL" /YX"krnl.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /base:"0x80000000" /entry:"start" /dll /pdb:none /map /machine:I386 /nodefaultlib /implib:"..\lib/krnl.lib" /fixed /comment:driver
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Creating boot disk
PostBuild_Cmds=mkbootdisk
# End Special Build Tool

!ELSEIF  "$(CFG)" == "krnl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "krnl___Win32_Debug"
# PROP BASE Intermediate_Dir "krnl___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj\krnl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O2 /X /I "..\src\include" /u /D "KERNEL" /YX"krnl.h" /FD /c
# ADD CPP /nologo /MT /W3 /Zi /Od /X /I "..\src\include" /u /D "KERNEL" /D "DEBUG" /YX"krnl.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /base:"0x80000000" /entry:"start" /dll /pdb:none /map /machine:I386 /nodefaultlib /implib:"..\lib/krnl.lib" /fixed /comment:driver
# ADD LINK32 /nologo /base:"0x80000000" /entry:"start" /dll /map /debug /machine:I386 /nodefaultlib /implib:"..\lib/krnl.lib" /fixed /comment:driver
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Creating boot disk
PostBuild_Cmds=mkbootdisk
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "krnl - Win32 SanOS"
# Name "krnl - Win32 Debug"
# Begin Group "krnl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\sys\krnl\buf.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\dbg.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\dev.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\fpu.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\intr.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\iop.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\kmalloc.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\kmem.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\ldr.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\object.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\pci.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\pdir.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\pframe.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\pic.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\pit.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\pnpbios.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\queue.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\sched.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\start.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\syscall.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\test.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\vfs.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\krnl\vmm.c
# End Source File
# End Group
# Begin Group "lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\lib\bitops.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\inifile.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\moddb.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\rmap.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\stdlib.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\string.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\time.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\vsprintf.c
# End Source File
# End Group
# Begin Group "dev"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\sys\dev\cons.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\dev\fd.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\dev\hd.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\dev\kbd.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\dev\ne2000.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\dev\null.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\dev\pcnet32.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\dev\ramdisk.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\dev\serial.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\dev\video.c
# End Source File
# End Group
# Begin Group "fs"

# PROP Default_Filter ""
# Begin Group "dfs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\sys\fs\dfs\dfs.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\fs\dfs\dir.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\fs\dfs\file.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\fs\dfs\group.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\fs\dfs\inode.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\fs\dfs\super.c
# End Source File
# End Group
# Begin Group "devfs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\sys\fs\devfs\devfs.c
# End Source File
# End Group
# End Group
# Begin Group "include"

# PROP Default_Filter ""
# Begin Group "include/os"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\include\os\buf.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\dbg.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\dev.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\devfs.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\dfs.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\fpu.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\intr.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\kbd.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\kmalloc.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\kmem.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\krnl.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\ldr.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\mbr.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\object.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\pci.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\pdir.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\pe.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\pframe.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\pic.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\pit.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\pnpbios.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\queue.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\sched.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\seg.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\syscall.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\syspage.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\tss.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\version.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\vfs.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\video.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\vmm.h
# End Source File
# End Group
# Begin Group "include/net"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\include\net\arp.h
# End Source File
# Begin Source File

SOURCE=..\src\include\net\ether.h
# End Source File
# Begin Source File

SOURCE=..\src\include\net\icmp.h
# End Source File
# Begin Source File

SOURCE=..\src\include\net\inet.h
# End Source File
# Begin Source File

SOURCE=..\src\include\net\ip.h
# End Source File
# Begin Source File

SOURCE=..\src\include\net\ipaddr.h
# End Source File
# Begin Source File

SOURCE=..\src\include\net\net.h
# End Source File
# Begin Source File

SOURCE=..\src\include\net\netif.h
# End Source File
# Begin Source File

SOURCE=..\src\include\net\opt.h
# End Source File
# Begin Source File

SOURCE=..\src\include\net\pbuf.h
# End Source File
# Begin Source File

SOURCE=..\src\include\net\stats.h
# End Source File
# Begin Source File

SOURCE=..\src\include\net\tcp.h
# End Source File
# Begin Source File

SOURCE=..\src\include\net\udp.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\include\bitops.h
# End Source File
# Begin Source File

SOURCE=..\src\include\inifile.h
# End Source File
# Begin Source File

SOURCE=..\src\include\moddb.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os.h
# End Source File
# Begin Source File

SOURCE=..\src\include\rmap.h
# End Source File
# Begin Source File

SOURCE=..\src\include\stdarg.h
# End Source File
# Begin Source File

SOURCE=..\src\include\stdlib.h
# End Source File
# Begin Source File

SOURCE=..\src\include\string.h
# End Source File
# Begin Source File

SOURCE=..\src\include\time.h
# End Source File
# Begin Source File

SOURCE=..\src\include\types.h
# End Source File
# End Group
# Begin Group "net"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\sys\net\arp.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\ether.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\icmp.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\inet.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\ip.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\ipaddr.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\loopif.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\netif.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\pbuf.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\stats.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\tcp.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\tcp_input.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\tcp_output.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\net\udp.c
# End Source File
# End Group
# End Target
# End Project
