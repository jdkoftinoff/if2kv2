#include "jdk_world.h"
#include "jdk_socket.h"
#include "jdk_string.h"
#include "jdk_dynbuf.h"

int main( int argc, char **argv )
{
  const char *bindhost="0";
  int bindport=0;
#if 0
  const char *desthost="localhost";
  int destport=80;
#endif
  
  if( argc>2 )
  {
    bindhost = argv[1];
    bindport = strtol( argv[2], 0, 10 );
    
    jdk_inet_server_socket serv( bindport, 10, bindhost );		
    jdk_inet_client_socket s;
    while( serv.accept( &s ) )
    {
      jdk_string_url local_addr, remote_addr;
      
      s.get_local_addr( local_addr.c_str(), local_addr.getmaxlen() );
      s.get_remote_addr( remote_addr.c_str(), remote_addr.getmaxlen() );
      
      fprintf( stderr, "local = %s, remote = %s\n", local_addr.c_str(), remote_addr.c_str() );
      
      jdk_str<4096> sbuf;
      while( s.read_string(sbuf)>0 );
      
      s.write_string_block( "HTTP 200 OK\r\nContent-type: text/html\r\n\r\n<html><h1>Hello world</h1></html>\r\n" );
      s.close();
    }
  }
  else
  {
    fprintf( stderr, "libjdk_testsock3 usage:\n\tlibjdk_testsock3 bind_host bind_port\n\n" );
  }
}
