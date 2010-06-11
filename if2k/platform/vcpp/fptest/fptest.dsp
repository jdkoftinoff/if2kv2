# Microsoft Developer Studio Project File - Name="fptest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=fptest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fptest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fptest.mak" CFG="fptest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fptest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "fptest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fptest - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\..\..\libwtl\include" /I "..\..\..\..\libwn\include" /I "..\..\..\..\libjdk\include" /I "..\..\..\..\libif2k\include" /I "..\..\..\..\libzip\include" /D "NDEBUG" /D "_WINDOWS" /D JDK_HAS_THREADS=1 /D IF2K_MINI_CONFIG_FIX_BROKEN_DLL=1 /D "WIN32" /D "_MBCS" /D JDK_IS_WIN32=1 /D JDK_IS_VCPP=1 /D IF2K_MINI_VERSION=\"2.3.1\" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "fptest - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\..\libwtl\include" /I "..\..\..\..\libwn\include" /I "..\..\..\..\libjdk\include" /I "..\..\..\..\libif2k\include" /I "..\..\..\..\libzip\include" /D "_DEBUG" /D "_WINDOWS" /D JDK_HAS_THREADS=1 /D IF2K_MINI_CONFIG_FIX_BROKEN_DLL=1 /D "WIN32" /D "_MBCS" /D JDK_IS_WIN32=1 /D JDK_IS_VCPP=1 /D IF2K_MINI_VERSION=\"2.3.1\" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "fptest - Win32 Release"
# Name "fptest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\fptest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_base64.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_buf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_cgi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_dynbuf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_dynbuf_http.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_http.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_httprequest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_linesplit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_sha1.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_socket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_string.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_thread_mutex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_url.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
