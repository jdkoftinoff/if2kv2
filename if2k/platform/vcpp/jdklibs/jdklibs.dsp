# Microsoft Developer Studio Project File - Name="jdklibs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=jdklibs - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jdklibs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jdklibs.mak" CFG="jdklibs - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jdklibs - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "jdklibs - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "jdklibs - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\..\libwn\include" /I "..\..\..\..\libjdk\include" /I "..\..\..\..\libif2k\include" /I "..\..\..\..\libzip\include" /D "NDEBUG" /D "_LIB" /D "_WINDOWS" /D JDK_HAS_THREADS=1 /D "WIN32" /D "_MBCS" /D JDK_IS_WIN32=1 /D JDK_IS_VCPP=1 /D IF2K_MINI_VERSION=\"2.3.0\" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "jdklibs - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\..\..\libwn\include" /I "..\..\..\..\libjdk\include" /I "..\..\..\..\libif2k\include" /I "..\..\..\..\libzip\include" /D "_DEBUG" /D "_LIB" /D "_WINDOWS" /D JDK_HAS_THREADS=1 /D "WIN32" /D "_MBCS" /D JDK_IS_WIN32=1 /D JDK_IS_VCPP=1 /D IF2K_MINI_VERSION=\"2.3.0\" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "jdklibs - Win32 Release"
# Name "jdklibs - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "libzip"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\libzip\src\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\autoconf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\crc32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\crypt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\deflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\deflate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\gzio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\infback.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\inffast.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\inflate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\ioapi.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\ioapi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\ioapi_mem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\ioapi_mem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\iowin32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\iowin32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\minigzip.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\miniunz.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\minizip.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\trees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\trees.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\unzip.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\zconf.in.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\zip.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\zip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\zlib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\src\zutil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libzip\include\zutil.h
# End Source File
# End Group
# Begin Group "libif2k"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\libif2k\include\if2k_mini_compile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\INCLUDE\if2k_mini_config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\SRC\if2k_mini_http_proxy.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\INCLUDE\if2k_mini_http_proxy.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\INCLUDE\if2k_mini_kernel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\SRC\if2k_mini_kernel_defaults.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\SRC\if2k_mini_kernel_v2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\INCLUDE\if2k_mini_kernel_v2.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\SRC\if2k_mini_log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\INCLUDE\if2k_mini_log.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\SRC\if2k_mini_nntp_proxy.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\INCLUDE\if2k_mini_nntp_proxy.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\INCLUDE\if2k_mini_scanner.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\SRC\if2k_mini_server.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\INCLUDE\if2k_mini_server.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\include\if2k_mini_tree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\SRC\if2k_mini_tunnel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\INCLUDE\if2k_mini_tunnel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\SRC\if2k_pattern_expander.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libif2k\INCLUDE\if2k_pattern_expander.h
# End Source File
# End Group
# Begin Group "libwn"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\libzip\include\unzip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\win32installutils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\win32installutils.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wnbitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnbitmap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnbrush.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wncursor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wndc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wndelself.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wndelself.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wndialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wndialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wnextdll.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnextdll.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wninstall.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wninstall.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnlib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wnloginquery.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnloginquery.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wnmain.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnmain.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnmidi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnpen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wnproxyprefs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnproxyprefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wnquestion.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnquestion.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wnrect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnrect.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wnreg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnreg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wnregistry.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnregistry.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wnwindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnwindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\src\wnwlist.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnwlist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnworld.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libwn\include\wnws2spi.h
# End Source File
# End Group
# Begin Group "libjdk"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_archive.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_array.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_array.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_atom.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_base64.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_base64.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_bindir.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_bindir.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_buf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_buf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_buf_zip.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_cgi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_cgi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_cgisettings.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_cgisettings.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_crypt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_daemon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_daemon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_dbm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_dbm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_dll.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_dll.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_dynbuf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_dynbuf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_dynbuf_http.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_elastic_queue.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_error.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_fast_url_lookup.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_fast_url_lookup.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_fasttree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_fork_server.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_fork_server.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_heap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_heap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_html.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_html.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_html_decode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_html_decode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_html_template.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_html_template.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_http.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_http.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_http_admin.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_http_server.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_http_server.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_http_server_lua.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_httprequest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_httprequest.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_inet_server.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_ipv4.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_linesplit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_linesplit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_list.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_log.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_lz.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_lz.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_map.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_map.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_mmap_buf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_mmap_buf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_nameserver.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_nameserver.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_pair.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_pair.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_platform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_platform_beos.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_platform_cygwin.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_platform_linux.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_platform_linuxthreads.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_platform_macosx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_platform_unix.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_platform_win32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_pool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_procmap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_procmap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_questions.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_questions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_queue.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_remote_buf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_remote_buf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_rs232.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_rs232.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_scheme.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_scheme.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_serial_number.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_serial_number.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_serialsocket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_serialsocket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_server.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_server.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_server_family.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_server_family.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_settings.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_settings.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_settings_objc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_sha1.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\include\jdk_sha1.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_shmem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_shmem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_smtp_send.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_smtp_send.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_socket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_socket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_socket_beos.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_socket_unix.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_socket_win32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_socketswitch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_socketswitch.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_socketutil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_socketutil.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_stream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_string.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_string.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_string_objc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_systemsettings_macosx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_thread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_thread_mem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_thread_mutex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_thread_pool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_thread_server.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_tree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_url.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_url.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_utf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_utf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_util.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_util.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_valarray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_valmap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\src\jdk_win32svc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\include\jdk_win32svc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libjdk\include\jdk_win32wtl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_winhook.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_winver.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_world.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_xml.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\SRC\jdk_zipfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdk_zipfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\JDKAuthorization.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\LIBJDK\INCLUDE\jdkvecstring.h
# End Source File
# End Group
# End Target
# End Project
