#ifndef _IF2K_CONFIG_H
#define _IF2K_CONFIG_H

#define IF2K_COPYRIGHT "Copyright 2004, Turner and Sons, Inc."

#define IF2K_MAJOR_VERSION "V2.0"
#define IF2K_MINOR_VERSION "alpha"

#define IF2K_VERSION_STRING IF2K_MAJOR_VERSION IF2K_MINOR_VERSION

#define IF2K_TITLE "Internet Filter"
#define IF2K_TITLE_VERSION IF2K_TITLE " " IF2K_VERSION_STRING
#define IF2K_TITLE_MAJOR_VERSION IF2K_TITLE " " IF2K_MAJOR_VERSION

#define IF2K_ENABLE_LUA (0) 
#define IF2K_ENABLE_PYTHON (0)
#define IF2K_USE_ADMIN_NAME 0

#define IF2K_REMOTE_DB "http://www.internetfilter.com/db/"IF2K_VERSION_STRING"/"

#ifndef IF2K_SERIAL_URL1
# define IF2K_SERIAL_URL1 "http://www.internetfilter.com/license/" IF2K_MAJOR_VERSION "/"
# define IF2K_SERIAL_URL2 "http://www1.internetfilter.com/license/" IF2K_MAJOR_VERSION "/"
#endif

#define IF2K_SHORT_SERVICE_NAME "if2kd"

#if JDK_IS_WIN32
# define IF2K_REDIR_DLL_NAME "ifredir.dll"
# define IF2K_REGISTRY_LOCATION "SOFTWARE\\Turner and Sons\\" IF2K_TITLE
# define IF2K_REGISTRY_FLAGS_LOCATION "SOFTWARE\\Turner and Sons\\" IF2K_TITLE
# define IF2K_REDIR_REGISTRY_LOCATION IF2K_REGISTRY_LOCATION
# define IF2K_UNINSTALL_REGISTRY "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" IF2K_TITLE_VERSION
# define IF2K_SERVICE_NAME IF2K_TITLE_VERSION
#endif

#if JDK_IS_WIN32
# define PLATFORM_SETTINGS jdk_settings_win32registry
# define PLATFORM_SAVE_SETTINGS() save_registry( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_LOCATION )
# define PLATFORM_LOAD_SETTINGS() load_registry( HKEY_LOCAL_MACHINE, IF2K_REGISTRY_LOCATION )
#elif JDK_IS_MAC_OSX
# define PLATFORM_SETTINGS jdk_settings_macosx_plist
# define PLATFORM_SAVE_SETTINGS() save_file( global_settings_filename )
# define PLATFORM_LOAD_SETTINGS() load_file( global_settings_filename )
#else
# define PLATFORM_SETTINGS jdk_settings_text
# define PLATFORM_SAVE_SETTINGS() save_file( global_settings_filename )
# define PLATFORM_LOAD_SETTINGS() load_file( global_settings_filename )
#endif


#if JDK_IS_MACOSX
# define DEFAULT_APP_DIR "/Library/Application Support/www.internetfilter.com/if2k/" IF2K_MAJOR_VERSION "/" IF2K_SHORT_SERVICE_NAME
# define DEFAULT_SETTINGS_FILE "$/settings.plist"
#else
# define DEFAULT_APP_DIR "/etc/if2k"
# define DEFAULT_SETTINGS_FILE "$/settings.txt"
#endif


#ifndef ENCRYPTION_KEY
#define ENCRYPTION_KEY (0xab121324)
#endif

#define IF2K_DECRYPTOR jdk_decryptor_complex
#define IF2K_ENCRYPTOR jdk_encryptor_complex

#if JDK_IS_UNIX
//#define IF2K_SVC_FORKED
#endif

#endif






