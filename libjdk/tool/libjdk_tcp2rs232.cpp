
#include "jdk_world.h"
#include "jdk_util.h"
#include "jdk_string.h"
#include "jdk_inet_server.h"
#include "jdk_socketutil.h"
#include "jdk_rs232.h"

class jdk_tcp2rs232_server_shared
{
public:
  jdk_tcp2rs232_server_shared(
    const char *serialport_, unsigned long bd
    )
  {
    rs232.rs232_open( serialport_, bd );
  }
  
  ~jdk_tcp2rs232_server_shared()
  {
  }
  
  jdk_rs232 rs232;
};

class jdk_tcp2rs232_server_worker : public jdk_inet_server_worker
{
public:
  jdk_tcp2rs232_server_worker( jdk_tcp2rs232_server_shared &shared_, int id_ )
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
    jdk_fd_client_socket out;
    out.force_connection( (jdk_socket_handle)shared.rs232.rs232_handle );
    
    jdk_socket_transfer( id, &sock, &out, true, stdout, stderr );
    
    return true;
  }
  
  jdk_tcp2rs232_server_shared &shared;
  int id;
};


int main( int argc, char **argv )
{
  if( argc<4 )
  {
    fprintf( stderr, "usage: localaddress serialport baudrate\n" );
    exit(1);
  }
  jdk_str<128> bind_port(argv[1]);
  jdk_tcp2rs232_server_shared shared(argv[2],atol(argv[3]));
  
  jdk_inet_server<
    jdk_tcp2rs232_server_worker,
    jdk_tcp2rs232_server_shared
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

