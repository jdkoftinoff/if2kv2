#include "jdk_world.h"
#include "jdk_util.h"

#if JDK_IS_VCPP
#include <malloc.h>
#endif

#if !JDK_IS_VCPP && !JDK_IS_VCPP8
#if JDK_IS_WIN32 && !JDK_IS_WINE
#include <dir.h>
#else
# include <sys/stat.h>
# include <sys/types.h>
# include <fcntl.h>
#endif
#endif


#if !JDK_IS_VCPP && !JDK_IS_VCPP8
#if JDK_IS_WIN32 && !JDK_IS_WINE 

typedef  BOOL (WINAPI* PFNINTERNETGETCONNECTEDSTATE)(LPDWORD, DWORD);

#ifndef INTERNET_CONNECTION_OFFLINE
#define INTERNET_CONNECTION_MODEM           0x01
#define INTERNET_CONNECTION_LAN             0x02
#define INTERNET_CONNECTION_PROXY           0x04
#define INTERNET_RAS_INSTALLED              0x10
#define INTERNET_CONNECTION_OFFLINE         0x20
#define INTERNET_CONNECTION_CONFIGURED      0x40
#endif

static int _InternetGetConnectedState(LPDWORD pdwFlags)
{
  PFNINTERNETGETCONNECTEDSTATE pfnInternetGetConnectedState;
  BOOL fConnected;
  
  if (!pdwFlags) return -1;
  
  HMODULE hModule = LoadLibrary("wininet.dll");
  if (!hModule)
    return -1;
  
  pfnInternetGetConnectedState =
    (PFNINTERNETGETCONNECTEDSTATE)GetProcAddress(hModule,
                                                 "InternetGetConnectedState");
  
  if (!pfnInternetGetConnectedState)
    return -1;
  
  fConnected = (*pfnInternetGetConnectedState)(pdwFlags, 0);
  
  FreeLibrary(hModule);
  return (fConnected ? 1 : 0);
}

bool jdk_internet_is_connected()
{
  DWORD con_state=0;
  int result =_InternetGetConnectedState(&con_state);
  if( result==-1 ||
      (result==1 && con_state!=INTERNET_CONNECTION_OFFLINE )
    )
  {
    return true;
  }
  else
  {
    return false;
  }	
}

#endif
#endif

bool jdk_formpath( 
  char *dest, 
  const char *path, 
  const char *file, 
  const char *extension,
  size_t max_len 
  ) 
{
  if( !dest || !path || !file )
  {
    jdk_null_pointer( "jdk_formpath()" );
    return false;
  }	
  
  if( !jdk_strncpy( dest, path, max_len ) )
  {
    return false;
  }
  
  // if path does not end in '/' or '\\' then append '/'
  
  size_t len = strlen( dest );
#if JDK_IS_WIN32 && !JDK_IS_WINE
  char sep = '\\';
#else
  char sep = '/';
#endif
  if( dest[len-1]!=sep )
  {
    if( len>=max_len )
    {
      return false;
    }
    
    dest[len]=sep;
    dest[len+1]='\0';		
  }
  
  if( !jdk_strncat( dest, file, max_len ) )
  {
    return false;
  }
  
  if( extension )
  {
    if( !jdk_strncat( dest, extension, max_len ) )
    {
      return false;	
    }
  }
  
  return true;
  
}

#if JDK_IS_WIN32
bool jdk_util_registry_get_string( HKEY area, const char *keyname, const char *field, char *result, int max_len )
{
  HKEY key;
  DWORD dummy;
  DWORD e;
  
  if( (e=RegCreateKeyExA(
         area,
         keyname,
         0,
         "",
         0,
         KEY_READ,
         0,
         &key,
         &dummy
         ))!=0 )
  {
#if 0
    LPVOID lpMsgBuf;
    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      e,
      MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &lpMsgBuf,
      0,
      NULL
      );
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK );
    LocalFree( lpMsgBuf);
#endif
    return false;
  }
  
  DWORD size=max_len;
  DWORD type;
  
  if( RegQueryValueExA(
        key,
        field,
        0,
        &type,
        (LPBYTE)result,
        &size
        )!=ERROR_SUCCESS || type!=REG_SZ )
  {
    RegCloseKey(key);
    
    return false;
  }
  RegCloseKey(key);
  
  return true;
}

bool jdk_util_registry_get_dword( HKEY area, const char *keyname, const char *field, DWORD *result )
{
  HKEY key;
  DWORD dummy;
  DWORD e;
  
  if( (e=RegCreateKeyExA(
         area,
         keyname,
         0,
         "",
         0,
         KEY_READ,
         0,
         &key,
         &dummy
         ))!=0 )
  {
#if 0
    LPVOID lpMsgBuf;
    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      e,
      MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &lpMsgBuf,
      0,
      NULL
      );
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK );
    LocalFree( lpMsgBuf);
#endif
    return false;
  }
  
  DWORD size=sizeof(DWORD);
  DWORD type;
  
  if( RegQueryValueExA(
        key,
        field,
        0,
        &type,
        (LPBYTE)result,
        &size
        )!=ERROR_SUCCESS || type!=REG_DWORD )
  {
    RegCloseKey(key);
    
    return false;
    
  }
  RegCloseKey(key);
  
  return true;
}

bool jdk_util_registry_set_string( HKEY area, const char *keyname, const char *field, const char *s )
{
  HKEY key;
  DWORD dummy;
  DWORD e;
  
  if( (e=RegCreateKeyExA(
         area,
         keyname,
         0,
         "",
         0,
         KEY_WRITE,
         0,
         &key,
         &dummy
         ))!=0 )
  {
#if 0
    LPVOID lpMsgBuf;
    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      e,
      MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &lpMsgBuf,
      0,
      NULL
      );
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK );
    LocalFree( lpMsgBuf);
#endif
    return false;
  }
  
  if( RegSetValueExA(
        key,
        field,
        0,
        REG_SZ,
        (LPBYTE)s,
        DWORD(strlen(s)+1)
        ) != ERROR_SUCCESS )
  {
    RegCloseKey(key);
    
    return false;
  }
  RegCloseKey(key);
  
  return true;
}

bool jdk_util_registry_set_dword( HKEY area, const char *keyname, const char *field, DWORD val )
{
  HKEY key;
  DWORD dummy;
  DWORD e;
  
  if( (e=RegCreateKeyExA(
         area,
         keyname,
         0,
         "",
         0,
         KEY_WRITE,
         0,
         &key,
         &dummy
         ))!=0 )
  {
#if 0
    LPVOID lpMsgBuf;
    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      e,
      MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &lpMsgBuf,
      0,
      NULL
      );
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK );
    LocalFree( lpMsgBuf);
#endif
    return false;
  }
  
  if( RegSetValueExA(
        key,
        field,
        0,
        REG_DWORD,
        (LPBYTE)&val,
        sizeof( val )
        )!=ERROR_SUCCESS )
  {
    RegCloseKey(key);
    
    return false;
  }
  RegCloseKey(key);
  
  return true;
}


bool jdk_util_registry_delete_key( HKEY area, const char *keyname, const char *field )
{
  HKEY key;
  DWORD dummy;
  DWORD e;
  
  if( (e=RegCreateKeyExA(
         area,
         keyname,
         0,
         "",
         0,
         KEY_WRITE,
         0,
         &key,
         &dummy
         ))!=0 )
  {
    return false;
  }
  
  RegDeleteKeyA( key, field );
  RegCloseKey(key);
  return true;
}


#endif

static bool jdk_home_dir_set=false;
char jdk_home_dir[4096];

void jdk_set_home_dir( const char *dir )
{
#if JDK_IS_WIN32 && !JDK_IS_WINE
  
  if( *dir=='~' )
  {
    GetSystemDirectoryA(jdk_home_dir,sizeof(jdk_home_dir) );
    jdk_strncat( jdk_home_dir, dir+1, sizeof(jdk_home_dir) );						   
  }
  else if (*dir=='@')
  {
    if( jdk_util_registry_get_string(
          HKEY_CURRENT_USER,
          "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
          "AppData",
          jdk_home_dir,
          sizeof(jdk_home_dir)
          ) )
    {
      jdk_strncat( jdk_home_dir, dir+1, sizeof(jdk_home_dir) );
    }		
    else
    {
      GetSystemDirectoryA(jdk_home_dir,sizeof(jdk_home_dir) );
      jdk_strncat( jdk_home_dir, dir+1, sizeof(jdk_home_dir) );
    }
  }
  else
  {
    jdk_strncpy( jdk_home_dir, dir, sizeof(jdk_home_dir) );	
  }
  
  // convert all '/' to NT '\\'
  
  size_t len = strlen(jdk_home_dir);
  for( size_t i=0; i<len; ++i )
  {
    if( jdk_home_dir[i]=='/' )
      jdk_home_dir[i]='\\';
  }
  
  jdk_home_dir_set=true;
  jdk_mkdir( jdk_home_dir, 0750 );
  jdk_chdir( jdk_home_dir );
  
#else		
  if( *dir=='~' || *dir=='@' )
  {		
    const char *home=getenv("HOME");
    jdk_strncpy( jdk_home_dir, home, sizeof(jdk_home_dir) );
    jdk_strncat( jdk_home_dir, dir+1, sizeof(jdk_home_dir) );		
  }
  else
  {		
    jdk_strncpy( jdk_home_dir, dir, sizeof(jdk_home_dir) );
  }
  
  jdk_home_dir_set=true;

  struct stat sbuf;
  
  if( stat(jdk_home_dir,&sbuf) !=0 )
  {
    jdk_mkdir( jdk_home_dir, 0750 );
  }

  jdk_chdir( jdk_home_dir );
#endif	
}

bool jdk_get_home_dir( char *dest, size_t max_len )
{
  if( jdk_home_dir_set )
  {
    jdk_strncpy( dest, jdk_home_dir, max_len );
    return true;
  }
  
#if JDK_IS_WIN32 && !JDK_IS_WINE
  if( !jdk_util_registry_get_string(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
        "AppData",
        dest,
        (int)max_len
        ) )
  {
    GetSystemDirectoryA(dest,(UINT)max_len);
  }
#else
  const char *home=getenv("HOME");
  jdk_strncpy( dest, home, max_len );
#endif
  
  size_t len=strlen(dest);
  if( len>0 )
  {
    if( dest[len-1]=='/' && dest[len-1]=='\\' )
    {
      dest[len-1]=0;
    }
  }
  return true;
}

static char jdk_app_name[4096] = "unknown";

bool jdk_set_app_name( const char *name )
{
  jdk_strncpy( jdk_app_name, name, sizeof( jdk_app_name ) );
  return true;
}

const char * jdk_get_app_name()
{
  return jdk_app_name;
}

bool jdk_process_path( const char *src, char *dest, size_t max_len )
{
  size_t pos=0;
  while( *src && pos<max_len-1 )
  {
    if( (*src=='~' || *src=='$') && pos==0)
    {
      if( !jdk_get_home_dir( dest, (max_len-1) - pos ) )
        return false;
      size_t len=strlen(dest);
      dest+=len;
      pos+=len;
      src++;
    }
    if( *src=='@' )
    {
#if JDK_IS_WIN32 && !JDK_IS_WINE
      GetSystemDirectoryA( dest, UINT((max_len-1)) );
      size_t len=strlen(dest);
      dest+=len;
      pos+=len;
      src++;
      if( dest[len-1]=='/' || dest[len-1]=='\\' )
      {
        dest[len-1]=0;
        pos-=1;
      }
#else
      if( !jdk_get_home_dir( dest, (max_len-1) - pos ) )
        return false;
      int len=strlen(dest);
      dest+=len;
      pos+=len;
      src++;
#endif
    }
#if JDK_IS_WIN32 && !JDK_IS_WINE		
    else if( *src=='/' )
    {
      *dest++ = '\\';
      pos++;
      src++;
    }
#endif		
    else
    {
      *dest++ = *src++;
      pos++;
    }
  }
  if( pos>=max_len-1 )
  {
    dest[pos]=0;
    return false;
  }
  *dest++=0;
  return true;
}


struct tm * jdk_localtime( time_t *t )
{
  return localtime( t );
}

FILE *jdk_fopen( const char *fname, const char *mode )
{
  return fopen( fname, mode );
}

void jdk_strcat( char *dest, const char *src )
{
  strcat( dest, src );
}

void jdk_strcpy( char *dest, const char *src )
{
   strcpy( dest, src );
}


#if JDK_IS_VCPP

int vsscanf(
  const char  *buffer,
  const char  *format,
    va_list     argPtr
  )
{
  // Get an upper bound for the # of args
  size_t count = 0;
  const char *p = format;
  while(1)
  {
    char c = *(p++);
    if(c==0) break;
    if(c=='%' && (p[0]!='*' && p[0]!='%')) ++count;
  }

  // Make a local stack
  size_t stackSize = (2+count)*sizeof(void*);
  void **newStack = (void**)alloca(stackSize);

  // Fill local stack the way sscanf likes it
  newStack[0] = (void*)buffer;
  newStack[1] = (void*)format;
  memcpy(newStack+2, argPtr, count*sizeof(void*));

  // Warp into system sscanf with new stack
  int result;
  void *savedESP;
    _asm
      {
        mov     savedESP, esp;
        mov     esp, newStack;
        call    sscanf;
        mov     esp, savedESP;
        mov     result, eax;
      }
    return result;
} 

#endif

int jdk_sscanf( const char *src, const char *fmt, ... )
{
#if !JDK_IS_VCPP8
  int r;
  va_list l;
  va_start(l,fmt);
  r=vsscanf( src, fmt, l );
  va_end(l);
  return r;
#else
	return -1;
#endif
}

int jdk_vsprintf( char *s, const char *fmt, va_list ap )
{
  int r;
  r=vsprintf( s, fmt, ap );
  return r;
}

int jdk_sprintf( char *s, const char *fmt, ... )
{
  int r;
  va_list l;
  va_start(l,fmt);
  r=vsprintf( s, fmt, l );
  va_end(l);
  return r;
}


