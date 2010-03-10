
#include "jdk_world.h"

#include "jdk_buf.h"
#include "jdk_string.h"
#include "jdk_httprequest.h"

int main( int argc, char **argv )
{
  if( argc > 4 )
  {
    const char *url = argv[1];
    const char *proxy = argv[2];
    const char *user = argv[3];
    const char *pass = argv[4];

    long max_response_size = 100*1024;
    jdk_dynbuf response( max_response_size );
    jdk_http_response_header response_header;
    bool use_ssl=false;


    int result = jdk_http_get(
      url,
      &response,
      max_response_size,
      &response_header,
      proxy,
      false,
      0,
      user,
      pass
      );

     if( result==200 )
     {
        response.extract_to_stream( stdout );
     }
     else
     {
        fprintf( stderr, "Error: %d\n", result );
     }
     fgetc( stdin );
  }
  return 0;
}
