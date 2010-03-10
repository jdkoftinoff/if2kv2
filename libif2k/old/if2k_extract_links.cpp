#include "jdk_world.h"
#include "jdk_http.h"
#include "jdk_dynbuf.h"
#include "jdk_httprequest.h"
#include "jdk_html_decode.h"

void wait_for_all_children()
{
#if JDK_HAS_FORK
  int status;
  pid_t pid;
  do
  {
    errno=0;
    pid=wait(&status);
    if( pid>0 )
    {
      //fprintf( stderr, "pid %d exited\n", pid );
    }
  } while( errno!=ECHILD || pid>0 );
#endif
}

void recursive_find_links( const jdk_string_url url, int depth, FILE *out )
{
  jdk_dynbuf url_contents;
  jdk_http_response_header response_header;
  fprintf( out, "%s\n", url.c_str() );
  
  if( jdk_http_head( url.c_str(), &response_header, "", false )==200 )
  {
    const jdk_http_response_header::value_t *v;
    response_header.find_entry( "content-type:", &v );
    if( v && v->nicmp( "text/html", 9 )==0 )
    {
      response_header.clear();
      
      if( jdk_http_get( url.c_str(), &url_contents, 1024*1024, &response_header, "", false )==200 )
      {
        jdk_dynbuf links_result;
        jdk_html_decode_to_links decoder;
        decoder.clear();
        decoder.process( url_contents, links_result );
//				links_result.extract_stream( out );
        
//				fprintf( stderr, "%s len = %d\n", url.c_str(), url_contents.get_data_length() );
//				fprintf( stderr, "%s is text/html\n", url.c_str() );
        
        jdk_string_url next_url;
        int pos=0;
        while( pos>=0 && (pos=links_result.extract_to_string(next_url,pos))>0 )
        {
          if( depth>0 )
          {
#if JDK_HAS_FORK
            sleep(1);				
            if( fork()==0 )
#endif						
            {
              recursive_find_links( next_url, depth-1, out );
#if JDK_HAS_FORK							
              wait_for_all_children();
              exit(0);
#endif							
            }
          }
          else
          {
            fprintf( out, "%s\n", next_url.c_str() );					
          }
        }
      }
    }
  }	
  
}

int if2k_extract_links_main( int argc, char **argv )
{
  jdk_string_url starting_url;
  while( jdk_read_string_line( &starting_url, stdin ) )
  {
    recursive_find_links( starting_url, 2, stdout );
  }
  
  wait_for_all_children();
  
  return 0;
}

