#include "jdk_world.h"
#include "jdk_dynbuf.h"
#include "jdk_lz.h"


int main( int argc, char **argv )
{
#if JDK_IS_WIN32
  _setmode( _fileno( stdin ), _O_BINARY );
  _setmode( _fileno( stdout ), _O_BINARY );		   
#endif	
  jdk_dynbuf in,out;

  if( argc==1 )
  {
    if( in.append_from_stream( stdin ) )
    {
      jdk_lz_compress( &in, &out );
      out.extract_to_stream( stdout );
      return 0;
    }    
  }
  else if( argc==3 )
  {
    if( in.append_from_file( argv[1] ) )
    {
      jdk_lz_compress( &in, &out );
      out.extract_to_file( argv[2] );
      return 0;
    }    
  }

  return 1;
}

