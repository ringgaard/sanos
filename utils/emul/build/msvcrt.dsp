# Microsoft Developer Studio Project File - Name="msvcrt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=msvcrt - Win32 SanOS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "msvcrt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "msvcrt.mak" CFG="msvcrt - Win32 SanOS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "msvcrt - Win32 SanOS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SanOS"
# PROP BASE Intermediate_Dir "SanOS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj\msvcrt"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSVCRT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /X /I "..\include" /u /D "MSVCRT_LIB" /FAs /YX"os.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 os.lib /nologo /entry:"dllmain" /dll /map /machine:I386 /nodefaultlib /implib:"..\lib/msvcrt.lib" /libpath:"..\lib"
# SUBTRACT LINK32 /pdb:none
# Begin Target

# Name "msvcrt - Win32 SanOS"
# Begin Source File

SOURCE=..\src\msvcrt\atox.c
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\bsearch.c
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\ctype.c
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\file.c
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\float.c
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\malloc.c
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\memmove.c
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\msvcrt.c
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\msvcrt.def
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\msvcrt.h
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\new.cpp
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\qsort.c
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\setjmp.c
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\string.c
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\strtol.c
# End Source File
# Begin Source File

SOURCE=..\src\msvcrt\time.c
# End Source File
# End Target
# End Project
