#include "jdk_world.h"
#include "jdk_html_decode.h"
#include "jdk_cgi.h"

int main( int argc, char **argv )
{
  jdk_dynbuf in,out;
  
  in.append_from_stream( stdin );
  jdk_html_decode decoder;
  decoder.process( in, out );
  out.extract_to_stream( stdout );
  return 0;
}
