# Microsoft Developer Studio Project File - Name="pcnet32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=pcnet32 - Win32 SanOS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pcnet32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pcnet32.mak" CFG="pcnet32 - Win32 SanOS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pcnet32 - Win32 SanOS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pcnet32 - Win32 SanOSDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "pcnet32"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pcnet32 - Win32 SanOS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SanOS"
# PROP BASE Intermediate_Dir "SanOS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj\pcnet32"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PCNET32_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /X /I "..\src\include" /u /D "PCNET32_LIB" /D "KERNEL" /YX"os/krnl.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 krnl.lib /nologo /base:"0x80000000" /entry:"start" /dll /map /machine:I386 /nodefaultlib /out:"..\bin/pcnet32.sys" /libpath:"..\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "pcnet32 - Win32 SanOSDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pcnet32___Win32_SanOSDebug"
# PROP BASE Intermediate_Dir "pcnet32___Win32_SanOSDebug"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\dbg\bin"
# PROP Intermediate_Dir "..\dbg\obj\pcnet32"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O2 /X /I "..\src\include" /u /D "PCNET32_LIB" /D "KERNEL" /YX"os/krnl.h" /FD /c
# ADD CPP /nologo /MT /W3 /Zi /Od /X /I "..\src\include" /u /D "PCNET32_LIB" /D "KERNEL" /D "DEBUG" /YX"os/krnl.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 krnl.lib /nologo /base:"0x80000000" /entry:"start" /dll /map /machine:I386 /nodefaultlib /out:"..\bin/pcnet32.sys" /implib:"..\lib/pcnet32.lib" /libpath:"..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 krnl.lib /nologo /base:"0x80000000" /entry:"start" /dll /pdb:"..\dbg\symbols\pcnet32.pdb" /map /debug /machine:I386 /nodefaultlib /out:"..\dbg\bin\pcnet32.sys" /implib:"..\dbg\lib/pcnet32.lib" /libpath:"..\dbg\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "pcnet32 - Win32 SanOS"
# Name "pcnet32 - Win32 SanOSDebug"
# Begin Source File

SOURCE=..\src\sys\krnl\iop.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\dev\pcnet32.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\string.c
# End Source File
# End Target
# End Project
