#ifndef _IF2K_LUA_H
#define _IF2K_LUA_H

#include "if2k_config.h"


#if IF2K_ENABLE_LUA
#include "jdk_lua.h"
#endif

#include "jdk_http.h"
#include "jdk_cgi.h"
#include "jdk_settings.h"
#include "if2k_kernel.h"


extern PLATFORM_SETTINGS *global_settings;

class if2k_lua_interpreter;


struct if2k_lua_web_response
{
	if2k_lua_web_response(
						 jdk_http_response_header &header_,
						 jdk_dynbuf &contents_
						 )
	: header(header_), contents(contents_)
	{
	}
	
	void clear() { header.clear(); contents.clear(); }
	
	jdk_http_response_header &header;
	jdk_dynbuf &contents;
};



#if IF2K_ENABLE_LUA

inline void jdk_lua_push( lua_State *L, const jdk_http_request_header &req )
{
	jdk_debug_block( "jdk_lua_push( jdk_http_request_header )" );
	
	// generate table like:
	// 	{ 
	//  	request = "http://www.blahblah.com/abc",
	//      method = "GET",
	//      protocol = "http:",
	//      host = "www.blahblah.com",
	//      port = 80,
	//      version = "HTTP/1.1",
	//		path = "/abc",
	//
	// 		headers = {
	//			host = "www.blahblah.com",
	//			referer = "http://www.blah1.com/",
	//			etcetcetc
	//		}
	// 	 --no post info
	//	};
	
	
	// the main table
	lua_newtable( L );

	jdk_lua_push_table( L, "request", req.get_url().unexplode()  );
	jdk_lua_push_table( L, "method",  req.http_methods[ req.get_request_type() ] );
	jdk_lua_push_table( L, "protocol", req.get_url().protocol );
	jdk_lua_push_table( L, "host", req.get_url().host );
	jdk_lua_push_table( L, "port", req.get_url().port );
	jdk_lua_push_table( L, "path", req.get_url().path );
	
	// the sub table 'headers'
	
	jdk_lua_push_table( L, "headers", req.map );	
}

inline void jdk_lua_get( lua_State *L, class jdk_http_response_header &v )
{
	jdk_debug_block( "jdk_lua_get jdk_http_response_header" );

	jdk_dynbuf raw_response_header;
	jdk_lua_get( L, raw_response_header );
	
	v.clear();
	v.parse_buffer( raw_response_header );
}

inline void jdk_lua_append( lua_State *L, jdk_buf &v )
{
	jdk_debug_block( "jdk_lua_append jdk_buf" );
	if( lua_isstring(L,-1) )
	{
		v.append_data((const unsigned char*)lua_tostring(L,-1),lua_strlen(L,-1));
	}
	lua_pop(L,1);
}

inline void jdk_lua_get( lua_State *L, class if2k_lua_web_response &v )
{
	v.clear();
	if( lua_istable(L,-1) )
	{
		int n = lua_getn(L,-1);
		if( n>0 )
		{
			lua_rawgeti(L,-1,1);
			jdk_lua_get( L, v.header );

			for( int i=1; i<n; ++i )
			{
				lua_rawgeti(L,-1,i+1);
				jdk_lua_append(L,v.contents);
			}
		}
		lua_pop(L,1);		
	}
}
#endif

#if IF2K_ENABLE_LUA
class if2k_lua_interpreter : public jdk_lua_interpreter
{	
public:
	if2k_lua_interpreter(
						const jdk_settings &settings_, 
						int stack_size_=0
						);

	virtual ~if2k_lua_interpreter();

	virtual void run();

	virtual void update();

	virtual bool parse_builtin( const char *fname );

	virtual bool if_parse_url( const char *fname );

	virtual bool if_parse_cached_url(
								  const char *url,
								  const char *cachefile,
								  bool ignore_remote_if_local_exists
								  );

	virtual bool if_webserve(
						  jdk_dynbuf &ret, 
						  const jdk_string &client_ip, 
						  const jdk_http_request_header &request_header
						  );

	virtual bool if_cgi_get(
							const jdk_string &cmd,
							if2k_lua_web_response  &ret,
							const jdk_string &client_ip, 
							const jdk_http_request_header &request_header, 
							const jdk_settings_text &cgiparams
						 );

	virtual bool if_cgi_post(
								const jdk_string &cmd,
								if2k_lua_web_response  &ret,
								const jdk_string &client_ip,
								const jdk_http_request_header &request_header,
								const jdk_dynbuf &postparams
						  );

	virtual bool if_cgi_put(
						  const jdk_string &cmd,
						  if2k_lua_web_response  &ret,
						  const jdk_string &client_ip,
						  const jdk_http_request_header &request_header,
						  const jdk_dynbuf &putcontents,
						  const jdk_settings_text &cgiparams
						 );
	

	virtual bool if_url_test(
						  jdk_buf &ret, 
						  const jdk_string &client_ip,
						  const jdk_http_request_header &request_header
						  );

	virtual bool if_url_accessed(
							  jdk_buf &ret, 
							  const jdk_string &client_ip,
							  const jdk_http_request_header &request_header
							  );

	virtual bool if_url_blocked(
							 jdk_string_url &redirect_url, 
							 const jdk_string &client_ip,
							 const jdk_http_request_header &request_header,
							 int reason,
							 const jdk_string &match
							 );
	

	virtual bool if_url_override(
							  jdk_buf &ret, 
							  const jdk_string &client_ip,
							  const jdk_http_request_header &request_header
							  );

	virtual bool if_url_error(
		                      const jdk_string &error_name,
		                      if2k_lua_web_response  &ret,							  
							  const jdk_string &client_ip,
							  const jdk_http_request_header &request_header
							  );

	inline bool reload_settings_was_triggered()
	{
		jdk_synchronized(mutex);	
		bool r=reload_settings_trigger;
		reload_settings_trigger=false;
		return r;
	}
	
	inline void trigger_reload_settings()
	{
		jdk_synchronized(mutex);	
		reload_settings_trigger = true;
	}

private:
	const jdk_settings &settings;
	bool reload_settings_trigger;
	jdk_recursivemutex mutex;

};
#else
class if2k_lua_interpreter 
{	
public:
	if2k_lua_interpreter(
						const jdk_settings &settings_, 
						int stack_size_=0
							   ) {}

	virtual ~if2k_lua_interpreter() {}

	virtual void run() {}

	virtual void update() {}

	virtual bool parse_builtin( const char *fname ) {return true;}

	virtual bool if_parse_url( const char *fname ) {return true;}

	virtual bool if_parse_cached_url(
								  const char *url,
								  const char *cachefile,
								  bool ignore_remote_if_local_exists
								  )
		{
			return false;
		}

	virtual bool if_webserve(
						  jdk_dynbuf &ret, 
						  const jdk_string &client_ip, 
						  const jdk_http_request_header &request_header
						  )
		{
			return false;
		}

	virtual bool if_cgi_get(
							const jdk_string &cmd,
							if2k_lua_web_response  &ret,
							const jdk_string &client_ip, 
							const jdk_http_request_header &request_header, 
							const jdk_settings_text &cgiparams
						 )
		{
			return false;
		}

	virtual bool if_cgi_post(
								const jdk_string &cmd,
								if2k_lua_web_response  &ret,
								const jdk_string &client_ip,
								const jdk_http_request_header &request_header,
								const jdk_dynbuf &postparams
						  )
		{
			return false;
		}

	virtual bool if_cgi_put(
						  const jdk_string &cmd,
						  if2k_lua_web_response  &ret,
						  const jdk_string &client_ip,
						  const jdk_http_request_header &request_header,
						  const jdk_dynbuf &putcontents,
						  const jdk_settings_text &cgiparams
						 )
		{
			return false;
		}
	

	virtual bool if_url_test(
						  jdk_buf &ret, 
						  const jdk_string &client_ip,
						  const jdk_http_request_header &request_header
						  )
		{
			return false;
		}

	virtual bool if_url_accessed(
							  jdk_buf &ret, 
							  const jdk_string &client_ip,
							  const jdk_http_request_header &request_header
							  )
		{
			return false;
		}

	virtual bool if_url_blocked(
							 jdk_string_url &redirect_url, 
							 const jdk_string &client_ip,
							 const jdk_http_request_header &request_header,
							 int reason,
							 const jdk_string &match
							 )
		{
			return false;
		}
	

	virtual bool if_url_override(
							  jdk_buf &ret, 
							  const jdk_string &client_ip,
							  const jdk_http_request_header &request_header
							  )
		{
			return false;
		}

	virtual bool if_url_error(
		                      const jdk_string &error_name,
		                      if2k_lua_web_response  &ret,							  
							  const jdk_string &client_ip,
							  const jdk_http_request_header &request_header
							  )
		{
			return false;
		}

	inline bool reload_settings_was_triggered()
	{
		return false;
	}
	
	inline void trigger_reload_settings()
	{
	}

};

#endif

#endif
