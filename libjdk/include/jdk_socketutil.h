#ifndef _JDK_SOCKETUTIL_H
#define _JDK_SOCKETUTIL_H

#include "jdk_socket.h"

class jdk_socket_transfer_processor
{
public:
	jdk_socket_transfer_processor()
	{		
	}
	
	virtual ~jdk_socket_transfer_processor()
	{
	}
	
	virtual int scan_incoming( void *buf, int data_len, int max_buf_size ) = 0;
	virtual int scan_outgoing( void *buf, int data_len, int max_buf_size ) = 0;
};

class jdk_socket_null_transfer_processor : public jdk_socket_transfer_processor
{
public:
	int scan_incoming( void *buf, int data_len, int max_buf_size )
	{
		return data_len;	
	}
	
	int scan_outgoing( void *buf, int data_len, int max_buf_size )
	{
		return data_len;	
	}
	
};



void jdk_socket_transfer( 
						 int my_id, 
						 jdk_client_socket *in, 
						 jdk_client_socket *out,
						 bool do_dumps = false,
						 FILE *incoming_dump = 0,
						 FILE *outgoing_dump = 0
						 );

int jdk_socket_transfer_one_block( 
								   int my_id, 
								   jdk_client_socket *in, 
								   jdk_client_socket *out,
								   int bytes_to_transfer,
								   bool direction,
								   bool do_dumps=false,
								   FILE *dumpfile=0
								   );

int jdk_socket_transfer_one_block_with_processing(
												  int my_id,
												  jdk_client_socket *in,
												  jdk_client_socket *out,
												  int bytes_to_transfer,
												  bool direction,
 												  jdk_socket_transfer_processor *proc,
												  bool *in_closed_flag=0,
												  bool do_dumps=false,
												  FILE *dumpfile=0
												  );


void jdk_socket_transfer_with_processing(
										 int my_id, 
										 jdk_client_socket *in, 
										 jdk_client_socket *out,
										 jdk_socket_transfer_processor *proc,
										 bool do_dumps=false,
										 FILE *incoming_dump=0,
										 FILE *outgoing_dump=0
										 );



#endif

