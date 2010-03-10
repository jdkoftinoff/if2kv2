#ifndef _JDK_STRING_H
#define _JDK_STRING_H

#include "jdk_util.h"
#include "jdk_error.h"

#ifndef DEBUG_JDK_STRING
# define DEBUG_JDK_STRING 0
# if 0
#  ifdef DEBUG
#	if DEBUG
#	 undef DEBUG_JDK_STRING
#	 define DEBUG_JDK_STRING 1
#	endif
#  endif
# endif
#endif

class jdk_string
{
protected:
  
	explicit jdk_string( char *o ) :
		s( o ),
		maxlen( strlen(o)+1 )	  
  {
    if( !s )
    {
      JDK_THROW_ERROR( "jdk_string() passed null", "" );	
    }				
  }
  
  explicit jdk_string( const jdk_string & );
  
  explicit jdk_string( const char *o )
    :
    s( (char *)o ),
    maxlen( strlen(o)+1 )	  
  {
    if( !s )
    {
      JDK_THROW_ERROR( "jdk_string() passed null", "" );	
    }        
  }
  
public:
  
	virtual ~jdk_string()
  {
  }
	
	const jdk_string &operator = (const jdk_string &o )
  {
    if( &o != this )
    {
      resize( o.len()+16 );
      cpy( o );
    }
    return *this;
  }
  
	const jdk_string &operator = (const char *o )
  {
    validate_pointer(o);
    if( !o )		  
    {
      clear();
    }
    else
      if( o != s )
      {
        resize( strlen(o)+16 );		  
        cpy( o );
      }
    return *this;
  }
	
	void clear()
  {
    *s = '\0';
  }	
	
	bool is_clear() const
  {
    return *s=='\0';
  }
  
	jdk_string *get()
  {
    return this;
  }
  
	const jdk_string *get() const
  {
    return this;
  }
  
  
  char * operator ()()
  {
    return s;
  }
  
  const char * operator ()() const
  {
    return s;
  }
  
	char *c_str()
  {
#if DEBUG_JDK_STRING
    fprintf( stderr, "%08lx c_str(): %s\n", (unsigned long)this, s );
#endif		
    return s;
  }
  
	const char *c_str() const
  {
    return s;
  }
	
	char *getaddr( size_t off )
  {
    if( off>=maxlen )
    {
      resize( off+32 );
    }
    
    return &s[off];
  }
	
	const char *getaddr( size_t off ) const
  {
    if( off>=maxlen )
    {
      resize( off+32 );
    }
    
    return &s[off];
  }
	
	char get( size_t off ) const
  {
    if( off>=maxlen )
    {
      resize( off+32 );
    }
    
    return s[off];
  }
	
	void set( size_t off, char c )
  {
    if( off>=maxlen )
    {
      resize(off+16);
    }
    
    if( off<maxlen )
    {
      s[off]=c;
    }
    else
    {
      resize( off );
      if( off<maxlen )
      {
        s[off]=c;
      }
    }
  }
	
	
	size_t getmaxlen() const
  {
    return maxlen;
  }
  
	size_t get_buffer_length() const
  {
    return maxlen;
  }
  
	size_t len() const
  {
    return strlen(s);
  }
  
	size_t get_data_length() const
  {
    return strlen(s);
  }
  
	const char * chr( char c ) const
  {
    return ::strchr( s, c );
  }
  
	char * chr( char c ) 
  {
    return ::strchr( s, c );
  }	
  
	const char * rchr( char c ) const
  {
    return ::strrchr( s, c );
  }
  
	char * rchr( char c ) 
  {
    return ::strrchr( s, c );
  }
  
  
	
	size_t spn( size_t pos, const char *delim ) const
  {
    if( !delim )
    {		   
      JDK_THROW_ERROR( "jdk_string::spn passed null\n", "" );
    }
    
    return ::strspn( &s[pos], delim );
  }
  
	size_t cspn( size_t pos, const char *delim ) const
  {
    if( !delim )
    {		   
      JDK_THROW_ERROR( "jdk_string::cspn passed null\n", "" );
    }
    
    return ::strcspn( &s[pos], delim );
  }
	
	
	void strip_crlf()
  {
    jdk_strip_crlf(s);
  }
	
	void strip_endws()
  {
    jdk_strip_endws(s);
  }
	
	void strip_begws()
  {
    jdk_strip_begws(s);
  }
	
  void strip_begendws()
  {
    jdk_strip_begws(s);
    jdk_strip_endws(s);
  }
	
	void upr()
  {
    jdk_strupr(s);
  }
  
	void lower()
  {
    jdk_strlower(s);
  }
  
  bool prepend( char c )
  {
    size_t l = len();
    if( l+1<maxlen )
    {
      memmove( s+1, s, l+1 );
      s[0] = c;
      return true;
    }
    else
    {
      resize( l+64 );
      if( l+1<maxlen )
      {
        memmove( s+1, s, l+1 );
        s[0] = c;
        return true;
      }
      else
        return false;
    }
  }
  
  bool remove_prefix( const char *o )
  {
    bool r=false;
    size_t olen = strlen(o);
    if( ncmp(o, olen)==0 )
    {
      size_t l=len() - olen +1;
      memmove( s, s+olen, l );
      r=true;
    }
    return r;
  }
  
  bool remove_prefix( const jdk_string &o )
  {
    return remove_prefix( o.c_str() );
  }
  
  bool append( const char *o )
  {
    return cat(o);
  }
  
  bool append( const jdk_string &o )
  {
    return append( o.c_str() );
  }
  
  bool prepend( const char *o )
  {
    size_t l=len();
    size_t ol=strlen(o);
    if( l+ol<maxlen )
    {
      memmove( s+ol, s, l+1 );
      memmove( s, o, ol );
      return true;
    }
    else
    {
      validate_pointer(o);			
      resize(l+ol+64);
      if( l+ol<maxlen )
      {
        memmove( s+ol, s, l+1 );
        memmove( s, o, ol );
        return true;
      }
      else
      {		
        return false;
      }
    }
  }
  
  bool prepend( const jdk_string &o )
  {
    return prepend( o.c_str() );
  }
  
  
	bool cat( char c )
  {			
    size_t l = len();
    if( l>=maxlen-16 )
      resize(l+64);
    return jdk_strncat( s, c, maxlen );
  }
  
	bool cat( const jdk_string &o )
  {
    bool ret=false;
    if( &o!=this )
    {			
      size_t l = len() + o.len();
      if( l>=maxlen-16 )
        resize(l+64);
      ret=jdk_strncat( s, o.c_str(), maxlen );
    }		
    else
    {
      ret=true;
    }
    return ret;
  }
	
	bool cat( const char *o )
  {			
    size_t l = len() + strlen(o);
    if( l>=maxlen-16 )
    {
      validate_pointer(o);
      
      resize(l+64);
    }
    return jdk_strncat( s, o, maxlen );
  }
	
	bool cpy( const jdk_string &o )
  {
	  bool ret;
    
    if( &o!=this )
    {			
      size_t l = o.len();
      if( l>=maxlen-16 )		  
        resize(l+64);
      ret=jdk_strncpy( s, o.c_str(), maxlen );
    }
    else
    {
      ret=true;
    }
    
    return ret;
  }
	
	bool cpy( const char *o )
  {		
    size_t l = strlen(o);
    if( l>=maxlen-16 )
    {
      validate_pointer(o);			
      resize(l+64);
    }
    
    return jdk_strncpy( s, o, maxlen );
  }
  
	
	bool ncpy( const jdk_string &o, size_t n )
  {
    bool ret=false;
    
    if( &o!=this )
    {
      ret=true;
    }
    else
    {		   		
      if( n>=maxlen-16 )
        resize(n+64);
      
      if( n>=maxlen )
        ret=false;			
      else		 
        ret = jdk_strncpy( s, o.c_str(), n );
    }
    
    return ret;
  }
	
	bool ncpy( const char *o, size_t n )
  {		
    bool ret;
    
    if( n>=maxlen-16 )
    {	
      validate_pointer(o);			
      resize(n+64);
    }						
    if( n>=maxlen )
      ret=false;
    else		 
      ret = jdk_strncpy( s, o, n );
    return ret;		
  }
  
	char * str( const char *o )
  {
    validate_pointer(o);
    
    return strstr( s, o );	
  }
  
	char * str( const jdk_string &o )
  {
    return strstr( s, o.c_str() );	
  }
  
	const char * str( const char *o ) const
  {
    validate_pointer(o);
    return strstr( s, o );	
  }
  
	const char * str( const jdk_string &o ) const
  {
    return strstr( s, o.c_str() );	
  }
	
	
	int cmp( const jdk_string &o ) const
  {
    return ::strcmp( s, o.c_str() );
  }
	
	int cmp( const char *o ) const
  {
    validate_pointer(o);		
    return ::strcmp( s, o );
  }
  
	int icmp( const jdk_string &o ) const
  {
    return jdk_stricmp( s, o.c_str() );
  }
	
	int icmp( const char *o ) const
  {
    if( !o )
    {		   
      JDK_THROW_ERROR( "jdk_string::icmp passed null\n", "" );
    }
    
    return jdk_stricmp( s, o );
  }
  
	
	int ncmp( const jdk_string &o, size_t n ) const
  {
    return ::strncmp( s, o.c_str(), n );
  }
	
	
	int ncmp( const char *o, size_t n ) const
  {
    return ::strncmp( s, o, n );
  }
  
  
	int nicmp( const jdk_string &o, size_t n ) const
  {
    return jdk_strnicmp( s, o.c_str(), n );
  }
  
  
	int nicmp( const char *o, size_t n ) const
  {
    validate_pointer(o);
    
    return jdk_strnicmp( s, o, n );
  }
  
  
	int extract_token(
		size_t pos,
		char *dest,
		size_t destmaxlen,
		const char *delim,
		const char *ignores=0
    ) const;
  
	int extract_token( 
		size_t pos, 
		jdk_string *dest, 
		const char *delim,
		const char *ignores=0 
    ) const;	
	
  
	bool form(
		const char *fmt,
		...
    );
  
  void form_path( const jdk_string &path, const jdk_string &file )
  {
    jdk_formpath( s, path.c_str(), file.c_str(), 0, maxlen );
  }
  
  void form_path( const jdk_string &path, const jdk_string &file, const jdk_string &extension )
  {
    jdk_formpath( s, path.c_str(), file.c_str(), extension.c_str(), maxlen );
  }
  
  void form_path( const jdk_string &path, const jdk_string &file, const char *extension=0 )
  {
    jdk_formpath( s, path.c_str(), file.c_str(), extension, maxlen );
  }
  
  void form_path( const jdk_string &path, const char *file, const char *extension=0 )
  {
    jdk_formpath( s, path.c_str(), file, extension, maxlen );
  }
  
	long strtol(int base=0) const
  {
    return ::strtol( s, 0, base );
  }
  
	unsigned long strtoul(int base=0) const
  {
    return ::strtoul( s, 0, base );
  }
  
	virtual void resize(size_t sz) const
  {
    // do nothing here
  }
  
  float strtof() const
  {
//    return ::strtof( s, 0 );
	  return 0.0f;
  }
  
  float strtod() const
  {
// return ::strtod( s, 0 );
	  return 0.0;
  }
  
protected:	
	mutable char *s;
	mutable size_t maxlen;
  
	explicit jdk_string( char *s_, size_t maxlen_ ) : s(s_),maxlen( maxlen_ )
  {
    if( !s )
    {
      JDK_THROW_ERROR( "jdk_string() passed null\n", "" );
    }
  }
  
	void validate_pointer( const char *o ) const
  {
    if( !o )
    {		   
      JDK_THROW_ERROR( "jdk_string::cat passed null\n", "" );
    }
#if 0		
    if( (unsigned long)o >= (unsigned long)s 
        && (unsigned long)o <= (unsigned long)s+maxlen )
    {
      JDK_THROW_ERROR( "ERROR jdk_str operation char * on self", "" );
    }
#endif		
  }
};



inline int operator == ( const jdk_string &a, const jdk_string &b )
{
	return jdk_strcmp( a.c_str(), b.c_str() )==0;
}

inline int operator != ( const jdk_string &a, const jdk_string &b )
{
	return jdk_strcmp( a.c_str(), b.c_str() )!=0;
}

inline int operator < ( const jdk_string &a, const jdk_string &b )
{
	return jdk_strcmp( a.c_str(), b.c_str() )<0;	   
}

inline int operator > ( const jdk_string &a, const jdk_string &b )
{
	return jdk_strcmp( a.c_str(), b.c_str() )>0;	   
}

inline int operator <= ( const jdk_string &a, const jdk_string &b )
{
	return jdk_strcmp( a.c_str(), b.c_str() )<=0;	   
}

inline int operator >= ( const jdk_string &a, const jdk_string &b )
{
	return jdk_strcmp( a.c_str(), b.c_str() )>=0;	   
}


class jdk_string_static : public jdk_string
{
public:
	jdk_string_static( const char *p ) : jdk_string((char*)p,strlen(p))
  {}
};

template <size_t LEN>
class jdk_str : public jdk_string
{
public:
	jdk_str() : jdk_string(buf,LEN)
  {
    buf[0]='\0';
  }
  
	jdk_str( const jdk_str<LEN> &o )
	  : jdk_string(buf,LEN)
  {
    jdk_strncpy( buf, o.c_str(), LEN );
  }
	
	jdk_str( const jdk_string &o )
	  : jdk_string(buf,LEN)
  {
    jdk_strncpy( buf, o.c_str(), LEN );
  }	
  
	jdk_str( const char *o )
	  : jdk_string(buf,LEN)
  {
#if DEBUG_JDK_STRING
    fprintf( stderr, "%08lx: jdk_str<%d>('%s')\n", (unsigned long)this, LEN, o );
#endif		
    if( !o )
    {
      JDK_THROW_ERROR( "jdk_str() passed null\n", "" );
    }
    
    jdk_strncpy( buf, o, LEN );
#if DEBUG_JDK_STRING
    fprintf( stderr, "%08lx: set to: '%s'\n", (unsigned long)this, buf );
#endif
  }	
	
	const jdk_str<LEN> &operator = ( const jdk_string &o )
  {
    jdk_strncpy( buf, o.c_str(), LEN );
    return *this;
  }
  
	const jdk_str<LEN> &operator = ( const char *o )
  {
    if( !o )
    {
      JDK_THROW_ERROR( "jdk_str::= passed null\n", "" );
    }
    
    jdk_strncpy( buf, o, LEN );
    return *this;
  }
	
  
	
private:
  
	char buf[LEN];	
};

typedef jdk_str<4096> jdk_string_filename;
typedef jdk_str<4096> jdk_string_url;
typedef jdk_str<256> jdk_string_host;

class jdk_dynstr : public jdk_string
{
	enum { initial_size=256 };
  
public:
  
	~jdk_dynstr()
  {
    free(s);
  }
  
	jdk_dynstr() 
		: 
		jdk_string((char *)malloc(initial_size),initial_size)
  {
    s[0]='\0';
  }
  
	
	jdk_dynstr( const jdk_string &o )
	  : 
	  jdk_string(		
      (char *)malloc(o.len() > initial_size ? o.len() + initial_size : initial_size),
      o.len() > initial_size ? o.len() + initial_size : initial_size)
    
  {
    jdk_strncpy( s, o.c_str(), maxlen );
  }	
  
	jdk_dynstr( const char *o )
	  : 
	  jdk_string(
      (char *)malloc(strlen(o)+32 ),
      strlen(o)+32)
  {
    if( !o )
    {
      JDK_THROW_ERROR( "jdk_str() passed null\n", "" );
    }
    
    jdk_strncpy( s, o, maxlen );		
  }	
	
  const jdk_dynstr &operator = ( const jdk_string &o )
  {
    if( &o != this )
    {
      cpy(o);
    }
    return *this;
  }
  
	const jdk_dynstr &operator = ( const jdk_dynstr &o )
  {
    if( &o != this )
    {
      cpy( o );
    }
    return *this;
  }
  
	const jdk_dynstr &operator = ( const char *o )
  {
    if( !o )
    {
      JDK_THROW_ERROR( "jdk_str::= passed null\n", "" );
    }
    validate_pointer(o);
    cpy(o);
    return *this;
  }
	
	void resize( size_t newlen ) const
  {
    if( newlen<64 )
      newlen=64;
    
    s = (char *)realloc( s, newlen );		
    maxlen=newlen;
  }
	
};



inline int jdk_strcmp( const jdk_string &a, const jdk_string &b )
{
  return jdk_strcmp( a.c_str(), b.c_str() );
}

inline int jdk_stricmp( const jdk_string &a, const jdk_string &b )
{
  return jdk_stricmp( a.c_str(), b.c_str() );
}

inline bool jdk_read_string_line( jdk_string &a, FILE *f )
{
	bool r = false;
	
  jdk_str<8192> t;
  
	if( fgets( t.c_str(), int(t.getmaxlen()), f ) )
	{
		t.strip_crlf();
    a.cpy( t );
		r=true;
	}
	
	return r;
} 


inline bool jdk_read_string_line( jdk_string *a, FILE *f ) 
{
	bool r = false;
  
	if( !a )
	{
		JDK_THROW_ERROR( "jdk_read_string_line passed null\n", "" );
	}
	
  jdk_str<4096> t;
  
	if( fgets( t.c_str(), int(t.getmaxlen()), f ) )
	{
		t.strip_crlf();
    a->cpy( t );
		r=true;
	}
	
	return r;
} 

inline bool jdk_write_string_line( const jdk_string *a, FILE *f ) 
{
	if( !a )
	{
		JDK_THROW_ERROR( "jdk_write_string_line passed null\n", "" );
	}
	
	fprintf( f, "%s\n", a->c_str() );
	return !ferror(f);
} 



inline void jdk_string_escape( const jdk_string &in, jdk_string &out )
{
  out.clear();
  size_t len=in.len();
  for( size_t i=0; i<len; ++i )
  {
    char c=in.get(i);
    if( c=='\n' )
    {
      out.cat("\\n");
    }
    else if( c=='\t' )
    {
      out.cat("\\t" );
    }
    else if( c=='\r' )
    {
      out.cat("\\r" );
    }
    else if( c=='\\' )
    {
      out.cat("\\\\");
    }
		else if( c=='"' )
		{
			out.cat("\\\"");
		}
    else if( jdk_isprint(c) || c=='~' )
    {
      out.cat(c);
    }
    else
    {
      jdk_str<32> num;
      num.form("\\%03hho",((unsigned char)c)&0xff);
      out.cat(num);
    }
  }
}

inline void jdk_string_unescape( const jdk_string &in, jdk_string &out )
{
  out.clear();
  size_t len=in.len();
  size_t i=0;
  while( i<len )
  {
    char c=in.get(i);
    if( c=='\\' )
    {
      i++;
      if( i>=len )
        break;
      c=in.get(i);
      if( c=='\\' )
      {
        out.cat('\\');
      }
      if( c=='n' )
      {
        out.cat('\n');
      }
      else if( c=='r' )
      {
        out.cat('\r');
      }
      else if( c=='\t' )
      {
        out.cat('\t');
      }
			else if( c=='"' )
			{
				out.cat('"');
			}
      else if( c>='0' && c<='9' && len>i+2)
      {
        char buf[4];
        buf[0]=in.get(i);
        buf[1]=in.get(i+1);
        buf[2]=in.get(i+2);
        buf[3]=0;
        c=(char)strtol(buf,0,8);
				out.cat(c);
        i+=2;
        if( i>=len )
        {
          break;
        }
      }
      
    }
    else
    {
      out.cat(c);
    }
    i++;
  }
}



#endif
