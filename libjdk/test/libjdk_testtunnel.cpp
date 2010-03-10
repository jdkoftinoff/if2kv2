
#include "jdk_world.h"
#include "jdk_util.h"
#include "jdk_string.h"
#include "jdk_inet_server.h"
#include "jdk_socketutil.h"

class jdk_tunnel_server_shared
{
public:
  jdk_tunnel_server_shared( 
    const char *remote_addr_,
    const char *infile_,
    const char *outfile_
    )
    : remote_addr( remote_addr_ )
  {
//	  infile = fopen( infile_, "wb" );
//	  outfile = fopen( outfile_, "wb" );
  }
  
  ~jdk_tunnel_server_shared()
  {
//	   fclose(infile);
//	   fclose(outfile);
  }
  
  jdk_str<4096> remote_addr;
//   FILE *infile;
//   FILE *outfile;
};

class jdk_tunnel_server_worker : public jdk_inet_server_worker
{
public:
  jdk_tunnel_server_worker( jdk_tunnel_server_shared &shared_, int id_ )
    :
    shared( shared_ ),
    id( id_ )
  {
  }
  
  void run()
  {
  }
  
  void update()
  {
  }
  
  bool handle_connection( jdk_client_socket &sock )
  {
    jdk_inet_client_socket out;
    out.make_connection( shared.remote_addr,0, false );
    
    jdk_socket_transfer( id, &sock, &out, true, stdout, stderr );
    
    return true;
  }
  
  jdk_tunnel_server_shared &shared;
  int id;
};


int main( int argc, char **argv )
{
  if( argc<5 )
  {
    fprintf( stderr, "usage: localaddress remoteaddress incapturefilename outcapturefilename\n" );
    exit(1);
  }
  jdk_str<128> bind_port(argv[1]);
  jdk_tunnel_server_shared shared(argv[2],argv[3],argv[4]);
  
  jdk_inet_server<
    jdk_tunnel_server_worker, 
    jdk_tunnel_server_shared
    >
    server( 
      shared, 
      bind_port,
      10 
      );
  
  server.run();
  while(1)
  {
    jdk_sleep( 60*10 );
  }
  server.stop();
  return 0;
}

