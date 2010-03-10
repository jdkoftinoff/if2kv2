#if JDK_IS_WIN32

#include "jdk_world.h"

#if JDK_IS_VCPP
#include <direct.h>
#endif

#include "jdk_bindir.h"
#include "wninstall.h"
#include "jdk_string.h"
#include "jdk_settings.h"
#include "jdk_util.h"
#include "jdk_lz.h"
#include "jdk_httprequest.h"
#include "jdk_dynbuf.h"

#include "if2k_config.h"
#include "if2k_win32install_redir.h"

static bool if2k_win32install_redir_extract_file( const char *src_filename, const char *dest_filename )
{	
	bool r=false;
	
 	jdk_dynbuf buf;
	if( jdk_lz_decompress( "redir_packages", src_filename, &buf ) )
	{
		r=buf.extract_file( dest_filename );		
	}
	return r;
}


bool if2k_win32install_redir( const jdk_settings &settings, bool old_windows, bool do_login_on_reboot )
{	

	// set required default registry settings, they may be overridden by the settings

//	jdk_util_registry_set_string( HKEY_LOCAL_MACHINE, IF2002_REGISTRY_LOCATION, 
//								  "interface", "127.0.0.1:8000" );
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
	
	bool no_redir = false;
	bool only_redir = true;

	jdk_str<4096> destfname;
	jdk_process_path( "~", destfname.c_str(), destfname.getmaxlen() );
	mkdir( destfname.c_str() );
	
	if( !no_redir )
	{				
		if( !old_windows )
		{
			jdk_process_path( "@/" IF2K_REDIR_DLL_NAME, destfname.c_str(), destfname.getmaxlen() );
			if( !if2k_win32install_redir_extract_file( IF2K_REDIR_DLL_NAME ".pkg", destfname.c_str() ) )
				return false;
			
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
	
	
	return true;
}

bool if2k_win32install_redir_check_old_versions()
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
				"You must uninstall it before you install this package.\n"
				"The uninstall program will run now.", IF2K_TITLE, already_installed_version );
		MessageBox( 0, buf, IF2K_TITLE_VERSION, MB_OK );
		char sysdir[1024];
    	GetSystemDirectory( sysdir, sizeof(sysdir) );
		strcat( sysdir, "\\if2k" );
		strcat( sysdir, already_installed_version );
		strcat( sysdir, "\\if2k_uninstall.exe" );
		WinExec( sysdir, SW_SHOW );
		return false;
	}	
	else
	{
		return true;	
	}
	
}




int if2k_win32install_redir_main( int argc, char **argv )
{
    bool old_windows=false;
    jdk_set_app_name( IF2K_SHORT_SERVICE_NAME " install" );

	jdk_set_home_dir( DEFAULT_APP_DIR );
	
	if( !WNInstall_IsWinNT4() && !WNInstall_IsWin2000() && !WNInstall_IsWinXP() )
	{
		old_windows=true;
	}

	if( !if2k_win32install_redir_check_old_versions() )
	{
		return false;	
	}
	
	jdk_settings_text install_settings;

	if( argc==1 )
	{
		if( !install_settings.load_file( "settings.txt" ) )
		{			
			char msg[4096];
			sprintf( msg, "Failed installation - Unable to load file:\n%s", "settings.txt" );
			MessageBox( 0, msg, IF2K_TITLE, MB_OK );
			return 1;				
		}		
	}	
	
	if( argc>1 && argv[1][0]!='-' )
	{
		// is the first parameter a url?
		if( strncmp( argv[1], "http://", 7 )==0 )
		{
			// yes! try load the settings from the specified URL.
			jdk_dynbuf settings_buf;
			jdk_http_response_header response_header;
			if( jdk_http_get( argv[1], &settings_buf, 100*1024, &response_header, 0, false )==200 )
			{
				// got it! now parse it
			   	install_settings.load_buf( settings_buf );
			}
			else
			{
				char msg[4096];
				sprintf( msg, "Failed installation - Unable to load URL:\n%s", argv[1] );
				MessageBox( 0, msg, IF2K_TITLE, MB_OK );
				return 1;
			}
		}
		else
		{
			if( !install_settings.load_file( argv[1] ) )
			{
				char msg[4096];
				sprintf( msg, "Failed installation - Unable to load file:\n%s", argv[1] );
				MessageBox( 0, msg, IF2K_TITLE, MB_OK );
				return 1;				
			}			
		}
		
		argc--;
		argv++;
	}


	install_settings.set_args(0,argc,argv );

	if( !if2k_win32install_redir( install_settings, old_windows, false ) )
	{
		//MessageBox( 0, "Failed installation", IF2002_TITLE, MB_OK );
		return 1;
	}
	return 0;
}



#endif









