#include "jdk_world.h"
#include "jdk_settings.h"
#include "jdk_serial_number.h"

int main( int argc, char **argv ) 
{ 
  jdk_settings_text params( 0, argc, argv );
  jdk_serial_number ser;

  ser.load( params );
  if( !ser.is_valid_code( params.get( "product_id" ) ) )
  {
    fprintf( stderr, "Serial # is not valid\n" );
    exit(1);
  }
  else
  {
    fprintf( stderr, "Serial # is valid\n" );
  }

  return 0; 
}


