#ifndef _IF2K_ADMIN_TEXTEDIT_H
#define _IF2K_ADMIN_TEXTEDIT_H

#include "if2k_config.h"
#include "jdk_html.h"
#include "jdk_http_server.h"
#include "jdk_util.h"
#include "jdk_string.h"
#include "jdk_settings.h"

class if2k_generator_textedit : public jdk_http_server_generator_html
{
public:
    if2k_generator_textedit( 
		const char *prefix_,
		jdk_html_style *style_,
		const jdk_settings &settings_,
		const char *settingname_of_file_to_edit,
		const char *settingname_of_title
		)    
        : 
	    jdk_http_server_generator_html( prefix_, style_ ),
	    settings(settings_),
		filename( settings_, settingname_of_file_to_edit ),
		title( settings_, settingname_of_title )
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
	jdk_live_string< jdk_str<1024> > filename;
	jdk_live_string< jdk_str<1024> > title;
};

class if2k_generator_textedit_set : public jdk_http_server_generator_html
{
public:
    if2k_generator_textedit_set( 
		const char *prefix_,
		jdk_html_style *style_,
		const jdk_settings &settings_,
		const char *settingname_of_file_to_edit,
		const char *settingname_of_title
		)    
        : 
	    jdk_http_server_generator_html( prefix_, style_ ),
	    settings(settings_),
		filename( settings_, settingname_of_file_to_edit ),
		title( settings_, settingname_of_title )
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
	jdk_live_string< jdk_str<1024> > filename;
	jdk_live_string< jdk_str<1024> > title;
};


#endif


