#include "jdk_world.h"
#include "jdk_util.h"
#include "wnworld.h"
#if JDK_IS_WIN32
#include <shlobj.h>
#include <olectl.h>
#endif
#include "wninstall.h"
#include "jdk_util.h"
#include "jdk_dll.h"

#include "jdk_string.h"
#include "jdk_settings.h"
#include "jdk_util.h"
#include "jdk_array.h"


bool WNInstall_ExtractFileListFromArchive(
                    FILE *f,
                    wn_archive_header *header,
                    wn_archive_entry *entries,
                    unsigned long max_items
                    )
{
    bool r=false;
    if( fseek( f, -((int)sizeof(*header)), SEEK_END )==0 )
    {
        if( fread( header, sizeof(*header), 1, f )==1 )
        {
            if( header->magic_number==WN_ARCHIVE_HEADER_MAGIC )
            {
                if( fseek( f, header->entry_offset, SEEK_SET )==0 )
                {
                    if( header->num_entries<=max_items )
                    {
                        if( fread( entries, sizeof(wn_archive_entry), header->num_entries, f )==header->num_entries )
                        {
                            unsigned long i;

                            for( i=0; i<header->num_entries; ++i )
                            {
                                if( entries[i].magic_number != WN_ARCHIVE_ENTRY_MAGIC )
                                {
                                    break;
                                }
                            }

                            if( i==header->num_entries )
                                r=true;
                        }
                    }
                }
            }
        }
    }

    return r;
}




bool WNInstall_ExtractFileListFromArchive(
                    const char *filename,
                    wn_archive_header *header,
                    wn_archive_entry *entries,
                    int max_items
                    )
{
    bool r=false;
    FILE *f=jdk_fopen( filename, "rb" );
    if( f )
    {
        r=WNInstall_ExtractFileListFromArchive(f,header,entries,max_items);

        fclose(f);
    }
    return r;
}

bool WNInstall_InstallFilesFromArchive( const char *fname )
{
    wn_archive_header header;
    wn_archive_entry entries[32];
    bool r=false;
    FILE *f=jdk_fopen( fname, "rb" );

    if( f )
    {
        if( WNInstall_ExtractFileListFromArchive(f, &header, entries, 32 ) )
        {
            if( WNInstall_InstallFiles( f, &header, entries ) )
            {
                r=true;
            }
        }
    }

    return r;
}

bool WNInstall_AppendFile( FILE *dest, const char *srcfile, wn_archive_entry *entry )
{
    bool r=false;

    FILE *f=jdk_fopen( srcfile, "rb" );
    if( f )
    {
        fseek( f, 0, SEEK_END );
        fseek( dest, 0, SEEK_END );

        entry->length= ftell( f );
        entry->start_offset = ftell(dest);

        unsigned char *buf=(unsigned char*)malloc( entry->length );
        fseek( f, 0, SEEK_SET );
        if( fread(buf,1,entry->length,f)==entry->length )
        {
            if( fwrite( buf, 1, entry->length, dest )==entry->length )
            {
                entry->magic_number = WN_ARCHIVE_ENTRY_MAGIC;            
                r=true;
            }
        }
        free(buf);

        fclose(f);
    }

    return r;
}

bool WNInstall_CreateArchive(
                        const char *destfname,
                        const filecopylist *files
                        )
{
    bool r=false;
    FILE *f=jdk_fopen( destfname, "ab" );
    if( f )
    {
        wn_archive_header header;
        wn_archive_entry entries[32];

        header.num_entries=0;
        header.entry_offset = 0;
        header.magic_number=WN_ARCHIVE_HEADER_MAGIC;

        while( files->src && files->dest )
        {
            jdk_strncpy(
                        entries[ header.num_entries ].src_filename,
                        files->src,
                        256
                        );
            jdk_strncpy(
                        entries[ header.num_entries ].dest_filename,
                        files->dest,
                        256
                        );

            if( !WNInstall_AppendFile( f, files->src, &entries[header.num_entries] ) )
            {
                fclose(f);
                return false;
            }

            header.num_entries++;
            files++;
        }
        fseek(f,0,SEEK_END);
        header.entry_offset = ftell( f );
        if( fwrite( entries, sizeof(wn_archive_entry), header.num_entries, f )==
            header.num_entries )
        {
            if( fwrite( &header, sizeof(header), 1, f )==1 )
            {
                r=true;
            }
        }

        fclose(f);
    }
    return r;
}


bool WNInstall_InstallFiles( filecopylist *list )
{
	char sysdir[1024];

    GetSystemDirectory( sysdir, sizeof(sysdir) );


    for( ;list->src;++list )
    {
        char realdest[1024];
        jdk_strcpy( realdest, sysdir );
        jdk_strcat( realdest, "\\" );
        jdk_strcat( realdest, list->dest );

        CopyFile( list->src, realdest, false );
    }
    return true;
}

bool WNInstall_DeleteFiles( filecopylist *list )
{
	char sysdir[1024];

    GetSystemDirectory( sysdir, sizeof(sysdir) );


    for( ;list->src;++list )
    {
        char realdest[1024];
        jdk_strcpy( realdest, sysdir );
        jdk_strcat( realdest, "\\" );
        jdk_strcat( realdest, list->dest );

        DeleteFile( realdest );
    }
    return true;
}

bool WNInstall_InstallFiles(
                FILE *f,
                const wn_archive_header *header,
                const wn_archive_entry *entries
                )
{
    unsigned long i;

    for( i=0; i<header->num_entries; ++i )
    {
        char real_dest[4096];

        jdk_process_path( entries[i].dest_filename, real_dest, 4096 );
        DeleteFile( real_dest );

        if( fseek(f,entries[i].start_offset, SEEK_SET )==0 )
        {
            FILE *dest=fopen(real_dest,"wb");
            if( dest )
            {
                unsigned char *buf=(unsigned char *)malloc( entries[i].length );

                if( fread(buf,1,entries[i].length,f)==entries[i].length )
                {
                    if( fwrite( buf, 1, entries[i].length, dest )!=entries[i].length )
                    {
                        free(buf);
                        fclose(dest);
                        break;
                    }
                }
                free(buf);

                fclose(dest);
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    return i==header->num_entries;
}

bool WNInstall_DeleteFiles(
                FILE *f,
                const wn_archive_header *header,
                const wn_archive_entry *entries
                )
{
    unsigned long i;
    for( i=0; i<header->num_entries; ++i )
    {
        char real_dest[4096];

        jdk_process_path( entries[i].dest_filename, real_dest, 4096 );
        DeleteFile( real_dest );
    }
    return true;
}

bool WNInstall_DeleteFilesFromArchive( const char *fname )
{
    wn_archive_header header;
    wn_archive_entry entries[32];
    bool r=false;
    FILE *f=fopen( fname, "rb" );

    if( f )
    {
        if( WNInstall_ExtractFileListFromArchive(f, &header, entries, 32 ) )
        {
            if( WNInstall_DeleteFiles( f, &header, entries ) )
            {
                r=true;
            }
        }
    }

    return r;
}

bool WNInstall_AddToRun( const char *key, const char *program )
{
	WNRegistry r( true, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run" );

	return r.WriteString( WNREGISTRY_MACHINE, key, key, program );
}

bool WNInstall_AddToRunOnce( const char *key, const char *program )
{
	WNRegistry r( true, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce" );

	return r.WriteString( WNREGISTRY_MACHINE, key, key, program );

}

bool WNInstall_AddToRunServices( const char *key, const char *program )
{
	WNRegistry r( true, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices" );

	return r.WriteString( WNREGISTRY_MACHINE, key, key, program );
}

bool WNInstall_RemoveFromRun( const char *key )
{
	WNRegistry r( true, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run" );
	return r.DeleteValue( key, key );

}

bool WNInstall_RemoveFromRunOnce( const char *key )
{
	WNRegistry r( true, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce" );
	return r.DeleteValue( key, key );
}

bool WNInstall_RemoveFromRunServices( const char *key )
{
	WNRegistry r( true, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices" );
	return r.DeleteValue( key, key );
}


bool WNInstall_CalcPath( const char *orig, char *dest, int destsize )
{
    if( !orig || !*orig )
        return false;

    if( strchr( orig, '\\' ) )
    {
        strncpy( dest, orig, destsize );
    }
    else
    {
        GetSystemDirectory( dest, destsize );
        strncat( dest, "\\", destsize );
        strncat( dest, orig, destsize );
    }
    return true;
}


HWND WNInstall_CreateInvisibleWindow(
                                    HINSTANCE instance,
                                    const char *clsname,
                                    const char *title,
                                    WNDPROC proc
                                    )
{
    HWND w=0;

    WNDCLASS *cls = new WNDCLASS;
    cls->style = 0;
    cls->lpfnWndProc = proc;
    cls->cbClsExtra = 0;
    cls->cbWndExtra = 0;
    cls->hInstance = instance;
    cls->hIcon = 0;
    cls->hCursor = 0;
    cls->hbrBackground = 0;
    cls->lpszMenuName = 0;
    cls->lpszClassName = new char[ strlen(clsname)+1 ];
    strcpy( (char *)cls->lpszClassName, clsname );

    if( RegisterClass( cls ) )
    {
        w = CreateWindow(
                        clsname,
                        title,
                        0, // invisible
                        0,0,10,10,
                        0, // no parent
                        0, // no menu
                        instance,
                        0  // no createparam
                        );
    }

    return w;
}


LRESULT WNInstall_SendMsgToWindow(
                            const char *clsname,
                            const char *title,
                            UINT msg,
                            WPARAM wparam,
                            LPARAM lparam
                            )
{
    HWND w = FindWindow( clsname, title );
    if( w )
    {
        return SendMessage( w, msg, wparam, lparam );
    }
    return -1;
}

#if JDK_IS_WIN32 && !JDK_IS_WINE && 0
int WNInstall_CreateShortCut(
                            LPCSTR pszShortcutFile,
                            LPCSTR pszIconFile,
                            int iconindex,
                            LPCSTR pszExe,
                            LPCSTR pszArg,
                            LPCSTR workingdir,
                            int showmode,
                            int hotkey
                            )
{
  HRESULT hres;
  int rv=1;
  IShellLink *psl;
  hres=OleInitialize(NULL);
  if (hres != S_FALSE && hres != S_OK) return rv;

  hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (void **) &psl);
  if (SUCCEEDED(hres))
  {
    IPersistFile *ppf;

    hres = psl->QueryInterface(IID_IPersistFile, (void **) &ppf);
    if (SUCCEEDED(hres))
    {
      WCHAR wsz[1024];
      MultiByteToWideChar(CP_ACP, 0, pszShortcutFile, -1, wsz, 1024);

       hres = psl->SetPath(pszExe);
       psl->SetWorkingDirectory(workingdir);
       if (showmode) psl->SetShowCmd(showmode);
       if (hotkey) psl->SetHotkey((unsigned short)hotkey);
       if (pszIconFile) psl->SetIconLocation(pszIconFile,iconindex);
       if (pszArg)
       {
         psl->SetArguments(pszArg);
       }

       if (SUCCEEDED(hres))
       {
		      hres=ppf->Save((const WCHAR*)wsz,TRUE);
          if (SUCCEEDED(hres)) rv=0;
       }
      ppf->Release();
    }
    psl->Release();
  }
  OleUninitialize();
  if( rv!=0 )
  {
    MessageBox( 0, pszShortcutFile, "Error creating shortcut", MB_OK );
  }
  return rv;
}
#endif

static char *wninstall_findinmem(char *a, char *b, int len_of_a)
{
  len_of_a -= lstrlen(b);
  while (*a && len_of_a >= 0)
  {
    char *t=a,*u=b;
    while (*t && *t == *u)
    {
      t++;
      u++;
    }
    if (!*u) return a;
    a++;
    len_of_a--;
  }
  return NULL;
}


bool WNInstall_MoveFileOnReboot(const char * pszExisting, const char * pszNew)
{	
  bool fOk = 0;
#if !JDK_IS_WINE   
  HMODULE hLib=LoadLibrary("kernel32.dll");
  if (hLib)
  {
    typedef BOOL (WINAPI *mfea_t)(LPCSTR lpExistingFileName,LPCSTR lpNewFileName,DWORD dwFlags);
    mfea_t mfea;
    mfea=(mfea_t) GetProcAddress(hLib,"MoveFileExA");
    if (mfea)
    {
      fOk=(mfea(pszExisting, NULL, MOVEFILE_DELAY_UNTIL_REBOOT)!=0);
    }
    FreeLibrary(hLib);
  }

  if (!fOk)
  {
    static char szRenameLine[1024];
    int cchRenameLine;
    char *szRenameSec = "[Rename]\r\n";
    HANDLE hfile, hfilemap;
    DWORD dwFileSize, dwRenameLinePos=0;
    static char wininit[1024];
    static char tmpbuf[1024];
    static char nulint[4]="NUL";

    if (pszNew) GetShortPathName(pszNew,tmpbuf,1024);
    else *((int *)tmpbuf) = *((int *)nulint);
    // wininit is used as a temporary here
    GetShortPathName(pszExisting,wininit,1024);
    pszExisting=wininit;
    cchRenameLine = wsprintf(szRenameLine,"%s=%s\r\n",tmpbuf,pszExisting);

    GetWindowsDirectory(wininit, 1024-16);
    lstrcat(wininit, "\\wininit.ini");
    hfile = CreateFile(wininit,
        GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hfile != INVALID_HANDLE_VALUE) 
    {
      dwFileSize = GetFileSize(hfile, NULL);
      hfilemap = CreateFileMapping(hfile, NULL, PAGE_READWRITE, 0, dwFileSize + cchRenameLine + 10, NULL);

      if (hfilemap != NULL) 
      {
        LPSTR pszWinInit = (LPSTR) MapViewOfFile(hfilemap, FILE_MAP_WRITE, 0, 0, 0);

        if (pszWinInit != NULL) 
        {
          int do_write=0;
          LPSTR pszRenameSecInFile = strstr(pszWinInit, szRenameSec);
          if (pszRenameSecInFile == NULL) 
          {
            lstrcpy(pszWinInit+dwFileSize, szRenameSec);
            dwFileSize += 10;
            dwRenameLinePos = dwFileSize;
            do_write++;
          } 
          else
          {
            char *pszFirstRenameLine = strstr(pszRenameSecInFile, "\n")+1;
            int l=pszWinInit + dwFileSize-pszFirstRenameLine;
            if (!wninstall_findinmem(pszFirstRenameLine,szRenameLine,l))
            {
              memmove(pszFirstRenameLine + cchRenameLine, pszFirstRenameLine, l);
              dwRenameLinePos = pszFirstRenameLine - pszWinInit;
              do_write++;
            }
          }

          if (do_write)
          {
            memcpy(&pszWinInit[dwRenameLinePos], szRenameLine,cchRenameLine);
            dwFileSize += cchRenameLine;
          }

          UnmapViewOfFile(pszWinInit);

          fOk++;
        }
        CloseHandle(hfilemap);
      }
      SetFilePointer(hfile, dwFileSize, NULL, FILE_BEGIN);
      SetEndOfFile(hfile);
      CloseHandle(hfile);
    }
  }
#endif
  return fOk;
	
}


void WNInstall_CreateDirectories(const char *directory)
{
    char buf[4096];
    strcpy( buf, directory );
	char *p = buf;
    while (*p == ' ') p++;
    if (!*p) return;
    if (p[1] == ':' && p[2] == '\\') p+=2;
    else if (p[0] == '\\' && p[1] == '\\')
    {
        while (*p != '\\' && *p) p++; // skip host
        if (*p) p++;
        while (*p != '\\' && *p) p++; // skip share
        if (*p) p++;
    }
    else return;

    while (*p)
    {
        while (*p != '\\' && *p) p++;
        if (!*p) CreateDirectory(directory,NULL);
        else
        {
            *p=0;
  	        CreateDirectory(directory,NULL);
            *p++ = '\\';
        }
    }
}


bool WNInstall_FileExists(const char *buf)
{
  bool a=0;
  HANDLE h;
  WIN32_FIND_DATA fd;
  h = FindFirstFile(buf,&fd);
  if (h != INVALID_HANDLE_VALUE)
  {
    FindClose(h);
    a=true;
  }
  return a;
}

void WNInstall_RemoveDirectories(const char *dirs, bool recurse)
{
    char buf[1024];

    strcpy( buf, dirs );

    if (recurse)
    {
        HANDLE h;
        WIN32_FIND_DATA fd;

        char *p = buf;
        while( *p )
            ++p;

        strcat(buf,"\\*.*" );

        h = FindFirstFile(buf,&fd);
        if (h != INVALID_HANDLE_VALUE)
        {
          do
          {
            if (fd.cFileName[0] != '.' ||
                (fd.cFileName[1] != '.' && fd.cFileName[1]))
            {
                strcpy(p+1,fd.cFileName);
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                    SetFileAttributes(buf,fd.dwFileAttributes^FILE_ATTRIBUTE_READONLY);
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    WNInstall_RemoveDirectories(buf,recurse);
                }
                else
                {
                    DeleteFile(buf);
                }
            }
          } while (FindNextFile(h,&fd));

          FindClose(h);
        }
    }

    RemoveDirectory(buf);
}



int WNInstall_RegDeleteKeys(HKEY thiskey, const char * lpSubKey)
{
	HKEY key;
	int retval=RegOpenKey(thiskey,lpSubKey,&key);
	if (retval==ERROR_SUCCESS)
	{
		char buffer[1024];
		while (RegEnumKey(key,0,buffer,1024)==ERROR_SUCCESS)
            if ((retval=WNInstall_RegDeleteKeys(key,buffer)) != ERROR_SUCCESS) break;

		RegCloseKey(key);
		retval=RegDeleteKey(thiskey,lpSubKey);
	}
	return retval;
}


void WNInstall_GetUserShellFolder(const char *name, char *out)
{
    HKEY hKey;
	if ( RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",0,KEY_READ,&hKey) == ERROR_SUCCESS)
    {
		DWORD l = 1024;
		DWORD t=REG_SZ;
		RegQueryValueEx(hKey,name,NULL,&t,(unsigned char *)out,&l );
        RegCloseKey(hKey);
    }
}



bool WNInstall_IsWin95()
{
    OSVERSIONINFO v;

    v.dwOSVersionInfoSize = sizeof( v );
    GetVersionEx( &v );

    return v.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
        && v.dwMajorVersion == 4 && v.dwMinorVersion==0;
}

bool WNInstall_IsWin98()
{
    OSVERSIONINFO v;

    v.dwOSVersionInfoSize = sizeof( v );
    GetVersionEx( &v );

    return v.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
        && v.dwMajorVersion == 4 && v.dwMinorVersion==10;
}

bool WNInstall_IsWinME()
{
    OSVERSIONINFO v;

    v.dwOSVersionInfoSize = sizeof( v );
    GetVersionEx( &v );

    return v.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
        && v.dwMajorVersion == 4 && v.dwMinorVersion==90;
}

bool WNInstall_IsWinNT4()
{
    OSVERSIONINFO v;

    v.dwOSVersionInfoSize = sizeof( v );
    GetVersionEx( &v );

    return v.dwPlatformId == VER_PLATFORM_WIN32_NT
        && v.dwMajorVersion == 4;
}

bool WNInstall_IsWin2000()
{
    OSVERSIONINFO v;

    v.dwOSVersionInfoSize = sizeof( v );
    GetVersionEx( &v );

    return v.dwPlatformId == VER_PLATFORM_WIN32_NT
        && v.dwMajorVersion == 5
		&& v.dwMinorVersion == 0;		
}

bool WNInstall_IsWinXP()
{
    OSVERSIONINFO v;

    v.dwOSVersionInfoSize = sizeof( v );
    GetVersionEx( &v );

    return v.dwPlatformId == VER_PLATFORM_WIN32_NT
        && v.dwMajorVersion == 5 
		&& v.dwMinorVersion == 1;
}



typedef  BOOL (WINAPI* PFNINTERNETGETCONNECTEDSTATE)(LPDWORD, DWORD);

#ifndef INTERNET_CONNECTION_OFFLINE
#define INTERNET_CONNECTION_MODEM           0x01
#define INTERNET_CONNECTION_LAN             0x02
#define INTERNET_CONNECTION_PROXY           0x04
#define INTERNET_RAS_INSTALLED              0x10
#define INTERNET_CONNECTION_OFFLINE         0x20
#define INTERNET_CONNECTION_CONFIGURED      0x40
#endif

int WNInstall_InternetGetConnectedState(LPDWORD pdwFlags)
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

bool WNInstall_InternetIsConnected()
{
    DWORD con_state=0;
    int result =WNInstall_InternetGetConnectedState(&con_state);
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

bool WNInstall_OpenBrowser( const char *url )
{
	SHELLEXECUTEINFO info;
	
	unsigned char *p = (unsigned char *)&info;
	for( size_t i=0; i<sizeof( info ); ++i )
	{
		*p++=0;	
	}
	
	
	info.cbSize = sizeof(info);
	info.fMask = 0;
	info.hwnd = 0;
	info.lpVerb = 0;
	info.lpFile = url;
	info.lpParameters = 0;
	info.lpDirectory = 0;
	info.nShow = SW_SHOW;
	info.hInstApp = 0;
	
	return ShellExecuteEx( &info ) != 0;	
}


