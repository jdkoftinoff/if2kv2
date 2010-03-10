#ifndef _JDK_CGI_H
#define _JDK_CGI_H

#include "jdk_string.h"
#include "jdk_dynbuf.h"

char jdk_cgi_x2c( const char *in );

char * jdk_cgi_c2x( char in, char *out );

inline bool jdk_cgi_isescapablechar( char c ) 
{
  bool r=true;
	
  if( jdk_isalnum(c) )
  {
    r=false;
  }
  else
  {
    if( strchr( ";/?:@&=+!*'(),$-_.%<>~", c )!=0 )
      r=false;
  }

  return r;	  
} 

inline bool jdk_cgi_isescapablechar_with_amp( char c )
{
  bool r=true;
  
  if( jdk_isalnum(c) )
  {
    r=false;
  }
  else
  {
    if( strchr( ";/?:@=+!*'(),$-_.%<>~", c )!=0 )
      r=false;
  }

	return r;	  
}

inline bool jdk_cgi_isescapablechar_for_var( char c ) 
{
  bool r=true;
	
  if( jdk_isalnum(c) )
  {
    r=false;
  }
  else
  {
    if( strchr( ";/?:@&=+!*'(),$-_.%<>~", c )!=0 )
      r=true;
  }

  return r;	  
} 

inline bool jdk_cgi_isescapablechar_with_amp_for_var( char c )
{
  bool r=true;
  
  if( jdk_isalnum(c) )
  {
    r=false;
  }
  else
  {
    if( strchr( ";/?:@=+!*'(),$-_.%<>~", c )!=0 )
      r=true;
  }

	return r;	  
}


// "in" can be "out" for unescape only to unescape in place
bool jdk_cgi_unescape( const char *in, char *out, size_t maxlen );

inline bool jdk_cgi_unescape( const jdk_string &in, jdk_string &out )
{
	return jdk_cgi_unescape( in.c_str(), out.c_str(), out.getmaxlen() );	
}

inline bool jdk_cgi_unescape( jdk_string &in_out )
{
	return jdk_cgi_unescape( in_out.c_str(), in_out.c_str(), in_out.getmaxlen() );	
}



bool jdk_cgi_escape( const char *in, char *out, size_t maxlen );

inline bool jdk_cgi_escape( const jdk_string &in, jdk_string &out )
{
	return jdk_cgi_escape( in.c_str(), out.c_str(), out.getmaxlen() );	
}


bool jdk_cgi_escape_with_amp( const char *in, char *out, size_t maxlen );

inline bool jdk_cgi_escape_with_amp( const jdk_string &in, jdk_string &out )
{
	return jdk_cgi_escape_with_amp( in.c_str(), out.c_str(), out.getmaxlen() );
}


const char * jdk_cgi_extract_pair(
							const char *in, size_t in_len,
							char *out_name, size_t out_name_maxlen,
							char *out_value, size_t out_value_maxlen
							);

inline const char * jdk_cgi_extract_pair(
                            const jdk_string &in,
			                jdk_string &out_name,
					        jdk_string &out_value
						)
{
    return jdk_cgi_extract_pair( 
                            in.c_str(), in.len(),
                            out_name.c_str(), out_name.getmaxlen(), 
			                out_value.c_str(), out_value.getmaxlen() 
					);
}

char * jdk_cgi_encode_pair(
						   const char *name,
						   const char *value,
						   char *out, size_t out_maxlen 
						   );
						   
inline char * jdk_cgi_encode_pair(
                            const jdk_string &name,
			                const jdk_string &value,
					        jdk_string &out
						)
{
    return jdk_cgi_encode_pair( name.c_str(), value.c_str(), out.c_str(), out.getmaxlen() );
}

char * jdk_html_amp_escape_text( 
					   const char *srcbuf, 
					   char *destbuf, 
					   size_t destbuf_len 
					   );

inline jdk_string & jdk_html_amp_escape_text( 
                                        const jdk_string &src,
					                    jdk_string &dest
							    )
{
    jdk_html_amp_escape_text( src.c_str(), dest.c_str(), dest.getmaxlen() );
    return dest;
}							    

inline jdk_string & jdk_html_amp_escape_text( 
                                        const char *src,
					                    jdk_string &dest
							    )
{
    jdk_html_amp_escape_text( src, dest.c_str(), dest.getmaxlen() );
    return dest;
}							    


char * jdk_html_amp_unescape_text( 
					   const char *srcbuf, 
					   char *destbuf, 
					   size_t destbuf_len 
					   );

inline jdk_string & jdk_html_amp_unescape_text( 
                                        const jdk_string &src,
					                    jdk_string &dest
							    )
{
    jdk_html_amp_unescape_text( src.c_str(), dest.c_str(), dest.getmaxlen() );
    return dest;
}							    


inline jdk_string & jdk_html_amp_unescape_text( 
                                        const char *src,
					                    jdk_string &dest
							    )
{
    jdk_html_amp_unescape_text( src, dest.c_str(), dest.getmaxlen() );
    return dest;
}					



bool jdk_html_requires_escaping( const char *src );
bool jdk_html_requires_escaping( const jdk_dynbuf &src );

inline bool jdk_html_requires_escaping( const jdk_string &src )
{
	return jdk_html_requires_escaping( src.c_str() );
}

bool jdk_html_requires_unescaping( const char *src );
bool jdk_html_requires_unescaping( const jdk_dynbuf &src );

inline bool jdk_html_requires_unescaping( const jdk_string &src )
{
	return jdk_html_requires_unescaping(src.c_str() );
}


#endif
