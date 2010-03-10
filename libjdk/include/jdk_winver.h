#ifndef __JDK_WINVER_H
#define __JDK_WINVER_H

#if JDK_IS_WIN32 

#include <windows.h>

enum jdk_win_ver
{
	jdk_win_unknown=0,
	jdk_win_95,
	jdk_win_98,
	jdk_win_me,
	jdk_win_nt4,
	jdk_win_2000,
	jdk_win_xp,
  jdk_win_xpsp2,
  jdk_win_xpsp3,
  jdk_win_server_2003
};

inline jdk_win_ver jdk_win_getver()
{
  jdk_win_ver r;

#if JDK_IS_WINE
	return jdk_win_2000;
#else
    OSVERSIONINFO v;

    r= jdk_win_unknown;

    v.dwOSVersionInfoSize = sizeof( v );
    GetVersionEx( &v );

    if( v.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
        && v.dwMajorVersion == 4 && v.dwMinorVersion==0 )
    {
     r=jdk_win_95;
    }
	
    if( v.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
        && v.dwMajorVersion == 4 && v.dwMinorVersion==10 )
    {
      r=jdk_win_98;
    }
	
    if( v.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
        && v.dwMajorVersion == 4 && v.dwMinorVersion==90 )
    {
      r=jdk_win_me;
    }
	
    if( v.dwPlatformId == VER_PLATFORM_WIN32_NT
        && v.dwMajorVersion == 4 && v.dwMinorVersion==0 )
    {
      r=jdk_win_nt4;
    }
	
    if( v.dwPlatformId == VER_PLATFORM_WIN32_NT
        && v.dwMajorVersion == 5 && v.dwMinorVersion==0 )
    {
      r=jdk_win_2000;
    }
	
    if( v.dwPlatformId == VER_PLATFORM_WIN32_NT
        && v.dwMajorVersion == 5 && v.dwMinorVersion==1 )
    {
      r=jdk_win_xp;
    }

    if( v.dwPlatformId == VER_PLATFORM_WIN32_NT
        && v.dwMajorVersion == 5 && v.dwMinorVersion==2 )
    {
      r=jdk_win_server_2003;
    }

    if( v.dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
      // look for more info
      OSVERSIONINFOEX vex;
      vex.dwOSVersionInfoSize = sizeof( vex );
      if( GetVersionEx( (OSVERSIONINFO*)&vex )!=0 )
      {
        if( r==jdk_win_xp )
        {
          if( vex.wServicePackMajor == 2 )
          {
            r=jdk_win_xpsp2;
          }
          if( vex.wServicePackMajor == 3 )
          {
            r=jdk_win_xpsp3;
          }
        }
      }
    }
    return r;
#endif
}


#endif

#endif
