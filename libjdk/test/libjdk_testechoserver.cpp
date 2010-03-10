
#include "jdk_world.h"
#include "jdk_util.h"
#include "jdk_inet_server.h"
#include "jdk_daemon.h"

class jdk_echo_server_shared
{
public:
};

class jdk_echo_server_worker : public jdk_inet_server_worker
{
public:
  jdk_echo_server_worker( jdk_echo_server_shared &shared_, int id_ )
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
    while( sock.is_open_for_reading() && sock.is_open_for_writing() )
    {
      char buf[4096];
      jdk_log( JDK_LOG_NOTICE, "id %d, worker read()", id );
      int len = sock.read_data(buf,sizeof(buf));
      if( len>0 )
      {
        jdk_log( JDK_LOG_NOTICE, "id %d, worker read %d bytes", id, len );
        sock.write_data( buf, len );
      }
      else
      {
        jdk_log( JDK_LOG_NOTICE, "id %d, worker ending", id );
        sock.close();
        break;
      }
    }
    return true;
  }
  
  jdk_echo_server_shared &shared;
  int id;
};


int main( int argc, char **argv)
{
  jdk_str<128> bind_port(argc>1 ? argv[1] : "127.0.0.1:8000");
  jdk_echo_server_shared shared;
  
  jdk_inet_server<
    jdk_echo_server_worker, 
    jdk_echo_server_shared
    >
    server( 
      shared, 
      bind_port,
      10 
      );

  jdk_daemonize( true, "libjdk_testechoserver", ".", "." );

  jdk_inet_server_drop_root( -1,-1 );  

  server.run();
  jdk_sleep( 60*5 );
  server.stop();
  jdk_daemon_end();

  return 0;
}

