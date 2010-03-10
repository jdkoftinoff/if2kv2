#ifndef _JDK_RS232_H
#define _JDK_RS232_H

class jdk_rs232
{
public:
	jdk_rs232();
	virtual ~jdk_rs232();
	
    int  rs232_open( const char *port, int bd );
    void rs232_close( void );
    int  rs232_in( void );
    int  rs232_out( int c );
	int  rs232_write( const void *buf, size_t len );
	int  rs232_read( void *buf, size_t len );	
    int  rs232_stat( void );
    void rs232_clear( int secs );
    int  rs232_out_string( const char *s );
	
#if !JDK_IS_WINE && (defined(WIN32) || defined(CYGWIN))
    HANDLE rs232_handle;
#else
    int rs232_handle;
#endif
};

#endif
