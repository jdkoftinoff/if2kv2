#include "jdk_world.h"
#include "jdk_util.h"
#include "jdk_log.h"
#include "jdk_thread.h"
#include "jdk_settings.h"

#define JDK_LOG_HAS_THREADS 0

#if JDK_LOG_HAS_THREADS
static jdk_recursivemutex log_mutex;
#endif

int jdk_log_detail_level=4;
bool jdk_log_initted=false;
int jdk_log_dest_type=JDK_LOG_DEST_STDERR;
char jdk_log_filename[4096];

const char *jdk_log_detailname[] =
{
  "0 ERROR    ",
  "1 WARNING  ",
  "2 NOTICE   ",
  "3 INFO     ",
  "4 DEBUG    ",
  "5 DEBUG 1  ",
  "6 DEBUG 2  ",
  "7 DEBUG 3  ",
  "8 DEBUG 4  ",
  "9 DEBUG 5  ",
};


extern "C" int jdk_log_multi_string(
  int detail_level,
  const char *prefix,
  const char *buf
  )
{
  char out[4096];
  char *outp = out;
  int sz=0;
  const char *inp = buf;
  
  do
  {
    char c = *inp;
    if( c=='\0' )
    {
      *outp = '\0';
      jdk_log_string( detail_level, prefix, out );
      break;
    }
    else if( c=='\n' || c=='\r' )
    {
      *outp = '\0';
      jdk_log_string( detail_level, prefix, out );
      outp = out;
      sz = 0;
      ++inp;
    }
    else if( (int)sz+1 < (int)sizeof(out) )
    {
      if( !jdk_isprint( c ) )
        c=' ';
      *outp++ = c;
      ++inp;
      ++sz;
    }
  } while(true);
  return 0;
}

extern "C" int jdk_varlog( int detail_level, const char *fmt, va_list *l )
{
  int ret=0;
  
  if( detail_level>JDK_LOG_DEBUG5 )
  {
    detail_level=JDK_LOG_DEBUG5;
  }
  
  if( detail_level<=jdk_log_detail_level )
  {
    char prefix[4100];
    char buf[8000];
    
    
    // put date level first on line
    
    time_t timep;
    time(&timep);
    
#if !JDK_IS_NETBSD	   
    strftime( prefix, sizeof(prefix), "%a, %d %b %Y %H:%M:%S\t", localtime(&timep) );
#endif		
    // put detail level
    strcat( prefix, jdk_log_detailname[ detail_level ] );
    strcat( prefix, ":\t" );
#if !JDK_IS_WIN32		
    char pid_text[128];
    snprintf( pid_text, sizeof(pid_text), "[%d] ", getpid() );
    strcat( prefix, pid_text );
#endif
    // calculate how many chars we have left in buffer - leave
    // slop at end so we can add CR if we have to
    
    // print to
#if JDK_IS_WIN32		
    if( vsprintf( buf, fmt, *l )==-1 )
#else
      if( vsnprintf( buf, sizeof(buf), fmt, *l )==-1 )
#endif		  
      {
        strcpy( buf, "LOG BUFFER OVERFLOW" );
      }			   
    
    ret = jdk_log_multi_string( detail_level, prefix, buf );    
    
  }
  
  va_end(*l);
  return ret;
}


extern "C" int _jdk_log_debug1( 
  const char *fmt, 
  ... 
  )
{
  int ret=0;
  
  va_list l;
  va_start( l, fmt );
  if( JDK_LOG_DEBUG1 <=jdk_log_detail_level )
  {
    ret = jdk_varlog( JDK_LOG_DEBUG1, fmt, &l );
  }
  va_end(l);
  return ret;	
}

extern "C" int _jdk_log_debug2( 
  const char *fmt,
  ... 
  )
{
  int ret=0;
  
  va_list l;
  va_start( l, fmt );
  if( JDK_LOG_DEBUG2 <=jdk_log_detail_level )
  {
    ret = jdk_varlog( JDK_LOG_DEBUG2, fmt, &l );
  }
  va_end(l);
  return ret;	
}

extern "C" int _jdk_log_debug3( 
  const char *fmt, 
  ... 
  )
{
  int ret=0;
  
  va_list l;
  va_start( l, fmt );
  if( JDK_LOG_DEBUG3 <=jdk_log_detail_level )
  {
    ret = jdk_varlog( JDK_LOG_DEBUG3, fmt, &l );
  }
  va_end(l);
  return ret;	
}


extern "C" int _jdk_log_debug4( 
  const char *fmt, 
  ... 
  )
{
  int ret=0;
  
  va_list l;
  va_start( l, fmt );
  if( JDK_LOG_DEBUG4 <=jdk_log_detail_level )
  {
    ret = jdk_varlog( JDK_LOG_DEBUG4, fmt, &l );
  }
  va_end(l);
  return ret;	
}

extern "C" int _jdk_log_debug5( 
  const char *fmt, 
  ... 
  )
{
  int ret=0;
  
  va_list l;
  va_start( l, fmt );
  if( JDK_LOG_DEBUG5 <=jdk_log_detail_level )
  {
    ret = jdk_varlog( JDK_LOG_DEBUG5, fmt, &l );
  }
  va_end(l);
  return ret;	
}



extern "C" int jdk_log(
  int detail_level,
  const char *fmt,
  ...
  )
{
  int ret=0;
  
  if( detail_level <=jdk_log_detail_level )
  {
    va_list l;
    va_start( l, fmt );
    
    ret = jdk_varlog( detail_level, fmt, &l );
    va_end(l);
  }
  return ret;
}


#if JDK_IS_UNIX
#include <syslog.h>
#include <sys/stat.h>

static int jdk_log_syslogmap[] = 
{
  LOG_ERR,
  LOG_WARNING,
  LOG_NOTICE,
  LOG_INFO,
  LOG_DEBUG,
  LOG_DEBUG,
  LOG_DEBUG,
  LOG_DEBUG,
  LOG_DEBUG,
  LOG_DEBUG
};


int jdk_log_fd=-1;

int jdk_log_synccountdown=5;


int jdk_log_setup( 
  int dest_type,
  const char *dest_,
  int max_detail_level
  )
{
#if JDK_LOG_HAS_THREADS
  jdk_synchronized( log_mutex );
#endif
  
  umask(0077);
  jdk_log_dest_type = dest_type;
  jdk_log_detail_level = max_detail_level;

  if( jdk_log_dest_type== JDK_LOG_DEST_FILE )
  {
    if( jdk_log_fd!=-1 )
    {
      close(jdk_log_fd);
    }
    jdk_process_path( dest_, jdk_log_filename, sizeof(jdk_log_filename) );
    
#if defined( O_NOFOLLOW )
    jdk_log_fd = open( jdk_log_filename, O_CREAT | O_APPEND | O_WRONLY | O_NOFOLLOW, 0640 );
#else
    jdk_log_fd = open( jdk_log_filename, O_CREAT | O_APPEND | O_WRONLY, 0640 );
#endif
    if( jdk_log_fd==-1 )
    {
      jdk_log_dest_type = 0;
      exit(1);
      return -1;
    }
  }
  else
    if( jdk_log_dest_type== JDK_LOG_DEST_SYSLOG )
    {
      openlog( (char *)dest_, LOG_PID, LOG_DAEMON );
    }
  jdk_log_initted=true;
  return 0;
}


#if JDK_IS_MACOSX && JDK_LOG_USE_NSLOG
extern "C" void jdk_log_NSLog(const char *prefix, const char *buf);
#endif

extern "C" int jdk_log_string(
  int detail_level,
  const char *prefix,
  const char *buf
  )
{
#if JDK_LOG_HAS_THREADS
  jdk_synchronized( log_mutex );
#endif
  
  int ret=0;
  
#if 0
  
  int len = strlen( buf );
  for( int i=0; i<len; ++i )
  {
    if( buf[i]=='\r' || buf[i]=='\n' )
      buf[i]=' ';
    else if( !jdk_isprint(buf[i]) && buf[i]!='\t' )
      buf[i]='?';
  }
#endif
  if( detail_level<=jdk_log_detail_level )
  {
    
    switch( jdk_log_dest_type )
    {
#if JDK_IS_MACOSX && JDK_LOG_USE_NSLOG
    case JDK_LOG_DEST_NSLOG:
    {
      jdk_log_NSLog(prefix,buf);
      break;
    }
#endif
    case JDK_LOG_DEST_NULL:
    {
      break;
    }
    
    case JDK_LOG_DEST_SYSLOG:
    {
      syslog(
        jdk_log_syslogmap[ detail_level ] + LOG_DAEMON,
        "%s: %s",
        prefix,
        buf
        );
      break;
    }
    
    case JDK_LOG_DEST_STDERR:
    {
      fprintf( stderr, "%s: %s\n", prefix, buf );
      break;
    }
    
    case JDK_LOG_DEST_FILE:
    {
      if( jdk_log_fd==-1 )
      {
#if defined( O_NOFOLLOW )
        jdk_log_fd = open( jdk_log_filename, O_CREAT | O_APPEND | O_WRONLY | O_NOFOLLOW, 0640 );
#else
        jdk_log_fd = open( jdk_log_filename, O_CREAT | O_APPEND | O_WRONLY, 0640 );
#endif
        if( jdk_log_fd==-1 )
        {
          jdk_log_dest_type = 0;
          exit(1);
          return -1;
        }
        
      }
      // write to file descriptor

      jdk_str<8192> full_line(prefix);
      full_line.cat( buf );
      full_line.cat( "\n" );
      
      write( jdk_log_fd, full_line.c_str(), full_line.len() );
      fsync( jdk_log_fd );
      
    }
    }
  }
  
  return ret;
}


#else

FILE *jdk_log_file = 0;



int jdk_log_setup(
  int dest_type,
  const char *dest_,
  int max_detail_level
  )
{
#if JDK_LOG_HAS_THREADS
  jdk_synchronized( log_mutex );
#endif
  
  jdk_log_dest_type = dest_type;
  jdk_log_detail_level = max_detail_level;
  if( jdk_log_dest_type== JDK_LOG_DEST_FILE )
  {
    
    if( jdk_log_file!=0 )
    {
      fclose( jdk_log_file );
    }
    jdk_process_path( dest_, jdk_log_filename, sizeof(jdk_log_filename) );
    
    jdk_log_file = fopen( jdk_log_filename, "at" );
    if( !jdk_log_file )
    {
      jdk_log_dest_type = 0;
      exit(1);
      return -1;
    }
  }
  else
  {
    return -1;
  }
  
  jdk_log_initted=true;
  return 0;		
}


extern "C" int jdk_log_string(
  int detail_level,
  const char *prefix,
  const char *buf
  )
{
#if JDK_LOG_HAS_THREADS
  jdk_synchronized( log_mutex );
#endif
  
  int ret=0;
  
  size_t len = strlen( buf );
#if 0
  for( int i=0; i<len; ++i )
  {
    if( buf[i]=='\r' || buf[i]=='\n' )
      buf[i]=' ';
    else if( !jdk_isprint(buf[i]) && buf[i]!='\t' )
      buf[i]='?';
  }
#endif
  if( detail_level<=jdk_log_detail_level )
  {
    switch( jdk_log_dest_type )
    {
    case JDK_LOG_DEST_NULL:
    {
      break;
    }
    
    case JDK_LOG_DEST_STDERR:
    {
      fprintf( stderr, "%s : %s\n", prefix, buf );
      break;
    }
    
    case JDK_LOG_DEST_FILE:
    {
      fprintf( jdk_log_file, "%s : %s\n", prefix, buf );
      fflush( jdk_log_file );
    }
    
    
    }
    
  }
  
  return ret;
  
}

#endif


int jdk_log_setup( const jdk_settings &settings )
{
  return jdk_log_setup( 
    settings.get_long( "log.type" ),
    settings.get( "log.file" ),
    settings.get_long( "log.detail" )
    );
}				 




