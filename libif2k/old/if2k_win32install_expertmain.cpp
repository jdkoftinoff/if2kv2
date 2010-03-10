#if JDK_IS_WIN32
#include "jdk_world.h"
#include "jdk_bindir.h"
#include "wninstall.h"
#include "jdk_string.h"
#include "jdk_settings.h"
#include "jdk_util.h"
#include "jdk_lz.h"
#include "jdk_httprequest.h"
#include "jdk_dynbuf.h"

#include "if2k_config.h"
#include "if2k_win32install_expert.h"
#include "if2k_win32install_guts.h"



int if2k_win32install_expert_main( int argc, char **argv )
{
    bool old_windows=false;
    jdk_set_app_name( IF2K_SHORT_SERVICE_NAME " install" );
	if( !WNInstall_IsWinNT4() && !WNInstall_IsWin2000() && !WNInstall_IsWinXP() )
	{
		old_windows=true;
	}

	if( !if2k_win32_check_old_versions() )
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

	jdk_set_home_dir( DEFAULT_APP_DIR );
	install_settings.set_args(0,argc,argv );

	jdk_str<4096> error_reason;
	if( !if2k_win32install( install_settings, old_windows, false, error_reason ) )
	{
		//MessageBox( 0, "Failed installation", IF2002_TITLE, MB_OK );
		return 1;
	}
	return 0;
}



#endif









