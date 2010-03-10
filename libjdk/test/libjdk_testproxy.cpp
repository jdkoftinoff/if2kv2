
#include "jdk_world.h"
#include "jdk_thread.h"
#include "jdk_socket.h"
#include "jdk_dynbuf.h"
#include "jdk_settings.h"
#include "jdk_url.h"
#include "jdk_string.h"
#include "jdk_map.h"
#include "jdk_linesplit.h"
#include "jdk_daemon.h"
#include "jdk_thread_server.h"
#include "jdk_log.h"
#include "jdk_socketutil.h"

#define SET_PORT 				"port"
#define SET_LISTEN_COUNT 		"listen_count"
#define SET_INTERFACE 			"interface"
#define SET_FURTHER_PROXY 		"further_proxy"
#define SET_FURTHER_PROXY_PORT 	"further_proxy_port"
#define SET_DO_DUMPS			"do_dumps"
#define SET_INCOMING_DUMP		"incoming_dump"
#define SET_OUTGOING_DUMP		"outgoing_dump"

jdk_setting_description setting_descriptions[] =
{
  {"log_type", "2", "Logging type: 0=none, 1=syslog, 2=stderr, 3=file." },
  {"log_file", "log.txt", "Logging file name." },
  {"log_detail", "3", "Logging detail level: 0=Errors, 1=Warnings, 2=Notices, 3=Info, 4-8=Debug" },
  {"threads", "1", "Number of threads to use." },
#if JDK_IS_UNIX     
  {"setuid", "0", "1 to set UID/GID on startup." },
  {"uid", "0", "UID to use." },
  {"gid", "0", "GID to use." },
  {"use_fork", "0", "Use forking processes instead of threads." },
  {"daemon", "0", "Become a background process." },
  {"pid_dir", "", "Directory to put PID file in: Blank=none." },     
  {"home", ".", "Home directory." },
  {"name", "", "Name to use for logging." },          
#endif     
  
  {"listen_count", "10", "TCP/IP listen queue size."},
  {"interface","0:8001", "IP address to bind to (0=ALL)."},
  {"further_proxy","", "Further proxy host" },
  {"do_dumps", "0", "store all data to local files." },
  {"incoming_dump", "incoming.txt", "file to store incoming data to." },
  {"outgoing_dump", "outgoing.txt", "file to store outgoing data to." },
  {0,0}
};

class proxy_shared_resources : public jdk_thread_shared
{
public:
  proxy_shared_resources( 
    const jdk_settings &settings,  
    jdk_server_socket &server_socket_
    )
    :
    dns_cache( 1000 ),
    server_socket(server_socket_)
  {
    if( settings.get_long( SET_DO_DUMPS ) )
    {			
      incoming_dump = fopen( settings.get( SET_INCOMING_DUMP ).c_str(),"a" );
      outgoing_dump = fopen( settings.get( SET_OUTGOING_DUMP ).c_str(),"a" );
    }		
    else
    {
      incoming_dump = 0;
      outgoing_dump = 0;
    }						
  }
  
  ~proxy_shared_resources()
  {
    if( incoming_dump )
      fclose( incoming_dump );
    if( outgoing_dump )
      fclose( outgoing_dump );
  }
  
  bool run()
  {
    return true;	
  }
  
  bool update()
  {
    return true;	
  }
  
  
  FILE *incoming_dump;
  FILE *outgoing_dump;
  
  jdk_dns_cache dns_cache;
  jdk_server_socket &server_socket;
};


class proxy_thread : public jdk_thread
{
public:
  typedef proxy_shared_resources shared_t;
  
  proxy_thread( 
    int my_id_, 
    const jdk_settings &settings_,
    proxy_shared_resources &shared_
    )
    : my_id( my_id_ ),
      settings( settings_ ),
      shared( shared_ ),
      do_dumps( (bool)settings.get_long( SET_DO_DUMPS ) )
  {
  }
  
protected:
  virtual void main()
  {
    const char *further_proxy = settings.get( SET_FURTHER_PROXY ).c_str();
    int further_proxy_port = settings.get_long( SET_FURTHER_PROXY_PORT );
    
    while(!please_stop)
    {			
      jdk_inet_client_socket incoming;
      
      if( shared.server_socket.accept( &incoming ) )
      {
        char remote_addr[256];
        
        if( incoming.get_remote_addr( remote_addr, sizeof(remote_addr) ) )
        {					
          jdk_log( JDK_LOG_INFO, "(%d) accepted connection from %s", my_id, remote_addr );
          jdk_inet_client_socket outgoing;
          
          if( outgoing.make_connection( further_proxy, further_proxy_port, &shared.dns_cache, false ) )
          {
            jdk_log( JDK_LOG_DEBUG, "(%d) made connection to %s:%d", my_id, further_proxy, further_proxy_port );	
            
            jdk_socket_transfer( 
              my_id, 
              &incoming, 
              &outgoing, 
              do_dumps, 
              shared.incoming_dump, 
              shared.outgoing_dump 
              );
            
            jdk_log( JDK_LOG_DEBUG, "(%d) closing connection to %s:%d", my_id, further_proxy, further_proxy_port );
            jdk_log( JDK_LOG_DEBUG, "(%d) closing connection from %s", my_id, remote_addr );
          }
          else
          {
            jdk_log( JDK_LOG_INFO, "(%d) unable to connect to %s:%d", my_id, further_proxy, further_proxy_port );
          }					
        }
        else
        {
          jdk_log( JDK_LOG_ERROR, "buffer overflow, file %s line %d", __FILE__, __LINE__ );
        }
        
      }			
    }
  }
  
private:
  int my_id;
  const jdk_settings &settings;
  shared_t &shared;
  bool do_dumps;	
};

typedef jdk_thread_server_simple_factory<proxy_thread,proxy_shared_resources> proxy_factory;

int main(int argc, char **argv )
{
  return jdk_thread_server_simplistic<proxy_factory>(
    argc,
    argv,
    "libjdk_testproxy_settings.txt", 
    setting_descriptions
    );
}

