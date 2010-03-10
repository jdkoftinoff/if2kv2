#ifndef _JDK_HTTP_SERVER_LUA_H
#define _JDK_HTTP_SERVER_LUA_H

#include "jdk_lua.h"
#include "jdk_http_server.h"

class jdk_http_generator_lua : public jdk_http_server_generator
{
public:
	jdk_http_generator_lua( lua_State *L_, const char *funcname_ ) : L(L_), funcname(funcname_)
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
		response.clear();
		response_data.clear();

		// TODO: call lua function in the following form
		//		http_code, response_header, response_data = func( request_header_map, request_data )

#if 0		
		const char *fname = request.get_url().file.c_str();
		if( *fname==0 )
		{
			fname = request.get_url().path.c_str()+1;
		}
		
		jdk_bindir *d = jdk_bindir_find( group.c_str(), fname );
		if( d )
		{
			const char *ext = strrchr(fname,'.' );
			const char *mime="application/octet-stream";
			
			if( ext )
			{
				++ext;
				if( strcmp( ext, "jpg")==0 )
					mime="image/jpeg";
				else if( strcmp( ext, "gif")==0 )
					mime="image/gif";
				else if( strcmp( ext, "html")==0 )
					mime="text/html; charset=UTF-8";
				else if( strcmp( ext, "txt")==0 )
					mime="text/plain; charset=UTF-8";
			}
		
			response.set_http_version( jdk_str<64>("HTTP/1.0") );
			response.set_http_response_code( 200 );
			response.set_http_response_text( jdk_str<64>("OK") );
			response.add_entry(	"Content-type:", mime);
			jdk_str<64> len; len.form( "%d", d->length );
			response.add_entry( "Content-length:", len.c_str() );
			response.add_entry( "Connection:", "close" );
			response.set_valid();	
			response_data.append_data( d->data, d->length );
			return true;
		}
		else
		{
			return false;
		}
	}
#endif

	lua_State *L;
	jdk_str<64> funcname;
};



#endif
