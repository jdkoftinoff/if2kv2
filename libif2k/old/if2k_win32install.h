#ifndef _IF2K_WIN32_INSTALL_H
#define _IF2K_WIN32_INSTALL_H

int WINAPI if2k_win32install_WinMain(
                                     HINSTANCE hInstance, 
                                     HINSTANCE hPrevInstance, 
                                     LPSTR lpCmdLine, 
                                     int nCmdShow
                                     );

bool if2k_win32_check_old_versions();

#endif
