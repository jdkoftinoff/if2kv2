#include "jdk_world.h"
#include "jdk_http.h"

int main( int argc, char **argv )
{
  jdk_dynbuf input;
  
  input.append_stream( stdin );
  
  jdk_http_request_header http_header;
  
  if( http_header.parse_buffer( input ) )
  {
    if( http_header.is_valid() )
    {
      jdk_dynbuf output;
      
      bool use_proxy_form = false;
      
      if( argc>1 )
        use_proxy_form = atoi( argv[1] );
      
      if( http_header.flatten( output, use_proxy_form ) )
      {
        output.extract_stream( stdout );
        
        fprintf( stdout, "-------ADDITIONAL DATA--------\n" );
        fwrite( 
          input.get_data() + http_header.get_buf_pos(),
          1,
          input.get_data_length()-http_header.get_buf_pos(), 
          stdout );
      }		
    }
  }
  
  
  return 0;
}

