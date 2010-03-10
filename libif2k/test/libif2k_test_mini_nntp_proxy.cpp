#include "jdk_world.h"
#include "if2k_mini_config.h"

#include "if2k_mini_kernel.h"
#include "if2k_mini_nntp_proxy.h"


int libif2k_test_mini_nntp_proxy( int argc, char **argv )
{
  jdk_settings_text settings( 0, argc, argv, 0);

  if2k_mini_kernel_dummy kernel;
  if2k_mini_nntp_proxy nntp_proxy( settings, kernel );
  
  nntp_proxy.load_settings();
 
  jdk_fd_client_socket stdio_socket;
  stdio_socket.force_connection( (jdk_socket_handle)STDIN_FILENO ); // supreme hack!

  nntp_proxy.handle_connection(stdio_socket);
  return 0;
}


int main( int argc, char **argv )
{
  return libif2k_test_mini_nntp_proxy( argc, argv );
}
