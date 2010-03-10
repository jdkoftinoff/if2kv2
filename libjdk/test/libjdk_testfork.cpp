#include "jdk_world.h"
#include "jdk_fork_server.h"
#include "jdk_daemon.h"
#include "jdk_log.h"

#if JDK_HAS_FORK


class my_test_fork : public jdk_fork_server_child
{
public:
  my_test_fork( const char *response_ )
    :
    response( response_ ),
    pretend_reload_count(0),
    client_count(0)
  {			
  }

  void load_settings()
  {
  }

  void handle_sighup()
  {
    pretend_reload_count++;
    jdk_log( JDK_LOG_INFO, "Got sighup, pretend to reload settings: count %d", pretend_reload_count );
  }

  bool handle_connection( jdk_client_socket &sock )
  {
    ++client_count;
    sock.write_string_block( response );
    jdk_log( JDK_LOG_INFO, "%d clients handled", client_count );
    return true;
  }

private:
  const char *response;
  int pretend_reload_count;
  int client_count;
};


int main( int argc, char **argv )
{
  jdk_log_setup( JDK_LOG_DEST_FILE, "log.txt", 8 );
  
  jdk_str<256> bind_addr("127.0.0.1:8000");
  if( argc>1 )
  {
    bind_addr.cpy(argv[1]);
  }
  int num_forks=10;
  if( argc>2 )
  {
    num_forks=strtol( argv[2], 0, 10 );
  }
  int max_child_requests=4;
  if( argc>3 )
  {
    max_child_requests=strtol( argv[3], 0, 10 );
  }
  int max_child_time=60;
  if( argc>4 )
  {
    max_child_time=strtol( argv[4], 0, 10 );
  }
  
  my_test_fork server_worker("Response!\r\n");
  jdk_fork_server_simple_main(
    bind_addr,
    server_worker,
    num_forks,
    max_child_requests,
    max_child_time,
    true, //real_daemon
    "libjdk_testfork",
    ".",
    "."
    );

  return 0;
}

#else
int main()
{
  printf( "NO FORK\n" );
  return 0;
}
#endif
