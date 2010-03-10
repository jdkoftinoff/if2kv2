#if defined(WIN32)

#ifndef _JDK_WIN32SVC_H
#define _JDK_WIN32SVC_H
#include <windows.h>

#include "jdk_world.h"
#include "jdk_thread.h"
#include "jdk_log.h"
#include "jdk_string.h"


class jdk_win32svc_base
{
public:
  jdk_win32svc_base()
  {
    global_service_object = this;
  }
  virtual ~jdk_win32svc_base()
  {
  }

  virtual const char *get_service_name() const=0;
  virtual const char *get_service_title() const=0;
  virtual const char *get_service_version() const=0;
  virtual bool install_service()=0;
  virtual bool uninstall_service()=0;
  virtual bool report_version()=0;
	virtual bool control_service(DWORD fdwControl)=0;
  virtual bool start_service( int argc, char **argv )=0;
  virtual jdk_thread *create_main_service_thread(int args, char **argv)=0;
  virtual jdk_thread *get_main_service_thread()=0;
  virtual int run_manually( int argc, char **argv)=0; 
  virtual bool notify_service_status( DWORD state )=0;
  virtual bool register_service()=0;
  virtual SERVICE_STATUS_HANDLE get_service_handle()=0;

public:

  static jdk_win32svc_base *global_service_object;
};


template <class THREADCLASS>
class jdk_win32svc : public jdk_win32svc_base
{
public:
  jdk_win32svc( const char *service_name_, const char *service_title_, const char *service_version_) 
    : 
    service_name( service_name_ ),
    service_title( service_title_ ),
    service_version( service_version_ )
    {
      main_service_thread= 0;
    }

  virtual ~jdk_win32svc() 
  {
  }

  const char *get_service_name() const 
  { 
    return service_name; 
  }

  const char *get_service_title() const
  {
    return service_title;
  }

  const char *get_service_version() const
  {
    return service_version;
  }

  bool install_service()
  {
    char path[1024]="";
    GetModuleFileNameA( 0, path, sizeof(path) );
    SC_HANDLE sc=OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);

    SC_HANDLE schService = CreateServiceA( 
        sc, 			   // SCManager database 
        get_service_name(),     	   // name of service 
        get_service_title(),         // service name to display 
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

    if (schService != NULL) 
    {
	    Sleep(1000);
	    StartService( schService, 0, NULL );
      CloseServiceHandle(schService);
    }

    return schService!=NULL;
  }

  bool uninstall_service()
  {
    bool r=false;

	  control_service( SERVICE_CONTROL_STOP );

    SC_HANDLE sc=OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);
    SC_HANDLE schService = OpenServiceA( 
        sc,       		   // SCManager database 
        get_service_name(), // name of service 
        DELETE);            // only need DELETE access 

    if (schService != NULL )
    {
      if( DeleteService(schService) )
      {
        CloseServiceHandle(schService);
        Sleep(3000);
        r=true;
      }
    }

    

    return r;
  }

  bool report_version()
  {
    printf( "%s (%s) version %s", get_service_title(), get_service_name(), get_service_version() );
    return true;
  }


	bool control_service(DWORD fdwControl) 
	{ 
		SERVICE_STATUS ssStatus; 
		DWORD fdwAccess; 
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

		SC_HANDLE schService = OpenServiceA( 
			sc, 			 // SCManager database 
	    get_service_name(),				// name of service 
			fdwAccess); 				 // specify access 

		if (schService == NULL) 
		{
			return false;
    }
		
		// Send a control value to the service. 

		if (! ControlService( 
			      schService, 	// handle to service 
			      fdwControl, 	// control value to send 
			      &ssStatus) )	// address of status info 
		{
			return false;
		}
		return true;
	}

  bool start_service( int argc, char **argv )
  {
    typedef void (WINAPI *service_main_proc)( DWORD argc, char **argv );

		static SERVICE_TABLE_ENTRYA entries[] =
		{
			{ 0, (service_main_proc)service_main },
			{ 0, 0 }
		};

    entries[0].lpServiceName = (char *)get_service_name();

    return StartServiceCtrlDispatcherA(entries)==0 ? 1 : 0;
  }


  jdk_thread *create_main_service_thread(int argc, char **argv)
  {
    main_service_thread = new THREADCLASS(argc,argv);
    return main_service_thread;
  }

  jdk_thread *get_main_service_thread()
  {
    return main_service_thread;
  }

  int run_manually( int argc, char **argv)
  {
    create_main_service_thread(argc, argv);
    return get_main_service_thread()->blockrun();
  }

  bool notify_service_status( DWORD state )
  {
    SERVICE_STATUS status;
  	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status.dwCurrentState = state;
  	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  	status.dwWin32ExitCode = 0;
  	status.dwServiceSpecificExitCode =0;
  	status.dwCheckPoint = 0;
  	status.dwWaitHint = 0;
    return SetServiceStatus( service_handle, &status ) ? true : false;	
  }

  bool register_service()
  {
    service_handle = RegisterServiceCtrlHandlerA( get_service_name(), service_handler );
    return service_handle!=0 ? true : false;
  }

  SERVICE_STATUS_HANDLE get_service_handle()
  {
    return service_handle;
  }

  static void WINAPI service_main( int argc, char **argv )
  {
    jdk_win32svc_base::global_service_object->create_main_service_thread(argc,argv);
    jdk_win32svc_base::global_service_object->register_service();
    jdk_win32svc_base::global_service_object->notify_service_status(SERVICE_RUNNING);
	  jdk_win32svc_base::global_service_object->get_main_service_thread()->blockrun();
    jdk_win32svc_base::global_service_object->notify_service_status(SERVICE_STOPPED);
  }

  static void WINAPI service_handler( DWORD mode )
  {
    if( mode==SERVICE_ACCEPT_STOP )
    {
      jdk_win32svc_base::global_service_object->notify_service_status(SERVICE_STOP_PENDING);
      jdk_win32svc_base::global_service_object->get_main_service_thread()->thread_pleasestop();
    }
  }

  static DWORD WinMain( 
					 HINSTANCE hInstance, 
					 HINSTANCE hPrevInstance, 
					 LPSTR lpCmdLine, 
					 int nShowCmd 
					 )
  {
    if( __argc>1 )
    {
      if( strcmp(__argv[1], "-run")==0 )
      {
        __argv[1] = __argv[0];

        return jdk_win32svc_base::global_service_object->run_manually( __argc-1, __argv+1 ) ? 0 : 1;
      }
      else if( strcmp(__argv[1], "-i")==0 )
      {
        return jdk_win32svc_base::global_service_object->install_service() ? 0 : 1;
      }
      else if( strcmp(__argv[1], "-u")==0 )
      {
        return jdk_win32svc_base::global_service_object->uninstall_service() ? 0 : 1;
      }
      else if( strcmp(__argv[1], "-v")==0 )
      {
        return jdk_win32svc_base::global_service_object->report_version() ? 0 : 1;
      }
    }
    return jdk_win32svc_base::global_service_object->start_service( __argc, __argv ) ? 0 : 1;
  }


  static jdk_win32svc<THREADCLASS> *global_service_object;

  const char *service_name;
  const char *service_title;
  const char *service_version;
  jdk_thread *main_service_thread;
  SERVICE_STATUS_HANDLE service_handle;
};


#endif

#endif
