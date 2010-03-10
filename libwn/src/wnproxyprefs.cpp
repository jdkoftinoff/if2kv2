#include "jdk_world.h"
#include "wnworld.h"

#if JDK_IS_WIN32 

#include <shlobj.h>
#include <wininet.h>
#if 0
#include <ras.h>
#include "wnproxyprefs.h"
#endif
#include "jdk_string.h"
#include "jdk_settings.h"
#include "jdk_util.h"
#include "jdk_array.h"
#include "jdk_dll.h"

#if 0
#if !defined(INTERNET_PER_CONN_FLAGS)
typedef struct {
    DWORD   dwOption;            // option to be queried or set
    union {                     
        DWORD    dwValue;        // dword value for the option 
        LPSTR    pszValue;       // pointer to string value for the option         
        FILETIME ftValue;        // file-time value for the option
    } Value;    
} INTERNET_PER_CONN_OPTIONA, * LPINTERNET_PER_CONN_OPTIONA;
typedef struct {
    DWORD   dwOption;            // option to be queried or set
    union {                     
        DWORD    dwValue;        // dword value for the option 
        LPWSTR   pszValue;       // pointer to string value for the option         
        FILETIME ftValue;        // file-time value for the option
    } Value;    
} INTERNET_PER_CONN_OPTIONW, * LPINTERNET_PER_CONN_OPTIONW;
#ifdef UNICODE
typedef INTERNET_PER_CONN_OPTIONW INTERNET_PER_CONN_OPTION;
typedef LPINTERNET_PER_CONN_OPTIONW LPINTERNET_PER_CONN_OPTION;
#else
typedef INTERNET_PER_CONN_OPTIONA INTERNET_PER_CONN_OPTION;
typedef LPINTERNET_PER_CONN_OPTIONA LPINTERNET_PER_CONN_OPTION;
#endif // UNICODE

typedef struct {
    DWORD   dwSize;             // size of the INTERNET_PER_CONN_OPTION_LIST struct 
    LPSTR   pszConnection;      // connection name to set/query options 
    DWORD   dwOptionCount;      // number of options to set/query 
    DWORD   dwOptionError;      // on error, which option failed 
    LPINTERNET_PER_CONN_OPTIONA  pOptions;
                                // array of options to set/query 
} INTERNET_PER_CONN_OPTION_LISTA, * LPINTERNET_PER_CONN_OPTION_LISTA;
typedef struct {
    DWORD   dwSize;             // size of the INTERNET_PER_CONN_OPTION_LIST struct 
    LPWSTR  pszConnection;      // connection name to set/query options 
    DWORD   dwOptionCount;      // number of options to set/query 
    DWORD   dwOptionError;      // on error, which option failed 
    LPINTERNET_PER_CONN_OPTIONW  pOptions;
                                // array of options to set/query 
} INTERNET_PER_CONN_OPTION_LISTW, * LPINTERNET_PER_CONN_OPTION_LISTW;
#ifdef UNICODE
typedef INTERNET_PER_CONN_OPTION_LISTW INTERNET_PER_CONN_OPTION_LIST;
typedef LPINTERNET_PER_CONN_OPTION_LISTW LPINTERNET_PER_CONN_OPTION_LIST;
#else
typedef INTERNET_PER_CONN_OPTION_LISTA INTERNET_PER_CONN_OPTION_LIST;
typedef LPINTERNET_PER_CONN_OPTION_LISTA LPINTERNET_PER_CONN_OPTION_LIST;
#endif // UNICODE

//
// Options used in INTERNET_PER_CONN_OPTON struct
//
#define INTERNET_PER_CONN_FLAGS                         1
#define INTERNET_PER_CONN_PROXY_SERVER                  2
#define INTERNET_PER_CONN_PROXY_BYPASS                  3
#define INTERNET_PER_CONN_AUTOCONFIG_URL                4
#define INTERNET_PER_CONN_AUTODISCOVERY_FLAGS           5

//
// PER_CONN_FLAGS
//
#define PROXY_TYPE_DIRECT                               0x00000001   // direct to net
#define PROXY_TYPE_PROXY                                0x00000002   // via named proxy
#define PROXY_TYPE_AUTO_PROXY_URL                       0x00000004   // autoproxy URL
#define PROXY_TYPE_AUTO_DETECT                          0x00000008   // use autoproxy detection

//
// PER_CONN_AUTODISCOVERY_FLAGS
//
#define AUTO_PROXY_FLAG_USER_SET                        0x00000001   // user changed this setting
#define AUTO_PROXY_FLAG_ALWAYS_DETECT                   0x00000002   // force detection even when its not needed
#define AUTO_PROXY_FLAG_DETECTION_RUN                   0x00000004   // detection has been run
#define AUTO_PROXY_FLAG_MIGRATED                        0x00000008   // migration has just been done 
#define AUTO_PROXY_FLAG_DONT_CACHE_PROXY_RESULT         0x00000010   // don't cache result of host=proxy name
#define AUTO_PROXY_FLAG_CACHE_INIT_RUN                  0x00000020   // don't initalize and run unless URL expired
#define AUTO_PROXY_FLAG_DETECTION_SUSPECT               0x00000040   // if we're on a LAN & Modem, with only one IP, bad?!?

#define INTERNET_OPTION_CALLBACK                1
#define INTERNET_OPTION_CONNECT_TIMEOUT         2
#define INTERNET_OPTION_CONNECT_RETRIES         3
#define INTERNET_OPTION_CONNECT_BACKOFF         4
#define INTERNET_OPTION_SEND_TIMEOUT            5
#define INTERNET_OPTION_CONTROL_SEND_TIMEOUT    INTERNET_OPTION_SEND_TIMEOUT
#define INTERNET_OPTION_RECEIVE_TIMEOUT         6
#define INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT INTERNET_OPTION_RECEIVE_TIMEOUT
#define INTERNET_OPTION_DATA_SEND_TIMEOUT       7
#define INTERNET_OPTION_DATA_RECEIVE_TIMEOUT    8
#define INTERNET_OPTION_HANDLE_TYPE             9
#define INTERNET_OPTION_LISTEN_TIMEOUT          11
#define INTERNET_OPTION_READ_BUFFER_SIZE        12
#define INTERNET_OPTION_WRITE_BUFFER_SIZE       13

#define INTERNET_OPTION_ASYNC_ID                15
#define INTERNET_OPTION_ASYNC_PRIORITY          16

#define INTERNET_OPTION_PARENT_HANDLE           21
#define INTERNET_OPTION_KEEP_CONNECTION         22
#define INTERNET_OPTION_REQUEST_FLAGS           23
#define INTERNET_OPTION_EXTENDED_ERROR          24

#define INTERNET_OPTION_OFFLINE_MODE            26
#define INTERNET_OPTION_CACHE_STREAM_HANDLE     27
#define INTERNET_OPTION_USERNAME                28
#define INTERNET_OPTION_PASSWORD                29
#define INTERNET_OPTION_ASYNC                   30
#define INTERNET_OPTION_SECURITY_FLAGS          31
#define INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT 32
#define INTERNET_OPTION_DATAFILE_NAME           33
#define INTERNET_OPTION_URL                     34
#define INTERNET_OPTION_SECURITY_CERTIFICATE    35
#define INTERNET_OPTION_SECURITY_KEY_BITNESS    36
#define INTERNET_OPTION_REFRESH                 37
#define INTERNET_OPTION_PROXY                   38
#define INTERNET_OPTION_SETTINGS_CHANGED        39
#define INTERNET_OPTION_VERSION                 40
#define INTERNET_OPTION_USER_AGENT              41
#define INTERNET_OPTION_END_BROWSER_SESSION     42
#define INTERNET_OPTION_PROXY_USERNAME          43
#define INTERNET_OPTION_PROXY_PASSWORD          44
//#define INTERNET_OPTION_CONTEXT_VALUE           45
#define INTERNET_OPTION_CONNECT_LIMIT           46
#define INTERNET_OPTION_SECURITY_SELECT_CLIENT_CERT 47
#define INTERNET_OPTION_POLICY                  48
#define INTERNET_OPTION_DISCONNECTED_TIMEOUT    49
#define INTERNET_OPTION_CONNECTED_STATE         50
#define INTERNET_OPTION_IDLE_STATE              51
#define INTERNET_OPTION_OFFLINE_SEMANTICS       52
#define INTERNET_OPTION_SECONDARY_CACHE_KEY     53
#define INTERNET_OPTION_CALLBACK_FILTER         54
#define INTERNET_OPTION_CONNECT_TIME            55
#define INTERNET_OPTION_SEND_THROUGHPUT         56
#define INTERNET_OPTION_RECEIVE_THROUGHPUT      57
#define INTERNET_OPTION_REQUEST_PRIORITY        58
#define INTERNET_OPTION_HTTP_VERSION            59
#define INTERNET_OPTION_RESET_URLCACHE_SESSION  60
#define INTERNET_OPTION_ERROR_MASK              62
#define INTERNET_OPTION_FROM_CACHE_TIMEOUT      63
#define INTERNET_OPTION_BYPASS_EDITED_ENTRY     64
#define INTERNET_OPTION_CODEPAGE                68
#define INTERNET_OPTION_CACHE_TIMESTAMPS        69
#define INTERNET_OPTION_DISABLE_AUTODIAL        70
#define INTERNET_OPTION_MAX_CONNS_PER_SERVER     73
#define INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER 74
#define INTERNET_OPTION_PER_CONNECTION_OPTION   75
#define INTERNET_OPTION_DIGEST_AUTH_UNLOAD             76
#define INTERNET_OPTION_IGNORE_OFFLINE           77

#endif

class dll_RASAPI32
{
	jdk_dll_manager dll;
	DWORD WINAPI (*proc_RasEnumEntries)(DWORD,DWORD, RASENTRYNAME *, DWORD *, DWORD * );	
public:
	dll_RASAPI32() : dll( "RASAPI32.DLL")
	{
		proc_RasEnumEntries = 0;
		dll.get_symbol( (void **)&proc_RasEnumEntries, "RasEnumEntriesA" );
	}
	
	DWORD Call_RasEnumEntries( DWORD a, DWORD b, RASENTRYNAME *c, DWORD *d, DWORD *e )
	{
		if( proc_RasEnumEntries )
		{
			return proc_RasEnumEntries(a,b,c,d,e);	
		}
		else
		{
			return 0xffffffff;
		}		
	}
	
};


class NetscapePrefs
{
public:

	bool load( const jdk_string &file );
	bool save( const jdk_string &file );
	void clear();

	bool get( const jdk_string &key, jdk_string &result );
	bool set( const jdk_string &key, const jdk_string &value );
	bool erase( const jdk_string &key );

private:
	jdk_array< jdk_str<4096> > lines;
};


class NetscapeUserIterator
{
public:
	NetscapeUserIterator();
	~NetscapeUserIterator();

    void start();
	bool next( jdk_string &result );
private:
	HKEY key;
	int cur_item;
};

class NetscapeDumbFileIterator
{
public:
    NetscapeDumbFileIterator( const char *path_="c:\\Program Files\\Netscape\\Users\\*.*" )
        : path( path_ )
    {
        fs=0;
    }

    ~NetscapeDumbFileIterator()
    {
        if( fs!=0 )
            FindClose(fs);
    }

    void start()
    {
        if( fs!=0 )
            FindClose(fs);
        fs=0;
    }

    bool next( jdk_string &result )
    {
        do
        {
            if( !fs )
            {
                fs = FindFirstFile( path.c_str(), &data );
            }
            else
            {
                if( !FindNextFile( fs, &data ) )
                    return false;
            }
        } while (data.cFileName[0]=='.');

        result.cpy( "c:\\Program Files\\Netscape\\Users\\" );
        result.cat( data.cFileName );
        result.cat( "\\prefs.js" );

        return true;
    }
private:
    HANDLE fs;
    WIN32_FIND_DATA data;
    jdk_str<4096> path;
};

class NetscapePrefsSetProxy
{
public:
    NetscapePrefsSetProxy( const jdk_string &ip_, const jdk_string &port_ )
        : ip( ip_ ), port( port_ )
    {
    }

    ~NetscapePrefsSetProxy()
    {
    }

    void ProcessAll()
    {
        {
            NetscapeUserIterator iterator;

            iterator.start();
            jdk_str<4096> path;
            while( iterator.next( path ) )
                Process( path );
        }
        {
            NetscapeDumbFileIterator iterator;

            iterator.start();
            jdk_str<4096> path;
            while( iterator.next( path ) )
                Process( path );
        }

    }

protected:
    void Process( const jdk_string &path )
    {
        NetscapePrefs prefs;


        if( prefs.load( path ) )
        {
            prefs.set( jdk_str<64>("network.proxy.ftp"), ip );
            prefs.set( jdk_str<64>("network.proxy.ftp_port"), port );
            prefs.set( jdk_str<64>("network.proxy.http"), ip );
            prefs.set( jdk_str<64>("network.proxy.http_port"), port );
            prefs.set( jdk_str<64>("network.proxy.ssl"), ip );
            prefs.set( jdk_str<64>("network.proxy.ssl_port"), port );
            prefs.set( jdk_str<64>("network.proxy.type"), jdk_str<16>("1") );
            prefs.save( path );
        }
    }

    jdk_str<4096> ip;
    jdk_str<4096> port;
};

class NetscapePrefsUnSetProxy
{
public:
    NetscapePrefsUnSetProxy()
    {
    }

    ~NetscapePrefsUnSetProxy()
    {
    }

    void ProcessAll()
    {
        {
            NetscapeUserIterator iterator;

            iterator.start();
            jdk_str<4096> path;
            while( iterator.next( path ) )
                Process( path );
        }
        {
            NetscapeDumbFileIterator iterator;

            iterator.start();
            jdk_str<4096> path;
            while( iterator.next( path ) )
                Process( path );
        }
    }

protected:
    void Process( const jdk_string &path )
    {
        NetscapePrefs prefs;


        if( prefs.load( path ) )
        {
            prefs.erase( jdk_str<64>("network.proxy.ftp") );
            prefs.erase( jdk_str<64>("network.proxy.ftp_port") );
            prefs.erase( jdk_str<64>("network.proxy.http") );
            prefs.erase( jdk_str<64>("network.proxy.http_port") );
            prefs.erase( jdk_str<64>("network.proxy.ssl") );
            prefs.erase( jdk_str<64>("network.proxy.ssl_port") );
            prefs.erase( jdk_str<64>("network.proxy.type") );
            prefs.save( path );
        }
    }

};





NetscapeUserIterator::NetscapeUserIterator()
{
    DWORD e;

    if( (e=RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Netscape\\Netscape Navigator\\Users",
        0,
        KEY_READ,
        &key
        ))==0 )
    {
        cur_item=0;
    }
    else
    {
        cur_item=-1;
    }
}

NetscapeUserIterator::~NetscapeUserIterator()
{
    RegCloseKey( key );
}

void NetscapeUserIterator::start()
{
    if( cur_item!=-1 )
        cur_item=0;
}

bool NetscapeUserIterator::next( jdk_string &result )
{
    if( cur_item==-1 )
        return false;
        
    char buf[4096];
    strcpy( buf, "SOFTWARE\\Netscape\\Netscape Navigator\\Users\\" );
    int buflen = strlen(buf);

    DWORD sz = 4096-buflen;
    char *pos = &buf[ buflen ];

    if( RegEnumKeyEx(
        key,
        cur_item,
        pos,
        &sz,
        0,
        0,
        0,
        0
        )== 0 )
    {
        DWORD e;
        HKEY subkey;

        if( (e=RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            buf,
            0,
            KEY_READ,
            &subkey
            ))==0 )
        {
            DWORD size=result.getmaxlen();
            DWORD type;

            if( RegQueryValueEx(
                    subkey,
                    "DirRoot",
                    0,
                    &type,
                    (LPBYTE)result.c_str(),
                    &size
                )==ERROR_SUCCESS && type==REG_SZ )
            {
                result.cat( "\\prefs.js" );
                RegCloseKey(subkey);
                cur_item++;
                return true;
            }
            else
            {
                RegCloseKey(subkey);

                return false;
            }

        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool NetscapePrefs::load( const jdk_string &file )
{
	lines.clear();
	
	bool r=false;
	FILE *f = fopen( file.c_str(), "rt" );
	if( f )
	{		
		while( !feof(f) && !ferror(f) )
		{
			jdk_str<4096> *line = new jdk_str<4096>;
			
			if( jdk_read_string_line( line, f ) )
			{
				lines.add( line );
			}
			else
			{
				delete line;
				break;
			}
		}
		r=true;
		fclose(f);
	}
	return r;
}


bool NetscapePrefs::save( const jdk_string &file )
{
	bool r=false;
	FILE *f = fopen( file.c_str(), "wt" );
	if( f )
	{		
		for( int i=0; i<lines.count(); ++i )
		{
			const jdk_str<4096> *line = lines.get(i);
			if( line )
			{
				jdk_write_string_line( line, f );	
			}
		}
		r=true;
		fclose(f);
	}
	return r;
		
}

void NetscapePrefs::clear()
{
	lines.clear();	
}

bool NetscapePrefs::get( const jdk_string &key, jdk_string &result )
{
	jdk_str<4096> fancy_key;
	fancy_key.form( "user_pref(\"%s\",", key.c_str() );
	int fancy_key_len = fancy_key.len();
	
	for( int i=0; i<lines.count(); ++i )
	{
		const jdk_str<4096> *line = lines.get(i);
		if( line )
		{
			if( line->ncmp( fancy_key, fancy_key_len )==0 )
			{
				// found it! Now extract it
				result.cpy( line->getaddr( fancy_key_len ) );
				// find last ");" and remove it
				char *pos = result.str( ");" );
				if( pos )
				{
					*pos='\0';	
				}
				return true;
			}			
		}			
	}	
	// couldnt find it
	return false;
}


bool NetscapePrefs::set( const jdk_string &key, const jdk_string &value )
{
	jdk_str<4096> fancy_key;
	fancy_key.form( "user_pref(\"%s\",", key.c_str() );
	int fancy_key_len = fancy_key.len();
	jdk_str<4096> new_line;
	new_line.form( "user_pref(\"%s\", %s);", key.c_str(), value.c_str() );
	
	for( int i=0; i<lines.count(); ++i )
	{
		jdk_str<4096> *line = lines.get(i);
		if( line )
		{
			if( line->ncmp( fancy_key, fancy_key_len )==0 )
			{
				// found it! Now replace it
				line->cpy( new_line );
				return true;
			}			
		}			
	}
	
	// couldnt find the setting in there already, so just add it to the end
	lines.add( new jdk_str<4096>(new_line) );
	return true;
}

bool NetscapePrefs::erase( const jdk_string &key )
{
	jdk_str<4096> fancy_key;
	fancy_key.form( "user_pref(\"%s\",", key.c_str() );
	int fancy_key_len = fancy_key.len();
	
	for( int i=0; i<lines.count(); ++i )
	{
		const jdk_str<4096> *line = lines.get(i);
		if( line )
		{
			if( line->ncmp( fancy_key, fancy_key_len )==0 )
			{
				// found it! Now erase it
				lines.remove(i);
				return true;
			}			
		}			
	}	
	return false; // couldnt find it
}



int WNInstall_UnsetIEProxy( char *connection_name )
{
    INTERNET_PER_CONN_OPTION_LIST list;
    BOOL    bReturn;
    DWORD   dwBufSize = sizeof(list);

    // Fill out list struct.
    list.dwSize = sizeof(list);

    // NULL == LAN, otherwise connectoid name.
    list.pszConnection = connection_name;

    // Set three options.
    list.dwOptionCount = 1;
    list.pOptions = new INTERNET_PER_CONN_OPTION[1];

    // Make sure the memory was allocated.
    if(0 == list.pOptions)
    {
        //Return FALSE if the memory wasn't allocated.
        return false;
    }

    // Set flags.
    list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    list.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT;


    // Set the options on the connection.
    bReturn = InternetSetOption(0,
        INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize);

    // Free the allocated memory.
    delete [] list.pOptions;
    return true;
}


int WNInstall_SetIEProxy( char *connection_name, char *proxy )
{

    INTERNET_PER_CONN_OPTION_LIST list;
    BOOL    bReturn;
    DWORD   dwBufSize = sizeof(list);

    // Fill out list struct.
    list.dwSize = sizeof(list);

    // NULL == LAN, otherwise connectoid name.
    list.pszConnection = connection_name;
    list.dwOptionError = 0;

    // Set three options.
    list.dwOptionCount = 3;
    list.pOptions = new INTERNET_PER_CONN_OPTION[3];

    // Make sure the memory was allocated.
    if(0 == list.pOptions)
    {
        //Return FALSE if the memory wasn't allocated.
        return false;
    }

    // Set flags.
    list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    list.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT |
        PROXY_TYPE_PROXY;

    // Set proxy name.
    list.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    list.pOptions[1].Value.pszValue = proxy;

    // Set proxy override.
    list.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    list.pOptions[2].Value.pszValue = "local";

    // Set the options on the connection.
    bReturn = InternetSetOption(0,
        INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize);

    // Free the allocated memory.
    delete [] list.pOptions;

    return true;

}


struct OldRasEntryName
{
    DWORD dwSize;
    CHAR szEntryName[ RAS_MaxEntryName+1];
};

int WNInstall_UnsetInetProxy()
{
    {
        NetscapePrefsUnSetProxy netscape_setter;
        netscape_setter.ProcessAll();
    }

    WNInstall_UnsetIEProxy( 0 );

    OldRasEntryName lpRasEntryName[32];
    DWORD cb;
    DWORD cEntries;
    DWORD nRet;


    lpRasEntryName->dwSize = sizeof(OldRasEntryName);
    cb=lpRasEntryName->dwSize * 32;
    cEntries = 32;

	dll_RASAPI32 ras;
	
    // Calling RasEnumEntries to enumerate the phone-book entries
    nRet = ras.Call_RasEnumEntries(0, 0, (RASENTRYNAME *)lpRasEntryName, &cb, &cEntries);

    if (nRet != ERROR_SUCCESS)
    {

        return false;
    }
    else
    {
        OldRasEntryName *e = lpRasEntryName;
        for(unsigned int i=0;i < cEntries;i++)
        {
            //MessageBox( 0, e->szEntryName, "Dialup:", MB_OK );
            WNInstall_UnsetIEProxy( e->szEntryName );
            e++;
        }
    }

    return true;
}


int WNInstall_SetInetProxy( char *proxy )
{
    {
        char host[256];
        int port;

        if( sscanf( proxy, "%[^:]:%d", host, &port )==2 )
        {

            jdk_str<258> quoted_host;
            quoted_host.cpy( "\"" );
            quoted_host.cat( host );
            quoted_host.cat( "\"" );
            jdk_str<64> port_text;
            port_text.form( "%d", port );

            NetscapePrefsSetProxy netscape_setter(
                                                quoted_host,
                                                port_text
                                                );

            netscape_setter.ProcessAll();
        }
    }
    WNInstall_SetIEProxy(0,proxy);

    OldRasEntryName lpRasEntryName[32];
    DWORD cb;
    DWORD cEntries;
    DWORD nRet;

    lpRasEntryName->dwSize = sizeof(OldRasEntryName);
    cEntries = 32;
    cb=lpRasEntryName->dwSize * 32;

	dll_RASAPI32 ras;
    // Calling RasEnumEntries to enumerate the phone-book entries
    nRet = ras.Call_RasEnumEntries(0, 0, (RASENTRYNAME *)lpRasEntryName, &cb, &cEntries);


    if (nRet != ERROR_SUCCESS)
    {
        // search through registry for all connections for all users
        return false;
    }
    else
    {
        OldRasEntryName *e = lpRasEntryName;
        for(unsigned int i=0;i < cEntries;i++)
        {
            WNInstall_SetIEProxy( e->szEntryName, proxy );
            e++;
        }
    }
    return true;
}

#endif

#endif
