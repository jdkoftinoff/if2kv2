#ifndef _IF2K_ADMIN_FILE_H
#define _IF2K_ADMIN_FILE_H

#include "if2k_config.h"
#include "jdk_http_server.h"
#include "jdk_html.h"

class if2k_admin_file_generator : public jdk_http_server_generator
{
public:
	if2k_admin_file_generator(
		   		jdk_settings &settings_,	
				const char *get_prefix_,
				const char *put_prefix_
			)
			:	
		settings( settings_ ),
		get_prefix( get_prefix_ ),
		put_prefix( put_prefix_ )
	{
	}

	~if2k_admin_file_generator()
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
		// TODO:
	}

};

#endif

