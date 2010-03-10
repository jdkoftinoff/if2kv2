#include "jdk_world.h"
#include "jdk_util.h"
#include "jdk_log.h"

#include "jdk_cgi.h"

char jdk_cgi_x2c( const char *in ) 
{
  char digit;
  
  digit = (in[0] >= 'A' ? ((in[0] & 0xdf) - 'A')+10 : (in[0] - '0'));
  digit *= 16;
  
  digit += (in[1] >= 'A' ? ((in[1] & 0xdf) - 'A')+10 : (in[1] - '0'));
  
  return digit;   
} 

char * jdk_cgi_c2x( char in, char *out ) 
{
  static char *hexchars="0123456789ABCDEF";
  
  *out++ = '%';
  *out++ = hexchars[ (in>>4)&0xf ];
  *out++ = hexchars[ (in>>0)&0xf ];
  
  return out;
} 


bool jdk_cgi_unescape( const char *in, char *out, size_t maxlen ) 
{
  bool r=true;
  size_t len=0;
  
  while( *in )
  {
    
    if( len>=maxlen-1 )
    {
      r=false;
      break;
    }
    
    // if in is % then convert it
    
    if( *in == '%')
    {
      if( !in[1] || !in[2] )
      {
        // string ended before full char!
        r=false;
        break;
      }
      
      *out++ = jdk_cgi_x2c( &in[1] );
      
      // skip over the input
      in+=3;
      len++;
    }
    else
    {
      // it wasnt % so just copy it over
      
      *out++ = *in++;
      len++;
    }
    
  }	
  
  *out='\0';
  return r;
} 


bool jdk_cgi_escape( const char *in, char *out, size_t maxlen ) 
{
  size_t len=0;
  bool r=true;
  
  while( *in )
  {
    if( len>=maxlen-1 )
    {
      r=false;
      break;
    }
    
    if( jdk_cgi_isescapablechar( *in ) )
    {
      // make sure we have room for the 3 chars
      if( len>=maxlen-4 )
      {
        r=false;
        break;
      }
      
      out=jdk_cgi_c2x( *in++, out );
      len+=3;
    }
    else
    {
      // copy the unescaped over.
      *out++=*in++;
      len++;
    }
    
  }
  
  *out='\0';
  
  return r;
} 

bool jdk_cgi_escape_with_amp( const char *in, char *out, size_t maxlen )
{
  size_t len=0;
  bool r=true;
  
  while( *in )
  {
    if( len>=maxlen-1 )
    {
      r=false;
      break;
    }
    
    if( jdk_cgi_isescapablechar_with_amp( *in ) )
    {
      // make sure we have room for the 3 chars
      if( len>=maxlen-4 )
      {
        r=false;
        break;
      }
      
      out=jdk_cgi_c2x( *in++, out );
      len+=3;
    }
    else
    {
      // copy the unescaped over.
      *out++=*in++;
      len++;
    }
    
  }
  
  *out='\0';
  
  return r;
}


const char * jdk_cgi_extract_pair(
  const char *in,  size_t in_len,
  char *out_name, size_t out_name_maxlen,
  char *out_value, size_t out_value_maxlen
  ) 
{
  char *p;
  size_t len;
  size_t maxlen;
  const char *orig_in = in;	
  
  
  // first ignore whitespace
  
  
  in+= strspn( in, " \t\r\n" );
  
  // return 0 if there is nothing left
  if( !*in )
  {
//		jdk_log_debug3( "nothing left" );
    return 0;
  }
  
  // we are now at the field name. copy it over until we hit a '='
  
  len=0;
  p=out_name;
  maxlen =out_name_maxlen;
  while( *in && in_len>0 )
  {
    char c=*in;
//		jdk_log_debug3( "cgiextract: %c, in_len=%d ", c, in_len ); 		
    ++in;		
    --in_len;		
    // skip whitespace
    
    if( c==' ' || c=='\t' || c=='\n' || c=='\r' )
    {			
      continue;
    }
    
    
    if( len>=maxlen-1 )
    {
//			jdk_log_debug3( "nothing left 1" );
      return 0;
    }
    
    
    if( c=='=' )
    {
      break;	
    }
    
    // convert '+' to space
    
    if( c=='+' )
      c=' ';
    
    // convert escaped character
    
    if( c=='%' )
    {
      // only if in doesnt end before that
      if( !in[0] || !in[1] || in_len<2 )
      {
//				jdk_log_debug3( "nothing left 2" );
        return 0;
      }
      
      c=jdk_cgi_x2c( in );
      
      // skip over the two chars on input
      in+=2;
      in_len-=2;
    }
    
    *p++ = c;
    
    ++len;
  }	
  *p='\0';
  
  // we are now at the field value. Copy it over until we 
  // hit '&' or end
  
  len=0;
  p=out_value;
  maxlen =out_value_maxlen;
  while( *in && in_len>0 )
  {
    char c=*in;
    
    ++in;		
    --in_len;		
    
    // skip whitespace
    
    if( c==' ' || c=='\t' || c=='\n' || c=='\r' )
    {
      continue;
    }
    
    if( len>=maxlen-1 )
    {
//			jdk_log_debug3( "nothing left 3" );
      return 0;
    }
    
    
    if( c=='&' )
    {
      break;	
    }
    
    
    
    // convert '+' to space
    
    if( c=='+' )
      c=' ';
    
    // convert escaped character
    
    if( c=='%' )
    {
      // only if in doesnt end before that
      if( !in[0] || !in[1] || in_len<2)
      {
//				jdk_log_debug3( "nothing left 4" );
        return 0;
      }
      
      c=jdk_cgi_x2c( in );
      
      // skip over the two chars on input
      in+=2;
      in_len-=2;
    }
    
    *p++ = c;
    len++;
  }	
  *p='\0';
  
  if( in==orig_in )
  {
//		jdk_log_debug3( "nothing parsed" );
    // nothing parsed. 
    return 0;
  }
  
  
  // return 'in' so caller knows where next field is
  
  return in;
} 


char * jdk_cgi_encode_pair(
  const char *name,
  const char *value,
  char *out, size_t out_maxlen 
  ) 
{
  const char *p;
  size_t len=0;
  
  // copy name over, escaping and encoding properly
  
  p=name;
  
  while( *p )
  {
    if( len>=out_maxlen-5 )
    {
      return 0;
    }
    
    char c = *p++;
    
    // convert spaces to +
    if( c==' ' )
    {
      c='+';
      *out++ = c;
    }
    else if( jdk_cgi_isescapablechar_with_amp_for_var(c) )
    {
      // otherwise escape it if it needs escaping
      out=jdk_cgi_c2x( c, out );
    }
    else
    {
      // character doesnt need escaping, just copy it over
      *out++ = c;
    }
    
  }
  
  // put equal sign
  *out++='=';
  
  // copy value over, escaping and encoding properly
  
  p=value;
  
  while( *p )
  {
    if( len>=out_maxlen-5 )
    {
      return 0;
    }
    
    char c = *p++;
    
    // convert spaces to +
    if( c==' ' )
    {
      c='+';
      *out++ = c;
    }
    else if( jdk_cgi_isescapablechar_with_amp_for_var(c) )
    {
      // otherwise escape it if it needs escaping
      out=jdk_cgi_c2x( c, out );
    }
    else
    {
      // character doesnt need escaping, just copy it over
      *out++ = c;
    }
    
  }
  
  // end with null. 
  
  *out='\0';
  
  return out;	
} 



char * jdk_html_amp_escape_text( 
  const char *srcbuf, 
  char *destbuf, 
  size_t destbuf_len
  ) 
{
  char *orig_destbuf = destbuf;
  size_t len = 0;
  
  while( len<destbuf_len-1 && *srcbuf )
  {
    switch( *srcbuf )
    {
    case '&':
      strcpy( destbuf, "&amp;" );
      len+=5;
      destbuf+=5;
      break;
    case '<':
      strcpy( destbuf, "&lt;" );
      len+=4;
      destbuf+=4;
      break;
    case '>':
      strcpy( destbuf, "&gt;" );
      len+=4;
      destbuf+=4;
      break;			
    default:
      *destbuf++ = *srcbuf;
      ++len;
      break;
    }
    ++srcbuf;
  }
  
  
  *destbuf++ = '\0';
  
  if( len >= destbuf_len )
  {
    return 0;
  }
  else
  {				
    return orig_destbuf;
  }
  
} 


char * jdk_html_amp_unescape_text( 
  const char *srcbuf, 
  char *destbuf, 
  size_t destbuf_len 
  ) 
{
  size_t len = 0;
  char *orig_destbuf=destbuf;
  bool in_angle_brackets=false;
  bool in_unknown_ampersand=false;
  
  while( len<destbuf_len-4 && *srcbuf )
  {
    if( in_angle_brackets )
    {
      if( *srcbuf=='>' )
      {
        in_angle_brackets = false;
      }
      
      ++srcbuf;	
    }
    else if( in_unknown_ampersand )
    {
      if( *srcbuf=='<' )
      {
        in_unknown_ampersand=false;
        in_angle_brackets = true;
        *destbuf++ = ' ';
        ++srcbuf;
        ++len;
      }
      else if( *srcbuf==';' )
      {
        in_unknown_ampersand=false;
        ++srcbuf;
      }			
    }
    else
    {			
      switch( *srcbuf )
      {
      case '<':
      {					
        in_angle_brackets = true;
        *destbuf++ = ' ';
        ++srcbuf;
        ++len;
        break;
      }
      
      case '>':
      {					
        ++srcbuf;
        break;
      }
      
      case '&':
      {					
        if( jdk_strnicmp( srcbuf, "&amp", 4 )==0 )
        {
          *destbuf++ = '&';
          srcbuf+=5;
          ++len;
        }
        else if( jdk_strnicmp( srcbuf, "&lt", 3 )==0 )
        {
          *destbuf++ = '<';
          srcbuf+=4;
          ++len;
        }
        else if( jdk_strnicmp( srcbuf, "&gt", 3 )==0 )
        {
          *destbuf++ = '>';
          srcbuf+=4;
          ++len;
        }
        else if( jdk_strnicmp( srcbuf, "&copy", 5 )==0 )
        {
          strncpy( destbuf, "(c)", destbuf_len-1-len );
          destbuf+=3;
          srcbuf+=6;
          len+=3;
        }					
        else
        {
          in_unknown_ampersand = true;
          *destbuf++ = ' ';
          ++len;
          ++srcbuf;
        }
        
        break;
      }								
      default:
        *destbuf++ = *srcbuf++;
        break;
      }
    }
  }
  
  *destbuf++ = '\0';
  
  if( len >= destbuf_len )
  {
    return 0;
  }
  else
  {				
    return orig_destbuf;
  }
  
} 


bool jdk_html_requires_escaping( const char *src )
{
  while( *src )
  {
    if( *src=='&' || *src=='<' || *src=='>' )
    {
      return true;
    }
    ++src;
  }
  return false;
}

bool jdk_html_requires_unescaping( const char *src )
{
  while( *src )
  {
    if( *src=='&' )
    {
      return true;
    }
    ++src;
  }
  return false;
}


bool jdk_html_requires_escaping( const jdk_dynbuf &buf )
{
  const char *src = (const char *)buf.get_data();
  size_t len = buf.get_data_length();
  while( len-- )
  {
    if( *src=='&' || *src=='<' || *src=='>' )
    {
      return true;
    }
    ++src;
  }
  return false;
  
}

bool jdk_html_requires_unescaping( const jdk_dynbuf &buf )
{
  const char *src = (const char *)buf.get_data();
  size_t len = buf.get_data_length();
  while( len-- )
  {
    if( *src=='&' )
    {
      return true;
    }
    ++src;
  }
  return false;
}
