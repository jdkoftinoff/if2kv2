#include "jdk_world.h"
#include "jdk_dynbuf.h"
#include "jdk_lz.h"

int main( int argc, char **argv )
{
  jdk_dynbuf in,out;
  if( in.append_from_stream( stdin ) )
  {
    jdk_lz_decompress( &in, &out );
    out.extract_to_stream( stdout );
    return 0;
  }    
  return 1;
}

