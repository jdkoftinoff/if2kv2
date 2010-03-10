#ifndef _IF2K_TUNNEL_H
#define _IF2K_TUNNEL_H

#include "jdk_string.h"
#include "jdk_settings.h"
#include "if2k_httpproxy.h"



class if2k_tunnel
{
public:
	if2k_tunnel( 
				int my_id_, 
				const jdk_settings &settings_,
				if2k_http_proxy_shared &shared_,
				volatile bool &please_stop_,
				const jdk_str<64> &tunnel_name_
			)
		:
		my_id( my_id_ ),
		settings( settings_ ),
		shared( shared_ ),
		please_stop( please_stop_ ),
		tunnel_name( tunnel_name_ )
	{	
	}

	virtual ~if2k_tunnel()
	{
	}

	virtual void dispatch( 
						jdk_inet_client_socket &incoming,
						unsigned char *ip,
						unsigned short port,
					    int uid,
						int pid
			)
	{	
		jdk_str<64> host_and_port;
		host_and_port.form( "%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], port );
			
		jdk_log_debug1( "(%d): tunnel %s going to %s from uid %d pid %d", my_id, tunnel_name.c_str(), host_and_port.c_str(), uid, pid );
		jdk_inet_client_socket outgoing;
		if( outgoing.make_connection( host_and_port, 0, false ) )
		{	
			jdk_socket_transfer( my_id, &incoming, &outgoing, false, 0, 0 );	
		}
		jdk_log_debug1( "(%d): Ending tunnel %s going to %s from uid %d pid %d", my_id, tunnel_name.c_str(), host_and_port.c_str(), uid, pid );
		
	}

protected:

	virtual int process_incoming(
						  const unsigned char *inbuf,
						  int inbuf_size,
						  unsigned char *outbuf,
						  int max_outbuf_size
			) = 0;

	virtual int process_outgoing(
						  const unsigned char *inbuf,
						  int inbuf_size,
						  unsigned char *outbuf,
						  int max_outbuf_size
			) = 0;
	
	int my_id;
	const jdk_settings &settings;
	if2k_http_proxy_shared &shared;	
	volatile bool &please_stop;
	jdk_str<64> tunnel_name;
};




class if2k_tunnel_clear : public if2k_tunnel
{
public:
	if2k_tunnel_clear( 
						int my_id_,
						const jdk_settings &settings_,
						if2k_http_proxy_shared &shared_,
						volatile bool &please_stop_,
						const jdk_str<64> &tunnel_name_
			)
		:
		if2k_tunnel( my_id_, settings_, shared_, please_stop_, tunnel_name_ )
	{	
	}

					
protected:
	virtual int process_incoming(
						  const unsigned char *inbuf,
						  int inbuf_size,
						  unsigned char *outbuf,
						  int max_outbuf_size
			)
	{
		memcpy( outbuf, inbuf, inbuf_size );
		return inbuf_size;
	}
	
	virtual int process_outgoing(
						  const unsigned char *inbuf,
						  int inbuf_size,
						  unsigned char *outbuf,
						  int max_outbuf_size
			)
	{
		memcpy( outbuf, inbuf, inbuf_size );
		return inbuf_size;
	}

};

typedef if2k_tunnel_clear if2k_tunnel_https;



class if2k_tunnel_censored : public if2k_tunnel
{
public:
	if2k_tunnel_censored( 
						int my_id_,
						const jdk_settings &settings_,
						if2k_http_proxy_shared &shared_,
						volatile bool &please_stop_,
						const jdk_str<64> &tunnel_name_
			)
		:
		if2k_tunnel( my_id_, settings_, shared_, please_stop_, tunnel_name_ )
	{	
	}

					
protected:
	virtual int process_incoming(
						  const unsigned char *inbuf,
						  int inbuf_size,
						  unsigned char *outbuf,
						  int max_outbuf_size
			)
	{
		memcpy( outbuf, inbuf, inbuf_size );
		return inbuf_size;
	}
	
	virtual int process_outgoing(
						  const unsigned char *inbuf,
						  int inbuf_size,
						  unsigned char *outbuf,
						  int max_outbuf_size
			)
	{
		memcpy( outbuf, inbuf, inbuf_size );
		return inbuf_size;
	}

};

typedef if2k_tunnel_censored if2k_tunnel_nntp;
typedef if2k_tunnel_censored if2k_tunnel_smtp;
typedef if2k_tunnel_censored if2k_tunnel_pop3;
typedef if2k_tunnel_censored if2k_tunnel_gnutella;





class if2k_multi_tunnel 
{
public:
	if2k_multi_tunnel( 
			int my_id_, 
			if2k_http_proxy_worker &worker_,
			const jdk_settings &settings_,
			if2k_http_proxy_shared &shared_,
			volatile bool &please_stop_
			)
		:
		clear_tunnel( my_id_, settings_, shared_, please_stop_, "clear" ),
		https_tunnel( my_id_, settings_, shared_, please_stop_, "https" ),
		censored_tunnel( my_id_, settings_, shared_, please_stop_, "generic" ),
		nntp_tunnel( my_id_, settings_, shared_, please_stop_, "nntp" ),
		smtp_tunnel( my_id_, settings_, shared_, please_stop_, "smtp" ),
		pop3_tunnel( my_id_, settings_, shared_, please_stop_, "pop3" ),
		gnutella_tunnel( my_id_, settings_, shared_, please_stop_, "gnutella" ),	
		worker( worker_ )
	{

	}

	void dispatch( 
				jdk_inet_client_socket &incoming, 
				unsigned char *ip, 
				unsigned short port, 
				int uid,
				int pid 
				)
	{
		if( port==80 || port==8000 || port==3128 || port==8080 )
		{
			jdk_inet_client_socket outgoing;
			worker.handle_incoming_connection( &incoming, &outgoing );
		}
		else if( port==443 )
		{
			https_tunnel.dispatch( incoming, ip, port, uid, pid );
		}
		else if( port==119 )
		{
			nntp_tunnel.dispatch( incoming, ip, port, uid, pid );
		}
		else if( port==25 )
		{
			smtp_tunnel.dispatch( incoming, ip, port, uid, pid );
		}
		else if( port==109 || port==110 )
		{
			pop3_tunnel.dispatch( incoming, ip, port, uid, pid );
		}
		else if( port==6346 )
		{
			gnutella_tunnel.dispatch( incoming, ip, port, uid, pid );	
		}
		else
		{
			clear_tunnel.dispatch( incoming, ip, port, uid, pid );
		}
	}
protected:
	if2k_tunnel_clear clear_tunnel;
	if2k_tunnel_https https_tunnel;
	if2k_tunnel_censored censored_tunnel;
	if2k_tunnel_nntp nntp_tunnel;
	if2k_tunnel_smtp smtp_tunnel;
	if2k_tunnel_pop3 pop3_tunnel;
	if2k_tunnel_gnutella gnutella_tunnel;
	
	if2k_http_proxy_worker &worker;
};

#endif
