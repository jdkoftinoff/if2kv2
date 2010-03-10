#ifndef _JDK_LOG_H
#define _JDK_LOG_H

#include "jdk_string.h"


#define JDK_LOG_ERROR (0)
#define JDK_LOG_WARNING (1)
#define JDK_LOG_NOTICE (2)
#define JDK_LOG_INFO (3)
#define JDK_LOG_DEBUG (4)
#define JDK_LOG_DEBUG0 (4)
#define JDK_LOG_DEBUG1 (5)
#define JDK_LOG_DEBUG2 (6)
#define JDK_LOG_DEBUG3 (7)
#define JDK_LOG_DEBUG4 (8)
#define JDK_LOG_DEBUG5 (9)

#define JDK_LOG_DEST_NULL (0)
#define JDK_LOG_DEST_SYSLOG (1)
#define JDK_LOG_DEST_STDERR (2)
#define JDK_LOG_DEST_FILE (3)
#if JDK_IS_MACOSX && JDK_LOG_USE_NSLOG
#define JDK_LOG_DEST_NSLOG (4)
#endif

extern int jdk_log_detail_level;
#if JDK_IS_UNIX
extern int jdk_log_fd;
#else
extern FILE *jdk_log_file;
#endif


int jdk_log_setup( 
				  int dest_type,
				  const char *dest,
				  int max_detail_level
				  );

inline int jdk_log_setup(
				  int dest_type,
				  const jdk_string &dest,
				  int max_detail_level
				  )
{
    return jdk_log_setup( dest_type, dest.c_str(), max_detail_level );
}

class jdk_settings;

int jdk_log_setup( const jdk_settings &settings );

extern "C" int jdk_varlog( 
  int detail_level, 
  const char *fmt, 
  va_list *l 
  );


extern "C"  int jdk_log_multi_string(
                    int detail_level,
					const char *prefix,
		            const char *str
			      );
				  
extern "C"  int jdk_log_string(
                    int detail_level,
					const char *prefix,
		            const char *str
			      );

extern "C" int jdk_log(
			int detail_level,
			const char *fmt, 
			... 
		    );


extern "C" int _jdk_log_debug1( 
			const char *fmt, 
			... 
		    );

extern "C"  int _jdk_log_debug2( 
			const char *fmt, 
			... 
		    );

extern "C" int _jdk_log_debug3( 
			const char *fmt, 
			... 
		    );

extern "C" int _jdk_log_debug4( 
			const char *fmt, 
			... 
		    );


extern "C" int _jdk_log_debug5( 
			const char *fmt, 
			... 
		    );


struct jdk_debug_block_
{
	jdk_debug_block_( const char *msg_, int level_=JDK_LOG_DEBUG4 )
	:
	level(level_),
	msg(msg_)
	{
		jdk_log( level, "+ %s", msg );
	}

	~jdk_debug_block_()
	{
		jdk_log( level, "- %s", msg );
	}

	int level;
	const char *msg;
private:
        jdk_debug_block_( const jdk_debug_block_ & );
    const jdk_debug_block_ & operator = ( const jdk_debug_block_ & );
};


#ifndef JDK_LOG_NO_DEBUG
#define JDK_LOG_NO_DEBUG 0
#endif

#if JDK_LOG_NO_DEBUG
#define jdk_debug_block(msg)
inline void jdk_log_debug1(const char *fmt,...) {}
inline void jdk_log_debug2(const char *fmt,...) {}
inline void jdk_log_debug3(const char *fmt,...) {}
inline void jdk_log_debug4(const char *fmt,...) {}
inline void jdk_log_debug5(const char *fmt,...) {}
#else
#define jdk_debug_block(msg) jdk_debug_block_ _logblock_(msg)
#define jdk_log_debug1 _jdk_log_debug1
#define jdk_log_debug2 _jdk_log_debug2
#define jdk_log_debug3 _jdk_log_debug3
#define jdk_log_debug4 _jdk_log_debug4
#define jdk_log_debug5 _jdk_log_debug5
#endif

#endif
