#if defined( _DEBUG )
# if !defined( JDK_IS_DEBUG )
#  define JDK_IS_DEBUG 1
# endif
#endif

#if JDK_IS_LINUX && JDK_HAS_THREADS

# define JDK_IS_WIN32 0
# define JDK_IS_BEOS 0
# ifndef JDK_IS_UNIX
#  define JDK_IS_UNIX 1
# endif
# define JDK_IS_CYGWIN 0
# ifndef JDK_IS_WINE
#  define JDK_IS_WINE 0
# endif
# define JDK_IS_MACOSX 0

# include "jdk_platform_linuxthreads.h"

# define JDK_IS_WIN32 0
# define JDK_IS_BEOS 0
# define JDK_IS_WINE 0
# ifndef JDK_IS_UNIX
#  define JDK_IS_UNIX 1
# endif
# define JDK_IS_CYGWIN 0
# define JDK_IS_MACOSX 0

#elif JDK_IS_LINUX && !JDK_HAS_THREADS

# define JDK_IS_WIN32 0
# define JDK_IS_BEOS 0
# ifndef JDK_IS_UNIX
#  define JDK_IS_UNIX 1
# endif
# define JDK_IS_CYGWIN 0
# define JDK_IS_MACOSX 0
# include "jdk_platform_linux.h"

#elif JDK_IS_MACOSX && JDK_HAS_THREADS

# define JDK_IS_WIN32 0
# define JDK_IS_BEOS 0
# ifndef JDK_IS_UNIX
#  define JDK_IS_UNIX 1
# endif
# define JDK_IS_CYGWIN 0
# define JDK_IS_MACOSX 1
# include "jdk_platform_macosx.h"

#elif JDK_IS_MACOSX && !JDK_HAS_THREADS

# define JDK_IS_WIN32 0
# define JDK_IS_BEOS 0
# define JDK_IS_WINE 0
# ifndef JDK_IS_UNIX
#  define JDK_IS_UNIX 1
# endif
# define JDK_IS_CYGWIN 0
# define JDK_IS_MACOSX 1
# include "jdk_platform_macosx.h"


#elif JDK_IS_UNIX && !JDK_IS_LINUX && !JDK_IS_CYGWIN

# define JDK_IS_WIN32 0
# define JDK_IS_BEOS 0
# define JDK_IS_LINUX 0
# define JDK_IS_CYGWIN 0
# define JDK_IS_MACOSX 0
# include "jdk_platform_unix.h"

#elif JDK_IS_WIN32

# define JDK_IS_BEOS 0
# define JDK_IS_LINUX 0
# define JDK_IS_UNIX 0
# define JDK_IS_CYGWIN 0
# define JDK_IS_MACOSX 0
# ifndef JDK_IS_WINE
#  define JDK_IS_WINE 0
#endif
# include "jdk_platform_win32.h"

#elif JDK_IS_BEOS

# define JDK_IS_WIN32 0
# define JDK_IS_LINUX 0
# define JDK_IS_UNIX 0
# define JDK_IS_CYGWIN 0
# define JDK_IS_MACOSX 0
# include "jdk_platform_beos.h"


#elif JDK_IS_CYGWIN

# define JDK_IS_WIN32 0
# define JDK_IS_LINUX 0
# define JDK_IS_UNIX 1
# define JDK_IS_CYGWIN 1
# define JDK_IS_WINE 0
# define JDK_IS_MACOSX 0
# include "jdk_platform_cygwin.h"

#else
#error platform not supported or platform definition incorrect
#endif

