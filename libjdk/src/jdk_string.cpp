#include "jdk_world.h"
#include "jdk_string.h"

int jdk_string::extract_token( 
  size_t pos, 
  char *dest, 
  size_t destmaxlen, 
  const char *delim,
  const char *ignores
  ) const 
{
  // skip any preceeding delims
  
  pos += spn( pos, delim );
  
  // skip any ignore characters
  
  if( ignores )
    pos += spn( pos, ignores );
  
  // find length of valid token
  
  size_t clen = cspn( pos, delim );
  
  // copy that section only
  
  if( clen+1>destmaxlen )
  {
    return -1;
  }
  
  jdk_strncpy( dest, &s[pos], clen+1 );
  
  return int(pos+clen);	
} 


int jdk_string::extract_token( 
  size_t pos, 
  jdk_string *dest, 
  const char *delim,
  const char *ignores 
  ) const 
{
  // skip any preceeding delims
  
  pos += spn( pos, delim );
  
  // skip any ignore characters
  
  if( ignores )
    pos += spn( pos, ignores );	
  
  // find length of valid token
  
  size_t clen = cspn( pos, delim );
  
  // copy that section only
  
  if( clen+1>dest->getmaxlen() )
  {
    return (int)-1;
  }
  
  
  dest->ncpy( &s[pos], clen+1 );
  
  return int(pos+clen);		
} 

bool jdk_string::form(
  const char *fmt,
  ...
  ) 
{
  bool ret=true;
  
  resize(4096);
  va_list l;
  va_start( l, fmt );
  
#if JDK_IS_UNIX || JDK_IS_BEOS
  if( vsnprintf( s, maxlen, fmt, l )==-1 )
  {
    ret=false;	
  }
#elif JDK_IS_WIN32
  
  if( jdk_vsprintf( s, fmt, l )==-1 )
  {
    ret=false;	
  }
  
#else
#error need vsnprintf or workalike
#endif
  va_end(l);
  
  return ret;
}



