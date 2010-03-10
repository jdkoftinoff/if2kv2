#ifndef _JDK_DYNBUF_H
#define _JDK_DYNBUF_H

#include "jdk_world.h"
#include "jdk_string.h"
#include "jdk_buf.h"


class jdk_dynbuf : public jdk_buf
{
public:
	
	// default constructor
	jdk_dynbuf()   
		:
		jdk_buf( new unsigned char[64], 0, 64 )
	{
		markup();
	}
	
	
	explicit jdk_dynbuf( size_t max_sz )  
		:
		jdk_buf(new unsigned char[max_sz], 0, max_sz)
	{
		markup();
	}
	
	
	// copy constructor
	jdk_dynbuf( const jdk_dynbuf &a ) 
	   	:
		jdk_buf( new unsigned char[a.get_buf_length()], a.get_data_length(), a.get_buf_length() )
	{	
		memcpy( data, a.get_data(), data_len );
	}

	jdk_dynbuf( const jdk_buf &a ) 
	   	:
		jdk_buf( new unsigned char[a.get_buf_length()], a.get_data_length(), a.get_buf_length() )
	{	
		memcpy( data, a.get_data(), data_len );
	}
	
		 
	// copy function operator
	const jdk_dynbuf & operator = ( const jdk_dynbuf &a ) 
	{
    if( &a == this )
		  return *this;
		
		clear(a.get_data_length());
		markup();				
		append_from_data( a.get_data(), a.get_data_length() );
		
		return *this;
	}

	const jdk_dynbuf & operator = ( const jdk_buf &a ) 
	{
    if( &a == this )
		  return *this;
		
		clear(a.get_data_length());
		markup();				
		append_from_data( a.get_data(), a.get_data_length() );
		
		return *this;
	}
	
		
	explicit jdk_dynbuf( const char *filename )   
		:
		jdk_buf( new unsigned char[64], 0, 64 )
	{
		markup();				
		append_from_file( filename );
	}
	
	explicit jdk_dynbuf( const jdk_string &filename )
	    :
		jdk_buf( new unsigned char[64], 0, 64 )	
	{
		markup();				
		append_from_file( filename );	
	}
	
	
	// destructor
	~jdk_dynbuf() 
	{
		if(data) delete [] data;
	}
	   		
	// expand size to len
	bool expand( size_t len );	
	
	// shrink size to len
	bool shrink( size_t len );
   	
	// reallocate buffer to len size	
	bool resize( size_t len );

private:
	void markup();
};


#endif
