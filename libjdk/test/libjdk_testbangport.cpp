
#include "jdk_world.h"
#include "jdk_thread.h"
#include "jdk_socket.h"
#include "jdk_dynbuf.h"
#include "jdk_settings.h"
#include "jdk_url.h"
#include "jdk_string.h"
#include "jdk_map.h"
#include "jdk_linesplit.h"
#include "jdk_thread_server.h"

jdk_setting_description setting_descriptions[] =
{
  {"log_type", "2", "Logging type: 0=none, 1=syslog, 2=stderr, 3=file." },
  {"log_file", "log.txt", "Logging file name." },
  {"log_detail", "3", "Logging detail level: 0=Errors, 1=Warnings, 2=Notices, 3=Info, 4-8=Debug" },
  {"name", "", "Name to use for logging." },     
#if JDK_IS_UNIX	
  {"setuid", "0", "1 to set UID/GID on startup." },
  {"uid", "0", "UID to use." },
  {"gid", "0", "GID to use." },
  {"use_fork", "0", "Use forking processes instead of threads." },
  {"daemon", "0", "Become a background process." },
  {"pid_dir", "", "Directory to put PID file in: Blank=none." },
  {"home", ".", "Home directory." },	     
#endif	     
  
  {"host", "127.0.0.1", "Remote host to hit." },
  {"port", "80", "remote port to hit."},
  {"threads", "10", "Number of threads."},
  
  {0,0}
};



class bangport_shared_resources : public jdk_thread_shared
{
public:
  bangport_shared_resources( const jdk_settings &settings) :
    dns_cache( 5000 )
  {
  }
  
  jdk_dns_cache dns_cache;
};



class bangport_thread : public jdk_thread
{
public:	
  bangport_thread( 
    int my_id_, 
    const jdk_settings &settings_,
    bangport_shared_resources &shared_
    )
    :
    my_id( my_id_ ),
    settings( settings_ ),
    shared( shared_ )
  {
  }
  
protected:
  virtual void main()
  {
    int good_count=0;
    int bad_count=0;
    int count=0;
    
    while(!please_stop)
    {
      
      if( outgoing_socket.make_connection( 
            settings.get( "host" ).c_str(), 
            settings.get_long( "port" ), 
            &shared.dns_cache,
            false
            ) )
      {
        outgoing_socket.write_string_block( "GET http://www.jdkoftinoff.com/index.html HTTP/1.0\r\n\r\n" );
        outgoing_socket.close();
        good_count++;
      }
      else
      {
        bad_count++;				
      }
      if( (++count % 500)==0 )
      {
        jdk_log( JDK_LOG_INFO, "%d\t%d\t%d\n", my_id, good_count, bad_count );    
      }
    }
  }
  
  
private:
  int my_id;
  jdk_inet_client_socket outgoing_socket;
  const jdk_settings &settings;
  bangport_shared_resources &shared;
};




int main( int argc, char **argv )
{
  jdk_settings_text settings( setting_descriptions, argc, argv );
  
  return 1; // TODO:
  //    return jdk_thread_server_simplistic<bangport_thread, bangport_shared_resources>( settings );
  
}

