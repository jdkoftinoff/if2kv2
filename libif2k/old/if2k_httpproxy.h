#ifndef _IF2K_HTTPPROXY_H
#define _IF2K_HTTPPROXY_H

#include "jdk_string.h"
#include "jdk_settings.h"
#include "jdk_socket.h"
#include "jdk_thread.h"
#include "jdk_thread_server.h"
#include "jdk_dynbuf.h"
#include "jdk_linesplit.h"
#include "jdk_url.h"
#include "jdk_http.h"
#include "jdk_httprequest.h"
#include "jdk_socketutil.h"
#include "jdk_http_server.h"
#include "jdk_ipv4.h"
#include "jdk_html.h"
#include "if2k_lua.h"


class if2k_http_proxy_filter_base;
    class if2k_http_proxy_nullfilter;
class if2k_http_proxy_worker;
class if2k_http_proxy_shared;



class if2k_http_proxy_filter_base
{
public:
	if2k_http_proxy_filter_base()
	{
	}
	
	virtual ~if2k_http_proxy_filter_base()
	{
	}

    virtual bool run() = 0;         // called just after creation to start any worker threads required for the filter

    virtual bool update() = 0;      // called to reload possibly changed settings
	
	virtual bool generate_blocked_page_url(
										   jdk_string_url &blocked_page_url,
										   const jdk_http_request_header &request_header,
										   const jdk_string &connection_id,
										   int reason,
										   const jdk_string &match_string
										   ) = 0;	
										   
				
	// test_url() returns false if url is to be denied. returns true if it is to be redirected	
	virtual bool test_url(
					      const jdk_http_request_header &request,
						  const jdk_string_url &url, 		// the requested url
						  jdk_string_url &redirect_url,		// the url that will be used
						  jdk_string_url &redirect_proxy, 	// If non-blank, the proxy server to use
                          const jdk_string & connection_id,
						  jdk_string &match_phrase
						  ) = 0;

	// test_request_header() returns false if the request is to be denied.
	// If it returns true it must rewrite the request
	virtual bool test_request_header(
                                     jdk_http_request_header &http_request_inout, 	// the original (tweaked) request header
                                     const jdk_string &connection_id,
									 jdk_string &match_phrase
									 ) = 0;

	// test_response_header() returns false if we want to deny the request because of something in the http response header.
	// if it returns true it must rewrite the response

	virtual bool test_response_header(
									  jdk_http_response_header &http_response_inout,	 // the original response header
									  const jdk_string & connection_id,
									  jdk_string &match_phrase
									  ) = 0;

	// test_client_send_packet() returns true if the data buffer is OK to send to the server.
	// if it returns true, it will put the data, possibly modified with possibly different length
	// into output_buf.
	// If it returns false, the proxy will abort the connection immediately and instead return an error message to
	// the client.

	virtual bool test_client_send_packet(
										 const jdk_string_url &url,
										 const jdk_http_request_header &http_request,
										 const unsigned char *input_buf,
										 int input_buf_data_length,
										 jdk_dynbuf &output_buf,
                                         const jdk_string & connection_id,
										 jdk_string &match_phrase
										 ) = 0;

	// test_client_receive_packet() returns true if the data buffer is OK to send to the client.
	// if it returns false, it will put the data, possibly modified with possibly different length
	// into output_buf.
	// if it returns false, the proxy will abort the connection immediately. If it is early enough to
	// return an error instead (ie this was the first data packet received from the server), then
	// an error will be returned to the client.
	// if it wasn't the first data packet received then all we can do is disconnect the socket.

	virtual bool test_client_receive_packet(
											const jdk_string_url &url,
											const jdk_http_response_header &http_response,
											const unsigned char *input_buf,
											int input_buf_data_length,
											jdk_dynbuf &output_buf,
                                            const jdk_string & connection_id,
											jdk_string &match_phrase
											) = 0;
private:
    if2k_http_proxy_filter_base( const if2k_http_proxy_filter_base & ); // not allowed
    const if2k_http_proxy_filter_base & operator = ( const if2k_http_proxy_filter_base & ); // not allowed
};


class if2k_http_proxy_nullfilter : public if2k_http_proxy_filter_base
{
public:
	explicit if2k_http_proxy_nullfilter( const jdk_settings &settings);

	virtual ~if2k_http_proxy_nullfilter();	
	
    virtual bool run();        // called just after creation to start any worker threads required for the filter

    virtual bool update();      // called to reload possibly changed settings

	virtual bool generate_blocked_page_url(
										jdk_string_url &blocked_page_url,
										const jdk_http_request_header &request_header,
										const jdk_string &connection_id,
										int reason,
										const jdk_string &match_string
										)
	{
		return false;	
	}
	

	
	// test_url() returns false if url is to be denied. returns true if it is to be redirected	
	virtual bool test_url(
					      const jdk_http_request_header &request,
						  const jdk_string_url &url, 		// the requested url
						  jdk_string_url &redirect_url,		// the url that will be used
						  jdk_string_url &redirect_proxy, 	// If non-blank, the proxy server to use
                          const jdk_string & connection_id,
						  jdk_string &match_phrase
						  );

	// test_request_header() returns false if the request is to be denied.
	// If it returns true it must rewrite the request
	virtual bool test_request_header(
                                     jdk_http_request_header &http_request_inout, 	// the original (tweaked) request header
                                     const jdk_string & connection_id,
									 jdk_string &match_phrase
									 );

	// test_response_header() returns false if we want to deny the request because of something in the http response header.
	// if it returns true it must rewrite the response

	virtual bool test_response_header(
									  jdk_http_response_header &http_response_inout,	 // the original response header
                                      const jdk_string & connection_id,
									  jdk_string &match_phrase
									  );

	// test_client_send_packet() returns true if the data buffer is OK to send to the server.
	// if it returns true, it will put the data, possibly modified with possibly different length
	// into output_buf.
	// If it returns false, the proxy will abort the connection immediately and instead return an error message to
	// the client.

	virtual bool test_client_send_packet(
										 const jdk_string_url &url,
										 const jdk_http_request_header &http_request,
										 const unsigned char *input_buf,
										 int input_buf_data_length,
										 jdk_dynbuf &output_buf,
                                         const jdk_string & connection_id,
										 jdk_string &match_phrase
										 );

	// test_client_receive_packet() returns true if the data buffer is OK to send to the client.
	// if it returns false, it will put the data, possibly modified with possibly different length
	// into output_buf.
	// if it returns false, the proxy will abort the connection immediately. If it is early enough to
	// return an error instead (ie this was the first data packet received from the server), then
	// an error will be returned to the client.
	// if it wasn't the first data packet received then all we can do is disconnect the socket.

	virtual bool test_client_receive_packet(
										 const jdk_string_url &url,
										 const jdk_http_response_header &http_response,
										 const unsigned char *input_buf,
										 int input_buf_data_length,
										 jdk_dynbuf &output_buf,
										 const jdk_string & connection_id,
										 jdk_string &match_phrase
										 );
private:
    if2k_http_proxy_nullfilter( const if2k_http_proxy_nullfilter & ); // not allowed
    const if2k_http_proxy_nullfilter & operator = ( const if2k_http_proxy_nullfilter & ); // not allowed
};



// if2k_http_proxy_shared contains all the data common to all if2k_http_proxy_thread objects.
// All settings are read into variables here. The server socket is bound here. The
// dns cache object that is used by all threads is created here. Contents of all
// error files are found here.

class if2k_http_proxy_shared : public jdk_thread_shared
{
public:
    explicit if2k_http_proxy_shared( 
							   jdk_server_socket &server_socket_, 
							   jdk_server_socket &admin_socket_,
							   const jdk_settings &settings, 
							   if2k_http_proxy_filter_base *filter_,
							   jdk_http_server_generator_list *generators_,
							   if2k_lua_interpreter &lua_,	
							   jdk_http_server_generator *generate_connect_error_,
							   jdk_http_server_generator *generate_bad_request_error_
							   );

    explicit if2k_http_proxy_shared( 
							   jdk_server_socket &server_socket_, 
							   jdk_server_socket &admin_socket_,
							   const jdk_settings &settings, 
							   if2k_http_proxy_filter_base *filter_,
							   jdk_http_server_generator_list *generators_,
							   if2k_lua_interpreter &lua_,	
							   jdk_html_document_generator *html_generate_connect_error_,
							   jdk_html_document_generator *html_generate_bad_request_error_
							   );

	virtual ~if2k_http_proxy_shared();

	virtual bool run();
    virtual bool update();


    static char *default_connect_error;
    static char *default_bad_request;
    static char *default_redirect;

	
	jdk_server_socket &server_socket;
	jdk_server_socket &admin_socket;

	jdk_dns_cache dns_cache;

	jdk_string_url further_proxy;
	int max_retries;
	int retry_delay;

	const jdk_settings &settings;
	if2k_http_proxy_filter_base *filter;
		
	jdk_http_server_generator_list *generators;
	jdk_ipv4_ip_range *allowed_client_ips;
	jdk_ipv4_ip_range *blocked_client_ips;
	jdk_ipv4_ip_range *nofilter_client_ips;

	jdk_http_server_generator *generate_connect_error;
	jdk_http_server_generator *generate_bad_request_error;
	jdk_html_document_generator *html_generate_connect_error;
	jdk_html_document_generator *html_generate_bad_request_error;

	jdk_http_request_header::value_t my_proxy_id;	
	jdk_str<256> my_interface;
	int my_interface_port;
	jdk_str<256> my_host_name;

	if2k_lua_interpreter &lua;	
	jdk_recursivemutex hack_mutex;

private:
    if2k_http_proxy_shared( const if2k_http_proxy_shared & ); // not allowed
    const if2k_http_proxy_shared & operator = ( const if2k_http_proxy_shared & ); // not allowed
};

class if2k_http_proxy_xfer_processor : public jdk_socket_transfer_processor
{	
public:
	explicit if2k_http_proxy_xfer_processor( 
							  if2k_http_proxy_shared &shared_,
							  const jdk_string & connection_id_,
							  const jdk_string_url &url_,
							  const jdk_http_request_header &http_request_,
							  const jdk_http_response_header &http_response_,
							  bool nofilter_
							  ) 
	  : 
		shared( shared_ ),
		connection_id( connection_id_ ),
		url( url_ ),
		http_request( http_request_ ),
		http_response( http_response_ ),
        dummybuf(),
		nofilter( nofilter_),
        match_phrase()
	{		
	}

	virtual int scan_incoming( void *buf, int data_len, int max_buf_size )
	{
		if( nofilter )
		{
			return data_len;
		}
		
		bool r= shared.filter->test_client_send_packet( 
													   url, 
													   http_request, 
													   (const unsigned char *)buf, 
													   data_len, 
													   dummybuf,
													   connection_id,
													   match_phrase
													   );
		if(!r)
		  	return -1;
		else
		  	return data_len;
	}
	
	virtual int scan_outgoing( void *buf, int data_len, int max_buf_size )
	{
		if( nofilter )
		{
			return data_len;
		}
		
		bool r=shared.filter->test_client_receive_packet( 
														 url, 
														 http_response, 
														 (const unsigned char *)buf, 
														 data_len, 
														 dummybuf, 
														 connection_id,
														 match_phrase
														 );
		if( !r )
		  	return -1;
		else
			return data_len;
	}
	
	
private:	
	if2k_http_proxy_shared &shared;
	const jdk_string & connection_id;
	const jdk_string_url &url;
	const jdk_http_request_header &http_request;
	const jdk_http_response_header &http_response;
	jdk_dynbuf dummybuf;
	bool nofilter;
public:
	jdk_str<1024> match_phrase;
};


// if2k_http_proxy_worker does all the magic for a single http proxy connection

class if2k_http_proxy_worker_base
{
public:		
	if2k_http_proxy_worker_base() {}
    virtual ~if2k_http_proxy_worker_base() {}
	
	virtual void handle_incoming_connection(jdk_client_socket *incoming, jdk_client_socket *outgoing) = 0;
	
private:
    if2k_http_proxy_worker_base( const if2k_http_proxy_worker_base & ); // not allowed
    const if2k_http_proxy_worker_base & operator = ( const if2k_http_proxy_worker_base & ); // not allowed
};


class if2k_http_proxy_worker : public if2k_http_proxy_worker_base
{
public:		
	explicit if2k_http_proxy_worker( 
				 int my_id_, 
				 const jdk_settings &settings_,
				 if2k_http_proxy_shared &shared_
				 );
    virtual ~if2k_http_proxy_worker();
	
	virtual void handle_incoming_connection(jdk_client_socket *incoming, jdk_client_socket *outgoing );
	
protected:
	virtual bool read_http_header();
	virtual bool write_http_header();

	virtual void respond_with_bad_request_page();
	virtual void respond_with_connection_error_page(const jdk_string &url);
	virtual void respond_with_redirect_page( const jdk_string &url );
	virtual bool respond_with_template( jdk_dynbuf &t, int http_code, const jdk_string &param1, const jdk_string &param2 );

	virtual bool handle_generated();
    virtual bool handle_https_mode();
    virtual bool handle_http_mode();
    virtual bool handle_gopher_mode();
    virtual bool handle_ftp_mode();

    virtual bool receive_server_response();
    virtual bool transform_server_response();
    virtual bool send_server_response();

	virtual bool transfer_from_server();
    virtual bool transfer_to_server();
	virtual bool transform_http_request();
	virtual bool make_retried_cached_connection_with_request();
	virtual bool make_cached_connection();
		
	int my_id;
	const jdk_settings &settings;
	if2k_http_proxy_shared &shared;
	
	jdk_client_socket *incoming;
	jdk_client_socket *outgoing;
	jdk_http_request_header http_request;
	jdk_string_url current_url;
	jdk_dynbuf http_transformed_request;
	jdk_http_response_header http_response;
	jdk_dynbuf http_transformed_response;

	jdk_string_host cached_destination_host;
	int cached_destination_port;

	jdk_string_url transformed_url;
	jdk_string_url transformed_proxy;
	bool do_redirect;
	bool use_proxy;

	jdk_string_host incoming_addr;
	jdk_string_host outgoing_addr;
	
    char buf[4096];
    int buf_len;    	
    int post_data_presend_count;
    int post_data_content_length;
    int remaining_post_data_content_length;
    int response_data_content_length;
    int remaining_response_data_content_length;
	bool nofilter;	        
    bool keep_alive;
private:
    if2k_http_proxy_worker( const if2k_http_proxy_worker & ); // not allowed
    const if2k_http_proxy_worker & operator = ( const if2k_http_proxy_worker & ); // not allowed        
};

// if2k_http_proxy_thread is a thread that just accepts incoming connections and passes them
// to a if2k_http_proxy_worker



class if2k_http_proxy_thread : public jdk_thread
{
public:		
	explicit if2k_http_proxy_thread( 
				 int my_id_, 
				 const jdk_settings &settings_,
				 if2k_http_proxy_shared &shared_
				 )  
				 : 
		my_id( my_id_ ),
		settings( settings_ ),
		shared( shared_ ),
		worker( my_id_, settings_, shared_ ),
		admin_mode( my_id<8 )
    {    
    }

	~if2k_http_proxy_thread()
	{
		if( thread_hasstarted() && !thread_isdone() )
		{			
			please_stop=true;
			sleep(1);
		}
		
		//jdk_log( JDK_LOG_ERROR, "if2k_http_proxy_thread dying" );	
	}
	
	
protected:
	virtual void main()
	{
		jdk_log_debug3( "(%d) thread starting", my_id );				
    	while(!please_stop)
    	{			
			jdk_server_socket &serv = admin_mode ? shared.admin_socket : shared.server_socket;
			jdk_select_manager sel;			
			sel.add_read( serv );
			sel.set_timeout( 1 );
			if( sel.do_select()>0 && sel.can_read( serv ) )
			{
		        jdk_inet_client_socket incoming;		
				
	    		if( serv.accept( &incoming ) )
    			{
				    jdk_inet_client_socket outgoing;
					
    				worker.handle_incoming_connection(&incoming,&outgoing);
    			}
			}			
    	}
		jdk_log_debug3( "(%d) thread ending", my_id );
    }	

	int my_id;
	const jdk_settings &settings;
	if2k_http_proxy_shared &shared;
	
	if2k_http_proxy_worker worker;
	bool admin_mode;
private:
    if2k_http_proxy_thread( const if2k_http_proxy_thread & ); // not allowed
    const if2k_http_proxy_thread & operator = ( const if2k_http_proxy_thread & ); // not allowed        

        
};




class if2k_http_proxy_factory_base : 
    public jdk_thread_server_factory
{
public:
    explicit if2k_http_proxy_factory_base( 
								jdk_settings_text &orig_settings_,
								jdk_nb_inet_server_socket &server_socket_,
								jdk_nb_inet_server_socket &admin_socket_
			                )
					: 
		settings( orig_settings_ ), 
		server_socket(server_socket_),
		admin_socket(admin_socket_)
	{
	}

	bool run()
	{
		return true;
	}

    bool update()
    {
        return true;
    }
		
    
	jdk_thread *create_thread( jdk_thread_shared &shared, int id )
	{
	    return new if2k_http_proxy_thread( id, settings, *(if2k_http_proxy_shared *)&shared );
	}
	
	jdk_thread_pool_base *create_thread_pool()
	{
	    return new jdk_thread_pool( settings.get_long( "threads" ), *this );
	}

	jdk_thread_shared *create_thread_shared() = 0;

    jdk_thread_server_base *create_thread_server()
    {
        return new jdk_thread_server( *this );
    }
    
    jdk_server_socket &get_server_socket()
    {
        return server_socket;
    }		

	jdk_server_socket &get_admin_socket()
	{
		return admin_socket;
	}

protected:
    jdk_settings_text &settings;    
    
    jdk_nb_inet_server_socket &server_socket;
    jdk_nb_inet_server_socket &admin_socket;
	
private:
    if2k_http_proxy_factory_base( const if2k_http_proxy_factory_base & ); // not allowed
    const if2k_http_proxy_factory_base & operator = ( const if2k_http_proxy_factory_base & ); // not allowed     
};

#endif

