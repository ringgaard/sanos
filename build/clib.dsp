# Microsoft Developer Studio Project File - Name="clib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=clib - Win32 SanOS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "clib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "clib.mak" CFG="clib - Win32 SanOS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "clib - Win32 SanOS" (based on "Win32 (x86) Static Library")
!MESSAGE "clib - Win32 SanOSDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "clib"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "clib - Win32 SanOS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SanOS"
# PROP BASE Intermediate_Dir "SanOS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj\clib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /X /I "..\src\include" /u /D "CLIB" /YX /FD /c
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\clib.lib"

!ELSEIF  "$(CFG)" == "clib - Win32 SanOSDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "clib___Win32_SanOSDebug"
# PROP BASE Intermediate_Dir "clib___Win32_SanOSDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\dbg\bin"
# PROP Intermediate_Dir "..\dbg\obj\clib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O2 /X /I "..\src\include" /u /D "CLIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /Zi /Od /X /I "..\src\include" /u /D "CLIB" /D "DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\clib.lib"
# ADD LIB32 /nologo /out:"..\dbg\lib\clib.lib"

!ENDIF 

# Begin Target

# Name "clib - Win32 SanOS"
# Name "clib - Win32 SanOSDebug"
# Begin Group "include"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\src\include\bitops.h
# End Source File
# Begin Source File

SOURCE=..\src\include\inifile.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os.h
# End Source File
# Begin Source File

SOURCE=..\src\include\rmap.h
# End Source File
# Begin Source File

SOURCE=..\src\include\stdio.h
# End Source File
# Begin Source File

SOURCE=..\src\include\stdlib.h
# End Source File
# Begin Source File

SOURCE=..\src\include\string.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\lib\bitops.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\inifile.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\opts.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\rmap.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\stdio.c
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
# End Target
# End Project
