#include "jdk_world.h"
#include "jdk_dynbuf.h"
#include "jdk_string.h"
#include "if2k_mini_config.h"

int main( int argc, char **argv )
{
  jdk_dynbuf buf;
  buf.append_from_stream( stdin );
  jdk_string_filename f( "/Library/StartupItems/if2k/" );
  jdk_string_filename fn( argv[1] );
  if( fn.str("..")==0 )
  {
    f.cat( fn );
    buf.extract_to_file( f );
  }
  return 0;
}
