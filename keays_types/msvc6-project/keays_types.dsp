# Microsoft Developer Studio Project File - Name="keays_types" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=keays_types - Win32 Debug MT Dll
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "keays_types.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "keays_types.mak" CFG="keays_types - Win32 Debug MT Dll"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "keays_types - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "keays_types - Win32 Release MT" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "keays_types - Win32 Release MT Dll" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "keays_types - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "keays_types - Win32 Debug MT" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "keays_types - Win32 Debug MT Dll" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "keays_types - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\build\Release6"
# PROP BASE Intermediate_Dir "..\build\Release6"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\build\Release6"
# PROP Intermediate_Dir "..\build\Release6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEAYS_TYPES_DLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEAYS_TYPES_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /machine:I386 /out:"..\build\Release6\KeaysTypes6.dll"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /machine:I386 /out:"..\build\Release6\KeaysTypes6.dll"
# Begin Special Build Tool
OutDir=.\..\build\Release6
TargetPath=\tmp\keays_types\build\Release6\KeaysTypes6.dll
TargetName=KeaysTypes6
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to bin directory
PostBuild_Cmds=echo  copy  $(TargetPath)  ..\bin\ 	copy  $(TargetPath)  ..\bin\ 	echo  copy  $(OutDir)\$(TargetName).lib  ..\bin\ 	copy  $(OutDir)\$(TargetName).lib  ..\bin\ 	echo  copy  $(OutDir)\$(TargetName).map  ..\bin\ 	copy  $(OutDir)\$(TargetName).map  ..\bin\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "keays_types - Win32 Release MT"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\build\Release6MT"
# PROP BASE Intermediate_Dir "..\build\Release6MT"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\build\Release6MT"
# PROP Intermediate_Dir "..\build\Release6MT"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEAYS_TYPES_EXPORTS" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_MT" /D "KEAYS_TYPES_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /machine:I386 /out:"..\build\Release6MT\KeaysTypes6mt.dll"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /machine:I386 /out:"..\build\Release6MT\KeaysTypes6mt.dll"
# Begin Special Build Tool
OutDir=.\..\build\Release6MT
TargetPath=\tmp\keays_types\build\Release6MT\KeaysTypes6mt.dll
TargetName=KeaysTypes6mt
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to bin directory
PostBuild_Cmds=echo  copy  $(TargetPath)  ..\bin\ 	copy  $(TargetPath)  ..\bin\ 	echo  copy  $(OutDir)\$(TargetName).lib  ..\bin\ 	copy  $(OutDir)\$(TargetName).lib  ..\bin\ 	echo  copy  $(OutDir)\$(TargetName).map  ..\bin\ 	copy  $(OutDir)\$(TargetName).map  ..\bin\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "keays_types - Win32 Release MT Dll"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\build\Release6MTDll"
# PROP BASE Intermediate_Dir "..\build\Release6MTDll"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\build\Release6MTDll"
# PROP Intermediate_Dir "..\build\Release6MTDll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_MT" /D "KEAYS_TYPES_EXPORTS" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_MT" /D "KEAYS_TYPES_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /machine:I386 /out:"..\build\Release6MTDll\KeaysTypes6md.dll"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /machine:I386 /out:"..\build\Release6MTDll\KeaysTypes6md.dll"
# Begin Special Build Tool
OutDir=.\..\build\Release6MTDll
TargetPath=\tmp\keays_types\build\Release6MTDll\KeaysTypes6md.dll
TargetName=KeaysTypes6md
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to bin directory
PostBuild_Cmds=echo  copy  $(TargetPath)  ..\bin\ 	copy  $(TargetPath)  ..\bin\ 	echo  copy  $(OutDir)\$(TargetName).lib  ..\bin\ 	copy  $(OutDir)\$(TargetName).lib  ..\bin\ 	echo  copy  $(OutDir)\$(TargetName).map  ..\bin\ 	copy  $(OutDir)\$(TargetName).map  ..\bin\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "keays_types - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\build\Debug6"
# PROP BASE Intermediate_Dir "..\build\Debug6"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\build\Debug6"
# PROP Intermediate_Dir "..\build\Debug6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEAYS_TYPES_DLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEAYS_TYPES_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"..\build\Debug6\KeaysTypes6D.dll" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"..\build\Debug6\KeaysTypes6D.dll" /pdbtype:sept
# Begin Special Build Tool
OutDir=.\..\build\Debug6
TargetPath=\tmp\keays_types\build\Debug6\KeaysTypes6D.dll
TargetName=KeaysTypes6D
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to bin directory
PostBuild_Cmds=echo  copy  $(TargetPath)  ..\bin\ 	copy  $(TargetPath)  ..\bin\ 	echo  copy  $(OutDir)\$(TargetName).lib  ..\bin\ 	copy  $(OutDir)\$(TargetName).lib  ..\bin\ 	echo  copy  $(OutDir)\$(TargetName).pdb  ..\bin\ 	copy  $(OutDir)\$(TargetName).pdb  ..\bin\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "keays_types - Win32 Debug MT"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\build\Debug6MT"
# PROP BASE Intermediate_Dir "..\build\Debug6MT"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\build\Debug6MT"
# PROP Intermediate_Dir "..\build\Debug6MT"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEAYS_TYPES_EXPORTS" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_MT" /D "KEAYS_TYPES_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"..\build\Debug6MT\KeaysTypes6mtD.dll" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"..\build\Debug6MT\KeaysTypes6mtD.dll" /pdbtype:sept
# Begin Special Build Tool
OutDir=.\..\build\Debug6MT
TargetPath=\tmp\keays_types\build\Debug6MT\KeaysTypes6mtD.dll
TargetName=KeaysTypes6mtD
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to bin directory
PostBuild_Cmds=echo  copy  $(TargetPath)  ..\bin\ 	copy  $(TargetPath)  ..\bin\ 	echo  copy  $(OutDir)\$(TargetName).lib  ..\bin\ 	copy  $(OutDir)\$(TargetName).lib  ..\bin\ 	echo  copy  $(OutDir)\$(TargetName).pdb  ..\bin\ 	copy  $(OutDir)\$(TargetName).pdb  ..\bin\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "keays_types - Win32 Debug MT Dll"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\build\Debug6MTDll"
# PROP BASE Intermediate_Dir "..\build\Debug6MTDll"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\build\Debug6MTDll"
# PROP Intermediate_Dir "..\build\Debug6MTDll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_MT" /D "KEAYS_TYPES_EXPORTS" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_MT" /D "KEAYS_TYPES_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"..\build\Debug6MTDll\KeaysTypes6mdD.dll" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"..\build\Debug6MTDll\KeaysTypes6mdD.dll" /pdbtype:sept
# Begin Special Build Tool
OutDir=.\..\build\Debug6MTDll
TargetPath=\tmp\keays_types\build\Debug6MTDll\KeaysTypes6mdD.dll
TargetName=KeaysTypes6mdD
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to bin directory
PostBuild_Cmds=echo  copy  $(TargetPath)  ..\bin\ 	copy  $(TargetPath)  ..\bin\ 	echo  copy  $(OutDir)\$(TargetName).lib  ..\bin\ 	copy  $(OutDir)\$(TargetName).lib  ..\bin\ 	echo  copy  $(OutDir)\$(TargetName).pdb  ..\bin\ 	copy  $(OutDir)\$(TargetName).pdb  ..\bin\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "keays_types - Win32 Release"
# Name "keays_types - Win32 Release MT"
# Name "keays_types - Win32 Release MT Dll"
# Name "keays_types - Win32 Debug"
# Name "keays_types - Win32 Debug MT"
# Name "keays_types - Win32 Debug MT Dll"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\keays_types.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\keays_types.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\include\keays_dll.ico
# End Source File
# Begin Source File

SOURCE=..\include\keays_types.rc
# End Source File
# End Group
# End Target
# End Project
