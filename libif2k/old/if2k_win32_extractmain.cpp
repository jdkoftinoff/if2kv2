#include "jdk_world.h"
#include "jdk_settings.h"
#include "if2k_config.h"
#include "if2k_win32_extractmain.h"

#if JDK_IS_WIN32

static void selective_copy( const jdk_settings &in, jdk_settings &out )
{
	static char *skipped_labels[] = 
	{
		"home",
		"name",
#if 0
		"license_valid",
		"license_name",
		"license_clients",
		"license_email",
		"license_expires",
#endif		
		"secret_username",
		"secret_password",
		0
	};

	for( int i=0; i<in.count(); ++i )
	{
		const jdk_settings::pair_t *a = in.get(i);
		if( a )
		{
			bool do_copy=true;
			char **skip_s = skipped_labels;
			while( *skip_s )
			{
				if( a->key.icmp( *skip_s )== 0 )
				{
					do_copy=false;
					break;
				}
  				++skip_s;
			}

			if( do_copy )
			{
				out.set( a->key, a->value );
			}
		}
	}
}


int if2k_win32_extract_main( int argc, char **argv )
{
	PLATFORM_SETTINGS settings;
	settings.PLATFORM_LOAD_SETTINGS();
	jdk_settings_text text;

	selective_copy( settings, text );
	
	const char *fname = "c:\\if2ksettings.txt";
	
	if( argc>1 )
	{
		fname = argv[1];
	}
	
	if( text.save_file( fname ) )
	{
		char buf[4096];
		sprintf( buf, "%s settings have been extracted to the file %s", IF2K_TITLE, fname );
		MessageBox( 0, buf, IF2K_TITLE, MB_OK );
	}
	else
	{
		char buf[4096];
		sprintf( buf, "Error writing to the file %s", fname );		
		MessageBox( 0, buf, IF2K_TITLE, MB_OK );			
	}
	
	return 0;
}

#endif
