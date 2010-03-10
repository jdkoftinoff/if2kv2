#ifndef _IF2K_FILTER_H
#define _IF2K_FILTER_H

#include "if2k_svc.h"
#include "if2k_httpproxy.h"
#include "if2k_override_map.h"
#include "if2k_admin.h"
#include "if2k_admin_textedit.h"


class if2k_http_proxy_filter : public if2k_http_proxy_filter_base
{
  explicit if2k_http_proxy_filter( const if2k_http_proxy_filter & );
  const if2k_http_proxy_filter & operator = ( const if2k_http_proxy_filter & );
public:
	explicit if2k_http_proxy_filter(
						  const jdk_settings &settings_,
						  const jdk_string &internal_hostnameport_,
						  if2_kernel_standard &kernel_,
						  if2k_lua_interpreter &lua_
						  );
	
	virtual ~if2k_http_proxy_filter();

    virtual bool run();

    virtual bool update();

	virtual bool generate_blocked_page_url(
										   jdk_string_url &blocked_page_url,
										   const jdk_http_request_header &request_header,
										   const jdk_string & connection_id,
										   int reason,
										   const jdk_string &match_string
										);
				
	virtual bool test_url(
					      const jdk_http_request_header &request,
						  const jdk_string_url &url, 		// the original requested url
						  jdk_string_url &redirect_url,		// the url that will be used
						  jdk_string_url &redirect_proxy, 	// If non-blank, the proxy server to use
                          const jdk_string & connection_id,
						  jdk_string &match_phrase
		);	

	virtual bool test_request_header(
                                     jdk_http_request_header &http_request_inout, 	// the original (tweaked) request header
                                     const jdk_string & connection_id,
									 jdk_string &match_phrase
		);

	virtual bool test_response_header(
									  jdk_http_response_header &http_response_inout,	 // the original response header
                                      const jdk_string & connection_id,
									  jdk_string &match_phrase
		);
	

	virtual bool test_client_send_packet(
										 const jdk_string_url &url,
										 const jdk_http_request_header &http_request,
										 const unsigned char *input_buf,
										 int input_buf_data_length,
										 jdk_dynbuf &output_buf,
                                         const jdk_string & connection_id,
										 jdk_string &match_phrase
		);


	virtual bool test_client_receive_packet(
											const jdk_string_url &url,
											const jdk_http_response_header &http_response,
											const unsigned char *input_buf,
											int input_buf_data_length,
											jdk_dynbuf &output_buf,
                                            const jdk_string & connection_id,
											jdk_string &match_phrase
		);

protected:

    bool is_override_enabled(const jdk_string &connection_id ) const;

	const jdk_settings &settings;
	if2_kernel_standard &kernel;
	jdk_live_long use_internal_blocked_page;
	jdk_live_string_url blocked_page;
	jdk_live_long fancy_blocked_page;
	jdk_live_long allow_override;	
	jdk_live_long override_unknown_only;
	jdk_str<256> internal_hostnameport;


	if2k_lua_interpreter &lua;

};



class if2k_generator_connect_error : public jdk_html_document_generator
{
  explicit if2k_generator_connect_error( const if2k_generator_connect_error & );
  const if2k_generator_connect_error & operator = ( const if2k_generator_connect_error & );
public:
	explicit if2k_generator_connect_error( 
									jdk_settings &settings,
									jdk_html_style *my_style_ 
									)
	:
		jdk_html_document_generator( *my_style_ ),
		my_style( my_style_ ),
		text_connection_error( settings, "text_connection_error" ),
		text_connection_error_description( settings, "text_connection_error_description" )
	{
	}
	
	~if2k_generator_connect_error()
	{
		delete my_style;
	}
	

	jdk_html_chunk *generate_content( jdk_html_chunk *next )
	{
		return 
				style.center(
					style.p(
						style.font( "size=+3",
							style.italic(
								style.text( text_connection_error.get(),
								style.br("",
								style.text( text_connection_error_description.get() )))))),
				next);		
	}	
	
private:
	jdk_html_style *my_style;
	jdk_live_string< jdk_str<1024> > text_connection_error;
	jdk_live_string< jdk_str<1024> > text_connection_error_description;
};

class if2k_generator_bad_request_error : public jdk_html_document_generator
{
  explicit if2k_generator_bad_request_error( const if2k_generator_bad_request_error & );
  const if2k_generator_bad_request_error operator = ( const if2k_generator_bad_request_error & );
public:
	if2k_generator_bad_request_error( 
									jdk_settings &settings,
									jdk_html_style *my_style_ 
									)
	:
		jdk_html_document_generator( *my_style_ ),
		my_style( my_style_ ),
		text_bad_request_error( settings, "text_bad_request_error" )
	{
	}
	
	~if2k_generator_bad_request_error()
	{
		delete my_style;
	}

	jdk_html_chunk *generate_content( jdk_html_chunk *next )
	{
		return 
				style.center(
					style.p(
						style.font( "size=+3",
							style.italic(
								style.text(text_bad_request_error.get())))),
				next);		
	}	
	

	
private:
	jdk_html_style *my_style;
	jdk_live_string< jdk_str<1024> > text_bad_request_error;	
};


class if2k_http_proxy_factory : public if2k_http_proxy_factory_base
{
public:
	explicit if2k_http_proxy_factory( 
									   jdk_settings_text &settings_,
									   jdk_nb_inet_server_socket &server_socket_,
									   jdk_nb_inet_server_socket &admin_socket_
									   ) 
	  	: 
		if2k_http_proxy_factory_base( settings_, server_socket_, admin_socket_ ),
		kernel(settings_),
		lua(settings_)
	{
	}
	
	jdk_thread_shared *create_thread_shared();

	if2_kernel_standard kernel;
	if2k_lua_interpreter lua;
};


#endif


