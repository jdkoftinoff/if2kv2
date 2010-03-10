#include "jdk_world.h"
#include "jdk_settings.h"
#include "if2k_config.h"


int main( int argc, char **argv )
{
  time_t expiry=0;
  if( argc>1 )
  {
    expiry=strtoul(argv[1],0,10) * 3600 * 24; // expiry days->seconds
    
    fprintf( stderr, "Expires in %lu seconds\n", expiry );
    expiry+=time(0);
  }
  jdk_settings_text in;
  in.load_file( stdin );	
  in.set_ulong("license_expires",expiry);
  jdk_settings_text out;
  jdk_settings_encrypt<jdk_encryptor_complex>( ENCRYPTION_KEY, in, out );
  out.save_file( stdout );
  return 0;
}
