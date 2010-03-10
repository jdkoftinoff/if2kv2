#include "jdk_world.h"
#include "jdk_settings.h"
#include "jdk_serial_number.h"

int main( int argc, char **argv ) 
{ 
  jdk_settings_text params( 0, argc, argv );
  jdk_serial_number ser;

  ser.load( params );
  ser.serial = ser.calculate_code( params.get( "product_id" ) );
  if( !ser.is_valid_code( params.get( "product_id" ) ) )
  {
    fprintf( stderr, "Internal error validating fresh code\n" );
    exit(1);
  }
  jdk_settings_text outfile;

  ser.save( outfile );
  outfile.save_file( stdout );

  return 0; 
}
