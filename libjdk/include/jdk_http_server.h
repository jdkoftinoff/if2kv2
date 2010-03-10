#ifndef _JDK_HTTP_SERVER_H
#define _JDK_HTTP_SERVER_H

#include "jdk_string.h"
#include "jdk_list.h"
#include "jdk_log.h"
#include "jdk_url.h"
#include "jdk_http.h"
#include "jdk_thread_server.h"
#include "jdk_html.h"
#include "jdk_cgisettings.h"
#include "jdk_cgi.h"

class jdk_http_server_generator
{
public:
	jdk_http_server_generator()
	{
	}
	
	virtual ~jdk_http_server_generator()
	{
	}
	
	// handle_request returns true if it knows how to handle this request
	// returns false if it should be given to a different generator
	virtual bool handle_request(
								const jdk_http_request_header &request,
								const jdk_dynbuf &request_data,
								jdk_http_response_header &response,
								jdk_dynbuf &response_data,
								const jdk_string &connection_id
								) = 0;
};

typedef jdk_list<jdk_http_server_generator> jdk_http_server_generator_list;

class jdk_http_server_worker
{
public:
	jdk_http_server_worker( jdk_http_server_generator_list &generators_ );	
	virtual ~jdk_http_server_worker();
	virtual void handle_request( jdk_client_socket &sock );
		
protected:

	virtual bool read_request( jdk_client_socket &sock );
	virtual bool send_response(jdk_client_socket &sock );
	virtual bool send_error( jdk_client_socket &sock, int code );
	
	jdk_http_server_generator_list &generators;
	
	jdk_http_request_header request;
	jdk_dynbuf request_buf;
	jdk_http_response_header response;
	jdk_dynbuf response_flattened;
	jdk_dynbuf response_buf;
	char tmp_buf[4096];
	int tmp_buf_len;	
};


class jdk_http_generator_bindir : public jdk_http_server_generator
{
public:
	jdk_http_generator_bindir( const char *group_, const char *prefix_="/" );
	
	virtual bool handle_request(
								const jdk_http_request_header &request,
								const jdk_dynbuf &request_data,
								jdk_http_response_header &response,
								jdk_dynbuf &response_data,
								const jdk_string &connection_id
								);

	jdk_str<64> group;
	jdk_str<512> prefix;
};

class jdk_http_server_generator_raw_get : public jdk_http_server_generator
{
public:
    jdk_http_server_generator_raw_get( 
				                    const char *prefix_
						        )    
        : prefix( prefix_ )
	{
	}
	
    ~jdk_http_server_generator_raw_get()
    {
    }

	bool handle_request(
						const jdk_http_request_header &request,
						const jdk_dynbuf &request_data,
						jdk_http_response_header &response,
						jdk_dynbuf &response_data,
						const jdk_string &connection_id
						)
	{
		if( request.get_request_type() != jdk_http_request_header::REQUEST_GET )
		{
			return false;
		}

		const char *fname;
		
		if( request.get_url().path.ncmp( prefix, prefix.len() )==0 )
	    	fname = request.get_url().path.c_str()+prefix.len();
		else
		    return false;
	
		if( create_response( fname, request, request_data, response, response_data, connection_id ) )
		{		
		    response.set_http_version(jdk_str<16>("HTTP/1.0"));
		    response.set_http_response_code( 200 );
		    response.set_http_response_text( jdk_str<8>("OK") );
		    if( !response.has_entry( "Content-Type:" ) )
		    {
		    	response.add_entry(	"Content-Type:", "application/octet-stream" );		
			}
		    response.add_entry(	"Pragma:", "no-cache" );					
		    response.add_entry(	"Cache-Control:", "no-cache" );					
			response.add_entry( "Expires:", "-1" );

		    response.set_valid();
			return true;
	    }
	    else
	    {
	        return false;
	    }
	    
	}

protected:
	jdk_str<512> prefix;    
    
protected:

    virtual 
	bool create_response(
						const char *fname,
						const jdk_http_request_header &request,
						const jdk_dynbuf &request_data,
						jdk_http_response_header &response,
						jdk_dynbuf &response_data,
						const jdk_string &connection_id
						) = 0;
					
};

class jdk_http_server_generator_raw_put : public jdk_http_server_generator
{
public:
    jdk_http_server_generator_raw_put( 
				                    const char *prefix_
						        )    
        : prefix( prefix_ )
	{
	}
	
    ~jdk_http_server_generator_raw_put()
    {
    }

	bool handle_request(
						const jdk_http_request_header &request,
						const jdk_dynbuf &request_data,
						jdk_http_response_header &response,
						jdk_dynbuf &response_data,
						const jdk_string &connection_id
						)
	{
		if( request.get_request_type() != jdk_http_request_header::REQUEST_PUT )
		{
			return false;
		}

		const char *fname;
		if( request.get_url().path.ncmp( prefix, prefix.len() )==0 )
	    	fname = request.get_url().path.c_str()+prefix.len();
		else
		    return false;
	
		if( create_response( fname, request, request_data, connection_id ) )
		{		
		    response.set_http_version(jdk_str<16>("HTTP/1.0"));
		    response.set_http_response_code( 200 );
		    response.set_http_response_text( jdk_str<8>("OK") );

		    response.set_valid();
			return true;
	    }
	    else
	    {
	        return false;
	    }
	    
	}

protected:
	jdk_str<512> prefix;    
    
protected:

    virtual 
	bool create_response(
						const char *fname,
						const jdk_http_request_header &request,
						const jdk_dynbuf &request_data,
						const jdk_string &connection_id
						) = 0;
					
};




class jdk_http_server_generator_redirect : public jdk_http_server_generator
{
public:
    jdk_http_server_generator_redirect( 
				                    const char *src_,
									const char *dest_
						        )    
        : src( src_ ), dest( dest_ )
	{
	}
	
    ~jdk_http_server_generator_redirect()
    {
    }

	bool handle_request(
						const jdk_http_request_header &request,
						const jdk_dynbuf &request_data,
						jdk_http_response_header &response,
						jdk_dynbuf &response_data,
						const jdk_string &connection_id
						)
	{
		if( request.get_url().path.cmp( src )!=0 )
		    return false;

		response.set_http_version(jdk_str<16>("HTTP/1.0"));
		response.set_http_response_code( 302 );
		response.set_http_response_text( jdk_str<8>("MOVED") );
		response.add_entry( "Location:", dest );
		response.set_valid();
		return true;
	}

protected:
	jdk_str<512> src;    
	jdk_str<512> dest;    
};




class jdk_http_server_generator_html : public jdk_http_server_generator
{
  jdk_http_server_generator_html( const jdk_http_server_generator_html & );
  const jdk_http_server_generator_html & operator = ( const jdk_http_server_generator_html & );
  
public:
    jdk_http_server_generator_html( 
				                    const char *prefix_,
						            jdk_html_style *style_
						        )    
        : prefix( prefix_ ), mystyle( style_ ), style( *style_ )
	{
	}
	
    ~jdk_http_server_generator_html()
    {
        delete mystyle;
    }

	bool handle_request(
						const jdk_http_request_header &request,
						const jdk_dynbuf &request_data,
						jdk_http_response_header &response,
						jdk_dynbuf &response_data,
						const jdk_string &connection_id
						)
	{
		const char *fname;
		if( request.get_url().path.ncmp( prefix, prefix.len() )==0 )
	    	fname = request.get_url().path.c_str()+prefix.len();
		else
		    return false;
	
        jdk_html_chunk *chunk = create_html(
											fname, 
											request, 
											request_data,
											response, 
											connection_id 
											);
        if( chunk )
	    {
       		chunk->flatten( response_data );
    		delete chunk;
		    
		    response.set_http_version(jdk_str<16>("HTTP/1.0"));
		    response.set_http_response_code( 200 );
		    response.set_http_response_text( jdk_str<8>("OK") );
		    response.add_entry(	"Content-Type:", "text/html; charset=UTF-8" );		
		    response.add_entry(	"Pragma:", "no-cache" );					
		    response.add_entry(	"Cache-Control:", "no-cache" );					
			response.add_entry( "Expires:", "-1" );

		    response.set_valid();
			return true;
	    }
	    else
	    {
	        return false;
	    }
	    
	}

protected:
	jdk_str<512> prefix;    
	jdk_html_style *mystyle;	
	const jdk_html_style &style;	
    
protected:

    virtual 
    jdk_html_chunk *
    create_html(	
                      const char *fname,
                      const jdk_http_request_header &request,
				      const jdk_dynbuf &request_data,
					  jdk_http_response_header &response,
					  const jdk_string &connection_id
					) = 0;		
					
};

class jdk_http_server_generator_404 : public jdk_http_server_generator
{
  jdk_http_server_generator_404( const jdk_http_server_generator_404 & );
  const jdk_http_server_generator_404 & operator = ( const jdk_http_server_generator_404 & );

public:
    jdk_http_server_generator_404( 
						            jdk_html_style *style_
						        )    
        : mystyle( style_ ), style(*style_)
	{
	}
	
    ~jdk_http_server_generator_404()
    {
        delete mystyle;
    }

	bool handle_request(
						const jdk_http_request_header &request,
						const jdk_dynbuf &request_data,
						jdk_http_response_header &response,
						jdk_dynbuf &response_data,
						const jdk_string &connection_id
						)
	{		
        jdk_html_chunk *chunk = create_html( request.get_url().path.c_str(), request, request_data, connection_id );
        if( chunk )
	    {
       		chunk->flatten( response_data );
    		delete chunk;
		    
		    response.set_http_version(jdk_str<16>("HTTP/1.0"));
		    response.set_http_response_code( 404 );
		    response.set_http_response_text( jdk_str<16>("NOT FOUND") );
		    response.add_entry(	"Content-Type:", "text/html; charset=UTF-8" );		
		    response.add_entry(	"Pragma:", "no-cache" );					
		    response.add_entry(	"Cache-Control:", "no-cache" );					
			response.add_entry( "Expires:", "-1" );

		    response.set_valid();
			return true;
	    }
	    else
	    {
	        return false;
	    }	    
	}

protected:
	jdk_html_style *mystyle;
	const jdk_html_style &style;	
    
protected:

	virtual
	jdk_html_chunk *
	create_contents(const char *fname, jdk_html_chunk *next)
	{
		return 
			style.center(
				style.p(
					style.font("SIZE=+4",
						style.text(
							"Error 404: File Not Found")),
				next));							
	}
					

    virtual 
    jdk_html_chunk *
    create_html(	
                      const char *fname,
                      const jdk_http_request_header &request,
				      const jdk_dynbuf &request_data,
					  const jdk_string &connection_id
					)
	{
		return 
			style.doc(
				style.body("",
					style.header(
					create_contents(fname,
					style.footer()))));														
	}
					
};


template <class DOC>
class jdk_http_server_generator_html_single_questions : public jdk_http_server_generator_html
{
public:
    jdk_http_server_generator_html_single_questions( 
					    const char *prefix_,   
				        jdk_html_style *style_,					     
                        const jdk_question *question_list_,
			            const jdk_settings &defaults_,
				        const jdk_string &action_url_,
					    const jdk_string &submit_label_
				    )
    :
        jdk_http_server_generator_html( prefix_, style_ ),
	    question_list( question_list_ ),
	    defaults( defaults_ ),
	    action_url( action_url_ ),
	    submit_label( submit_label_ )
    {
    }
    
        
protected:

    virtual 
    jdk_html_chunk *
    create_html(	
                      const char *fname,
                      const jdk_http_request_header &request,
				      const jdk_dynbuf &request_data,
					  jdk_http_response_header &response,
					  const jdk_string &connection_id				      
					)
	{
	    if( *fname != '\0' )
	        return 0;
		
		jdk_settings postinfo;
		jdk_cgi_loadsettings( &postinfo, request_data );
				
		jdk_settings result;
		result.merge(defaults);
		jdk_settings prev_values;
		prev_values.merge(defaults);
		prev_values.merge(postinfo);
		
		DOC gen(
		    style,
		    postinfo,
		    request,
		    question_list,
		    prev_values,
		    action_url,
		    submit_label
		    );        	
	    return gen.generate_document();
	}

	const jdk_question *question_list;
    const jdk_settings &defaults;
    const jdk_str<1024> action_url;
    const jdk_str<128> submit_label;
};

class jdk_http_server_post_event
{
public:
    jdk_http_server_post_event()
    {
    }

    virtual ~jdk_http_server_post_event()
    {
    }    
    

    virtual bool post_event( const jdk_settings &postinfo, const jdk_string &connection_id ) = 0;			               
};


class jdk_http_server_post_validator
{
public:
    typedef jdk_pair<jdk_html_chunk *,bool> validation_response;
    
    jdk_http_server_post_validator()
    {
    }
    
    virtual ~jdk_http_server_post_validator()
    {
    }
    
    virtual
    validation_response
    post_validate(
             const jdk_html_style &style, 
	         const jdk_settings &postinfo,
             const jdk_http_request_header &request,
	         const jdk_string &connection_id
	        ) = 0;
};


class jdk_http_server_generator_html_post : public jdk_http_server_generator_html
{
  explicit jdk_http_server_generator_html_post( const jdk_http_server_generator_html_post &);
  const jdk_http_server_generator_html_post & operator = ( const jdk_http_server_generator_html_post & );
public:
    jdk_http_server_generator_html_post( 
					    const char *prefix_,
				        jdk_html_style *style_,					    
					    jdk_http_server_post_validator *validator_,
					    jdk_http_server_post_event *event_
				    )
    :
        jdk_http_server_generator_html( prefix_, style_ ),
	    validator(validator_),
	    event( event_ )
    {
    }
    
    ~jdk_http_server_generator_html_post()
    {
        delete validator;
	    delete event;
    }
        
protected:

    virtual 
    jdk_html_chunk *
    create_html(	
                  const char *fname,
                  const jdk_http_request_header &request,
                  const jdk_dynbuf &request_data,
                  jdk_http_response_header &response,
                  const jdk_string &connection_id
				)
	{
	    if( *fname != '\0' )
	        return 0;
			
		jdk_settings postinfo;
		jdk_cgi_loadsettings( &postinfo, request_data );

		jdk_http_server_post_validator::validation_response r =
		    validator->post_validate( style, postinfo, request, connection_id ); 
		
		if( r.value && event )
		{
		    // info is good, trigger the event
		    event->post_event(postinfo,connection_id);
		}
		
		return r.key;
	}


    jdk_http_server_post_validator *validator;
    jdk_http_server_post_event *event;
};


#endif






