#ifndef _JDK_NAMESERVER_H
#define _JDK_NAMESERVER_H

#include "jdk_socket.h"
#include "jdk_thread.h"

#if 0
#if JDK_HAS_THREADS
#define JDK_NAMESERVER_MAGIC (0x4a444b4e)

struct jdk_nameserver_request_packet 
{
	jdk_nameserver_request_packet();
	
	unsigned long magic;
	unsigned long id1;
	unsigned long id2;
	char name[128-12];
}; 

struct jdk_nameserver_response_packet 
{
	jdk_nameserver_response_packet();
	
	unsigned long magic;
	unsigned long id1;
	unsigned long id2;
	char name[128-12];
	char ip[128];
}; 

class jdk_nameserver_event 
{
public:

	jdk_nameserver_event();
	virtual ~jdk_nameserver_event();
	
	virtual void nameserver_response(
									 const char *host,
									 const char *ipaddr
									 ) = 0;
									 
	
}; 

class jdk_nameserver_worker : public jdk_thread 
{
  jdk_nameserver_worker( const jdk_nameserver_worker & );
  const jdk_nameserver_worker operator = ( const jdk_nameserver_worker & );
public:	
	jdk_nameserver_worker( 
						  jdk_client_socket *request_rd,
						  jdk_client_socket *response_wr
						  );
	
	virtual ~jdk_nameserver_worker();
	
protected:

	void main();

	
	jdk_client_socket *request_rd;
	jdk_client_socket *response_wr;
	
}; 

#endif

#endif
#endif
