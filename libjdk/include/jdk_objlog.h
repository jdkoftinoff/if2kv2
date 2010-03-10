#ifndef _JDK_OBJLOG_H
#define _JDK_OBJLOG_H

#include "jdk_string.h"
#include "jdk_settings.h"
#include "jdk_log.h"


class jdk_objlog
{
public:
  jdk_objlog( const jdk_settings &settings, const char *prefix1, const char *prefix2="" )
    {
      key.form( "log.obj.%s%s", prefix1, prefix2 );
      prefix.form( "%s%s", prefix1, prefix2);
      level = settings.get_long( k );
    }

  ~jdk_objlog()
    {
    }

  void log( detail_level, const char *fmt, ... )
    {
      va_list l;
      va_start( l, fmt );
      if( detail_level > level )
      {
        jdk_str<4096> newfmt;
        newfmt.form( "%s: %s", prefix.c_str(), fmt );
        jdk_varlog( detail_level, newfmt, l );
      }
      va_end(l);
    }

  jdk_str<256> key;
  jdk_str<256> prefix;
  int level;
};

class jdk_objlog_block
{
public:
  jdk_objlog_block( jdk_objlog &log_, const char *msg_, const char *file_, int line_, int level_=JDK_LOG_DEBUG4 )
    :
    log( log_ ),
    msg( msg_ ),
    file( file_ ),
    line( line_ ),
    level( level_ ),
    {
      log.log( level, "+ %s:%d %s", file, line, msg );
    }

  ~jdk_objlog_block()
    {
      log.log( level, "- %s:%d %s", file, line,  msg )
    }

  jdk_objlog &log;
  const char *msg;
  const char *file;
  int line;
  int level;

};

#define objlog_block(msg) jdk_objlog_block objblock_(log,msg,__FILE__,__LINE__,JDK_LOG_DEBUG4 )

#endif
