#ifndef __IF2002_WIN32_INSTALL_GUTS_H
#define __IF2002_WIN32_INSTALL_GUTS_H

#include "jdk_world.h"
#include "jdk_settings.h"
#include "jdk_string.h"
#include "jdk_bindir.h"
#include "if2k_config.h"

bool if2k_win32_check_old_versions();
bool if2k_win32install( 
                       const jdk_settings &settings, 
                       bool old_windows, 
                       bool do_login_on_reboot, 
                       jdk_str<4096> &error_reason 
                       );


#endif


