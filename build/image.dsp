# Microsoft Developer Studio Project File - Name="image" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=image - Win32 SanOS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "image.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "image.mak" CFG="image - Win32 SanOS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "image - Win32 SanOS" (based on "Win32 (x86) Generic Project")
!MESSAGE "image - Win32 SanOSDebug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "image"
# PROP Scc_LocalPath "."
MTL=midl.exe

!IF  "$(CFG)" == "image - Win32 SanOS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SanOS"
# PROP BASE Intermediate_Dir "SanOS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\img"
# PROP Intermediate_Dir "..\bin"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "image - Win32 SanOSDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "image___Win32_SanOSDebug"
# PROP BASE Intermediate_Dir "image___Win32_SanOSDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\img"
# PROP Intermediate_Dir "..\dbg\bin"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "image - Win32 SanOS"
# Name "image - Win32 SanOSDebug"
# Begin Source File

SOURCE=.\mkbootdisk.cmd

!IF  "$(CFG)" == "image - Win32 SanOS"

# PROP Intermediate_Dir "..\bin"
USERDEP__MKBOO="..\bin\boot"	"..\bin\osldr.dll"	"..\bin\krnl.dll"	"..\bin\pcnet32.sys"	"..\bin\ne2000.sys"	"..\bin\os.dll"	"..\bin\sh.exe"	"..\bin\fdisk.exe"	"..\bin\setup.exe"	"..\bin\msvcrt.dll"	"..\bin\kernel32.dll"	"..\bin\user32.dll"	"..\bin\advapi32.dll"	"..\bin\winmm.dll"	"..\bin\jinit.exe"	"krnl.ini"	"os.ini"	"setup.ini"	
# Begin Custom Build - Create boot disk
InputPath=.\mkbootdisk.cmd

"..\img\bootdisk.img" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkbootdisk

# End Custom Build

!ELSEIF  "$(CFG)" == "image - Win32 SanOSDebug"

USERDEP__MKBOO="$(IntDir)\boot"	"$(IntDir)\osldr.dll"	"$(IntDir)\krnl.dll"	"$(IntDir)\pcnet32.sys"	"$(IntDir)\ne2000.sys"	"$(IntDir)\os.dll"	"$(IntDir)\sh.exe"	"$(IntDir)\fdisk.exe"	"$(IntDir)\setup.exe"	"$(IntDir)\msvcrt.dll"	"$(IntDir)\kernel32.dll"	"$(IntDir)\user32.dll"	"$(IntDir)\advapi32.dll"	"$(IntDir)\winmm.dll"	"$(IntDir)\jinit.exe"	"krnl.ini"	"os.ini"	"setup.ini"	
# Begin Custom Build - Create boot disk
IntDir=.\..\dbg\bin
InputPath=.\mkbootdisk.cmd

"..\img\bootdisk.img" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkbootdisk $(IntDir) img\bootdisk

# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
