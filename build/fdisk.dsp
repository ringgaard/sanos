# Microsoft Developer Studio Project File - Name="fdisk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=fdisk - Win32 SanOS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fdisk.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fdisk.mak" CFG="fdisk - Win32 SanOS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fdisk - Win32 SanOS" (based on "Win32 (x86) Console Application")
!MESSAGE "fdisk - Win32 SanOSDebug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "fdisk"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fdisk - Win32 SanOS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SanOS"
# PROP BASE Intermediate_Dir "SanOS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj\fdisk"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /X /I "..\src\include" /u /D "FDISK" /YX /FD /c
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 os.lib clib.lib /nologo /entry:"main" /subsystem:console /map /machine:I386 /nodefaultlib /libpath:"..\lib" /fixed:no
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fdisk - Win32 SanOSDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fdisk___Win32_SanOSDebug"
# PROP BASE Intermediate_Dir "fdisk___Win32_SanOSDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\dbg\bin"
# PROP Intermediate_Dir "..\dbg\obj\fdisk"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O2 /X /I "..\src\include" /u /D "FDISK" /YX /FD /c
# ADD CPP /nologo /MT /W3 /Zi /Od /X /I "..\src\include" /u /D "FDISK" /D "DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 os.lib clib.lib /nologo /entry:"main" /subsystem:console /map /machine:I386 /nodefaultlib /libpath:"..\lib" /fixed:no
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 os.lib clib.lib /nologo /entry:"main" /subsystem:console /pdb:"..\dbg\,KERNELfdisk.pdb" /map /debug /machine:I386 /nodefaultlib /libpath:"..\dbg\lib" /fixed:no
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "fdisk - Win32 SanOS"
# Name "fdisk - Win32 SanOSDebug"
# Begin Group "include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\include\os\dev.h
# End Source File
# Begin Source File

SOURCE=..\src\include\inifile.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\mbr.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\utils\fdisk\fdisk.c
# End Source File
# End Target
# End Project
