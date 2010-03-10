#ifndef _JDK_HTTP_ADMIN_H
#define _JDK_HTTP_ADMIN_H

#include "jdk_string.h"
#include "jdk_util.h"
#include "jdk_list.h"
#include "jdk_http.h"
#include "jdk_html.h"
#include "jdk_inet_server.h"


class jdk_http_admin_thread;
class jdk_http_admin_handler_shared;
class jdk_http_admin_handler_base;
class jdk_http_admin_shared_param;
class jdk_http_admin_shared;


class jdk_http_admin_shared_param
{
public:
    jdk_http_admin_shared_param( int max_handlers ) 
        : handlers( max_handlers )
	{
	}
	
    ~jdk_http_admin_shared_param();
    
    bool add_handler( jdk_http_admin_handler_base *h )
    {
        return handlers.add( h );
    }

    jdk_list< jdk_http_admin_handler_base > handlers;
};


class jdk_http_admin_shared
{
public:
	explicit jdk_http_admin_shared( jdk_http_admin_shared_param &params_ )
	    : params( params_ )
	{
		
	}

	~jdk_http_admin_shared()
	{
	}

    jdk_http_admin_shared_param &params;
};


class jdk_http_admin_handler_base
{
public:
	virtual ~jdk_http_admin_handler_base() 
	{
	}
	
	virtual bool handle_http_request( 
									 const jdk_http_request_header &request,
									 const jdk_dynbuf &request_content,
									 jdk_http_response_header &response_header,
									 jdk_dynbuf &response_content
									 ) = 0;
	
};


class jdk_http_admin_worker : public jdk_inet_server_worker
{
public:
    jdk_http_admin_worker( jdk_http_admin_shared &shared_, int id_ )
        : 
	    shared( shared_ ),
	    id( id_ )
	{
	}
	
	void run()
	{
	}
	
	void update()
	{
	}
	
	bool handle_connection( jdk_client_socket &sock )
	{
	    // grab data until request header is parsed
	    request.clear();
	    int len;
	    
	    while( (len=sock.read_data( buf, sizeof(buf) ))>0 )
	    {
	        if( request_header.parse_buffer( buf, len ) )
		    {
		        request_content.clear();
				
				int post_data_content_length=0;
				int remaining_post_data_content_length=0;
					
				if( request_header.get_request_type() == request.REQUEST_POST )
				{
						// extract additional data from tmp_buf and read the rest of the post data into request_buf
						const jdk_http_request_header::value *v;
						if( request_header.find_entry("Content-Length:", &v )!=-1 )
						{
							post_data_content_length = v->strtol();
							remaining_post_data_content_length = post_data_content_length;
						}
						else
						{
							post_data_content_length = 0;
							remaining_post_data_content_length = 0;
						}

						// yup, we may still have data in buf[] that needs to be sent as well.
						int pos = request_header.get_buf_pos();
						int len = tmp_buf_len - pos;
						if( len>0 && post_data_content_length>0 )
						{
							request_content.resize( post_data_content_length );
						
							request_content.append_data(
										&tmp_buf[pos],
										len
									);
									
							remaining_post_data_content_length = post_data_content_length - len;
							
							int additional_len = sock.read_data( request_content.get_data()+len, remaining_post_data_content_length );
							if( additional_len!=remaining_post_data_content_length )
								return false;
							request_content.set_data_length( post_data_content_length );
						}
						else
						{
//							jdk_log_debug3( "(%d) %s no pre-sending of POST data", my_id, incoming_addr.c_str()  );
						}
						
					}
					
			    response_header.clear();
			    response_content.clear();
			    
			    for( int i=0; i<shared.params.count(); ++i )
			    {
			        jdk_http_admin_handler_base *b = shared.params.get( i );
				    if( b )
				    {
				        if( b->handle_http_request( 
					                                request,
									                request_content,
											        response,
												    response_content
												    ) )
						{
						    break;
						}
				    }
			    }
			    
			    if( response.is_valid())
				{
					response.flatten( header_response );
					sock.write_data_block( header_response );
					sock.write_data_block( response_content ); 				
				}
				else
			    {
					int code=404;
					sprintf( buf, "HTTP/1.0 %d Code\r\nConnection: close\r\nContent-type: text\r\n\r\nError %d\n", code, code );
					if( !sock.write_string_block( buf ) )
						return false;				
			    }
			    
		    }
	    }
	    
	    return true;
	}
	
protected:
    jdk_http_admin_shared &shared;
    int id;
    char buf[4096];
    jdk_http_request_header request;
    jdk_dynbuf request_content;
    jdk_http_response_header response;
    jdk_dynbuf response_content;
    jdk_dynbuf header_response;
}

#endif
