#include "jdk_world.h"
#include "if2k_mini_compile.h"

extern jdk_setting_description if2k_mini_kernel_defaults[];


int main( int argc, char **argv )
{
  jdk_settings_text settings(if2k_mini_kernel_defaults,argc,argv);
  
  jdk_string_filename home( settings.get("home").c_str() );

  jdk_set_app_name( "if2k_precompile_all" );

  settings.load_file( settings.get("kernel.settings.master.local") );
  settings.load_file( settings.get("kernel.settings.additional.local") );

	jdk_set_home_dir(
      home.c_str()
		  );

	jdk_log_setup(
				  settings.get_long( "log.type" ),
				  settings.get( "log.file" ),
				  settings.get_long( "log.detail" )
				  );				  
  
  if2k_mini_compile_all( settings );

  return 0;
}
