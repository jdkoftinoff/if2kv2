#if defined(WIN32)

#include "jdk_world.h"
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
#define IF2K_MAX_THREADS 20
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
    }
  }

  jdk_settings_text &settings;

  if2k_mini_kernel_v2 &kernel;

  jdk_nb_inet_server_socket &server_socket;
};







class if2k_mini_win32_service_thread : public jdk_thread
{
public:
  if2k_mini_win32_service_thread( int argc_, char **argv_ )
    :
    argc( argc_ ),
    argv( argv_ ),
    num_threads(0)
  {
    for( int i=0; i<IF2K_MAX_THREADS; ++i )
    {
      threads[i]=0;
    }
  }



  ~if2k_mini_win32_service_thread()
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

  int argc;
  char **argv;
  jdk_settings_text *settings;
  jdk_nb_inet_server_socket server_socket;
  jdk_thread *threads[IF2K_MAX_THREADS];
  int num_threads;
};





void if2k_mini_win32_service_thread::notify_if2k_redir()
{
	HMODULE if2k_redir=GetModuleHandle( IF2K_MINI_REDIR_DLL_NAME );
	if( if2k_redir )
	{
		typedef void (*mode_proc)(DWORD,DWORD *);	

		mode_proc mode=(mode_proc)GetProcAddress(if2k_redir,"mode");
		if( mode )
		{
			DWORD a=0;
			mode( 1, &a );		
		}
	}
}



bool if2k_mini_win32_service_thread::load_settings()
{
  return true;
}



void if2k_mini_win32_service_thread::setup_log()
{
  jdk_string_filename home( settings->get("home").c_str() );

	jdk_set_home_dir(
      home.c_str()
		  );

	jdk_log_setup(
				  settings->get_long( "log.type" ),
				  settings->get( "log.file" ),
				  settings->get_long( "log.detail" )
				  );				  
}



bool if2k_mini_win32_service_thread::bind_server()
{
  jdk_str<512> ip( settings->get("httpfilter.interface") );
  bool r=server_socket.bind_socket(ip.c_str(), 10 );
  if( r )
  {
    jdk_log( JDK_LOG_DEBUG1, "Listening on '%s'", ip.c_str() );
  }
  else
  {
    jdk_log( JDK_LOG_ERROR, "Error listening on '%s'", ip.c_str() );
  }

  return r;

}



bool if2k_mini_win32_service_thread::start_worker_threads( if2k_mini_kernel_v2 &kernel)
{
  bool r;
  int threads_started = 0;

  num_threads = settings->get_long("httpfilter.max.sockets");

  if( num_threads > IF2K_MAX_THREADS )
    num_threads = IF2K_MAX_THREADS;
  if( num_threads ==0 )
    num_threads=1;


  for( int i=0; i<num_threads; ++i )
  {
    threads[i] = new if2k_server_thread( *settings, kernel, server_socket);
    if( threads[i]->run() )
      threads_started++;
  }

  r=(threads_started==num_threads);

  if( !r )
  {
    jdk_log( JDK_LOG_ERROR, "Tried to start %d threads, started %d threads", num_threads, threads_started );
  }

  return r;

}



bool if2k_mini_win32_service_thread::stop_worker_threads()
{
  bool r;
  int stopping_threads=0;
  int stopped_threads=0;

  int i;

  for( i=0; i<num_threads; ++i )
  {
    if( threads[i] )
    {
      threads[i]->thread_pleasestop();
      stopping_threads++;
    }
  }

  sleep(2);

  for( i=0; i<num_threads; ++i )
  {
    if( threads[i] && threads[i]->thread_isdone() )
    {
      delete threads[i];
      threads[i] = 0;
      stopped_threads++;
    }
  }

  r=(stopped_threads==stopping_threads);

  if( !r )
  {
    jdk_log( JDK_LOG_ERROR, "%d threads, %d asked to stop, %d did stop", num_threads, stopping_threads, stopped_threads );
  }

  return r;
}



void if2k_mini_win32_service_thread::kill_worker_threads()
{
  int killed_count=0;
  int i;
  for( i=0; i<num_threads; ++i )
  {
    if( threads[i] )
    {
      if( !threads[i]->thread_isdone() )
      {
        threads[i]->thread_cancel();
        killed_count++;
      }
    }
  }

  sleep(1);

  for( i=0; i<num_threads; ++i )
  {
    delete threads[i];
    threads[i] = 0;
  }

  jdk_log( JDK_LOG_ERROR, "Forcibly killed %d threads", killed_count );
}





void if2k_mini_win32_service_thread::main()
{
  notify_if2k_redir();
 

  bool restart_triggered=false;
  bool precompile_triggered=false;

  do
  {
    if2k_mini_kernel_v2_settings_collection settings_collection(argc,argv);
    settings_collection.reload();
    settings = &settings_collection.merged_settings;

    setup_log();
      
    jdk_log( JDK_LOG_INFO, "Starting %s service", IF2K_MINI_SERVER_SERVICE_NAME );
    jdk_log( JDK_LOG_DEBUG1, "argc=%d", argc );

    if( precompile_triggered )
    {
      if2k_mini_compile_all( settings_collection.get_merged_settings() );
    }

    if2k_mini_kernel_v2 kernel( settings_collection );
    kernel.load();

    if( bind_server() && start_worker_threads(kernel) )
    {
#if JDK_DEBUG
      int self_destruct=1000;
#endif

      int reload_time = settings_collection.get_reload_time_in_seconds();

      while( !please_stop && !kernel.is_restart_triggered() )
      {
#if JDK_DEBUG
        if( --self_destruct==0 )
          break;
#endif
  
        sleep(1);


        if( reload_time!=0 )
        {
          if( --reload_time==0 )
          {
            kernel.trigger_restart();
          }
        }
      }
    }

    server_socket.close();

    restart_triggered = kernel.is_restart_triggered();
    precompile_triggered = kernel.is_precompile_triggered();

    jdk_log( JDK_LOG_INFO, "%s %s service", 
      restart_triggered ? "Restarting" : "Stopping" ,IF2K_MINI_SERVER_SERVICE_NAME );

    if( !stop_worker_threads() )
    {
      sleep(2);
      if( !stop_worker_threads() )
      {
        sleep(1);
        stop_worker_threads();
        kill_worker_threads();
      }
    }
  

  } while( restart_triggered );
}







WINAPI WinMain( 
					 HINSTANCE hInstance, 
					 HINSTANCE hPrevInstance, 
					 LPSTR lpCmdLine, 
					 int nShowCmd 
					 )
{
  jdk_win32svc<if2k_mini_win32_service_thread> svc( 
        IF2K_MINI_SERVER_SERVICE_NAME, 
        IF2K_MINI_PRODUCT_NAME, 
        IF2K_MINI_VERSION
        );

	return svc.WinMain( hInstance, hPrevInstance, lpCmdLine, nShowCmd );
}



#endif

