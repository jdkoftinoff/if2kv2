#include "jdk_world.h"
#include "if2kd.h"


inline void jdk_no_log_debug4( const char *fmt, ... ) {}

#undef jdk_log_debug4
#define jdk_log_debug4 jdk_no_log_debug4

#if JDK_IS_WIN32 && IF2K_MINI_CONFIG_FIX_BROKEN_DLL
#include "resource.h"


void fix_broken_redir_dll()
{

  static HRSRC h=0;
  static HGLOBAL g=0;
  static DWORD sz=0;
  static LPVOID p=0;
  static char if2k_redir_path[1024] = "";
  char dll_hook_value[1024] = "";
  bool dll_is_good;

  if( *if2k_redir_path == '\0' )
  {
    GetSystemDirectory( if2k_redir_path, sizeof( if2k_redir_path ) );
    strcat( if2k_redir_path, "\\" IF2K_MINI_REDIR_DLL_NAME );
    jdk_log_debug4("fixer: if2k_redir_path is : %s", if2k_redir_path );
  }

  
  if( h==0 )
  {    
    h=FindResource(0,(LPCSTR)IDR_BINARY1,"BINARY");
    jdk_log_debug4("fixer: resource handle is : 0x%x", (DWORD)h );
  }


  if( h && *if2k_redir_path )
  {
    if( g==0 )
    {
      g=LoadResource(0, h );
      jdk_log_debug4("fixer: resource global is : 0x%x", (DWORD)g );
    }

    if( g )
    {
      if( sz==0 )
      {
        sz=SizeofResource(0,h);
        jdk_log_debug4("fixer: resource size is : %d", sz );
      }

      if( sz )
      {
        if( p==0 )
        {
          p=LockResource(g);
          jdk_log_debug4("fixer: resource ptr is : 0x%x", p  );
        }
      }
    }
  }

  if( p )
  {
    HMODULE if2k_redir_module = GetModuleHandle(IF2K_MINI_REDIR_DLL_NAME);
    jdk_log_debug4("fixer: GetModuleHandle result is : 0x%x", (DWORD)if2k_redir_module  );

    dll_is_good=true;

    FILE *fp = jdk_fopen( if2k_redir_path, "rb" );
    if( fp )
    {
      fclose(fp);
    }
    else
    {
      jdk_log_debug4( "fixer: %s is not openable", if2k_redir_path );
      dll_is_good=false;
    }

    if( if2k_redir_module )
    {
      typedef void (*mode_proc)(DWORD,DWORD *);	
      
      mode_proc mode=(mode_proc)GetProcAddress(if2k_redir_module,"mode");
      jdk_log_debug4("fixer: GetProcAddress mode result is : 0x%x", (DWORD)mode  );
      if( mode )
      {
        DWORD a=0;
        mode( 1, &a );		
      }
    }
    else
    {
      dll_is_good=false;
    }

    if( !dll_is_good )
    {
      HANDLE f = CreateFile( 
        if2k_redir_path, 
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL, // security
        CREATE_ALWAYS, // CreationDisposition
        FILE_ATTRIBUTE_NORMAL, // attributes
        NULL // template file
        );

      jdk_log_debug4("fixer: CreateFile (%s) result is : 0x%x", if2k_redir_path, (DWORD)f  );
      
      if( f != INVALID_HANDLE_VALUE )
      {
        DWORD write_count=0;
        if( WriteFile( f, p, sz, &write_count, NULL ) )
        {
          jdk_log_debug4("fixer: WriteFile success" );
          dll_is_good=true;
        }
        else
        {
          jdk_log_debug4("fixer: WriteFile failure (%ld)", GetLastError() );
        }
        CloseHandle( f );
        LoadLibrary( IF2K_MINI_REDIR_DLL_NAME );
      }      
    }
  }

  jdk_log_debug4("dll_is_good: %d", (int)dll_is_good );

  bool need_to_set_hook=false;

  if( dll_is_good )
  {
    if( jdk_util_registry_get_string( 
      HKEY_LOCAL_MACHINE, 
      IF2K_MINI_REDIR_HOOK_REGISTRY_LOCATION, 
      IF2K_MINI_REDIR_HOOK_KEY, 
      dll_hook_value, 
      sizeof( dll_hook_value )
      ) )
    {    
      jdk_log_debug4("got hook registry val: %s", dll_hook_value );

      if( strstr( dll_hook_value, "if2k_redir.dll" )==0 )
      {
        need_to_set_hook=true;
      }
    }
    else
    {
      jdk_log_debug4("error getting hook registry val" );
      need_to_set_hook=true;
    }

    if( need_to_set_hook && *if2k_redir_path )
    {
      strcat( dll_hook_value, " " );
      strcat( dll_hook_value, if2k_redir_path );
      
      jdk_log_debug4("setting hook registry val: %s", dll_hook_value );
      
      if(jdk_util_registry_set_string(
        HKEY_LOCAL_MACHINE,
        IF2K_MINI_REDIR_HOOK_REGISTRY_LOCATION,
        IF2K_MINI_REDIR_HOOK_KEY,
        dll_hook_value))
      {
        jdk_log_debug4("set new hook registry val" );
      }
      else
      {
        jdk_log_debug4("error setting hook registry val" );
      }      
    }
  }

}

#endif

#if JDK_IS_WIN32 && IF2K_MINI_CONFIG_FIX_BROKEN_DLL
void if2k_mini_service_thread::notify_if2k_redir()
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
  else
  {
    fix_broken_redir_dll();
  }
}
#else
void if2k_mini_service_thread::notify_if2k_redir()
{
}
#endif



bool if2k_mini_service_thread::load_settings()
{
  return true;
}

void if2k_mini_service_thread::setup_log()
{
  jdk_string_filename home( settings.get("home").c_str() );
  
	jdk_set_home_dir(
    home.c_str()
    );
  
	jdk_log_setup(
    settings.get_long( "log.type" ),
    settings.get( "log.file" ),
    settings.get_long( "log.detail" )
    );				  
}



bool if2k_mini_service_thread::bind_server()
{
  jdk_str<512> ip( settings.get("httpfilter.interface") );
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



bool if2k_mini_service_thread::start_worker_threads( if2k_mini_kernel_v2 &kernel)
{
  bool r;
  int threads_started = 0;
  
  num_threads = settings.get_long("httpfilter.max.sockets");
  
  if( num_threads > IF2K_MAX_THREADS )
    num_threads = IF2K_MAX_THREADS;
  if( num_threads ==0 )
    num_threads=1;
  
  
  for( int i=0; i<num_threads; ++i )
  {
    threads[i] = new if2k_server_thread( settings, kernel, server_socket);
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



bool if2k_mini_service_thread::stop_worker_threads()
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
  
  return true;
}



void if2k_mini_service_thread::kill_worker_threads()
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

void if2k_mini_service_thread::finish_worker_threads()
{
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
}

bool if2k_mini_service_thread::handle_signals( if2k_mini_kernel_v2 &kernel )
{
  bool please_stop=false;
#if JDK_HAS_FORK
  if( jdk_fork_server_saw_sigalarm )
  {
    jdk_log( JDK_LOG_INFO, "SIGALARM: exit requested" );
    please_stop=true;
  }
  
  if( jdk_fork_server_saw_sighup )
  {
    jdk_log( JDK_LOG_INFO, "SIGHUP: Reload requested");
    kernel.trigger_restart();
  }
  
  if( jdk_fork_server_saw_sigterm )
  {
    jdk_log( JDK_LOG_INFO, "SIGTERM: Exit requested" );
    please_stop=true;
  }
#endif        
  return please_stop;
}

void if2k_mini_service_thread::do_precompiling()
{
  if( is_precompile_triggered )
  {
    if2k_mini_compile_all( settings );
    is_precompile_triggered=false;
  }
}

void if2k_mini_service_thread::do_restart_countdown( if2k_mini_kernel_v2 &kernel )
{
  if( reload_time!=0 )
  {
    if( --reload_time==0 )
    {
      kernel.trigger_restart();
    }
  }
}


void if2k_mini_service_thread::main()
{
  notify_if2k_redir();

  setup_log();

#if JDK_IS_WIN32 && IF2K_MINI_CONFIG_FIX_BROKEN_DLL
  fix_broken_redir_dll();
#endif

  bind_server();
 
  do
  {
    is_restart_triggered = false;
    settings_collection.reload();
    
    jdk_log( JDK_LOG_INFO, "Starting %s service", IF2K_MINI_SERVER_SERVICE_NAME );
    
    do_precompiling();
    
    if2k_mini_kernel_v2 kernel( settings_collection );
    kernel.load();
    
    if( start_worker_threads(kernel) )
    {
      jdk_remote_buf::enable_remote_loading();
      reload_time = settings_collection.get_reload_time_in_seconds();
      
      while( !please_stop && !kernel.is_restart_triggered() && !handle_signals(kernel) )
      {        
        sleep(1);
#if JDK_IS_WIN32 && IF2K_MINI_CONFIG_FIX_BROKEN_DLL
        fix_broken_redir_dll();
#endif
        do_restart_countdown(kernel);
      }
    }

    is_restart_triggered = kernel.is_restart_triggered();
    is_precompile_triggered = kernel.is_precompile_triggered();

    jdk_log( JDK_LOG_INFO, "%s %s service", 
             is_restart_triggered ? "Restarting" : "Stopping" ,IF2K_MINI_SERVER_SERVICE_NAME );

    finish_worker_threads();
       
  } while( is_restart_triggered );

  server_socket.close();
}

#if JDK_IS_WIN32
int WINAPI  WinMain( 
					 HINSTANCE hInstance, 
					 HINSTANCE hPrevInstance, 
					 LPSTR lpCmdLine, 
					 int nShowCmd 
					 )
{
  jdk_remote_buf::disable_remote_loading();
  jdk_win32svc<if2k_mini_service_thread> svc( 
        IF2K_MINI_SERVER_SERVICE_NAME, 
        IF2K_MINI_PRODUCT_NAME, 
        IF2K_MINI_VERSION
        );

	return svc.WinMain( hInstance, hPrevInstance, lpCmdLine, nShowCmd );
}
#else
int main( int argc, char **argv )
{
  jdk_remote_buf::disable_remote_loading();
#if JDK_IS_MACOSX
  extern bool jdk_socket_override_filtering;
  jdk_socket_override_filtering = true;
#endif
  jdk_settings settings( if2k_mini_kernel_defaults, argc, argv );
  
  jdk_set_home_dir( settings.get("home").c_str() );
  jdk_set_app_name( settings.get("daemon.name").c_str() );
  
  jdk_log_setup( settings );

#if JDK_IS_MACOSX
  if( settings.get_long( "nokext" )==0 )
  {
    system( "/sbin/kextload /System/Library/Extensions/if2kext.kext" );
  }
#endif
  
  if( settings.get_long( "restart" )==1 )
  {
    if( jdk_fork_server_signal_via_pidfile( settings, SIGTERM ) )
    {
      sleep(2);
      jdk_fork_server_signal_via_pidfile( settings, SIGKILL );
    }
  }
  else
  if( settings.get_long( "stop" )==1 )
  {
    if( jdk_fork_server_signal_via_pidfile( settings, SIGTERM ) )
    {
      sleep(2);
      jdk_fork_server_signal_via_pidfile( settings, SIGKILL );
    }
    return 0;
  }

  jdk_fork_server_daemonize( settings );
  jdk_fork_handle_sighup_sigterm_sigalarm();
  
  if2k_mini_service_thread service_thread(argc,argv);
  return service_thread.blockrun();
}
#endif

