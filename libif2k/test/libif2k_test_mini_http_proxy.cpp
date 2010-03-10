#include "jdk_world.h"
#include "if2k_mini_config.h"

#include "if2k_mini_kernel_v2.h"
#include "if2k_mini_http_proxy.h"


int libif2k_test_mini_http_proxy( int argc, char **argv )
{
  jdk_settings_text settings( 0, argc, argv, 0);
  jdk_log_setup( 2, "", 8 );

  if2k_mini_kernel_v2 kernel(settings);
  if2k_mini_http_proxy http_proxy( settings, kernel );
  
  http_proxy.load_settings();
 
  if( settings.find( "request" )>=0 )
  {
    jdk_string_filename request_fname( settings.get("request") );
    
    int fd=open( request_fname.c_str(), O_RDONLY, 0 );
    if( fd>=0 )
    {
      jdk_fd_client_socket request_socket;
      request_socket.force_connection( (jdk_socket_handle)fd );
      http_proxy.handle_connection(request_socket);
    }
    else
    {
      jdk_log( JDK_LOG_ERROR, "Error opening '%s'", request_fname.c_str() );
    }
  }
  else
  {
    jdk_fd_client_socket stdio_socket;
    stdio_socket.force_connection( (jdk_socket_handle)STDIN_FILENO ); // supreme hack!

    http_proxy.handle_connection(stdio_socket);
  }
  return 0;
}


int main( int argc, char **argv )
{
  return libif2k_test_mini_http_proxy( argc, argv );
}
