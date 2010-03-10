#include "jdk_world.h"
#include "jdk_socket.h"
#include "jdk_string.h"
#include "jdk_dynbuf.h"

int main( int argc, char **argv )
{
  const char *bindhost="0";
  int bindport=0;
  const char *desthost="localhost";
  int destport=80;
  
  if( argc>4 )
  {
    bindhost = argv[1];
    bindport = strtol( argv[2], 0, 10 );
    desthost = argv[3];
    destport = strtol( argv[4], 0, 10 );
    
    jdk_inet_client_socket s;
    if( s.make_connection_from( desthost, destport, bindhost, bindport, 0, false ) )
    {
      jdk_string_url local_addr, remote_addr;
      
      s.get_local_addr( local_addr.c_str(), local_addr.getmaxlen() );
      s.get_remote_addr( remote_addr.c_str(), remote_addr.getmaxlen() );
      
      fprintf( stderr, "Connected to %s:%d\nlocal = %s, remote = %s\n", desthost, destport, local_addr.c_str(), remote_addr.c_str() );
      s.write_string_block( "GET / HTTP/1.0\r\nHost: www.jdkoftinoff.com\r\n\r\n" );
      
      jdk_dynbuf buf;
      while( s.read_buf( buf )>0 );
      fwrite( buf.get_data(), buf.get_data_length(), 1, stdout );
    }
  }
  else
  {
    fprintf( stderr, "libjdk_testsock2 usage:\n\tlibjdk_testsock2 bind_host bind_port dest_host dest_port\n\n" );
  }
}
