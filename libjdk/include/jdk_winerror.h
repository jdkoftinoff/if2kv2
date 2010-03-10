#ifndef _JDK_WINERROR_H
#deifne _JDK_WINERROR_H

#if JDK_IS_WIN32
#include "jdk_world.h"
#include "jdk_string.h"

inline void jdk_winerror( jdk_string &destination )
{
  LPVOID lpMsgBuf;
  FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
    );
  destination.cpy( lpMsgMuf );
  LocalFree( lpMsgBuf );
}

inline void jdk_winerror( FILE *f )
{
  jdk_str<1024> s;
  jdk_winerror( s );
  fprintf( f, "%s", s.c_str() );
}

inline void jdk_winerror_messagebox( const char *pattern, const char *title="Error", HWND parent=0 )
{
  jdk_str<1024> e;
  jdk_winerror(e);
  jdk_str<1024> s;
  s.form( pattern, e.c_str() );
  MessageBox( 0, s.c_str(), title, MB_ERROR );
}

#endif
#endif
