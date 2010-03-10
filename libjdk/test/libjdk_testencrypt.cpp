
#include "jdk_world.h"
#include "jdk_settings.h"



int main( int argc, char **argv )
{
  if( argc==1 )
  {
    jdk_settings_text in;
    in.load_file( stdin );
    jdk_settings_text out;
    jdk_settings_decrypt<jdk_decryptor_complex>(0xab121324,in,out);
    out.save_file(stdout);
  }
  else
  {     
    jdk_settings_text in(0,argc,argv);
    jdk_settings_text out;
    jdk_settings_encrypt<jdk_encryptor_complex>(0xab121324,in,out);
    out.save_file( stdout );
  }
  return 0;
}
