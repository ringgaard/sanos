# Microsoft Developer Studio Project File - Name="boot" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=boot - Win32 SanOS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "boot.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "boot.mak" CFG="boot - Win32 SanOS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "boot - Win32 SanOS" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SanOS"
# PROP BASE Intermediate_Dir "SanOS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj\boot"
# PROP Target_Dir ""
# Begin Target

# Name "boot - Win32 SanOS"
# Begin Source File

SOURCE=..\src\sys\boot\boot.asm
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Assemble boot.asm
InputPath=..\src\sys\boot\boot.asm

"..\bin\boot" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nasmw.exe -f bin ..\src\sys\boot\boot.asm -o ..\bin\boot  -l ..\obj\boot\boot.lst

# End Custom Build
# End Source File
# End Target
# End Project
