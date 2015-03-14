# Microsoft Developer Studio Project File - Name="keays_stringfile_wx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=keays_stringfile_wx - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "keays_stringfile_wx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "keays_stringfile_wx.mak" CFG="keays_stringfile_wx - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "keays_stringfile_wx - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "keays_stringfile_wx - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "keays_stringfile_wx - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "keays_stringfile_wx - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "keays_stringfile_wx - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\build\Release"
# PROP Intermediate_Dir "..\build\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\keays_stringfile_wx_r.lib"

!ELSEIF  "$(CFG)" == "keays_stringfile_wx - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\build\Debug"
# PROP Intermediate_Dir "..\build\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\keays_stringfile_wx_d.lib"

!ELSEIF  "$(CFG)" == "keays_stringfile_wx - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug Static"
# PROP BASE Intermediate_Dir "Debug Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\build\DebugStatic"
# PROP Intermediate_Dir "..\build\DebugStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\..\include" /I "..\..\..\keays_math\include" /I "..\..\..\keays_types\include" /I "..\..\...\string\include" /I "..\..\..\keays_wx_ksr\include" /I "..\..\..\keays_triangle\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\keays_stringfile_wx_ds.lib"

!ELSEIF  "$(CFG)" == "keays_stringfile_wx - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release Static"
# PROP BASE Intermediate_Dir "Release Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\build\Release Static"
# PROP Intermediate_Dir "..\build\ReleaseStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\include" /I "..\..\include" /I "..\..\..\keays_math\include" /I "..\..\..\keays_types\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "__WXMSW__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\keays_stringfile_wx_rs.lib"

!ENDIF 

# Begin Target

# Name "keays_stringfile_wx - Win32 Release"
# Name "keays_stringfile_wx - Win32 Debug"
# Name "keays_stringfile_wx - Win32 Debug Static"
# Name "keays_stringfile_wx - Win32 Release Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\Data.cpp
# End Source File
# Begin Source File

SOURCE=..\src\wxBSPFile.cpp
# End Source File
# Begin Source File

SOURCE=..\src\wxTRFile.cpp
# End Source File
# Begin Source File

SOURCE=..\src\wxURFile.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\Data.h
# End Source File
# Begin Source File

SOURCE=..\..\include\File.h
# End Source File
# Begin Source File

SOURCE=..\..\include\keays_stringfile.h
# End Source File
# Begin Source File

SOURCE=..\include\wxBSPFile.h
# End Source File
# Begin Source File

SOURCE=..\include\wxTRFile.h
# End Source File
# Begin Source File

SOURCE=..\include\wxURFile.h
# End Source File
# End Group
# End Target
# End Project
