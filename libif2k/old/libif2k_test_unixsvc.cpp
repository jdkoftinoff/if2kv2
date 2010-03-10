#include "jdk_world.h"
#include "jdk_socket.h"
#include "if2k_kernel.h"
#include "jdk_daemon.h"
#include "if2k_httpproxy.h"

extern jdk_setting_description default_client_settings[];

int main( int argc, char **argv )
{
  jdk_settings_text settings( default_client_settings, argc, argv );
  
  if( jdk_daemonize(
        (bool)settings.get_long("daemon"),
        settings.get("name").c_str(),
        settings.get("home").c_str(),
        settings.get("pid_dir").c_str() ) <0
    )
  {
    return 1;
  }
  
  //if2002_http_proxy_factory factory( &settings );
  
  jdk_thread_server_drop_root( settings);
  
  jdk_log_setup(
    settings.get_long( "log_type" ),
    settings.get( "log_file" ),
    settings.get_long( "log_detail" )
    );				  
#if 0  
  jdk_thread_server_base * server = factory.create_thread_server();
  bool stop_requested=false;
  
  if( server->run() /*&& server->update()*/ )
  {		
    while(!jdk_sigterm_caught && !jdk_sighup_caught )
    {
      pause();
    }
    
    jdk_log( JDK_LOG_NOTICE, "SHUTTING DOWN" );
    server->stop();
    sleep(2);
  }
  else
  {
    jdk_log( JDK_LOG_ERROR, "ERROR RUNNING SERVER" );			
  }
  
  delete server;
#endif
  jdk_log( JDK_LOG_NOTICE, "SHUT DOWN COMPLETE" );
  return 0;
}


