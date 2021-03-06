# Microsoft Developer Studio Project File - Name="if2k_licensor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=if2k_licensor - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "if2k_licensor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "if2k_licensor.mak" CFG="if2k_licensor - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "if2k_licensor - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "if2k_licensor - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "if2k_licensor - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../results"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /O2 /I "..\..\..\..\libwtl\include" /I "..\..\..\..\libwn\include" /I "..\..\..\..\libjdk\include" /I "..\..\..\..\libif2k\include" /I "..\..\..\..\libzip\include" /D "NDEBUG" /D "_WINDOWS" /D "STRICT" /D JDK_HAS_THREADS=1 /D "WIN32" /D "_MBCS" /D JDK_IS_WIN32=1 /D JDK_IS_VCPP=1 /D IF2K_MINI_VERSION=\"2.3.1\" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "c:\wtl71\include" /i "..\..\..\..\libwtl\include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "if2k_licensor - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\..\..\libwtl\include" /I "..\..\..\..\libwn\include" /I "..\..\..\..\libjdk\include" /I "..\..\..\..\libif2k\include" /I "..\..\..\..\libzip\include" /D "_DEBUG" /D JDK_DEBUG=1 /D "_WINDOWS" /D "STRICT" /D JDK_HAS_THREADS=1 /D "WIN32" /D "_MBCS" /D JDK_IS_WIN32=1 /D JDK_IS_VCPP=1 /D IF2K_MINI_VERSION=\"2.3.1\" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\..\..\..\libwtl\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "if2k_licensor - Win32 Release"
# Name "if2k_licensor - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\if2k_licensor.cpp
# End Source File
# Begin Source File

SOURCE=.\if2k_licensor.rc
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_array.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_buf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_dynbuf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_dynbuf_http.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_map.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_pair.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_serial_number.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_settings.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_string.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_thread_mem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_thread_mutex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_thread_pool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_util.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\include\if2k_mini_client.h
# End Source File
# Begin Source File

SOURCE=.\maindlg.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\if2k_licensor.exe.manifest
# End Source File
# Begin Source File

SOURCE=.\res\if2k_licensor.ico
# End Source File
# End Group
# End Target
# End Project
