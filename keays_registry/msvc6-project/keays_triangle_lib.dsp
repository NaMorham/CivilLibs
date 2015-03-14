# Microsoft Developer Studio Project File - Name="keays_triangle_lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=keays_triangle_lib - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "keays_triangle_lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "keays_triangle_lib.mak" CFG="keays_triangle_lib - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "keays_triangle_lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "keays_triangle_lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "keays_triangle_lib - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "keays_triangle_lib - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "keays_triangle_lib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\build\Release"
# PROP BASE Intermediate_Dir "..\build\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\build\Release"
# PROP Intermediate_Dir "..\build\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\keays_types\include" /I "..\..\keays_math\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "__WXMSW__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\build\Release\keays_triangle_r.lib"
# Begin Special Build Tool
OutDir=.\..\build\Release
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to lib directory (RELEASE)
PostBuild_Cmds=copy  $(OutDir)\keays_triangle_r.lib  ..\lib\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "keays_triangle_lib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\build\Debug"
# PROP BASE Intermediate_Dir "..\build\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\build\Debug"
# PROP Intermediate_Dir "..\build\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\keays_types\include" /I "..\..\keays_math\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "__WXMSW__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\build\Debug\keays_triangle_d.lib"
# Begin Special Build Tool
OutDir=.\..\build\Debug
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to lib directory (DEBUG)
PostBuild_Cmds=copy  $(OutDir)\keays_triangle_d.lib  ..\lib\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "keays_triangle_lib - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\build\DebugStatic"
# PROP BASE Intermediate_Dir "..\build\DebugStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\build\DebugStatic"
# PROP Intermediate_Dir "..\build\DebugStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "__WXMSW__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\keays_types\include" /I "..\..\keays_math\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "__WXMSW__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\build\Debug\keays_triangle_d.lib"
# ADD LIB32 /nologo /out:"..\build\DebugStatic\keays_triangle_ds.lib"
# Begin Special Build Tool
OutDir=.\..\build\DebugStatic
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to lib directory (DEBUG STATIC)
PostBuild_Cmds=copy  $(OutDir)\keays_triangle_ds.lib  ..\lib\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "keays_triangle_lib - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\build\ReleaseStatic"
# PROP BASE Intermediate_Dir "..\build\ReleaseStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\build\ReleaseStatic"
# PROP Intermediate_Dir "..\build\ReleaseStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\keays_types\include" /I "..\..\keays_math\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "__WXMSW__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\keays_triangle.lib"
# ADD LIB32 /nologo /out:"..\build\ReleaseStatic\keays_triangle_rs.lib"
# Begin Special Build Tool
OutDir=.\..\build\ReleaseStatic
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to lib directory (RELEASE STATIC)
PostBuild_Cmds=copy  $(OutDir)\keays_triangle_rs.lib  ..\lib\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "keays_triangle_lib - Win32 Release"
# Name "keays_triangle_lib - Win32 Debug"
# Name "keays_triangle_lib - Win32 Debug Static"
# Name "keays_triangle_lib - Win32 Release Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\triangle.cpp
# End Source File
# Begin Source File

SOURCE=..\src\UTFile.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\keays_triangle.h
# End Source File
# Begin Source File

SOURCE=..\include\triangle.h
# End Source File
# Begin Source File

SOURCE=..\include\UTFile.h
# End Source File
# End Group
# End Target
# End Project
