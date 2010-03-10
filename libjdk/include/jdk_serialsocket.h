/* -*- mode: C; mode: fold; -*- */
#ifndef _JDK_SERIALSOCKET_H
#define _JDK_SERIALSOCKET_H

#include "jdk_world.h"
#include "jdk_socket.h"


#if 0
class jdk_serial_socket : public jdk_fd_client_socket
{
public:

	jdk_serial_socket();
    
	virtual ~jdk_serial_socket();
	
	bool set_baud_rate( unsigned long baud_rate_ );

	bool set_read_timeout( unsigned long t );
	
	bool make_connection( const char *hostname, int port, jdk_dns_cache *cache=0, bool x=false );

	void close();

	int wait_readable();

	int wait_writable();

	int read_data( void *buf, int len, int timeout_seconds  );
	
	int write_data( const void *buf, int len, int timeout_seconds);
private:
	unsigned long baud_rate;		
	char fname[512];
	unsigned long read_timeout;
};
#endif

#endif
