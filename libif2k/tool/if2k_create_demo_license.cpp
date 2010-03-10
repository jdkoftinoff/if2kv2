#include "jdk_world.h"
#include "jdk_serial_number.h"
#include "jdk_settings.h"
#include "if2k_mini_config.h"


extern jdk_setting_description if2k_mini_kernel_defaults[];

int main( int argc, char **argv )
{
  jdk_set_app_name( argv[0] );
  jdk_settings_text settings( if2k_mini_kernel_defaults, argc, argv );  
  
  jdk_serial_number serial;
  serial.load( settings );
  serial.serial=serial.calculate_code( settings.get( "product.id" ) );
  if( serial.is_valid_code(settings.get( "product.id")) )
  {
    fprintf( stdout, "%s\n", serial.serial.c_str() );
    return 0; 
  }
  return 1;
}
