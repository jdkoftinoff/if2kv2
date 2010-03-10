#include "jdk_world.h"
#include "jdk_socket.h"
#include "if2k_kernel.h"
#include "jdk_daemon.h"
#include "if2k_serial.h"
#include "jdk_html.h"
#include "jdk_cgi.h"
#include "jdk_cgisettings.h"
#include "if2k_httpproxy.h"
#include "if2k_config.h"
#include "if2k_override_map.h"
#include "if2k_svc.h"
#include "if2k_admin.h"
#include "if2k_filter.h"
#include "if2k_settings.h"

#if JDK_IS_VCPP
#include "wnwindow.h"
#endif

extern "C" void register_if2k_httpfiles(void);
extern "C" void register_if2k_licenses(void);
#if IF2K_ENABLE_LUA
extern "C" void register_if2k_lua_files(void);
#endif

#define REMOTE_UPDATE_RATIO 60 // 3600

if2k_override_map *override_map=0;
if2_kernel_standard *global_kernel=0;
if2k_http_proxy_shared *global_shared=0;
jdk_recursivemutex *if2k_load_settings_mutex=0;

PLATFORM_SETTINGS *global_settings=0;
unsigned long global_session_id=0;

#if !JDK_IS_WIN32
jdk_string_filename global_settings_filename;
#endif



#if JDK_IS_WIN32

volatile static bool stop_requested=false;

static if2k_main_service_thread *svc_thread=0;

void if2k_notify_ifredir()
{
  HMODULE if2redir=GetModuleHandle( IF2K_REDIR_DLL_NAME );
  if( if2redir )
  {
    typedef void (*mode_proc)(DWORD,DWORD *);
    
    mode_proc mode=(mode_proc)GetProcAddress(if2redir,"mode");
    if( mode )
    {
      DWORD a=0;
      mode( 1, &a );		
    }
  }
}


void if2k_main_service_thread::main()
{
  jdk_recursivemutex load_mutex;
  if2k_load_settings_mutex=&load_mutex;
  SERVICE_STATUS status;
  
  status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  status.dwCurrentState = SERVICE_RUNNING;
  status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  status.dwWin32ExitCode = 0;
  status.dwServiceSpecificExitCode =0;
  status.dwCheckPoint = 0;
  status.dwWaitHint = 0;
  SetServiceStatus( handle, &status );
  
  PLATFORM_SETTINGS settings;
  
  {
    PLATFORM_SETTINGS encrypted_settings( 
      HKEY_LOCAL_MACHINE, 
      IF2K_REGISTRY_LOCATION, 
      default_server_settings, 
      argc, 
      argv 
      );
    
    jdk_settings_decrypt<IF2K_DECRYPTOR>(ENCRYPTION_KEY,encrypted_settings,settings,true);
  }
  
  global_settings = &settings;
  
  save_global_settings();
  
  jdk_set_home_dir(
    settings.get("home").c_str()
    );
  
  jdk_log_setup(
    settings.get_long( "log_type" ),
    settings.get( "log_file" ),
    settings.get_long( "log_detail" )
    );				  
  
  jdk_nb_inet_server_socket server_socket( settings.get("interface"), settings.get_long("listen_count"));
  jdk_nb_inet_server_socket admin_socket( settings.get("admin_interface"), settings.get_long("listen_count"));
  
  jdk_log_debug1( "About to load settings and check serial #" );
  load_global_settings();
  
  jdk_log_debug2( "Creating thread server factory" );	
  if2k_http_proxy_factory factory( settings, server_socket, admin_socket );
  
  jdk_log_debug2( "creating override map" );
  override_map = new if2k_override_map;
  
  jdk_log_debug2( "Creating thread server base" );
  jdk_thread_server_base * server = factory.create_thread_server();
  
  jdk_log_debug2( "About to server->run()" );
  if( server->run() /*&& server->update()*/ )
  {
    time_t last_update_time = time(0);
    jdk_live_long remote_update_rate( settings, "remote_update_rate" );
    
    while(!stop_requested)
    {
      Sleep(1000);	
      
      bool do_reload=false;
      
      if( global_shared )
      {
        if( global_shared->lua.reload_settings_was_triggered() )
        {
          jdk_log_debug1( "lua triggered reload" );
          save_global_settings();
          do_reload=true;
        }
      }
      
      if( remote_update_rate.get()!=0
          && ( time(0) - last_update_time > remote_update_rate.get()*REMOTE_UPDATE_RATIO )
        )
      {
        jdk_log_debug1( "time triggered reload" );
        do_reload=true;
      }
      
      if( do_reload )
      {
        jdk_synchronized( *if2k_load_settings_mutex );
        jdk_log_debug1( "performing settings reload" );
        last_update_time = time(0);
        load_global_settings();
        if( global_shared )
          global_shared->update();
        if( global_kernel )
          global_kernel->update();
        save_global_settings();				
        jdk_log_debug1( "done settings reload" );
      }
    }
    
    jdk_log( JDK_LOG_NOTICE, "SHUTTING DOWN" );
    
    server->stop();
    Sleep(4000);
    server->kill();
  }
  delete server;
  
  
  status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  status.dwCurrentState = SERVICE_STOPPED;
  status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  status.dwWin32ExitCode = 0;
  status.dwServiceSpecificExitCode =0;
  status.dwCheckPoint = 0;
  status.dwWaitHint = 0;
  SetServiceStatus( handle, &status );	
  delete override_map;
}


void WINAPI if2k_ServiceMain( DWORD argc, LPTSTR *argv )
{
  if2k_main_service_thread main_thread(argc,argv);
  svc_thread = &main_thread;
  main_thread.blockrun();
}


void WINAPI if2k_Handler( DWORD mode )
{
  if(mode==SERVICE_ACCEPT_STOP && svc_thread)
  {
    SERVICE_STATUS status;
    
    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status.dwCurrentState = SERVICE_STOP_PENDING;
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    status.dwWin32ExitCode = 0;
    status.dwServiceSpecificExitCode =0;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;
    SetServiceStatus( svc_thread->handle, &status );	
    
    stop_requested=true;		
  }	
}


bool if2k_ControlService(DWORD fdwControl) 
{ 
  SERVICE_STATUS ssStatus; 
  DWORD fdwAccess; 
  DWORD dwStartTickCount, dwWaitTime;
  
  SC_HANDLE sc=OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);
  
  // The required service object access depends on the control. 
  
  switch (fdwControl) 
  { 
  case SERVICE_CONTROL_STOP: 
    fdwAccess = SERVICE_STOP; 
    break; 
    
  case SERVICE_CONTROL_PAUSE: 
  case SERVICE_CONTROL_CONTINUE: 
    fdwAccess = SERVICE_PAUSE_CONTINUE; 
    break; 
    
  case SERVICE_CONTROL_INTERROGATE: 
    fdwAccess = SERVICE_INTERROGATE; 
    break; 
    
  default: 
    fdwAccess = SERVICE_INTERROGATE;
    break;
  } 
  
  // Open a handle to the service. 
  
  SC_HANDLE schService = OpenService( 
    sc,        // SCManager database 
    TEXT(IF2K_SHORT_SERVICE_NAME),        // name of service 
    fdwAccess);          // specify access 
  
  if (schService == NULL) 
  {
    return false;
  }
  
  // Send a control value to the service. 
  
  if (! ControlService( 
        schService,   // handle to service 
        fdwControl,   // control value to send 
        &ssStatus) )  // address of status info 
  {
    return false;
  }
  return true;
}

bool if2k_CreateService( const char *path)
{ 
  SC_HANDLE sc=OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);
  
  
  
  SC_HANDLE schService = CreateService( 
    sc, 			   // SCManager database 
    TEXT(IF2K_SHORT_SERVICE_NAME),     	   // name of service 
    IF2K_SERVICE_NAME,         // service name to display 
    SERVICE_ALL_ACCESS,        // desired access 
    SERVICE_WIN32_OWN_PROCESS, // service type 
    SERVICE_AUTO_START,      // start type 
    
    SERVICE_ERROR_NORMAL,      // error control type 
    path,        // services binary 
    NULL,                      // no load ordering group 
    NULL,                      // no tag identifier 
    NULL,                      // no dependencies 
    NULL,                      // LocalSystem account 
    NULL                     // no password 
    );
  
  
  if (schService == NULL) 
    return false;
  Sleep(1000);
  StartService( schService, 0, NULL );
  
  CloseServiceHandle(schService); 
  
  return true;
} 


bool if2k_DeleteService()  
{
  if2k_ControlService( SERVICE_CONTROL_STOP );
  
  SC_HANDLE sc=OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);
  SC_HANDLE schService = OpenService( 
    sc,       		   // SCManager database 
    TEXT(IF2K_SHORT_SERVICE_NAME), // name of service 
    DELETE);            // only need DELETE access 
  
  if (schService == NULL) 
    return false;
  
  if (! DeleteService(schService) ) 
    return false;
  
  CloseServiceHandle(schService); 
  return true;
} 


int if2k_RunManually( int argc, char **argv )
{
  jdk_recursivemutex load_mutex;
  if2k_load_settings_mutex=&load_mutex;
  
  PLATFORM_SETTINGS settings;
  
  {
    PLATFORM_SETTINGS encrypted_settings( 
      HKEY_LOCAL_MACHINE, 
      IF2K_REGISTRY_LOCATION, 
      default_server_settings, 
      argc, 
      argv 
      );
    
    jdk_settings_decrypt<IF2K_DECRYPTOR>(ENCRYPTION_KEY,encrypted_settings,settings,true);
  }
  
  global_settings = &settings;
  
  save_global_settings();
  
  jdk_set_home_dir(
    settings.get("home").c_str()
    );
  
  jdk_log_setup(
    settings.get_long( "log_type" ),
    settings.get( "log_file" ),
    settings.get_long( "log_detail" )
    );				  
  
  jdk_nb_inet_server_socket server_socket( settings.get("interface"), settings.get_long("listen_count"));
  jdk_nb_inet_server_socket admin_socket( settings.get("admin_interface"), settings.get_long("listen_count"));
  
  jdk_log_debug1( "About to load settings and check serial # (2)" );
  load_global_settings();
  jdk_log_debug2( "Creating thread server factory" );	
  
  if2k_http_proxy_factory factory( settings, server_socket, admin_socket );
  
  
  override_map = new if2k_override_map;
  jdk_log_debug1( "About to check serial # (2)" );	
  if2_serial_get( settings, ENCRYPTION_KEY, IF2K_SERIAL_URL1, IF2K_SERIAL_URL2, 0);
  jdk_log_debug1( "Creating thread server factory" );
  
  jdk_thread_server_base * server = factory.create_thread_server();
  
  time_t last_update_time = time(0);
  jdk_live_long remote_update_rate( settings, "remote_update_rate" );
  
  if( server->run() /*&& server->update()*/ )
  {
    while(!stop_requested)
    {
      Sleep(1000);	
      
      bool do_reload=false;
      
      if( global_shared )
      {
        if( global_shared->lua.reload_settings_was_triggered() )
        {
          save_global_settings();
          do_reload=true;
        }
      }
      
      if( remote_update_rate.get()!=0
          && ( time(0) - last_update_time > remote_update_rate.get()*REMOTE_UPDATE_RATIO )
        )
      {
        do_reload=true;
      }
      
      if( do_reload )
      {
        jdk_synchronized( *if2k_load_settings_mutex );
        last_update_time = time(0);
        load_global_settings();
        if( global_shared )
          global_shared->update();
        if( global_kernel )
          global_kernel->update();
        save_global_settings();
      }
      
    }
    
    server->stop();
    Sleep(4000);
    server->kill();
    jdk_sleep(2);		
  }
  else
  {
    delete server;
    delete override_map;
    return 1;	
  }
  
  delete server;
  delete override_map;
  return 0;
}




int if2k_svc_main( int argc, char **argv )
{
  srand( time(0) );
  global_session_id=rand();
  jdk_set_app_name( IF2K_SHORT_SERVICE_NAME );
  jdk_set_home_dir( DEFAULT_APP_DIR );
  
  //register_if2k_httpfiles();
  //register_if2k_defaultfiles();
  //register_if2k_licenses();
#if IF2K_ENABLE_LUA
  register_if2k_lua_files();	
#endif	
  if2k_notify_ifredir();
  
  if( argc>1 && strcmp(argv[1],"--run")==0 )
  {
    return if2k_RunManually( argc-1, argv+1 );
  }	
  
  if( argc>2 && strcmp(argv[1],"--install")==0 )
  {
    if( !if2k_CreateService(argv[2]) )
    {
      MessageBox(0,"Unable to install " IF2K_SHORT_SERVICE_NAME,"Error", MB_OK );
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else if( argc>1 && strcmp(argv[1],"--uninstall")==0 )
  {		
    if( !if2k_DeleteService() )
    {
      MessageBox(0,"Unable to uninstall " IF2K_SHORT_SERVICE_NAME,"Error",MB_OK );
      return 1;
    }
    else
    {
      return 0;
    }
    
  }
  else
  {
    // do service
    static SERVICE_TABLE_ENTRY entries[] =
      {
        { IF2K_SHORT_SERVICE_NAME, if2k_ServiceMain },
        { 0, 0 }
      };
    
    if( !StartServiceCtrlDispatcher(entries) )
    {		
      exit(1);
    }
    return 0;
  }
  
}
#else

#ifndef IF2K_SVC_FORKED

int if2k_svc_main( int argc, char **argv )
{
  jdk_recursivemutex load_mutex;
  if2k_load_settings_mutex=&load_mutex;
  
  //register_if2k_httpfiles();
  //register_if2k_defaultfiles();	
  //register_if2k_licenses();
#if IF2K_ENABLE_LUA
  register_if2k_lua_files();
#endif		
  srand( time(0) );	
  global_session_id=rand();
  jdk_set_app_name( IF2K_SHORT_SERVICE_NAME );
  jdk_string_filename home_dir( DEFAULT_APP_DIR );	
  bool new_home_dir=false;
  
  if( argc>1 && argv[1][0]!='-' )
  {
    jdk_process_path( argv[1], home_dir.c_str(), home_dir.getmaxlen() );
    argc--;
    argv++;
    new_home_dir=true;
  }
  jdk_set_home_dir( home_dir.c_str() );	
  jdk_process_path( 
    DEFAULT_SETTINGS_FILE, 
    global_settings_filename.c_str(), 
    global_settings_filename.getmaxlen() 
    );
  
  PLATFORM_SETTINGS settings;
  
  {
    PLATFORM_SETTINGS encrypted_settings( 
      default_server_settings, 
      argc, 
      argv, 
      global_settings_filename.c_str() 
      );
    jdk_settings_decrypt<IF2K_DECRYPTOR>(ENCRYPTION_KEY,encrypted_settings,settings,true);
  }
  if( new_home_dir )
  {
    settings.set("home",home_dir);
  }
  global_settings = &settings;
  
  save_global_settings();
  
  jdk_log_setup(
    settings.get_long( "log_type" ),
    settings.get( "log_file" ),
    settings.get_long( "log_detail" )
    );				  
  
  jdk_nb_inet_server_socket server_socket( settings.get("interface"), settings.get_long("listen_count"));
  jdk_nb_inet_server_socket admin_socket( settings.get("admin_interface"), settings.get_long("listen_count"));
  
  load_global_settings();
  
  if( jdk_daemonize(
        (bool)settings.get_long("daemon"),
        settings.get("name").c_str(),
        settings.get("home").c_str(),
        settings.get("pid_dir").c_str() ) <0
    )
  {
    return 1;
  }
  
  override_map = new if2k_override_map;
  
  if2k_http_proxy_factory factory( settings, server_socket, admin_socket );
  
  jdk_thread_server_drop_root( settings);
  
//    if2_serial_get( settings, ENCRYPTION_KEY, IF2K_SERIAL_URL, 0);
  
  jdk_thread_server_base * server = factory.create_thread_server();
  
  if( server->run()  )
  {
    time_t last_update_time = time(0);
    jdk_live_long remote_update_rate( settings, "remote_update_rate" );
    
    while(!jdk_sigterm_caught && !jdk_sighup_caught )
    {
      sleep(1);
      bool do_reload=false;
      
      if( global_shared )
      {
        if( global_shared->lua.reload_settings_was_triggered() )
        {
          save_global_settings();
          do_reload=true;
        }
      }
      
      if( remote_update_rate.get()!=0
          && ( time(0) - last_update_time > remote_update_rate.get()*REMOTE_UPDATE_RATIO )
        )
      {
        do_reload=true;
      }
      
      if( do_reload )
      {
        jdk_synchronized( *if2k_load_settings_mutex );
        last_update_time = time(0);
        load_global_settings();
        if( global_shared )
          global_shared->update();
        if( global_kernel )
          global_kernel->update();
        save_global_settings();
      }
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
  delete override_map;
  jdk_log( JDK_LOG_NOTICE, "SHUT DOWN COMPLETE" );
  jdk_daemon_end();
  return 0;
}
#else

#include "jdk_server_family.h"

this_is_broken


#endif
#endif


#if JDK_IS_VCPP
int __stdcall if2k_svc_WinMain(
  HINSTANCE hInstance_,          	// current instance
  HINSTANCE hPrevInstance_,        // previous instance
  LPSTR lpCmdLine_,                // command line
  int nCmdShow_                    // show-window type (open/icon)
  )
{
  return if2k_svc_main( 0, 0 );
}

bool jdk_internet_is_connected() 
{ 
  return true; 
}

void WNKillApplication()
{
}

WNWindow * WNCreateMainWindow()
{
  return 0;
}

bool WNInitializeApplication()
{
  return true;
}

extern "C" void register_if2k_httpfiles(void)
{
}

extern "C" void register_if2k_defaultfiles(void)
{
}

extern "C" void register_if2k_licenses(void)
{
}


#endif
