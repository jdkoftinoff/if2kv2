#ifndef _IF2K_WIN32UNINSTALL_H
#define _IF2K_WIN32UNINSTALL_H

bool if2k_win32uninstall_do_silent_uninstall( 
      const char *password, 
	    bool &reboot_required, 
	    bool &password_incorrect,
	    bool &no_redir
	    );
bool if2k_win32uninstall_ControlService(DWORD fdwControl);
bool if2k_win32uninstall_DeleteService();
bool if2k_win32uninstall_uninstall( const WNDlgAskTextQuestionList &info );
bool if2k_win32uninstall_silent_uninstall( 
        const char *password, 
	    bool &reboot_required, 
	    bool &password_incorrect,
	    bool &no_redir
	    );



int WINAPI if2k_win32uninstall_WinMain(
                                       HINSTANCE hInstance, 
                                       HINSTANCE hPrevInstance, 
                                       LPSTR lpCmdLine, 
                                       int nCmdShow
                                       );

#endif
