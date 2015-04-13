# Microsoft Developer Studio Project File - Name="lfWindows" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=lfWindows - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "lfWindows.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "lfWindows.mak" CFG="lfWindows - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "lfWindows - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "lfWindows - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "lfWindows - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "win_include" /I "ann_0.2/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "USE_LIBPNG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib glut32.lib glui32.lib libpng.lib /nologo /subsystem:console /profile /machine:I386 /libpath:"win_lib"

!ELSEIF  "$(CFG)" == "lfWindows - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "lfWindows___Win32_Debug"
# PROP BASE Intermediate_Dir "lfWindows___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "lfWindows___Win32_Debug"
# PROP Intermediate_Dir "lfWindows___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "win_include" /I "ann_0.2/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "USE_LIBPNG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib glut32.lib glui32.lib libpng.lib /nologo /subsystem:console /profile /debug /machine:I386 /nodefaultlib:"libc" /libpath:"win_lib"

!ENDIF 

# Begin Target

# Name "lfWindows - Win32 Release"
# Name "lfWindows - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ANN"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ann_0.2\src\ANN.cpp
# End Source File
# Begin Source File

SOURCE=.\ann_0.2\src\bd_pr_search.cpp
# End Source File
# Begin Source File

SOURCE=.\ann_0.2\src\bd_search.cpp
# End Source File
# Begin Source File

SOURCE=.\ann_0.2\src\bd_tree.cpp
# End Source File
# Begin Source File

SOURCE=.\ann_0.2\src\brute.cpp
# End Source File
# Begin Source File

SOURCE=.\ann_0.2\src\kd_pr_search.cpp
# End Source File
# Begin Source File

SOURCE=.\ann_0.2\src\kd_search.cpp
# End Source File
# Begin Source File

SOURCE=.\ann_0.2\src\kd_split.cpp
# End Source File
# Begin Source File

SOURCE=.\ann_0.2\src\kd_tree.cpp
# End Source File
# Begin Source File

SOURCE=.\ann_0.2\src\kd_util.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\BmpReader.cpp
# End Source File
# Begin Source File

SOURCE=.\color.cpp
# End Source File
# Begin Source File

SOURCE=.\lf.cpp
# End Source File
# Begin Source File

SOURCE=.\MLP.cpp
# End Source File
# Begin Source File

SOURCE=.\ParseArgs.cpp
# End Source File
# Begin Source File

SOURCE=.\SearchEnvironment.cpp
# End Source File
# Begin Source File

SOURCE=.\TSVQR.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BmpReader.h
# End Source File
# Begin Source File

SOURCE=.\color.h
# End Source File
# Begin Source File

SOURCE=.\compat.h
# End Source File
# Begin Source File

SOURCE=.\cvec2t.h
# End Source File
# Begin Source File

SOURCE=.\cvec3t.h
# End Source File
# Begin Source File

SOURCE=.\CVector.h
# End Source File
# Begin Source File

SOURCE=.\CVectorInline.h
# End Source File
# Begin Source File

SOURCE=.\FilterLearner.h
# End Source File
# Begin Source File

SOURCE=.\FilterLearnerInline.h
# End Source File
# Begin Source File

SOURCE=.\Image.h
# End Source File
# Begin Source File

SOURCE=.\ImageInline.h
# End Source File
# Begin Source File

SOURCE=.\Pyramid.h
# End Source File
# Begin Source File

SOURCE=.\PyramidInline.h
# End Source File
# Begin Source File

SOURCE=.\sampler.h
# End Source File
# Begin Source File

SOURCE=.\TSVQ.h
# End Source File
# Begin Source File

SOURCE=.\TSVQInline.h
# End Source File
# Begin Source File

SOURCE=.\TSVQR.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
