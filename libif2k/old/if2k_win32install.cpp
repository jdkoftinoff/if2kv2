
#if JDK_IS_WIN32
#include "jdk_world.h"
#include "jdk_socket.h"
#include "jdk_bindir.h"
#include "wninstall.h"
#include "wnquestion.h"
#include "wnregistry.h"
#include "jdk_string.h"
#include "jdk_settings.h"
#include "jdk_util.h"
#include "jdk_lz.h"
#include "wnmain.h"
#include "if2k_config.h"

#include "if2k_win32install.h"
#include "if2k_win32install_guts.h"

bool if2k_win32install_install( const WNDlgAskTextQuestionList &info );


class IFInstallQuestionEvent : public WNDlgAskTextQuestionEvent
{
public:
	void ButtonOK( const WNDlgAskTextQuestionList &info );
	void ButtonCancel();
};


static bool old_windows=false;

static WNDlgAskTextQuestionList *my_questions=0;
static WNDlgAskTextQuestionEvent *my_event=0;

#if IF2K_WIN32INSTALL_SERVER	
static char initial_host_and_ip[128];
static char initial_admin_host_and_ip[128];
#endif

static WNDlgAskTextQuestion my_list[] = 
{	
#if IF2K_WIN32INSTALL_SERVER
	{ "Product Serial #", "demoserver", false },
#else
	{ "Product Serial #", "demo", false },	
#endif	
#if IF2K_USE_ADMIN_NAME	
	{ "User name for administration", "admin", false },
#endif
	{ "Password for administration", "", true },
	{ "Retype password", "", true },
	{ "Additional HTTP proxy", "", false },	
#if IF2K_WIN32INSTALL_SERVER	
	{ "IP address for proxy", initial_host_and_ip, false },
	{ "IP address for administration", initial_admin_host_and_ip, false },
#endif		
	{ "Special Options", "", false }
};

#if IF2K_WIN32INSTALL_SERVER
#define NUM_QUESTIONSX 7
#else
#define NUM_QUESTIONSX 6
#endif

#if IF2K_USE_ADMIN_NAME
#define NUM_QUESTIONS NUM_QUESTIONSX
#else
#define NUM_QUESTIONS (NUM_QUESTIONSX-1)
#endif

static bool if2k_win32install_click_install( const WNDlgAskTextQuestionList &info )
{

   	jdk_settings_text install_settings;
	
	// extract normal settings
	int pos=0;
	install_settings.set( jdk_settings::key_t("serial"), info.answers[pos++] );
#if IF2K_USE_ADMIN_NAME
	install_settings.set( jdk_settings::key_t("admin_username"), info.answers[pos++] );
#endif
	install_settings.set( jdk_settings::key_t("admin_password"), info.answers[pos++] ); pos++;
	install_settings.set( jdk_settings::key_t("further_proxy"), info.answers[pos++] );

  	install_settings.set( jdk_settings::key_t("admin_interface"), jdk_settings::value_t("127.0.0.1:8001"));

#if IF2K_WIN32INSTALL_SERVER
	install_settings.set( jdk_settings::key_t("interface"), info.answers[pos++] );
	install_settings.set( jdk_settings::key_t("no_redir"), jdk_settings::value_t("1") );
	install_settings.set( jdk_settings::key_t("threads"), jdk_settings::value_t("50") );
	install_settings.set( jdk_settings::key_t("mode_ifserver"), jdk_settings::value_t("1") );	
	install_settings.set( jdk_settings::key_t("mode_expert"), jdk_settings::value_t("1") );
#else
	install_settings.set( jdk_settings::key_t("redir_redirect_dest1"), jdk_settings::value_t("127.0.0.1:8000"));
	install_settings.set( jdk_settings::key_t("interface"), jdk_settings::value_t("127.0.0.1:8000"));
	install_settings.set( jdk_settings::key_t("mode_ifserver"), jdk_settings::value_t("0") );
	install_settings.set( jdk_settings::key_t("mode_ifonly"), jdk_settings::value_t("0") );						 	
#endif	
						  
	// extract special options
	for( int i=pos; i<=pos; ++i )
	{
		jdk_str<4096> s( info.answers[i] );
		jdk_str<4096> key( info.answers[i] );
		char *p = key.chr('=');
		if( p )
		{
			*p='\0';
			jdk_str<4096> value;
			value.cpy( p+1 );
			
			install_settings.set( key, value );
		}		
	}	

	jdk_str<4096> error_reason;

#ifndef IF2K_WIN32INSTALL_FAKE
	bool r= if2k_win32install( install_settings, old_windows, true, error_reason );
#else
	bool r=false;
#endif	
	if( r )
	{
		bool no_redir=install_settings.get_long("no_redir") || old_windows;
		if( no_redir )
		{
			char buf[1024];
			sprintf( buf, "%s is installed and running. You do not need to reboot your computer,\nbut "
					 "you must set your web browser's HTTP proxy setting to %s\nYour browser will open"
					" to the filter login screen now.",
					 IF2K_TITLE,
					 install_settings.get("interface").c_str() );
			
			MessageBox( 0, buf, IF2K_TITLE, MB_OK );			
			sprintf( buf, "http://%s/", install_settings.get("admin_interface").c_str() );
			Sleep(2000);
			WNInstall_OpenBrowser( buf );
		}		
		else	
		{
			MessageBox( 0, IF2K_TITLE " is installed. It is recommended that you reboot your computer now.\n"
					   	"The filter login screen will appear when you reboot.", IF2K_TITLE, MB_OK );
		}
	}
	else
	{
		MessageBox( 0,error_reason.c_str(), IF2K_TITLE, MB_OK );		
	}
	
	return r;
}


void IFInstallQuestionEvent::ButtonOK( const WNDlgAskTextQuestionList &info )
{			
#if IF2K_USE_ADMIN_NAME		
	if( info.answers[2] != info.answers[3] )
	{
		MessageBox( 0, "Passwords do not match. Please try again.", IF2K_TITLE, MB_OK );
	}
	else if( info.answers[2].len()<2 )
	{
		MessageBox( 0, "Your password is too short. Please try again.", IF2K_TITLE, MB_OK );
	}	
#else
	if( info.answers[1] != info.answers[2] )
	{
		MessageBox( 0, "Passwords do not match. Please try again.", IF2K_TITLE, MB_OK );
	}
	else if( info.answers[1].len()<2 )
	{
		MessageBox( 0, "Your password is too short. Please try again.", IF2K_TITLE, MB_OK );
	}		
#endif	
	else
	{
		if( if2k_win32install_click_install( info ) )
		{			
			PostQuitMessage(0);
		}
		else
		{
			PostQuitMessage(0);
		}
		
	}
}

void IFInstallQuestionEvent::ButtonCancel()
{
	MessageBox( 0, "Installation cancelled", IF2K_TITLE, MB_OK );
	PostQuitMessage(0);			
}


static bool if2k_win32install_WNInitializeApplication()
{
#if DEBUG
	fprintf( stderr, "my_list[0].initial_value = %s\n", my_list[0].initial_value );
#endif	
	my_questions = new WNDlgAskTextQuestionList(NUM_QUESTIONS,my_list);
	my_questions->title = IF2K_TITLE_VERSION;
#if IF2K_WIN32INSTALL_SERVER
	my_questions->info = "IF-SERVER\r\n" IF2K_TITLE_VERSION "\r\n" IF2K_COPYRIGHT;
	{
	   	// server version defaults to hostname of computer
		char hname[256];
		if( jdk_gethostname(hname,sizeof(hname))==0 )
		{					    
#if IF2K_USE_ADMIN_NAME /* dumb hack alert */
			sprintf( initial_host_and_ip, "%s:8000", hname );
			sprintf( initial_host_and_ip, "%s:8001", hname );
#else			
			sprintf( initial_host_and_ip, "%s:8000", hname );
			sprintf( initial_host_and_ip, "%s:8001", hname );
#endif			
		}		
	}	
#else
	my_questions->info = IF2K_TITLE_VERSION "\r\n" IF2K_COPYRIGHT;	
#endif	
	my_questions->info_lines=4;
	my_questions->width=500;
	
	my_event = new IFInstallQuestionEvent;
	
	return true;	
}

static void if2k_win32install_WNKillApplication()
{
	delete my_event;
	delete my_questions;
}


WNWindow *if2k_win32install_WNCreateMainWindow()
{
	AskTextWindow *w = new AskTextWindow(*my_questions,*my_event);
	
	w->Create();
	w->Show( nCmdShow );
	
	return w;
}


int WINAPI if2k_win32install_WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    jdk_set_app_name( IF2K_SHORT_SERVICE_NAME " install" );

	jdk_set_home_dir( DEFAULT_APP_DIR );

  if( WNInstall_IsWin95() )
	{
		MessageBox( 0, IF2K_TITLE " does not run on Windows 95", IF2K_TITLE, MB_OK );
		return 0;
	}

  if( WNInstall_IsWin95() || WNInstall_IsWin98() || WNInstall_IsWinME() )
	{
		old_windows=true;
	}
	if( !strcmp(lpCmdLine,"--old") )
	{
		old_windows=true;
	}

#ifndef IF2K_WIN32INSTALL_FAKE	
	if( if2k_win32_check_old_versions() )
#endif		
	  return WNWinMain( 
      hInstance, 
      hPrevInstance, 
      lpCmdLine, 
      nCmdShow,
      if2k_win32install_WNInitializeApplication,
      if2k_win32install_WNCreateMainWindow,
      if2k_win32install_WNKillApplication
      );
#ifndef IF2K_WIN32INSTALL_FAKE	
	else
	  return 1;
#endif	
}



#endif









