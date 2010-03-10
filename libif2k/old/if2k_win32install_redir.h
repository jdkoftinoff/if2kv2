#ifndef _IF2K_WIN32INSTALL_REDIR_H
#define _IF2K_WIN32INSTALL_REDIR_H

extern "C" void register_if2k_win32install_redir_packages(void);

bool if2k_win32install_redir_check_old_versions();
bool if2k_win32install_redir( const jdk_settings &settings, bool old_windows, bool do_login_on_reboot );
int if2k_win32install_redir_main( int argc, char **argv );

#endif
