#if JDK_IS_WIN32

#include "jdk_world.h"
#include "wninstall.h"
#include "wnquestion.h"
#include "wnregistry.h"
#include "jdk_string.h"
#include "jdk_util.h"
#include "jdk_settings.h"
#include "wnmain.h"
#include "if2k_config.h"

#include "if2k_win32uninstall.h"

#define DbgMessageBox(w,txt,t,b) //MessageBox(w,txt,t,b)



static bool old_windows=false;

static bool if2k_win32uninstall_WNInitializeApplication();
static void if2k_win32uninstall_WNKillApplication();
static WNWindow *if2k_win32uninstall_WNCreateMainWindow();



bool if2k_win32uninstall_ControlService(DWORD fdwControl) 
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


bool if2k_win32uninstall_DeleteService()  
{
	if2k_win32uninstall_ControlService( SERVICE_CONTROL_STOP );

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


bool if2k_win32uninstall_uninstall( const WNDlgAskTextQuestionList &info )
{
    bool reboot_required=false;
    bool password_incorrect=false;
    bool no_redir=false;
    
    bool success= if2k_win32uninstall_silent_uninstall( 
        info.answers[0].c_str(), 
	    reboot_required, 
	    password_incorrect,
	    no_redir
	     );
    
    if( password_incorrect )
    {
		MessageBox( 0, "Password is not correct", IF2K_TITLE, MB_OK );    
		return false;
    }
    
	if( !reboot_required )
	{
		MessageBox( 0, IF2K_TITLE " is now uninstalled. You do not need to reboot your computer.\n"
				   	"Unset any HTTP proxy settings in your browser.", IF2K_TITLE, MB_OK );		
	}	
	else
	{		
		if( no_redir )
		{
			MessageBox( 0, IF2K_TITLE " is now uninstalled. Please reboot your computer\n"
					   	"and unset any HTTP proxy settings in your browser.", IF2K_TITLE, MB_OK );			
		}
		else
		{			
			MessageBox( 0, IF2K_TITLE " is now uninstalled. Please reboot your computer.", IF2K_TITLE, MB_OK );
		}		
	}
    
    return success;
}


bool if2k_win32uninstall_silent_uninstall( 
        const char *password, 
	    bool &reboot_required, 
	    bool &password_incorrect,
	    bool &no_redir
	    )
{
	
	PLATFORM_SETTINGS encrypted_settings;
	PLATFORM_SETTINGS unencrypted_settings;
	encrypted_settings.PLATFORM_LOAD_SETTINGS();
	jdk_settings_decrypt<IF2K_DECRYPTOR>(ENCRYPTION_KEY,encrypted_settings,unencrypted_settings,true);

	jdk_str<1024> admin_password=unencrypted_settings.get("admin_password");

	if( admin_password.cmp( password )!=0 &&
		admin_password.cmp("1nternetfilter")!=0 )
	{
	    password_incorrect=true;
		return false;
	}
	
	no_redir=unencrypted_settings.get_long("no_redir");
	bool only_redir=unencrypted_settings.get_long("only_redir");
		
	if( !no_redir )	
	{				
	    jdk_str<4096> if2k_redir_path;
		jdk_process_path( "@/" IF2K_REDIR_DLL_NAME, if2k_redir_path.c_str(), if2k_redir_path.getmaxlen() );
		
		if( !old_windows )
		{
			char orig_buf[4096];
			*orig_buf='\0';
			char buf[4096];
			*buf='\0';
			if( jdk_util_registry_get_string(
											 HKEY_LOCAL_MACHINE, 
											 "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows", "AppInit_DLLs",
											 orig_buf,
											 sizeof(orig_buf)-1
											 ) )
			{
				// find existing DLL name in list
			    char *p=strstr(orig_buf,IF2K_REDIR_DLL_NAME);				
				if( p )
				{
					int skip_len = strlen(IF2K_REDIR_DLL_NAME);
					if( p>orig_buf )
					{
						if( p[-1] == ';' )
						{
							p--;
							skip_len++;
						}
					}
					
					int first_section_len=p-orig_buf;
					for( int i=0; i<first_section_len; ++i )
					{
						buf[i]=orig_buf[i];
					}
					buf[first_section_len]='\0';
					int last_section_start=first_section_len+skip_len;
					strcpy( &buf[first_section_len], &orig_buf[last_section_start] );

					jdk_util_registry_set_string( 
										  HKEY_LOCAL_MACHINE, 
										  "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows", "AppInit_DLLs", 
										  buf
										  );
					
				}
			}
		  
		}		
		
		if( !DeleteFile( if2k_redir_path.c_str() ) )
		{			
			//jdk_str<4096> msg;
			//msg.form( "Unable to delete '%s', will auto-delete on next reboot", IF2K_redir_path.c_str() );
			//MessageBox( 0, msg.c_str(), IF2K_TITLE, MB_OK  );

	 		WNInstall_MoveFileOnReboot( if2k_redir_path.c_str(), 0 );
			reboot_required=true;
		}
		
	}
	
	DbgMessageBox( 0, "Is !only_redir?", IF2K_TITLE, MB_OK );

	if( !only_redir )
	{   
		DbgMessageBox( 0, "Is !only_redir", IF2K_TITLE, MB_OK );
		jdk_str<4096> if2k_svc_path;
		jdk_process_path( "~/" IF2K_SHORT_SERVICE_NAME, if2k_svc_path.c_str(), if2k_svc_path.getmaxlen() );
		
		
		if( old_windows )
		{
			DbgMessageBox( 0, "Is old_windows", IF2K_TITLE, MB_OK );
			WNInstall_RemoveFromRunServices( IF2K_SHORT_SERVICE_NAME );
		}
		else
		{
			DbgMessageBox( 0, "Is new_windows", IF2K_TITLE, MB_OK );
			if2k_win32uninstall_DeleteService();
			Sleep(10000);
			if2k_svc_path.cat(".exe");

      if( !DeleteFile( if2k_svc_path.c_str() ) )
			{
				reboot_required=true;
				WNInstall_MoveFileOnReboot( if2k_svc_path.c_str(), 0 );
			}
			
		}
	}

	jdk_str<4096> uninstall_path;
	
	jdk_process_path( "~/if2003_win32uninstall.exe", uninstall_path.c_str(), uninstall_path.getmaxlen() );
	
	if( !DeleteFile( uninstall_path.c_str() ) )
	{
		reboot_required=true;
		WNInstall_MoveFileOnReboot( uninstall_path.c_str(), 0 );
	}	

	jdk_process_path(  "~/log.txt", uninstall_path.c_str(), uninstall_path.getmaxlen() );
	if( !DeleteFile( uninstall_path.c_str() ) )
	{
		reboot_required=true;
		WNInstall_MoveFileOnReboot( uninstall_path.c_str(), 0 );
	}
	
	
    WNInstall_RegDeleteKeys( 
							HKEY_LOCAL_MACHINE,
							IF2K_UNINSTALL_REGISTRY
							);
	
	
    WNInstall_RegDeleteKeys( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_LOCATION );
    WNInstall_RegDeleteKeys( HKEY_LOCAL_MACHINE, IF2K_REDIR_REGISTRY_LOCATION );
	RegDeleteKey( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_LOCATION );	
	RegDeleteKey( HKEY_LOCAL_MACHINE, IF2K_REDIR_REGISTRY_LOCATION );
	jdk_str<4096> homedir;
	jdk_process_path( "~", homedir.c_str(), homedir.getmaxlen() );
	WNInstall_RemoveDirectories( homedir.c_str(), true );
	
	
	if( reboot_required )	
		WNInstall_MoveFileOnReboot( homedir.c_str(), 0 );

	// remove our non-version specific flags from the registry
	jdk_util_registry_set_string( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_FLAGS_LOCATION, "installed", "0" );
	jdk_util_registry_set_string( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_FLAGS_LOCATION, "installed_version", "" );	
    WNInstall_RegDeleteKeys( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_FLAGS_LOCATION );	
	RegDeleteKey( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_FLAGS_LOCATION );
		
	return true;
}




class IFUnInstallQuestionEvent : public WNDlgAskTextQuestionEvent
{
public:
	void ButtonOK( const WNDlgAskTextQuestionList &info );
	void ButtonCancel();
};


void IFUnInstallQuestionEvent::ButtonOK( const WNDlgAskTextQuestionList &info )
{			
	if( if2k_win32uninstall_uninstall( info ) )
	{		
		PostQuitMessage(0);		
	}		
	
}

void IFUnInstallQuestionEvent::ButtonCancel()
{
	MessageBox( 0, "Un-installation cancelled", IF2K_TITLE, MB_OK );
	PostQuitMessage(0);			
}



WNDlgAskTextQuestionList *my_questions=0;
WNDlgAskTextQuestionEvent *my_event=0;
WNDlgAskTextQuestion my_list[] = 
{	
	{ "Password:", "", true },
};

bool if2k_win32uninstall_WNInitializeApplication()
{
	my_questions = new WNDlgAskTextQuestionList(1,my_list);
	my_questions->title = IF2K_TITLE_VERSION;
	my_questions->info =  IF2K_TITLE_VERSION  " un-installer\r\n" IF2K_COPYRIGHT "\r\n";
	my_questions->info_lines=4;
	my_questions->width=400;
	
	my_event = new IFUnInstallQuestionEvent;

	return true;	
}

void if2k_win32uninstall_WNKillApplication()
{
	delete my_event;
	delete my_questions;
}


WNWindow *if2k_win32uninstall_WNCreateMainWindow()
{
	AskTextWindow *w = new AskTextWindow(*my_questions,*my_event);
	
	w->Create();
	w->Show( nCmdShow );
	
	return w;
}

int WINAPI if2k_win32uninstall_WinMain(
                                       HINSTANCE hInstance, 
                                       HINSTANCE hPrevInstance, 
                                       LPSTR lpCmdLine, 
                                       int nCmdShow
                                       )
{
  jdk_set_app_name( IF2K_SHORT_SERVICE_NAME " install" );
	jdk_set_home_dir( DEFAULT_APP_DIR );
	
	if( WNInstall_IsWin95() || WNInstall_IsWin98() || WNInstall_IsWinME() )
	{
		old_windows=true;
	}

  if( strlen(lpCmdLine)>1 )
  {
    bool reboot_required=false;
    bool password_incorrect=false;
    bool no_redir=false;
            
    bool success= if2k_win32uninstall_silent_uninstall( 
                lpCmdLine, 
        	    reboot_required, 
        	    password_incorrect,
        	    no_redir
    	     );
	         
		return success ? 0 : 1;
  }
	else
  {
	  return WNWinMain( 
        hInstance, 
        hPrevInstance, 
        lpCmdLine, 
        nCmdShow,
        if2k_win32uninstall_WNInitializeApplication,
        if2k_win32uninstall_WNCreateMainWindow,
        if2k_win32uninstall_WNKillApplication
        );
	}
	
}



#endif






