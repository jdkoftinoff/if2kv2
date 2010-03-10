#include "jdk_world.h"
#include "jdk_util.h"
#include "if2k_serial.h"


int main( int argc, char **argv )
{
  jdk_set_app_name( "libif2_testlicense" );
  jdk_log_setup( JDK_LOG_DEST_STDERR, 0, 8 );
  jdk_settings_text settings( 0, argc, argv );
  
  // TODO: enable encryption key here
  if2_serial_get( settings, 0xab121324, "http://www.internetfilter.com/license/V2.0/",  "http://www1.internetfilter.com/license/V2.0/", settings.get("serial").c_str() );
  
  printf( "Serial # %s\n", settings.get("serial").c_str() );
  
  printf( "Licensed to: %s %s\n", 
          settings.get("license_name").c_str(),
          settings.get("license_email").c_str() 
    );
  printf( "For %ld clients\n",
          settings.get_long("license_clients")
    );
  printf( "License valid=%ld\n",
          settings.get_long("license_valid")
    );
  return 0;
}

