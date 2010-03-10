#include "jdk_world.h"
#include "jdk_socket.h"
#include "jdk_util.h"

#define NUM_CLIENTS 16

int main( int argc, char **argv )
{
  bool print_hex=false;
  if( argc>1 )
  {
    jdk_str<256> addr(argv[1]);
    jdk_inet_server_socket srv(addr);
    jdk_inet_client_socket s[NUM_CLIENTS];
    jdk_str<4096> line;
    
    while(1)
    {
      jdk_select_manager sel;
      sel.add_read( srv );
      for( int i=0; i<NUM_CLIENTS; ++i )
      {
        if( s[i].is_open_for_reading() )
        {
          sel.add_read( s[i] );
        }				
      }
      
      int r=sel.do_select();
      
      if( r>0 )
      {
        if( sel.can_read(srv) )
        {
          for( int i=0; i<NUM_CLIENTS; ++i )
          {
            if( !s[i].is_open_for_reading() )
            {
              srv.accept( &s[i] );
              fprintf( stdout, "\n%d: new connection\n", i );
              break;
            }						
          }					
        }
        
        for( int i=0; i<NUM_CLIENTS; ++i )
        {
          if( sel.can_read( s[i] ) )
          {
            unsigned char buf[4096];
            int len = s[i].read_data( buf, sizeof(buf) );
            if( len<=0 )
            {
              fprintf( stdout, "\n%d: closed\n", i );
              s[i].close();
              break;
            }
            fprintf( stdout, "%d: ", i );
            if( print_hex )
            {
              for( int i=0; i<len; ++i )
              {
                fprintf( stdout, "%02x ", buf[i] );
              }
            }
            else
            {
              line.clear();
              for( int i=0; i<len; ++i )
              {
                char c=buf[i];
                if( jdk_isprint(c) )
                {
                  line.cat(c);
                }
                else if( c=='\r' )
                {
                  line.cat( "\\r" );
                }
                else if( c=='\n' )
                {
                  line.cat( "\\n" );
                }
                else
                {
                  jdk_str<16> hex;
                  hex.form( "\\x%02x", (int)c );
                  line.cat(hex);
                }
                
              }
              fprintf( stdout, "%s", line.c_str() );
            }
            
            fprintf( stdout, "\n" );
            fflush(stdout);
          }					
        }				
      }
    }
    
  }
  else
  {
    fprintf( stderr, "Give me IP:port to bind to\n" );
    return 1;
  }
  return 0;
}
