# Microsoft Developer Studio Project File - Name="dlb_main" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=dlb_main - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "dlb_main.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "dlb_main.mak" CFG="dlb_main - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "dlb_main - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "dlb_main - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dlb_main - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\include" /I "..\sys\winnt" /I "..\win\share" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DLB" /D "WIN32CON" /D "MSWIN_GRAPHICS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"..\util\dlb_main.exe"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Packaging via DLB
PostBuild_Cmds=echo chdir ..\dat	\
chdir ..\dat	\
chdir	\
 echo data >dlb.lst	\
 echo oracles >>dlb.lst	\
 if exist options echo options >>dlb.lst	\
 if exist ttyoptions echo ttyoptions >>dlb.lst	\
 if exist guioptions echo guioptions >>dlb.lst	\
 if NOT exist porthelp copy ..\sys\winnt\porthelp porthelp	\
 if exist porthelp echo porthelp >>dlb.lst	\
 echo quest.dat >>dlb.lst	\
 echo rumors >>dlb.lst	\
 echo help >>dlb.lst	\
 echo hh >>dlb.lst	\
 echo cmdhelp >>dlb.lst	\
 echo history >>dlb.lst	\
 echo opthelp >>dlb.lst	\
 echo wizhelp >>dlb.lst	\
 echo dungeon >>dlb.lst	\
 echo license >>dlb.lst	\
 for %%N in (*.lev) do echo %%N >>dlb.lst	\
 ..\util\dlb_main.exe cIf dlb.lst nhdat	\
 echo chdir ..\build	\
chdir ..\build	\
echo if NOT exist ..\binary\*.* mkdir ..\binary	\
 if NOT exist ..\binary\*.* mkdir ..\binary
# End Special Build Tool

!ELSEIF  "$(CFG)" == "dlb_main - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\include" /I "..\sys\winnt" /I "..\win\share" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DLB" /D "WIN32CON" /D "MSWIN_GRAPHICS" /FD /GZ /c
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"..\util\dlb_main.exe" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Packaging via dlb
PostBuild_Cmds=echo chdir ..\dat	\
chdir ..\dat	\
chdir	\
 echo data >dlb.lst	\
 echo oracles >>dlb.lst	\
 if exist options echo options >>dlb.lst	\
 if exist ttyoptions echo ttyoptions >>dlb.lst	\
 if exist guioptions echo guioptions >>dlb.lst	\
 if NOT exist porthelp copy ..\sys\winnt\porthelp porthelp	\
 if exist porthelp echo porthelp >>dlb.lst	\
 echo quest.dat >>dlb.lst	\
 echo rumors >>dlb.lst	\
 echo help >>dlb.lst	\
 echo hh >>dlb.lst	\
 echo cmdhelp >>dlb.lst	\
 echo history >>dlb.lst	\
 echo opthelp >>dlb.lst	\
 echo wizhelp >>dlb.lst	\
 echo dungeon >>dlb.lst	\
 echo license >>dlb.lst	\
 for %%N in (*.lev) do echo %%N >>dlb.lst	\
 ..\util\dlb_main.exe cIf dlb.lst nhdat	\
echo chdir ..\build	\
chdir ..\build	\
echo if NOT exist ..\binary\*.* mkdir ..\binary	\
if NOT exist ..\binary\*.* mkdir ..\binary
# End Special Build Tool

!ENDIF

# Begin Target

# Name "dlb_main - Win32 Release"
# Name "dlb_main - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\alloc.c
# End Source File
# Begin Source File

SOURCE=..\src\dlb.c
# End Source File
# Begin Source File

SOURCE=..\util\dlb_main.c
# End Source File
# Begin Source File

SOURCE=..\util\panic.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\dlb.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project