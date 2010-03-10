#include "jdk_world.h"
#include "if2k_serial.h"
#include "if2k_config.h"
#include "if2k_filter.h"

jdk_thread_shared *if2k_http_proxy_factory::create_thread_shared()
{
  jdk_str<256> hostnameport;
  jdk_str<256> hostname;
  
  // get the IP address and name of the interface we are listening to for admin requests
  jdk_url interface_exploded( settings.get("admin_interface") );
  
  if( interface_exploded.port == -1 )
  {
    interface_exploded.port = 80;
  }
  
  // look up the user specified ip or host name and convert to ascii ip
  if( jdk_gethostbyname( interface_exploded.host.c_str(), hostname.c_str(), hostname.getmaxlen() )==0 )
  {
    hostnameport.form( "%s:%d", hostname.c_str(), interface_exploded.port );
  }
  else
  {
    hostnameport = settings.get("admin_interface");
  }
  
  
  global_kernel = &kernel;
  
  if2k_http_proxy_filter *filter = new if2k_http_proxy_filter(
    settings, 
    hostnameport, 
    kernel,
    lua 
    );
  
  jdk_http_server_generator_list *generators = new jdk_http_server_generator_list(11);
  
#if 0
  if2k_http_proxy_shared *shared = new if2k_http_proxy_shared( 
    server_socket,
    admin_socket,
    settings, 
    filter,
    generators,
    lua,
    new if2k_generator_connect_error(
      settings, 
      new if2k_html_style(settings,hostnameport)
      ),
    new if2k_generator_bad_request_error(
      settings, 
      new if2k_html_style(settings,hostnameport)
      )
    );
#endif
#if IF2K_ENABLE_LUA
  if2k_http_proxy_shared *shared = new if2k_http_proxy_shared( 
    server_socket, 
    admin_socket,
    settings, 
    filter,
    generators,
    lua,
    new if2k_generator_lua_error(
      lua,
      "connect"
      ),
    new if2k_generator_lua_error(
      lua,
      "badrequest"
      )
    );
#else
  if2k_http_proxy_shared *shared = new if2k_http_proxy_shared( 
    server_socket, 
    admin_socket,
    settings, 
    filter,
    generators,
    lua,
    new if2k_generator_connect_error(
      settings, 
      new if2k_html_style(settings,hostnameport)
      ),
    new if2k_generator_bad_request_error(
      settings, 
      new if2k_html_style(settings,hostnameport)
      )
    );
  
#endif
  
  generators->add(
    new jdk_http_server_generator_redirect(
      "/",
      "login.cgi"
      )
    );
  
#if IF2K_ENABLE_LUA
  generators->add(
    new if2k_generator_lua(
      lua,
      "/cgi/"							  
      )
    );
#endif
  
  generators->add(
    new if2k_http_generator_getsettings(
      "/settings-get.cgi",
      settings
      )
    );
  
  generators->add(
    new if2k_http_generator_getfile(
      "/file-get.cgi?",
      settings
      )
    );
  
  
  generators->add(
    new if2k_http_generator_blockpage(
      "/blocked.cgi?",
      new jdk_html_style_simple,
      settings
      )
    );
  
  generators->add(
    new if2k_http_generator_override(
      settings,
      "/override.cgi?"
      )
    );
  
  generators->add(
    new if2k_http_generator_adminlogin(
      "/login.cgi",
      new if2k_html_style(settings,hostnameport),
      settings,
      "admin-settings.cgi"
      )
    );
  
  generators->add(
    new jdk_http_server_generator_html_post(
      "/admin-settings.cgi",
      new if2k_html_style(settings,hostnameport),
      new if2k_http_administration(settings,"admin-done.cgi"),
      0
      )
    );
  
  generators->add(
    new if2k_http_generator_adminset(
      "/admin-done.cgi",
      new if2k_html_style(settings,hostnameport),
      settings
      )
    );
  
  generators->add( new jdk_http_generator_bindir("web","/") );
  
#if 0
  generators->add(
    new if2k_generator_textedit(
      "/admin-textedit.cgi",
      new if2k_html_style(settings,hostnameport),
      settings,
      "good_urls_file",
      "text_edit_good_urls_title"
      )
    );
  
  generators->add(
    new if2k_generator_textedit_set(
      "/admin-textedit-set.cgi",
      new if2k_html_style(settings,hostnameport),
      settings,
      "good_urls_file",
      "text_edit_good_urls_title"
      )
    );
#endif
#if IF2K_ENABLE_LUA
  // the 404 generator must be last in line!
  generators->add(
    new if2k_generator_lua_error(
      lua, "404"
      )
    );
#else
  generators->add(
    new if2k_generator_404(
      new if2k_html_style(settings,hostnameport),
      settings
      )
    );
#endif	
  
  global_shared = shared;
  
  
  return shared;
}


if2k_http_proxy_filter::if2k_http_proxy_filter( 
  const jdk_settings &settings_,
  const jdk_string &internal_hostnameport_,
  if2_kernel_standard &kernel_,
  if2k_lua_interpreter &lua_	
  )
  :
  settings( settings_ ),
  kernel( kernel_ ),
  use_internal_blocked_page( settings, "use_internal_blocked_page" ),
  blocked_page( settings, "blocked_page" ),
  fancy_blocked_page( settings, "fancy_blocked_page" ),
  allow_override( settings, "allow_override" ),
  override_unknown_only( settings, "override_unknown_only" ),
  internal_hostnameport( internal_hostnameport_ ),
  lua( lua_ )
{
}

if2k_http_proxy_filter::~if2k_http_proxy_filter()
{
}

bool if2k_http_proxy_filter::run()         // called just after creation to start any worker threads required for the filter
{
  return true;		
}


bool if2k_http_proxy_filter::update()      // called to reload possibly changed settings
{
  return true;
}

bool if2k_http_proxy_filter::generate_blocked_page_url(
  jdk_string_url &blocked_page_url,
  const jdk_http_request_header &request_header,	
  const jdk_string & connection_id,   
  int reason,
  const jdk_string &match_string
  )
{
  if( lua.if_url_blocked(
        blocked_page_url,
        connection_id,
        request_header,
        reason,
        match_string
        ) )
  {
    return true;
  }
  
  if( use_internal_blocked_page.get() )
  {
    blocked_page_url.form( "http://%s/blocked.cgi?", internal_hostnameport.c_str() );		
  }
  else
  {				
    blocked_page_url.cpy( blocked_page.get() );
  }
  
  
  if( fancy_blocked_page )
  {
    jdk_string_url escaped_url;
    jdk_cgi_escape_with_amp( request_header.get_url().unexplode(), escaped_url );
    jdk_string_url escaped_match;
    if( !match_string.is_clear() )
    {
      jdk_cgi_escape( match_string, escaped_match );
    }
    
    jdk_string_url cgi_parts;
    
    cgi_parts.form( "url=%s&reason=%d&match=%s&ref=%s",
                    escaped_url.c_str(),
                    reason,
                    escaped_match.c_str(),
                    request_header.get("referer:").c_str()
      );
    blocked_page_url.cat( cgi_parts );
  }
  
  return true;
}



// test_url() returns false if url is to be denied. returns true if it is to be redirected	
bool if2k_http_proxy_filter::test_url(
  const jdk_http_request_header &request,
  const jdk_string_url &url, 		// the original requested url
  jdk_string_url &redirect_url,		// the url that will be used
  jdk_string_url &redirect_proxy, 	// If non-blank, the proxy server to use
  const jdk_string & connection_id,
  jdk_string &match_phrase
  )
{
  if( global_settings->get_long("license_valid")==0 )
  {
    jdk_log( JDK_LOG_INFO, "usage when license invalid" );
    redirect_url.clear();
    match_phrase.clear();
    int reason=0;
    generate_blocked_page_url(
      redirect_url, 
      request,							
      connection_id, 
      reason,
      match_phrase
      );
    return true;
  }
  
  bool override = allow_override && is_override_enabled(connection_id);
  
  redirect_url = url;
  if2_kernel_result r=
    kernel.verify_url(
      connection_id.c_str(),
      url.c_str(),
      match_phrase,
      override,
      override_unknown_only
      );
  
  // is it to be blocked when we dont have an override allowance?
  if( !r.value && !override )
  {
    redirect_url.clear();
    int reason=1;
    if( r.key==0)
      reason=2;
    generate_blocked_page_url(
      redirect_url, 
      request,							
      connection_id, 
      reason, 
      match_phrase
      );
    if( !match_phrase.is_clear() )
    {
      jdk_log( JDK_LOG_DEBUG1, "Matched phrase in URL: %s", match_phrase.c_str() );
    }
    
    return true;
  }
  
  return false;
}



// test_request_header() returns false if the request is to be denied.
// If it returns true it must rewrite the request
bool if2k_http_proxy_filter::test_request_header(
  jdk_http_request_header &http_request_inout, 	// the original (tweaked) request header
  const jdk_string & connection_id,
  jdk_string &match_phrase
  )
{
  return true;	
}


// test_response_header() returns false if we want to deny the request because of something in the http response header.
// if it returns true it must rewrite the response

bool if2k_http_proxy_filter::test_response_header(
  jdk_http_response_header &http_response_inout,	 // the original response header
  const jdk_string & connection_id,
  jdk_string &match_phrase
  )
{
  return true;	
}


// test_client_send_packet() returns true if the data buffer is OK to send to the server.
// if it returns true, it will put the data, possibly modified with possibly different length
// into output_buf.
// If it returns false, the proxy will abort the connection immediately and instead return an error message to
// the client.

bool if2k_http_proxy_filter::test_client_send_packet(
  const jdk_string_url &url,
  const jdk_http_request_header &http_request,
  const unsigned char *input_buf,
  int input_buf_data_length,
  jdk_dynbuf &output_buf,
  const jdk_string & connection_id,
  jdk_string &match_phrase
  )
{
  // TODO:
  output_buf.append_from_data( input_buf, input_buf_data_length );
  return true;	
}


// test_client_receive_packet() returns true if the data buffer is OK to send to the client.
// if it returns false, it will put the data, possibly modified with possibly different length
// into output_buf.
// if it returns false, the proxy will abort the connection immediately. If it is early enough to
// return an error instead (ie this was the first data packet received from the server), then
// an error will be returned to the client.
// if it wasn't the first data packet received then all we can do is disconnect the socket.


bool if2k_http_proxy_filter::test_client_receive_packet(
  const jdk_string_url &url,
  const jdk_http_response_header &http_response,
  const unsigned char *input_buf,
  int input_buf_data_length,
  jdk_dynbuf &output_buf,
  const jdk_string & connection_id,
  jdk_string &match_phrase
  )
{
  bool override=is_override_enabled(connection_id);
  if2_kernel_result ret=kernel.verify_received_data( 
    connection_id.c_str(), 
    url.c_str(), 
    (const char *)input_buf, 
    input_buf_data_length,
    match_phrase,
    override,													  
    override_unknown_only													  
    );
  
  if( !match_phrase.is_clear() )
  {
    jdk_log( JDK_LOG_DEBUG1, "Matched phrase in receive packet: %s", match_phrase.c_str() );	
  }
  
  if( !ret.value && !override )
    return false;
  else
  {			
    output_buf.append_from_data( input_buf, input_buf_data_length );
    return true;
  }
  
}

bool if2k_http_proxy_filter::is_override_enabled(const jdk_string &connection_id ) const
{
  return override_map->is_client_in_override( connection_id );	
}














