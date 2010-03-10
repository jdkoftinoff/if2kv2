#ifndef _JDK_LINESPLIT_H
#define _JDK_LINESPLIT_H

#include "jdk_string.h"
#include "jdk_buf.h"

class jdk_linesplit;

//----------------------------------------------------------------------

class jdk_linesplit
{    
public:
	jdk_linesplit( size_t line_sz );

  jdk_linesplit( const jdk_linesplit &o );
  const jdk_linesplit & operator = ( const jdk_linesplit & );   

	virtual ~jdk_linesplit();
    
    void clear()
    {
        buf_pos=0;
	    buf_len=0;
        line_pos=0;
    }
    
	void set_buf( char *buf_, size_t buf_len_ );

	void set_buf( jdk_buf &buf_ )
	{
		set_buf( (char *)buf_.get_data(), buf_.get_data_length() );	
	}
	
	// ScanForLine() returns false if finished scanning buffer,
	// or true if Line has been formed and stored
	
	bool scan_for_line( 
					   jdk_string *line_buf=0, 
					   const char *ign="\r", 
					   const char *term="\n" 
					   );

    char *get_line_buf() 
    {
        return line;
    }
	
	size_t get_buf_pos() const
	{
	    return buf_pos;
	}
	
	void rewind_buf_pos( size_t count )
	{
		if( buf_pos>=count )
		{
			buf_pos-=count;
		}		
	}

	void clear_buf_pos()
	{
		buf_pos=0;	
	}

	size_t get_buf_len()
  {
    return buf_len;
  }
	
private:

	char *buf;
	size_t buf_len;
	size_t buf_pos;
	
	char *line;
	size_t line_sz;	
	size_t line_pos;
};

#endif
