# Microsoft Developer Studio Project File - Name="os" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=os - Win32 SanOS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "os.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "os.mak" CFG="os - Win32 SanOS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "os - Win32 SanOS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "os - Win32 SanOSDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "os"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "os - Win32 SanOS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SanOS"
# PROP BASE Intermediate_Dir "SanOS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj\os"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /X /I "..\src\include" /u /D "OS_LIB" /YX"os.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /base:"0x7FF00000" /entry:"start" /dll /map /machine:I386 /nodefaultlib /implib:"..\lib/os.lib" /libpath:"..\lib" /fixed
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "os - Win32 SanOSDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "os___Win32_SanOSDebug"
# PROP BASE Intermediate_Dir "os___Win32_SanOSDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\dbg\bin"
# PROP Intermediate_Dir "..\dbg\obj\os"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O2 /X /I "..\src\include" /u /D "OS_LIB" /YX"os.h" /FD /c
# ADD CPP /nologo /MT /W3 /Zi /Od /X /I "..\src\include" /u /D "OS_LIB" /D "DEBUG" /YX"os.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /base:"0x7FF00000" /entry:"start" /dll /map /machine:I386 /nodefaultlib /implib:"..\lib/os.lib" /libpath:"..\lib" /fixed
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 /nologo /base:"0x7FF00000" /entry:"start" /dll /pdb:"..\dbg\symbols\os.pdb" /map /debug /machine:I386 /nodefaultlib /implib:"..\dbg\lib/os.lib" /libpath:"..\dbg\lib" /fixed
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "os - Win32 SanOS"
# Name "os - Win32 SanOSDebug"
# Begin Group "os"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\sys\os\critsect.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\os\heap.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\os\heap.h
# End Source File
# Begin Source File

SOURCE=..\src\sys\os\netdb.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\os\os.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\os\resolv.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\os\resolv.h
# End Source File
# Begin Source File

SOURCE=..\src\sys\os\sntp.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\os\sysapi.c

!IF  "$(CFG)" == "os - Win32 SanOS"

# ADD CPP /FAs

!ELSEIF  "$(CFG)" == "os - Win32 SanOSDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\sys\os\thread.c
# End Source File
# Begin Source File

SOURCE=..\src\sys\os\tls.c
# End Source File
# End Group
# Begin Group "include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\include\bitops.h
# End Source File
# Begin Source File

SOURCE=..\src\include\os\cpu.h
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

SOURCE=..\src\include\os\pe.h
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

SOURCE=..\src\lib\stdlib.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\string.c
# End Source File
# Begin Source File

SOURCE=..\src\lib\vsprintf.c
# End Source File
# End Group
# End Target
# End Project
