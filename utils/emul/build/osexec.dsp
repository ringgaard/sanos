# Microsoft Developer Studio Project File - Name="osexec" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=osexec - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "osexec.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "osexec.mak" CFG="osexec - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "osexec - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "osexec - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "osexec - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\obj\osexec"
# PROP Intermediate_Dir "..\obj\osexec"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "KERNEL" /YX /FD /c
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /entry:"start" /subsystem:console /machine:I386 /nodefaultlib /out:"../bin/osexec.exe"

!ELSEIF  "$(CFG)" == "osexec - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\obj\osexec"
# PROP Intermediate_Dir "..\obj\osexec"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "KERNEL" /YX /FD /c
# ADD BASE RSC /l 0x406 /d "_DEBUG"
# ADD RSC /l 0x406 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /entry:"start" /subsystem:console /debug /machine:I386 /nodefaultlib /out:"../bin/osexec.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "osexec - Win32 Release"
# Name "osexec - Win32 Debug"
# Begin Group "dfs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\osexec\bitops.c
# End Source File
# Begin Source File

SOURCE=..\src\osexec\bitops.h
# End Source File
# Begin Source File

SOURCE=..\src\osexec\buf.c
# End Source File
# Begin Source File

SOURCE=..\src\osexec\buf.h
# End Source File
# Begin Source File

SOURCE=..\src\osexec\dfs.c
# End Source File
# Begin Source File

SOURCE=..\src\osexec\dfs.h
# End Source File
# Begin Source File

SOURCE=..\src\osexec\dir.c
# End Source File
# Begin Source File

SOURCE=..\src\osexec\file.c
# End Source File
# Begin Source File

SOURCE=..\src\osexec\group.c
# End Source File
# Begin Source File

SOURCE=..\src\osexec\inode.c
# End Source File
# Begin Source File

SOURCE=..\src\osexec\super.c
# End Source File
# Begin Source File

SOURCE=..\src\osexec\types.h
# End Source File
# Begin Source File

SOURCE=..\src\osexec\vfs.c
# End Source File
# Begin Source File

SOURCE=..\src\osexec\vfs.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\osexec\osexec.c
# End Source File
# End Target
# End Project
