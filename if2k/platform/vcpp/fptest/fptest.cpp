#include "jdk_world.h"
#include "jdk_dynbuf.h"
#include "if2k_mini_config.h"

int main(int argc, char **argv)
{
  if( argc>1 ) 
  {
    jdk_string_filename fname(argv[1]);

    jdk_dynbuf b;
    b.append_from_file( fname );

    jdk_str<256> hash;
    if2k_hash_for_dbfile( hash, b, fname );
    fprintf( stdout, "file name: '%s'\nHash: %s\n", fname.c_str(), hash.c_str() );
  }
  else
  {
    fprintf( stderr, "please give me file name" );
  }
  return 0;
}
