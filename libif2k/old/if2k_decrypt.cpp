#include "jdk_world.h"
#include "jdk_settings.h"
#include "if2k_config.h"


int if2k_decrypt_main( int argc, char **argv )
{
  jdk_settings_text in;
  in.load_file( stdin );
  jdk_settings_text out;
  jdk_settings_decrypt<jdk_decryptor_complex>( ENCRYPTION_KEY, in, out );
  out.save_file( stdout );
  return 0;
}
