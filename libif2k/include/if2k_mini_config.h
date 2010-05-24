#ifndef __IF2K_MINI_CONFIG_H
#define __IF2K_MINI_CONFIG_H

#include "jdk_string.h"
#include "jdk_buf.h"
#include "jdk_sha1.h"

#define IF2K_MINI_SERVER_PRODUCT_ID "The Internet Filter IF-2K"

#ifdef IF2K_MINI_VERSION
# undef IF2K_MINI_VERSION
#endif

#define IF2K_MINI_VERSION "2.4.1"


#define IF2K_MINI_NNTP 1

 
#define IF2K_MINI_SERVER_SERVICE_NAME "if2kd"
#define IF2K_MINI_PRODUCT_NAME "Internet Filter - if2k"
#define IF2K_MINI_PRODUCT_FULL_NAME IF2K_MINI_PRODUCT_NAME " Version " IF2K_MINI_VERSION
#define IF2K_MINI_CRYPT_KEY (0xab121324)
#define IF2K_MINI_SHA1_SALT "141es1e11e"

#if defined(WIN32)
# define IF2K_MINI_REDIR_DLL_NAME "if2k_redir.dll"
# define IF2K_MINI_REGISTRY_LOCATION "SOFTWARE\\Turner and Sons\\if2k\\"
# define IF2K_MINI_LICENSE_REGISTRY_LOCATION IF2K_MINI_REGISTRY_LOCATION "license\\"
# define IF2K_MINI_ADDITIONAL_REGISTRY_LOCATION IF2K_MINI_REGISTRY_LOCATION "additional\\"
# define IF2K_MINI_INSTALL_REGISTRY_LOCATION IF2K_MINI_REGISTRY_LOCATION "install\\"
# define IF2K_MINI_REDIR_REGISTRY_LOCATION IF2K_MINI_REGISTRY_LOCATION "if2k_redir\\"
# define IF2K_MINI_REDIR_HOOK_REGISTRY_LOCATION "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows"
# define IF2K_MINI_REDIR_HOOK_KEY "AppInit_DLLs"
#endif

#if defined(JDK_IS_MACOSX)
# define IF2K_MINI_STARTUP_PATH "/Library/If2k/"
# define IF2K_MINI_UNINSTALL_PATH IF2K_MINI_STARTUP_PATH "bin/if2kuninstall"
#endif

inline void if2k_hash_for_dbfile( jdk_string &r, const jdk_buf &b, const jdk_string &base_filename )
{
  SHA1 h;

  unsigned raw[5];

  h.Reset();
  h << IF2K_MINI_SHA1_SALT;
  h << base_filename.c_str();
  h.Input( b.get_data(), b.get_data_length() );
  
  h.Result( raw );
  
  r.form( "%08x%08x%08x%08x%08x",
          raw[0],
          raw[1],
          raw[2],
          raw[3],
          raw[4]
    );
}


inline void if2k_hash_for_password( jdk_string &r, const jdk_string &v )
{
  SHA1 h;

  unsigned raw[5];

  h.Reset();
  h << IF2K_MINI_SHA1_SALT;
  h << v.c_str();
  
  h.Result( raw );
  
  r.form( "%08x%08x%08x%08x%08x",
          raw[0],
          raw[1],
          raw[2],
          raw[3],
          raw[4]
    );
}

#endif
