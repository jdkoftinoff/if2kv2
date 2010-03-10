#ifdef SWIG
%module jdk
%{
#include "jdk_buf.h"
%}
#endif
#ifndef _JDK_BUF_H
#define _JDK_BUF_H

#include "jdk_world.h"
#include "jdk_string.h"

class jdk_buf
{
protected:
	jdk_buf() : data_len(0), buf_len(0), data(0)
	{	
	}
		
	const jdk_buf & operator = ( const jdk_buf &a ); // not allowed	
	jdk_buf( const jdk_buf &); // not allowed
	
public:
	
	jdk_buf( void *data_, size_t data_len_, size_t buf_len_ )
	  : 
	data_len( data_len_ ),
	buf_len( buf_len_ ),
	data( (unsigned char *)data_ )	
	{		
	}
	
	// destructor
	virtual ~jdk_buf() 
	{
	}
	
  bool copy( const jdk_buf &o )
  {
    bool r=false;
    clear();
    if( resize( o.get_data_length()+1 ) )
    {
      r=append_from_data( o.get_data(), o.get_data_length() );
    }
    return r;
  }
	
	// clear but dont resize
	virtual void clear() 
	{
    data_len=0;
	}
	
		
	// clear and resize to len
	virtual void clear( size_t len ) 
	{
    data_len=0;
		shrink(1);
	}
	
		
	// expand size to len
	virtual bool expand( size_t len )
	{
		return false;	
	}
	
	
	// shrink size to len
	virtual bool shrink( size_t len )
	{
		return false;	
	}
	
   	
	// reallocate buffer to len size	
	virtual bool resize( size_t len )
	{
		return false;	
	}
	
	
	virtual void fill()
	{
	    data_len=buf_len;    
	}
		
	// get raw ptr to data
  unsigned char * get_data() 
	{	
		return data;
	}
	
 	const unsigned char * get_data() const 
	{
		return data;
	}
	
	// get the current size of the buffer
	size_t get_buf_length() const
	{
		return buf_len;
	}
		
	// get length of data
	size_t get_data_length() const 
	{
		return data_len;
	}
	
	// manually set the data length - only for use by functions dealing with the buf directly
	void set_data_length( size_t len )
	{
		data_len = len;
	}
	
		
	// append data
	virtual bool append_from_data( const unsigned char *b, size_t len );
	virtual bool append_from_data( const char *b, size_t len ) 
	{
		return append_from_data( (const unsigned char *)b, len );
	}
	
	virtual bool append_from_file( const char *file_name, int start=0, int len=-1 );
	virtual bool append_from_file( const jdk_string &file_name, int start=0, int len=-1 )
	{
	    return append_from_file( file_name.c_str(), start, len );
	}
	virtual bool append_from_file( FILE *f, int start=0, int len=-1, bool allow_partial=false );	

	virtual bool append_from_stream( FILE *f );
	virtual bool append_from_url( const char *url, unsigned long max_len, const char *proxy=0, bool use_ssl=false );
	virtual bool append_from_string( const char *str );
	virtual bool append_from_string( const jdk_string &s )
	{
	    return append_from_string( s.c_str() );
	}
	virtual bool append_from_buf( const jdk_buf &other, int start=0, int len=-1 );
	virtual bool append_form( const char *fmt, ... );
	

	// extract data
	virtual bool extract_to_file( FILE *f, int start=0, int len=-1 ) const;
	virtual bool extract_to_stream( FILE *f ) const;
	virtual bool extract_to_file( const char *file_name, int start=0, int len=-1 ) const;	
	virtual bool extract_to_file( const jdk_string &file_name, int start=0, int len=-1 ) const
	{
	    return extract_to_file( file_name.c_str(), start, len );
	}

	virtual bool extract_to_data( unsigned char *b, int start, int len ) const;
	virtual bool extract_to_data( char *b, int start, int len ) const
	{
		return extract_to_data( (unsigned char *)b, start, len );
	}
	
	virtual int extract_to_string( 
					  char *str, 
					  size_t start, 
					  size_t max_len, 
					  const char *separators="\n", 
					  const char *ignores="\r"
					  ) const;
	
	virtual int extract_to_string( 
					  jdk_string &str,
					  size_t start,  
					  const char *separators="\n", 
					  const char *ignores="\r"
					  ) const
    {
        return extract_to_string( str.c_str(), start, str.getmaxlen(), separators, ignores );
	}
	
protected:	

	size_t data_len;
	size_t buf_len;
	unsigned char *data;	
};

template <int SZ>
class jdk_staticbuf : public jdk_buf
{
public:
	inline jdk_staticbuf() : jdk_buf( staticbuf, 0, SZ )
	{
	}
	
private:
	unsigned char staticbuf[ SZ ];
};


#ifdef __OBJC__
#import <Cocoa/Cocoa.h>

inline void jdk_buf_from_NSString( jdk_buf &buf, NSString *s )
{
  buf.clear();
  buf.append_from_string( [s lossyCString] );
}

inline NSString * jdk_buf_to_NSString( jdk_buf &buf )
{
  return [NSString stringWithCString: (const char*)buf.get_data() length: buf.get_data_length() ];  
}

#endif



#endif
