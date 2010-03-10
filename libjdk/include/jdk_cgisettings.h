#ifndef _JDK_CGISETTINGS_H
#define _JDK_CGISETTINGS_H

#include "jdk_settings.h"
#include "jdk_cgi.h"


bool jdk_cgi_loadsettings( jdk_settings *settings, const char *cgidata );

bool jdk_cgi_loadsettings( jdk_settings *settings, const jdk_dynbuf &cgidata );


bool jdk_cgi_savesettings( const jdk_settings &settings, char *cgidata, size_t maxlen );

bool jdk_cgi_savesettings( const jdk_settings &settings, jdk_dynbuf *cgidata );

#endif
