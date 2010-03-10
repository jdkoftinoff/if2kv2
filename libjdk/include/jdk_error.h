#ifndef _JDK_ERROR_H
#define _JDK_ERROR_H

#undef JDK_HAS_EXCEPTIONS // this is horribly broken for real exceptions
#ifndef JDK_HAS_EXCEPTIONS
#define JDK_HAS_EXCEPTIONS 0
#endif

extern "C" int jdk_log(
			int detail_level,
			const char *fmt, 
			... 
		    );

#if JDK_HAS_EXCEPTIONS
# define JDK_THROW_ERROR(a,b) throw jdk_error( a, b, __FILE__, __LINE__ )
#else
# define JDK_THROW_ERROR(a,b) do { jdk_log( 0, "Exception: '%s %s' in line %d of '%s'", a, b, __LINE__, __FILE__ ); abort(); } while(0)
#endif

#if 0 
#include "jdk_string.h"
struct jdk_error
{
    jdk_error( const char *name_, const char *info_, const char *file_, int line_ )
        : 
	    name( name_),
	    info( info_),
	    file( file_),
	    line( line_)
	{
	}
	
    jdk_str<4096> name;
    jdk_str<4096> info;
    jdk_str<4096> file;
    int line;

    virtual void Dump() const
    {
        jdk_str<4096> s;
        DumpText( s );
        jdk_log( JDK_LOG_ERROR, "%s", s.c_str() );
    }

    virtual void DumpText( jdk_string &s ) const
    {
        s.form( "Exception: '%s %s' in line %d of '%s'", name.c_str(), info.c_str(), line, file.c_str() );
    }

};
#endif

#endif

