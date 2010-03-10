#ifndef IF2KD_H
#define IF2KD_H

#include "jdk_log.h"
#include "jdk_win32svc.h"
#include "jdk_settings.h"
#include "jdk_crypt.h"
#include "jdk_thread.h"

#include "if2k_mini_config.h"
#include "if2k_mini_http_proxy.h"
#include "if2k_mini_kernel_v2.h"
#include "if2k_mini_compile.h"

#ifndef IF2K_MAX_THREADS
#define IF2K_MAX_THREADS 1000
#endif

extern jdk_setting_description if2k_mini_kernel_defaults[];

class if2k_server_thread : public jdk_thread
{
public:
  if2k_server_thread( 
    jdk_settings_text &settings_, 
    if2k_mini_kernel_v2 &kernel_,
    jdk_nb_inet_server_socket &server_socket_
    )
    : 
    settings( settings_ ),
    kernel( kernel_ ),
    server_socket( server_socket_ )
  {
  }
protected:

  void main()
  {
    while( !please_stop )
    {
      jdk_select_manager sel;
      sel.add_read( server_socket );
      sel.set_timeout(1);
      
      int r = sel.do_select();

      if( r==1 )
      {
        jdk_inet_client_socket client;
        if( server_socket.accept( &client ) )
        {
          if2k_mini_http_proxy proxy(settings,kernel);          
          proxy.load_settings();
          proxy.handle_connection( client );
          client.close();
        }
      }
      else if( r<0 )
      {
        break;
      }
    }
  }

  jdk_settings_text &settings;

  if2k_mini_kernel_v2 &kernel;

  jdk_nb_inet_server_socket &server_socket;
};


class if2k_mini_service_thread : public jdk_thread
{
public:
  if2k_mini_service_thread( int argc_, char **argv_ )
    :
    argc( argc_ ),
    argv( argv_ ),
    settings_collection( argc_, argv_ ),
    settings( settings_collection.get_merged_settings() ),
    num_threads(0),
    reload_time(0),
    is_precompile_triggered(false),
    is_restart_triggered(false)
  {
    for( int i=0; i<IF2K_MAX_THREADS; ++i )
    {
      threads[i]=0;
    }
  }

  ~if2k_mini_service_thread()
  {
  }

protected:

  void main();

  void notify_if2k_redir();

  bool load_settings();
  void setup_log();

  bool bind_server();

  bool start_worker_threads(if2k_mini_kernel_v2 &kernel);
  bool stop_worker_threads();
  void kill_worker_threads();
  void finish_worker_threads();

  void do_restart_countdown(if2k_mini_kernel_v2 &kernel);
  void do_precompiling();
  bool handle_signals(if2k_mini_kernel_v2 &kernel);

  int argc;
  char **argv;
  if2k_mini_kernel_v2_settings_collection settings_collection;
  jdk_settings_text &settings;
  jdk_nb_inet_server_socket server_socket;
  jdk_thread *threads[IF2K_MAX_THREADS];
  int num_threads;
  int reload_time;
  bool is_precompile_triggered;
  bool is_restart_triggered;
};



#endif
