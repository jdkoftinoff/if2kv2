#include "jdk_world.h"
#include "jdk_httprequest.h"
#include "jdk_log.h"

int main( int argc, char **argv )
{
  jdk_log_setup( JDK_LOG_DEST_STDERR, 0, 8 );
  
  const char *url="";
  const char *proxy="";
  
  if( argc>1 )
  {
    url = argv[1];
    if( argc>2 )
    {
      proxy=argv[2];
    }
    jdk_dynbuf response;
    jdk_http_response_header response_header;
    int result = jdk_http_get( url, &response, 1024*1024L, &response_header, proxy, false );
    fprintf( stderr, "result=%d\n", result );
    jdk_dynbuf response_header_flattened;
    if( response_header.flatten( response_header_flattened ) )
    {
      fprintf( stderr, "\nresponse header:\n" );
      response_header_flattened.extract_stream( stderr );
    }
    fprintf( stderr, "\nresponse:\n" );
    response.extract_stream( stdout );				
  }
  else
  {
    fprintf( stderr, "Give me URL to access and proxy to use\n" );
    return 1;
  }
  return 0;
}
