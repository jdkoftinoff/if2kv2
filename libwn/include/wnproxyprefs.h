#ifndef __WNPROXYPREFS_H
#define __WNPROXYPREFS_H

#if JDK_IS_WIN32

#include "wnregistry.h"


int WNInstall_UnsetIEProxy(  char *connection_name );

int WNInstall_SetIEProxy(  char *connection_name,  char *proxy );

int WNInstall_UnsetInetProxy();

int WNInstall_SetInetProxy( char *proxy );


#endif

#endif
