#if JDK_IS_WIN32

#include "jdk_world.h"
#if JDK_IS_VCPP
#include <direct.h>
#define mkdir _mkdir
#endif
#include "wninstall.h"
#include "jdk_bindir.h"
#include "jdk_string.h"
#include "jdk_settings.h"
#include "jdk_lz.h"
#include "if2k_config.h"
#include "if2k_win32install_guts.h"

//extern "C" void register_if2k_win32install_packages();

static bool if2k_extract_file( const char *src_filename, const char *dest_filename )
{	
	bool r=false;
	
 	jdk_dynbuf buf;
	if( jdk_lz_decompress( "packages", src_filename, &buf ) )
	{
		r=buf.extract_file( dest_filename );		
	}
	return r;
}

static bool if2k_win32install_CreateService( const char *path)
{ 
    SC_HANDLE sc=OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);


	
    SC_HANDLE schService = CreateService( 
        sc, 			   // SCManager database 
        TEXT(IF2K_SHORT_SERVICE_NAME),     	   // name of service 
        IF2K_SERVICE_NAME,         // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_AUTO_START,      // start type 

        SERVICE_ERROR_NORMAL,      // error control type 
        path,        // services binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL                     // no password 
	);


    if (schService == NULL) 
	{
		LPVOID lpMsgBuf;
		if (!FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL ))
		{
			
			return false;
		}
		
// Process any inserts in lpMsgBuf.
// ...
		
// Display the string.
		MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
		
// Free the buffer.
		LocalFree( lpMsgBuf );
		
		return false;
	}
 
	StartService( schService, 0, NULL );

    CloseServiceHandle(schService); 
    return true;
} 




bool if2k_win32install( const jdk_settings &settings, bool old_windows, bool do_login_on_reboot, jdk_str<4096> &error_reason )
{
	error_reason.clear();

	//register_if2003_win32install_packages();

	// set required default registry settings, they may be overridden by the settings

	jdk_util_registry_set_string( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_LOCATION, 
								  "interface", "127.0.0.1:8000" );
	jdk_util_registry_set_string( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_LOCATION, 
								  "admin_interface", "127.0.0.1:8001" );

	jdk_util_registry_set_string( HKEY_LOCAL_MACHINE, IF2K_REDIR_REGISTRY_LOCATION, 
								  "redir_allow", "127.0.0.1 192.168.0.0/255.255.0.0 176.16.0.0/255.224.0.0 10.0.0.0/255.0.0.0" );	
	jdk_util_registry_set_string( HKEY_LOCAL_MACHINE, IF2K_REDIR_REGISTRY_LOCATION, 
								  "redir_redirect1", "0.0.0.0/0.0.0.0:80 0.0.0.0/0.0.0.0:8000 0.0.0.0/0.0.0.0:8080 0.0.0.0/0.0.0.0:3128" );
	jdk_util_registry_set_string( HKEY_LOCAL_MACHINE, IF2K_REDIR_REGISTRY_LOCATION, 
								  "redir_redirect_dest1", "127.0.0.1:8000" );	
	jdk_util_registry_set_string( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_LOCATION, "further_proxy", "" );		

	// now write all values in settings to the registry
	{
		PLATFORM_SETTINGS platform_settings;
		platform_settings.merge( settings );
		platform_settings.PLATFORM_SAVE_SETTINGS();
	}
	
	bool no_redir = settings.get_long("no_redir");
	bool only_redir = settings.get_long("only_redir");

	jdk_str<4096> destfname;
	jdk_process_path( "~", destfname.c_str(), destfname.getmaxlen() );
	mkdir( destfname.c_str() );
	
	if( !no_redir )
	{				
		if( !old_windows )
		{
			jdk_process_path( "@/" IF2K_REDIR_DLL_NAME, destfname.c_str(), destfname.getmaxlen() );
			if( !if2k_extract_file( IF2K_REDIR_DLL_NAME ".pkg", destfname.c_str() ) )
			{
				error_reason.form( "Error extracting file '%s' into '%s'", IF2K_REDIR_DLL_NAME ".pkg", destfname.c_str() );
				return false;
			}

			char buf[4096];
			*buf='\0';
			jdk_util_registry_get_string(
										 HKEY_LOCAL_MACHINE, 
										 "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows", "AppInit_DLLs",
										 buf,
										 sizeof(buf)-1
										 );
			
			if( *buf )
			{
				int len=strlen(buf);
				if( buf[len-1]!=';' )
				{
					strncat( buf, ";", sizeof(buf)-1 );
				}
			}
			strncat( buf, IF2K_REDIR_DLL_NAME, sizeof(buf)-1 );
			
			jdk_util_registry_set_string( HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows", "AppInit_DLLs", buf );
		}
	}	
	
	jdk_process_path( "~/if2003_win32uninstall.exe", destfname.c_str(), destfname.getmaxlen() );
	if( !if2k_extract_file( "if2003_win32uninstall.exe.pkg", destfname.c_str() ) )
	{
		error_reason.form( "Error extracting '%s' into '%s'", "if2003_win32uninstall.exe.pkg", destfname.c_str() );
		return false;
	}
	
	if( !jdk_util_registry_set_string(
									  HKEY_LOCAL_MACHINE,
									  IF2K_UNINSTALL_REGISTRY,
									  "DisplayName",
									  IF2K_TITLE_VERSION
									  ) )
	{
		error_reason.form( "Error writing to registry 'DisplayName'" );
		return false;
	}
	
	if( !jdk_util_registry_set_string(
									  HKEY_LOCAL_MACHINE,
									  IF2K_UNINSTALL_REGISTRY,
									  "UninstallString",
									  destfname.c_str()
									  ) )
	{
		error_reason.form( "Error writing to registry 'UninstallString'" );
		return false;
	}	
	
	
	if( !only_redir )
	{		
		jdk_process_path( "~/" IF2K_SHORT_SERVICE_NAME ".exe", destfname.c_str(), destfname.getmaxlen() );
		if( !if2k_extract_file( IF2K_SHORT_SERVICE_NAME ".exe.pkg", destfname.c_str() ) )
		{
			error_reason.form( "Error extracting '%s' into '%s'", IF2K_SHORT_SERVICE_NAME ".exe.pkg", destfname.c_str() );
			return false;
		}
		
		if( old_windows )
		{
			jdk_str<4096> cmdline;
			cmdline.form( "\"%s\" --run", destfname.c_str() );
			
			WNInstall_AddToRunServices( IF2K_SHORT_SERVICE_NAME, cmdline.c_str() );
			WinExec( cmdline.c_str(), SW_HIDE );
		}
		else
		{
			if( !if2k_win32install_CreateService( destfname.c_str() ) )
			{
				error_reason.form("Unable to create service '%s'", destfname.c_str() );
				return false;
			}
		}
	}

	if( !no_redir )
	{
		jdk_string_url login_url;
		login_url.form( "http://%s/", settings.get("admin_interface").c_str() );
		
		if( do_login_on_reboot )
		{
	    	WNInstall_AddToRunOnce( "iflogin", login_url.c_str() );
	    }
	}
	jdk_util_registry_set_string( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_FLAGS_LOCATION, "installed", "1" );
	jdk_util_registry_set_string( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_FLAGS_LOCATION, "installed_version", IF2K_VERSION_STRING );
	
	return true;
}

bool if2k_win32_check_old_versions()
{
	char already_installed_flag[64]="";
	char already_installed_version[64]="";
	jdk_util_registry_get_string( 
								 HKEY_LOCAL_MACHINE, 
								 IF2K_REGISTRY_FLAGS_LOCATION, 
								 "installed", 
								 already_installed_flag, 
								 sizeof( already_installed_flag ) 
								 );
	jdk_util_registry_get_string( 
								 HKEY_LOCAL_MACHINE, 
								 IF2K_REGISTRY_FLAGS_LOCATION, 
								 "installed_version",
								 already_installed_version, 
								 sizeof( already_installed_version ) 
								 );
	if( strcmp(already_installed_flag,"1")==0 )
	{
		char buf[2048];
		sprintf( buf, "%s %s is already installed.\n"
				"You must uninstall it before you install this package (%s).\n",
				 IF2K_TITLE, already_installed_version, IF2K_VERSION_STRING );
		MessageBox( 0, buf, IF2K_TITLE_VERSION, MB_OK );
		char sysdir[1024];
    	GetSystemDirectory( sysdir, sizeof(sysdir) );
		strcat( sysdir, "\\if2003" );
		strcat( sysdir, already_installed_version );
		strcat( sysdir, "\\if2003_win32uninstall.exe" );
		WinExec( sysdir, SW_SHOW );
		return false;
	}	
	else
	{
		return true;	
	}
	
}




#endif
