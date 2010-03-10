#ifndef _IF2K_ADMIN_H
#define _IF2K_ADMIN_H

#include "if2k_config.h"
#include "if2k_serial.h"
#include "if2k_svc.h"
#include "jdk_questions.h"
#include "jdk_html.h"
#include "jdk_http_server.h"
#include "jdk_util.h"
#include "jdk_string.h"

class if2k_html_style : public jdk_html_style_simple
{
public:
	if2k_html_style( jdk_settings &settings, const jdk_string &hostnameport_ )
		: 
		    text_font( settings, "text_font" ),
		    text_css( settings, "text_css" ),
			product_title( settings, "text_product_title1" ),
			product_homepage( settings, "text_product_homepage" ),
			product_logo( settings, "text_product_logo" ),
			product_links( settings, "text_product_links" ),
			body_options( settings, "text_body_options" ),
		    internal_hostname( hostnameport_ ) 
	{
	}
	
	virtual ~if2k_html_style();
	
	const jdk_string &get_internal_hostname() const
	{
		return internal_hostname;
	}

	jdk_html_chunk *head( jdk_html_chunk *contents, jdk_html_chunk *next ) const;
	jdk_html_chunk *body(  const char *options, jdk_html_chunk *contents, jdk_html_chunk *next ) const;

	virtual jdk_html_chunk *navigation_bar( jdk_html_chunk *next=0 ) const;

	jdk_html_chunk *header( jdk_html_chunk *next ) const;
	jdk_html_chunk *footer( jdk_html_chunk *next ) const;
private:
	static const char * my_stylesheet;
	jdk_live_string< jdk_str<128> > text_font;
	jdk_live_string< jdk_str<4096> > text_css;
	jdk_live_string< jdk_str<1024> > product_title;
	jdk_live_string_url product_homepage;
	jdk_live_string_url product_logo;
	jdk_live_long product_links;
	jdk_live_string< jdk_str<1024> > body_options;	
	jdk_str<256> internal_hostname;
};

class if2k_generator_lua : public jdk_http_server_generator
{
public:
	if2k_generator_lua( if2k_lua_interpreter &lua_, const char *prefix_ )
	: lua(lua_), prefix( prefix_ )
	{
	}

	virtual ~if2k_generator_lua()
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
							 )
	{
		if( request.get_request_type() != jdk_http_request_header::REQUEST_GET 
		&&  request.get_request_type() != jdk_http_request_header::REQUEST_POST
		&&  request.get_request_type() != jdk_http_request_header::REQUEST_PUT )
		{
			return false;
		}
		
		// copy the filename
		jdk_str<4096> fname( request.get_url().path );
		
		if( fname.ncmp( prefix, prefix.len() )!=0 )
		{
			return false;
		}

		// skip the prefix
		fname.cpy( fname.c_str()+prefix.len() );

		if2k_lua_web_response lua_response(response,response_data);
		
		// try call the selected script
		bool script_success = false;

        // find the start of cgi parameters
		char *rawcgi=fname.chr('?');

		jdk_settings_text cgiinfo;

		if( rawcgi )
		{
			jdk_cgi_loadsettings( &cgiinfo, rawcgi+1 );
			// nullify the '?' to get just the plain cgi function name
			*rawcgi='\0';
		}
			
		if( request.get_request_type() == jdk_http_request_header::REQUEST_GET )
		{
			script_success = lua.if_cgi_get(
									fname,
									lua_response,
									connection_id,
									request,
									cgiinfo
									);
		}
		else if( request.get_request_type() == jdk_http_request_header::REQUEST_POST )
		{
			script_success = lua.if_cgi_post(
				  fname,
				  lua_response,
				  connection_id,
				  request,
				  request_data
				  );
		}
		else if( request.get_request_type() == jdk_http_request_header::REQUEST_PUT )
		{
			script_success = lua.if_cgi_put(
									fname,
									lua_response,
									connection_id,
									request,
									request_data,
									cgiinfo
									);
		}
		
		
		if( script_success )
		{
			response.add_entry(	"Pragma:", "no-cache" );
			response.add_entry(	"Cache-Control:", "no-cache" );
			response.add_entry( "Expires:", "-1" );
			jdk_log_debug1( "response length is %d", lua_response.contents.get_data_length() );

			return true;
		}
		else
		{
			return false;
		}
	}
	
	
private:
	if2k_lua_interpreter &lua;
	const jdk_str<512> prefix;
};

class if2k_generator_lua_error : public jdk_http_server_generator
{
public:
	if2k_generator_lua_error( if2k_lua_interpreter &lua_, const char *error_type_ )
		: lua(lua_), error_type( error_type_ )
	{
	}

	virtual ~if2k_generator_lua_error()
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
							 )
	{		
		// try call the selected script
		bool script_success = false;

		if2k_lua_web_response lua_response(response,response_data);
			
		script_success = lua.if_url_error(
			error_type,
			lua_response,
			connection_id,
			request
			);
		
		if( script_success )
		{
			response.add_entry(	"Pragma:", "no-cache" );
			response.add_entry(	"Cache-Control:", "no-cache" );
			response.add_entry( "Expires:", "-1" );
			jdk_log_debug1( "response length is %d", lua_response.contents.get_data_length() );

			return true;
		}
		else
		{
			return false;
		}
	}
	
	
private:
	if2k_lua_interpreter &lua;
	const jdk_str<512> error_type;
};


class if2k_generator_404 : public jdk_http_server_generator_404
{
public:
    if2k_generator_404( 
						jdk_html_style *style_,
						jdk_settings &settings
						)					
        : 
		jdk_http_server_generator_404( style_ ),
		errormsg( settings, "text_404_page_errormsg" ),
		loginmsg( settings, "text_loginmsg" ),
		loginlink( settings, "text_loginlink" ),
		enable_login( settings, "enable_login" )		
	{
	}

protected:
	jdk_live_string< jdk_str<1024> > errormsg;
	jdk_live_string< jdk_str<1024> > loginmsg;	
	jdk_live_string< jdk_str<1024> > loginlink;
	jdk_live_long enable_login;
	
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
							errormsg.get())),
				enable_login ?
				style.p(
					style.font("SIZE=+2",
						style.link(
							loginlink.get().c_str(),
							style.text(
								loginmsg.get()))),	
				next)
				:
				next));							
	}
};					



class if2k_blockpage_document_generator : public jdk_html_document_generator
{
public:
	if2k_blockpage_document_generator( 
										const jdk_html_style &style_, 
										const jdk_settings &cgiparams_,
										const jdk_http_request_header &request_,
										const jdk_settings &settings_,
										const jdk_string &override_url_
										)
	  : 
	jdk_html_document_generator( style_ ), 
	cgiparams( cgiparams_ ), 
	request(request_), 
	settings(settings_),
	override_url(override_url_),
    login_url(),
	loginmsg( settings, "text_loginmsg" ),
	loginlink( settings, "text_loginlink" ),
	text_block_invalid( settings, "text_block_invalid" ),
	block_info1( settings, "text_block_info1" ),
	block_info2( settings, "text_block_info2" ),
	block_info3( settings, "text_block_info3" ),
	text_block_override( settings, "text_block_override" ),
	text_block_override_click( settings, "text_block_override_click" ),
	text_block_override_warn(settings, "text_block_override_warn" )	
	{
	}
	
	virtual jdk_html_chunk *generate_head( jdk_html_chunk *next );
	jdk_html_chunk *generate_violation_info( jdk_html_chunk *next );	
	virtual jdk_html_chunk *generate_admin_section( jdk_html_chunk *next );		
	jdk_html_chunk *generate_content( jdk_html_chunk *next );			 
	jdk_html_chunk *generate_header( jdk_html_chunk *next );    
	jdk_html_chunk *generate_footer( jdk_html_chunk *next );	
private:
	const jdk_settings &cgiparams;
	const jdk_http_request_header &request;
	const jdk_settings &settings;

	jdk_string_url override_url;	
	jdk_string_url login_url;
	jdk_live_string< jdk_str<1024> > loginmsg;	
	jdk_live_string< jdk_str<1024> > loginlink;	
	jdk_live_string< jdk_str<1024> > text_block_invalid;	
	jdk_live_string< jdk_str<1024> > block_info1;	
	jdk_live_string< jdk_str<1024> > block_info2;	
	jdk_live_string< jdk_str<1024> > block_info3;	
	jdk_live_string< jdk_str<1024> > text_block_override;	
	jdk_live_string< jdk_str<1024> > text_block_override_click;	
	jdk_live_string< jdk_str<1024> > text_block_override_warn;
};

class if2k_http_generator_getsettings : public jdk_http_server_generator
{
public:
	if2k_http_generator_getsettings(
				                    const char *prefix_,
							        const jdk_settings &settings_
								)
			:
			prefix( prefix_ ),
			settings( settings_ )
	{
	}
	
	virtual ~if2k_http_generator_getsettings()
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
								);

private:
	jdk_str<256> prefix;
	const jdk_settings &settings;
};



class if2k_http_generator_getfile : public jdk_http_server_generator
{
public:
	if2k_http_generator_getfile(
				                    const char *prefix_,
							        const jdk_settings &settings_
								)
			:
			prefix( prefix_ ),
			settings( settings_ )
	{
	}
	
	virtual ~if2k_http_generator_getfile()
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
								);

private:
	jdk_str<256> prefix;
	const jdk_settings &settings;
};


class if2k_http_generator_blockpage : public jdk_http_server_generator_html
{
public:
    if2k_http_generator_blockpage( 
				                    const char *prefix_,
						            jdk_html_style *style_,
							        const jdk_settings &settings_
						        )    
        : 
	    jdk_http_server_generator_html( prefix_, style_ ),
	    settings(settings_),	   
	    override_url( settings, "blocked_page_override_url" ),
	    login_url( settings, "text_loginlink" )
	{
	}
	
	  
protected:
     
    jdk_html_chunk *
    create_html(	
                      const char *fname,
                      const jdk_http_request_header &request,
				      const jdk_dynbuf &request_data,
					  jdk_http_response_header &response,
					  const jdk_string &connection_id
					);

    const jdk_settings &settings;
    jdk_live_string_url override_url;
    jdk_live_string_url login_url;
};

class if2k_http_generator_override : public jdk_http_server_generator
{
public:
	if2k_http_generator_override( 
	                                jdk_settings &settings_,
							        const char *prefix_ 
								)
	  : 
	settings( settings_ ),
	prefix( prefix_ )
	{		
	}
	
	
	// handle_request returns true if it knows how to handle this request
	// returns false if it should be given to a different generator
	bool handle_request(
								const jdk_http_request_header &request,
								const jdk_dynbuf &request_data,
								jdk_http_response_header &response,
								jdk_dynbuf &response_data,
								const jdk_string &connection_id
								);
	
private:
	jdk_settings &settings;
	jdk_str<256> prefix;
};

class if2k_adminlogin_document_generator : public jdk_html_document_generator
{
public:
	if2k_adminlogin_document_generator( 
										const jdk_html_style &style_, 
										const jdk_settings &cgiparams_,
										const jdk_http_request_header &request_,
										const jdk_settings &settings_,
										const jdk_string &action_url_
										)
	  : 
	jdk_html_document_generator( style_ ), 
	cgiparams( cgiparams_ ), 
	request(request_), 
	settings(settings_),
	action_url(action_url_),
	product_name( settings, "product_name" ),
	text_login_title( settings, "text_login_title" ),
	text_login_username( settings, "text_login_username" ),
	text_login_password( settings, "text_login_password" )
	{
	}
	
	jdk_html_chunk *generate_head( jdk_html_chunk *next );
	jdk_html_chunk *generate_content( jdk_html_chunk *next );			 
	jdk_html_chunk *generate_header( jdk_html_chunk *next );
	jdk_html_chunk *generate_footer( jdk_html_chunk *next );
	
private:
	const jdk_settings &cgiparams;
	const jdk_http_request_header &request;
	const jdk_settings &settings;
	const jdk_str<1024> action_url;
	jdk_live_string< jdk_str<1024> > product_name;	
	jdk_live_string< jdk_str<1024> > text_login_title;
	jdk_live_string< jdk_str<1024> > text_login_username;
	jdk_live_string< jdk_str<1024> > text_login_password;
};



class if2k_http_generator_adminlogin : public jdk_http_server_generator_html
{
public:
    if2k_http_generator_adminlogin( 
				                    const char *prefix_,
						            jdk_html_style *style_,
							        const jdk_settings &settings_,
								    const char *action_url_
						        )    
        : 
	    jdk_http_server_generator_html( prefix_, style_ ),
	    settings(settings_),
	    action_url( action_url_ )
	{
	}
	
protected:
     
    jdk_html_chunk *
    create_html(	
                      const char *fname,
                      const jdk_http_request_header &request,
				      const jdk_dynbuf &request_data,
					  jdk_http_response_header &response,
					  const jdk_string &connection_id
					);

    const jdk_settings &settings;
    jdk_str<4096> action_url;
};

class if2k_questions_asker : public jdk_questions_asker_html_generator_simple
{
public:
    if2k_questions_asker(    
	                            const jdk_html_style &style_, 
				                const jdk_settings_text &cgiparams_,
						        const jdk_http_request_header &request_,
            	    			const jdk_question *question_list_,
        	    				const jdk_settings_text &defaults_,
						        const jdk_string &action_url_,
							    const jdk_string &submit_label_,
								const jdk_settings &settings
        						)
	:
	    jdk_questions_asker_html_generator_simple(
	        style_, cgiparams_, request_, question_list_, defaults_, action_url_, submit_label_
		),
		text_title(settings,"text_filter_settings_title"),
		text_header(settings,"text_filter_settings_header")
    {    
    }

	jdk_html_chunk *generate_head( jdk_html_chunk *next );

	jdk_html_chunk *generate_header( jdk_html_chunk *next );
	
    jdk_html_chunk *generate_footer( jdk_html_chunk *next );

protected:
	jdk_live_string< jdk_str<1024> > text_title;
	jdk_live_string< jdk_str<1024> > text_header;

};



class if2k_http_administration : public jdk_http_server_post_validator
{
public:
    if2k_http_administration( 
                                const jdk_settings_text &settings_,
				                const char *submit_url_
				                )
        : 
		settings( settings_ ), 
		submit_url( submit_url_ ), 	    
		login_url( settings, "text_loginlink" ),
		text_loginmsg( settings, "text_loginmsg" ),
		text_login_reject( settings, "text_login_reject" ),
		text_login_error( settings, "text_login_error" )
    {
    }
    
    
    validation_response post_validate(
                                     const jdk_html_style &style, 
                        	         const jdk_settings_text &postinfo,
                                     const jdk_http_request_header &request,
				                     const jdk_string &connection_id
				                     );
protected:
    const jdk_settings_text &settings;	         
    const jdk_string_url submit_url;
    jdk_live_string_url login_url;	
	jdk_live_string< jdk_str<1024> > text_loginmsg;
	jdk_live_string< jdk_str<1024> > text_login_reject;
	jdk_live_string< jdk_str<1024> > text_login_error;
};


class if2k_http_generator_adminset : public jdk_http_server_generator_html
{
public:
    if2k_http_generator_adminset( 
				                    const char *prefix_,
						            jdk_html_style *style_,
							        const jdk_settings &settings_
						        )    
        : 
	    jdk_http_server_generator_html( prefix_, style_ ),
	    settings(settings_),
		login_url( settings, "text_loginlink" ),
		text_loginmsg( settings, "text_loginmsg" ),
		text_set_demo( settings, "text_set_demo" ),
		text_set_good( settings, "text_set_good" )
	{
	}
	
protected:
     
    jdk_html_chunk *
    create_html(	
                      const char *fname,
                      const jdk_http_request_header &request,
				      const jdk_dynbuf &request_data,
					  jdk_http_response_header &response,
					  const jdk_string &connection_id
					);

    const jdk_settings &settings;
    jdk_live_string_url login_url;	
	jdk_live_string< jdk_str<1024> > text_loginmsg;
	jdk_live_string< jdk_str<1024> > text_set_demo;
	jdk_live_string< jdk_str<1024> > text_set_good;
};



#endif
