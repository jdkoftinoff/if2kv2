#include "jdk_world.h"
#include "jdk_socket.h"
#include "if2k_kernel.h"


int main()
{
  return 0;
}
#if 0
#include "jdk_httpproxy.h"
#define REGISTRY_LOCATION "SOFTWARE\\Turner and Sons\\if2svc\\v2.0"

void WINAPI ServiceMain( DWORD argc, LPTSTR *argv );
void WINAPI Handler( DWORD fdwControl );

volatile bool stop_requested=false;

jdk_setting_description default_settings[] =
{
  {"port", "8000", "TCP/IP port to listen on." },
  {"listen_count", "10", "TCP/IP listen queue size."},
  {"interface","0", "IP address to bind to (0=ALL)."},
  {"use_further_proxy","0", "Enable connections to additional proxy." },
  {"further_proxy","", "Additional proxy URL." },
  {"max_retries", "4", "Maximum retries when connecting to external server." },
  {"retry_delay", "100", "Delay in ms before retrying connection to external server." },
  {"connect_error_file", "connect_error.txt", "File name of 'Connection Error' response page." },
  {"bad_request_file", "bad_request.txt", "File name of 'Bad Request Error' response page." },
  {"redirect_file", "redirect.txt", "File name of 'HTTP Redirect' response page." },
  
  {"cached_settings", "", "local file to hold remote settings" },
  {"remote_settings", "", "URL of remote settings" },
  {"username", "", "username for remote settings" },	
  {"password", "", "password for remote settings" },
  {"remote_connect_error_file", "", "URL of remote connect_error_file" },
  {"remote_bad_request_file", "", "URL of remote bad_request_file" },
  {"remote_redirect_file", "", "URL of remote_redirect_file" },
  {"remote_update_rate", "3600", "update rate in seconds" },
  
#if JDK_IS_UNIX
  {"setuid", "0", "1 to set UID/GID on startup." },
  {"uid", "0", "UID to use." },
  {"gid", "0", "GID to use." },
  {"use_fork", "0", "Use forking processes instead of threads." },
  {"daemon", "1", "Become a background process." },
  {"pid_dir", "$", "Directory to put PID file in: Blank=none." },
  {"home", "$", "Home directory." },
#endif
  {"proxy", "", "http proxy to use." },
  {"username", "", "username." },
  {"password", "", "password." },
  {"name", "ifserver", "Name to use for logging." },
  {"log_type", "3", "Logging type: 0=none, 1=syslog, 2=stderr, 3=file." },
  {"log_file", "c:\\log.txt", "Logging file name." },
  {"log_detail", "4", "Logging detail level: 0=Errors, 1=Warnings, 2=Notices, 3=Info, 4-8=Debug" },
  {"threads", "10", "Number of threads to use." },
  
  // kernel settings
  {"good_urls_precomp_file", "", "Precompiled good urls." },
  {"good_urls_precomp_url", "", "Remote precompiled good urls." },
  {"good_urls_file", "", "Good url file." },
  {"good_urls_url", "", "Remote good url file." },
  
  {"bad_urls_precomp_file", "", "Precompiled bad urls." },
  {"bad_urls_precomp_url", "", "Remote precompiled bad urls." },
  {"bad_urls_file", "", "Bad url file." },
  {"bad_urls_url", "", "Remote bad url file." },
  
  {"bad_phrases_precomp_file", "", "Precompiled bad phrases." },
  {"bad_phrases_precomp_url", "", "Remote precompiled bad phrases." },
  {"bad_phrases_file", "", "Bad phrases file." },
  {"bad_phrases_url", "", "Remote bad phrases file." },
  
  {"url_loggers", "2", "Number of logging units." },
  {"loggerA_type", "file", "Type of logger A {none,file,socket,cgi,mail}." },
  {"loggerA_file", "log.txt", "Destination file name." },
  {"loggerA_level_0", "1", "Enable level 0 logging." },
  {"loggerA_level_1", "1", "Enable level 1 logging." },
  
  {"loggerB_type", "mail", "Type of logger A {none,file,socket,cgi,mail}." },	
  {"loggerB_level_0", "0", "Enable level 0 logging." },
  {"loggerB_level_1", "1", "Enable level 1 logging." },
  {"loggerB_smtp_server", "jdkoftinoff.com", "smtp server." },	
  {"loggerB_mail_from", "<jeffk@jdkoftinoff.com>", "send mail from email address." },
  {"loggerB_mail_to", "<jeffk@jdkoftinoff.com>", "send mail from email address." },	
  {"loggerB_mail_subject", "Filter notification", "send mail subject line." },
  
  // end of list
  {0,0}
};


void notify_if2redir()
{
  HMODULE if2redir=LoadLibrary("if2redir.dll");
  if( if2redir )
  {
//		MessageBox( 0, "able to load if2redir.dll", "not an error", MB_OK );
    typedef void (*mode_proc)(DWORD,DWORD *);
    
    mode_proc mode=(mode_proc)GetProcAddress(if2redir,"mode");
    if( mode )
    {
      DWORD a=0;
//			MessageBox( 0, "about to call if2redir.mode", "not an error", MB_OK );
      mode( 1, &a );		
    }	
    FreeLibrary(if2redir);
  }
}


void WINAPI ServiceMain( DWORD argc, LPTSTR *argv )
{
  SERVICE_STATUS_HANDLE handle = RegisterServiceCtrlHandler( "if2svc", Handler );
  SERVICE_STATUS status;
  
  jdk_settings_win32registry settings( HKEY_LOCAL_MACHINE, REGISTRY_LOCATION, default_settings );
  
  http_proxy_factory factory( settings );	
  jdk_log_setup(
    settings.get_long( "log_type" ),
    settings.get( "log_file" ),
    settings.get_long( "log_detail" )
    );				  
  
  
  jdk_thread_server_base * server = factory.create_thread_server();
  
  if( server->run() /*&& server->update()*/ )
  {
    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status.dwCurrentState = SERVICE_RUNNING;
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    status.dwWin32ExitCode = 0;
    status.dwServiceSpecificExitCode =0;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;
    SetServiceStatus( handle, &status );
    
    while(!stop_requested)
    {
      Sleep(1000);			
    }
    
    jdk_log( JDK_LOG_NOTICE, "SHUTTING DOWN" );
    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status.dwCurrentState = SERVICE_STOP_PENDING;
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    status.dwWin32ExitCode = 0;
    status.dwServiceSpecificExitCode =0;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;
    SetServiceStatus( handle, &status );	
    
    server->stop();
    jdk_sleep(2);
    server->kill();
    jdk_sleep(2);		
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
}


void WINAPI Handler( DWORD mode )
{
  if(mode==SERVICE_ACCEPT_STOP )
  {
    stop_requested=true;		
  }	
}


bool CreateService( const char *path)
{ 
  SC_HANDLE sc=OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);
  
  
  
  SC_HANDLE schService = CreateService( 
    sc, 			   // SCManager database 
    TEXT("if2svc"),     	   // name of service 
    "Internet Filter",         // service name to display 
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
  
  
  CloseServiceHandle(schService); 
  return true;
} 


bool DeleteService()  
{
  SC_HANDLE sc=OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);
  SC_HANDLE schService = OpenService( 
    sc,       		   // SCManager database 
    TEXT("if2svc"), // name of service 
    DELETE);            // only need DELETE access 
  
  if (schService == NULL) 
    return false;
  
  if (! DeleteService(schService) ) 
    return false;
  
  CloseServiceHandle(schService); 
  return true;
} 


int RunManually( int argc, char **argv )
{
  jdk_settings_win32registry settings( HKEY_LOCAL_MACHINE, REGISTRY_LOCATION, default_settings, argc, argv );
  
  http_proxy_factory factory( settings );	
  jdk_log_setup(
    settings.get_long( "log_type" ),
    settings.get( "log_file" ),
    settings.get_long( "log_detail" )
    );				  
  
  
  jdk_thread_server_base * server = factory.create_thread_server();
  
  if( server->run() /*&& server->update()*/ )
  {
    while(!stop_requested)
    {
      Sleep(1000);
    }
    
    server->stop();
    jdk_sleep(2);
    server->kill();
    jdk_sleep(2);		
  }
  else
  {
    delete server;
    return 1;	
  }
  
  delete server;
  return 0;
}




int main( int argc, char **argv )
{
  notify_if2redir();
  
  if( argc>1 && strcmp(argv[1],"--run")==0 )
  {
    return RunManually( argc, argv );	
  }	
  
  if( argc>2 && strcmp(argv[1],"--install")==0 )
  {
    if( !CreateService(argv[2]) )
    {
      MessageBox(0,"Unable to install if2svc","Error", MB_OK );
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else if( argc>1 && strcmp(argv[1],"--uninstall")==0 )
  {
    if( !DeleteService() )
    {
      MessageBox(0,"Unable to uninstall if2svc","Error",MB_OK );
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
        { "if2svc", ServiceMain },
        { 0, 0 }
      };
    
    if( !StartServiceCtrlDispatcher(entries) )
    {		
      exit(1);
    }
    return 0;
  }
  
}

#endif
