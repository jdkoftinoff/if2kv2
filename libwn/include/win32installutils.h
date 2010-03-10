#ifndef __WIN32INSTALLUTILS_H
#define __WIN32INSTALLUTILS_H


#include <Registry.hpp>

#include "../libif98/if_config.h"

void Install_Registry(bool erase=false, const char *key=DEFAULT_REGISTRY_LOCATION);
void Install_RegistryEnd();

extern TRegistry *settings;


struct filecopylist
{
    const char *src;
    const char *dest;
};


bool Install_InstallFiles( filecopylist * );
bool Install_DeleteFiles( filecopylist * );


bool Install_AddToRun( const char *key, const char *program );
bool Install_AddToRunOnce( const char *key, const char *program );
bool Install_RemoveFromRun( const char *key );
bool Install_RemoveFromRunOnce( const char *key );
bool Install_CalcPath( const char *orig, char *dest, int destlen );

HWND Install_CreateInvisibleWindow(
                                    HINSTANCE instance,
                                    const char *clsname,
                                    const char *title,
                                    WNDPROC proc
                                    );
                                    
LRESULT Install_SendMsgToWindow(
                            const char *clsname,
                            const char *title,
                            UINT msg,
                            WPARAM wparam,
                            LPARAM lparam
                            );


#endif
