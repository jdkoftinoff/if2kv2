
#include "jdk_world.h"
#include "if2k_httpproxy.h"
#include "jdk_socketutil.h"
#include "jdk_cgi.h"
#if JDK_IS_WIN32
#include "wnloginquery.h"
#endif

char *if2k_http_proxy_shared::default_connect_error =
"<html>\r\n"
"<head><title>Connection failed</title></head>\r\n"
"<body bgcolor=\"white\" text=\"black\"><center>"
"<h1>Connection failed</h1><p>Unable to connect</p>\r\n"
"<p>The server could be down temporarily, or the URL may be incorrect</p>\r\n"
"</center></html>\r\n";

char *if2k_http_proxy_shared::default_bad_request =
"<html>\r\n"
"<head><title>Incorrect request</title></head>\r\n"
"<body bgcolor=\"white\" text=\"black\"><h1>Incorrect request</h1>\r\n"
"</html>\r\n";

char *if2k_http_proxy_shared::default_redirect =
"<html>\r\n"
"<head><title>Moved Temporarily</title></head>\r\n"
"<body bgcolor=\"white\" text=\"black\">\r\n"
"<font size=+1><a href=\"%s\">Moved Temporarily</a></font>\r\n"
"</html>\r\n";

if2k_http_proxy_nullfilter::if2k_http_proxy_nullfilter(const jdk_settings &settings )
{
}


if2k_http_proxy_nullfilter::~if2k_http_proxy_nullfilter()
{
}

bool if2k_http_proxy_nullfilter::run()
{
  return true;    // no threads to start
}

bool if2k_http_proxy_nullfilter::update()
{
  return false;    // no settings to reload
}


// test_url() returns true if it is to be redirected
bool if2k_http_proxy_nullfilter::test_url(
  const jdk_http_request_header &request,
  const jdk_string_url &url, 		// the requested url
  jdk_string_url &redirect_url,		// the url that will be used
  jdk_string_url &redirect_proxy, 	// If non-blank, the proxy server to use
  const jdk_string & connection_id,
  jdk_string &match_phrase
  )
{
  
  redirect_url = url;
  redirect_proxy.clear();
  
  return false; // Do not redirect
}

// test_request_header() returns false if the request is to be denied.
// If it returns true it must rewrite the request
bool if2k_http_proxy_nullfilter::test_request_header(
  jdk_http_request_header &http_request,		// the original (tweaked) request header
  const jdk_string & connection_id,
  jdk_string &match_phrase
  )
{
  // do nothing for headers
  
  return true;
}

// test_response_header() returns false if we want to deny the request because of something in the http response header.
// if it returns true it must rewrite the response

bool if2k_http_proxy_nullfilter::test_response_header(
  jdk_http_response_header &http_response,	 // the original response header
  const jdk_string & connection_id,
  jdk_string &match_phrase
  )
{
  // do nothing for headers
  return true;
}

// test_client_send_packet() returns true if the data buffer is OK to send to the server.
// if it returns true, it will put the data, possibly modified with possibly different length
// into output_buf.
// If it returns false, the proxy will abort the connection immediately and instead return an error message to
// the client.

bool if2k_http_proxy_nullfilter::test_client_send_packet(
  const jdk_string_url &url,
  const jdk_http_request_header &http_request,
  const unsigned char *input_buf,
  int input_buf_data_length,
  jdk_dynbuf &output_buf,
  const jdk_string & connection_id,
  jdk_string &match_phrase
  )
{
  // allow all proxied client data to go through unharmed
  output_buf.clear();
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

bool if2k_http_proxy_nullfilter::test_client_receive_packet(
  const jdk_string_url &url,
  const jdk_http_response_header &http_response,
  const unsigned char *input_buf,
  int input_buf_data_length,
  jdk_dynbuf &output_buf,
  const jdk_string & connection_id,
  jdk_string &match_phrase
  )														 
{
  // allow all proxied server data to go through unharmed
  output_buf.clear();
  output_buf.append_from_data( input_buf, input_buf_data_length );
  
  return true;
}


if2k_http_proxy_shared::if2k_http_proxy_shared( 
  jdk_server_socket &server_socket_, 
  jdk_server_socket &admin_socket_,
  const jdk_settings &settings_, 
  if2k_http_proxy_filter_base *filter_,
  jdk_http_server_generator_list *generators_,	
  if2k_lua_interpreter &lua_,
  jdk_http_server_generator *generate_connect_error_,
  jdk_http_server_generator *generate_bad_request_error_
  )
  :
  server_socket( server_socket_ ),
  admin_socket( admin_socket_ ),
  dns_cache( 5000 ),
  further_proxy(),
  max_retries(),
  retry_delay(),
  settings( settings_ ),
  filter(filter_),
  generators(generators_),
  allowed_client_ips(0),
  blocked_client_ips(0),
  nofilter_client_ips(0),
  generate_connect_error( generate_connect_error_ ),
  generate_bad_request_error( generate_bad_request_error_ ),
  html_generate_connect_error( 0 ),
  html_generate_bad_request_error( 0 ),
  my_proxy_id(),
  my_interface(),
  my_interface_port(),  
  my_host_name(),
  lua( lua_ ),
  hack_mutex()
{
  // generate random proxy id so we can sense loopbacks
  unsigned long r1=rand();
  unsigned long r2=rand();
  my_proxy_id.form("%08lx%08lx", r1, r2 );
  
  // get the IP address and name of the interface we are listening to
  jdk_url interface_exploded( settings.get("admin_interface") );
  
  // look up the user specified ip or host name and convert to ascii ip
  jdk_gethostbyname( interface_exploded.host.c_str(), my_interface.c_str(), my_interface.getmaxlen() );
  
  my_interface_port = interface_exploded.port;
  jdk_log_debug2( "admin my_interface is %s:%d", my_interface.c_str(), my_interface_port );
  
  // now, look up the real hostname for this address
  char tmp[256];
  if( jdk_gethostbyaddr( my_interface.c_str(), tmp, sizeof(tmp) )>=0 )
  {
    my_host_name.cpy(tmp);
  }
  else
  {
    // hmmm, we cant look it up, so lets just copy the interface
    my_host_name.cpy( my_interface );
  }
  jdk_log_debug2( "my_host_name is %s", my_host_name.c_str() );
  
}

if2k_http_proxy_shared::if2k_http_proxy_shared( 
  jdk_server_socket &server_socket_, 
  jdk_server_socket &admin_socket_,
  const jdk_settings &settings_, 
  if2k_http_proxy_filter_base *filter_,
  jdk_http_server_generator_list *generators_,	
  if2k_lua_interpreter &lua_,
  jdk_html_document_generator *html_generate_connect_error_,
  jdk_html_document_generator *html_generate_bad_request_error_
  )
  :
  server_socket( server_socket_ ),
  admin_socket( admin_socket_ ),
  dns_cache( 5000 ),
  further_proxy(),
  max_retries(),
  retry_delay(),
  settings( settings_ ),
  filter(filter_),
  generators(generators_),
  allowed_client_ips(0),
  blocked_client_ips(0),
  nofilter_client_ips(0),
  generate_connect_error( 0 ),
  generate_bad_request_error( 0 ),
  html_generate_connect_error( html_generate_connect_error_ ),
  html_generate_bad_request_error( html_generate_bad_request_error_ ),
  my_proxy_id(),
  my_interface(),
  my_interface_port(),  
  my_host_name(),
  lua( lua_ ),
  hack_mutex()
{
  // generate random proxy id so we can sense loopbacks
  unsigned long r1=rand();
  unsigned long r2=rand();
  my_proxy_id.form("%08lx%08lx", r1, r2 );
  
  // get the IP address and name of the interface we are listening to
  jdk_url interface_exploded( settings.get("admin_interface") );
  
  // look up the user specified ip or host name and convert to ascii ip
  jdk_gethostbyname( interface_exploded.host.c_str(), my_interface.c_str(), my_interface.getmaxlen() );
  
  my_interface_port = interface_exploded.port;
  jdk_log_debug2( "my_interface is %s:%d", my_interface.c_str(), my_interface_port );
  
  // now, look up the real hostname for this address
  char tmp[256];
  if( jdk_gethostbyaddr( my_interface.c_str(), tmp, sizeof(tmp) )>=0 )
  {
    my_host_name.cpy(tmp);
  }
  else
  {
    // hmmm, we cant look it up, so lets just copy the interface
    my_host_name.cpy( my_interface );
  }
  jdk_log_debug2( "my_host_name is %s", my_host_name.c_str() );
  
}

bool if2k_http_proxy_shared::run()
{
  jdk_synchronized( hack_mutex );					
  
  jdk_str<4096> allowed_ips = settings.get( "allowed_client_ips" );
  jdk_str<4096> blocked_ips = settings.get( "blocked_client_ips" );
  jdk_str<4096> nofilter_ips = settings.get( "nofilter_client_ips" );	
  
  if( allowed_ips.is_clear() )
    allowed_client_ips = new jdk_ipv4_ip_range( 0,0,0,0, 0,0,0,0, 0,65535 );
  else
    allowed_client_ips = new jdk_ipv4_ip_range( allowed_ips.c_str() );
  
  blocked_client_ips = new jdk_ipv4_ip_range( blocked_ips.c_str() );    
  nofilter_client_ips = new jdk_ipv4_ip_range( nofilter_ips.c_str() );	
  
  further_proxy = settings.get( "further_proxy" );
  
  max_retries = settings.get_long( "max_retries" );
  retry_delay = settings.get_long( "retry_delay" );
  
  if( server_socket.is_open_for_reading() )
  {
    jdk_log( JDK_LOG_NOTICE, "Listening on %s",
             settings.get( "interface" ).c_str()
      );
  }
  else
  {
    jdk_log( JDK_LOG_ERROR, "Unable to listen on %s - ABORTING",
             settings.get( "interface" ).c_str() );
    exit(1);
  }
  
  lua.run();
  
  return filter->run();
}

bool if2k_http_proxy_shared::update()
{
  jdk_log( JDK_LOG_INFO, "Starting to load settings" );	
  jdk_synchronized( hack_mutex );					
  
  further_proxy = settings.get( "further_proxy" );
  
  jdk_str<4096> allowed_ips = settings.get( "allowed_client_ips" );
  jdk_str<4096> blocked_ips = settings.get( "blocked_client_ips" );
  jdk_str<4096> nofilter_ips = settings.get( "nofilter_client_ips" );	
  
  delete allowed_client_ips;
  delete blocked_client_ips;
  delete nofilter_client_ips;
  
  if( allowed_ips.is_clear() )
    allowed_client_ips = new jdk_ipv4_ip_range( 0,0,0,0, 0,0,0,0, 0,65535 );
  else
    allowed_client_ips = new jdk_ipv4_ip_range( allowed_ips.c_str() );
  
  blocked_client_ips = new jdk_ipv4_ip_range( blocked_ips.c_str() );            
  nofilter_client_ips = new jdk_ipv4_ip_range( nofilter_ips.c_str() );	
  
  filter->update();
  lua.update();
  jdk_log( JDK_LOG_INFO, "Finished loading settings" );	
  return true;
}

if2k_http_proxy_shared::~if2k_http_proxy_shared()
{
  delete generators;
  delete filter;
  delete generate_bad_request_error;
  delete generate_connect_error;
  delete html_generate_connect_error;
  delete html_generate_bad_request_error;
  delete allowed_client_ips;
  delete blocked_client_ips;
  delete nofilter_client_ips;
}


if2k_http_proxy_worker::if2k_http_proxy_worker(
  int my_id_,
  const jdk_settings &settings_,
  if2k_http_proxy_shared &shared_
  )
  :
  my_id( my_id_ ),
  settings( settings_ ),
  shared( shared_ ),
  incoming(0),
  outgoing(0),
  http_request(),
  current_url(),
  http_transformed_request(),
  http_response(),
  http_transformed_response(),
  cached_destination_host(),
  cached_destination_port(),
  transformed_url(),
  transformed_proxy(),
  do_redirect(),
  use_proxy(),
  incoming_addr(),
  outgoing_addr(),
  buf_len(),
  post_data_presend_count(),
  post_data_content_length(),
  remaining_post_data_content_length(),
  response_data_content_length(),
  remaining_response_data_content_length(),
  nofilter(),
  keep_alive()
{	
  jdk_log( JDK_LOG_DEBUG4, "if2k_http_proxy_worker starting" );
}

if2k_http_proxy_worker::~if2k_http_proxy_worker()
{
  jdk_log( JDK_LOG_DEBUG4, "if2k_http_proxy_proxy dying" );
}


void if2k_http_proxy_worker::handle_incoming_connection(
  jdk_client_socket *incoming_connection,
  jdk_client_socket *outgoing_connection
  )
{
  incoming = incoming_connection;
  outgoing = outgoing_connection;
  
  incoming->get_remote_addr( incoming_addr.c_str(), incoming_addr.getmaxlen() );
  {
    // kill port on incoming address
    char *p=incoming_addr.chr(':');
    if( p )
      *p=0;
  }
  
  // validate incoming IP address
  {
    struct sockaddr_in ip;
    if( !incoming_connection->get_remote_addr( &ip ) )
    {
      incoming_connection->close();
      return;
    }
    
    jdk_synchronized( shared.hack_mutex );
    if( !shared.allowed_client_ips->is_ip_in_range( ntohl(ip.sin_addr.s_addr), ntohs(ip.sin_port) )
        || shared.blocked_client_ips->is_ip_in_range( ntohl(ip.sin_addr.s_addr), ntohs(ip.sin_port) ) )
    {
      jdk_log( JDK_LOG_DEBUG1, "Incoming connection from %s denied", incoming_addr.c_str() );
      incoming_connection->close();
      return;
    }
    
    if( shared.nofilter_client_ips->is_ip_in_range( ntohl(ip.sin_addr.s_addr), ntohs(ip.sin_port) ) )
    {
      nofilter=true;
      jdk_log( JDK_LOG_DEBUG1, "Incoming connection from %s marked as no filtering", incoming_addr.c_str() );			
    }		
    else
    {
      nofilter=false;	
    }
    
  }
  
#if JDK_IS_WIN32
  if( incoming_addr.cmp("127.0.0.1")==0 )
  {
    wn_get_logged_in_user_name( &incoming_addr );
  }
#endif
  
  incoming->set_default_timeout(60);
  outgoing->set_default_timeout(60);
  
  jdk_log_debug3( "(%d) entering handle_incoming_connection", my_id );
  
  keep_alive=settings.get_long("use_keepalive");
  
  while( incoming->is_open_for_reading() ) 
  {
    // read a http request
    if( read_http_header() )
    {
      jdk_log_debug1( "(%d) %s read http request", my_id, incoming_addr.c_str()  );
      
      jdk_dynbuf lua_response;
      
      if( shared.lua.if_webserve( lua_response, incoming_addr, http_request ))
      {
        incoming->write_data_block( lua_response );
        keep_alive=false;
      }
      else
        if( handle_generated() )
        {
          keep_alive=false;
        }
        else
          if( transform_http_request() )
          {
            jdk_log_debug1("(%d) %s http request transformed successfully", my_id, incoming_addr.c_str()  );
            
            if( do_redirect )
            {
              // we were asked to redirect the client to a different url.
              respond_with_redirect_page( transformed_url );
            }
            else if( http_request.get_request_type() == jdk_http_request_header::REQUEST_CONNECT
                     || transformed_url.nicmp( "https:", 6 )==0 )
            {
              handle_https_mode();
            }
            else if( transformed_url.nicmp( "http:", 5 )==0 )
            {
              handle_http_mode();
            }
            else if( transformed_url.nicmp( "gopher:", 7 )==0 )
            {
              handle_gopher_mode();
            }
            else if( transformed_url.nicmp( "ftp:", 4 )==0 )
            {
              handle_ftp_mode();
            }
            
          }
          else
          {
            jdk_log_debug1( "(%d) %s http request transform failed", my_id, incoming_addr.c_str()   );
            respond_with_bad_request_page();
          }
      
    }
    else
    {
      break;
    }
    if( !keep_alive )
      break;
  }
  jdk_log_debug3( "(%d) leaving handle_incoming_connection", my_id );	
  incoming->close();
  outgoing->close();
}


bool if2k_http_proxy_worker::read_http_header()
{	
  http_request.clear();
  while( incoming->is_open_for_reading() )
  {
    buf_len = incoming->read_data( buf, sizeof(buf)-1 );
    if( buf_len>0 )
    {
      if( http_request.parse_buffer( buf, buf_len ) )
      {
        // done parsing! Did it succeed?
        jdk_log_debug2( "(%d) %s client header parsed, is_valid=%d buf_len=%d parse pos=%d", my_id, incoming_addr.c_str(), http_request.is_valid(), buf_len, http_request.get_buf_pos() );
        // TODO: If this was a post request and we haven't received all of the post data, and the
        // post data is less than 4 K, then try read the whole entire post. 
#if 0 // TODO: Scan additional post data, but cant use current_url just yet
        jdk_dynbuf dummy;
        if( !nofilter && shared.filter->test_client_send_packet( current_url, http_request, &buf[pos], len, dummy, my_id )==false )
        {
          respond_with_redirect_page( shared.settings.get("blocked_page") );
          return 0;
        }																   				
#endif
        if( http_request.get_http_version().icmp("HTTP/0.9")==0 )
        {
          keep_alive=false;
        }
        else
        {
          const jdk_http_request_header::value_t *v;
          if( http_request.find_entry("connection:",&v)!=-1 )
          {
            if( v->icmp("close")==0 )
            {
              keep_alive=false;
            }
          }                    
          if( http_request.find_entry("proxy-connection:",&v)!=-1 )
          {
            if( v->icmp("close")==0 )
            {
              keep_alive=false;
            }
          }                    
          
        }
        return http_request.is_valid();
      }
    }	    
  }
  jdk_log_debug1( "(%d) %s Socket closed while reading http header from client", my_id, incoming_addr.c_str()  );
  return false;
}

bool  if2k_http_proxy_worker::write_http_header()
{
  jdk_log_debug2( "(%d) %s write_http_header() length = %d", my_id, incoming_addr.c_str() , http_transformed_request.get_data_length() );
  
#if 0	
  jdk_str<4096> line;
  int pos=0;
  while( (pos= http_transformed_request.extract_to_string(line,pos))>0 )
  {
    jdk_log_debug4( "(%d) %s sending: %s\n", my_id, incoming_addr.c_str(),
                    line.c_str() );
  }
#endif	
  if( outgoing->write_data_block( 
        http_transformed_request.get_data(), 
        http_transformed_request.get_data_length() 
        )==http_transformed_request.get_data_length()
    )
  {
    // if it was a request with a content-length payload, then keep track of how much content length has been sent with this write
    
    if( http_request.find("content-length:")!=-1 )
    {
      if( remaining_post_data_content_length!=-1 )
      {
        remaining_post_data_content_length -= post_data_presend_count;
        if( remaining_post_data_content_length< 0 )
        {
          jdk_log_debug3( "(%d) %s write_http_header() wrote %d extra bytes in post request",
                          my_id, incoming_addr.c_str() , -remaining_post_data_content_length );
          remaining_post_data_content_length=0;
        }
      }
    }
    return true;
  }
  else
  {
    return false;
  }
}



void if2k_http_proxy_worker::respond_with_bad_request_page()
{
  jdk_dynbuf tmpbuf;
  {		
    jdk_synchronized( shared.hack_mutex );
    if( shared.generate_bad_request_error )
    {
      
      jdk_http_request_header request;
      request.set_url( "" );
      jdk_dynbuf request_data;
      jdk_http_response_header response;
      jdk_dynbuf response_data;
      
      if( shared.generate_bad_request_error->handle_request( request, request_data, response, response_data, incoming_addr ) )
      {
        response.flatten( tmpbuf );
        tmpbuf.append_from_buf( response_data );
        if( incoming->write_data_block( tmpbuf.get_data(), tmpbuf.get_data_length() ) == tmpbuf.get_data_length() )
        {    
          incoming->close();
          return;
        }
        else
        {
          jdk_log_debug1("(%d) %s Error sending badrequest error to client %s", my_id, incoming_addr.c_str() );
          incoming->close();
          return;
        }
        
      }
      else
      {
        tmpbuf.append_from_string( shared.default_bad_request );
        respond_with_template( tmpbuf, 400, jdk_str<4>(""), jdk_str<4>("") );   
      }
    }
    else
    {
      if( shared.html_generate_bad_request_error )
      {
        jdk_html_chunk *chunk = shared.html_generate_bad_request_error->generate_document();
        chunk->flatten(tmpbuf);
        delete chunk;			
      }
      else
      {
        tmpbuf.append_from_string( shared.default_bad_request );		
      }
      
      respond_with_template( tmpbuf, 400, jdk_str<4>(""), jdk_str<4>("") );   
    }		
  }
  
}

void if2k_http_proxy_worker::respond_with_connection_error_page(const jdk_string &url)
{
  jdk_dynbuf tmpbuf;
  {		
    jdk_synchronized( shared.hack_mutex );
    if( shared.generate_connect_error )
    {
#if 0
      jdk_html_chunk *chunk = shared.generate_connect_error->generate_document();
      chunk->flatten(tmpbuf);
      delete chunk;			
#endif
      jdk_http_request_header request;
      request.set_url( url );
      jdk_dynbuf request_data;
      jdk_http_response_header response;
      jdk_dynbuf response_data;
      
      if( shared.generate_connect_error->handle_request( request, request_data, response, response_data, incoming_addr ) )
      {
        jdk_dynbuf full_response;
        response.flatten( tmpbuf );
        tmpbuf.append_from_buf( response_data );
        if( incoming->write_data_block( tmpbuf.get_data(), tmpbuf.get_data_length() ) == tmpbuf.get_data_length() )
        {    
          incoming->close();
          return;
        }
        else
        {
          jdk_log_debug1("(%d) %s Error sending connection error to client %s", my_id, incoming_addr.c_str() );
          incoming->close();
          return;
        }
        
      }
      else
      {
        tmpbuf.append_from_string( shared.default_connect_error );
        respond_with_template( tmpbuf, 404, url, url );    // 404 = Not found	
      }
    }
    else
    {
      if( shared.html_generate_connect_error )
      {
        jdk_html_chunk *chunk = shared.html_generate_connect_error->generate_document();
        chunk->flatten(tmpbuf);
        delete chunk;			
      }
      else
      {
        tmpbuf.append_from_string( shared.default_connect_error );		
      }
      respond_with_template( tmpbuf, 404, url, url );    // 404 = Not found	
    }
  }
  
  
}

void if2k_http_proxy_worker::respond_with_redirect_page(const jdk_string &url)
{
  jdk_dynbuf tmpbuf;
  {		
    jdk_synchronized( shared.hack_mutex );
    tmpbuf.append_from_string( shared.default_redirect );
  }
  respond_with_template( tmpbuf, 302, url, url );    // 302 is HTTP response MOVED TEMPORARILY
}


bool if2k_http_proxy_worker::respond_with_template( 
  jdk_dynbuf &t, 
  int http_code,
  const jdk_string &param1, 
  const jdk_string &param2
  )
{
  // we need to convert the parameters to HTML escaped sequences.
//    jdk_str<8192> param1_escaped, param2_escaped;
  
//    jdk_html_amp_escape_text( param1, param1_escaped );
//    jdk_html_amp_escape_text( param2, param2_escaped );
  
  jdk_dynbuf formatted( t.get_data_length()+4096 ); // fudge a large enough buffer.
  
  formatted.append_form( "HTTP/1.0 %d Code\r\n", http_code );
  formatted.append_form( "Connection: close\r\n" );
  formatted.append_form( "Pragma: no-cache\r\n" );
  formatted.append_form( "Expires: -1\r\n" );	
  formatted.append_form( "Cache-Control: no-cache\r\n" );			
  formatted.append_form( "Content-type: text/html; charset=UTF-8\r\n" );
  formatted.append_form( "Content-length: %ld\r\n", t.get_data_length() );		
  if( http_code==301 || http_code==302 )
  {
    formatted.append_form( "Location: %s\r\n", param1.c_str() );
  }
  
  formatted.append_form( "\r\n" );
//    formatted.append_form( (char *)t.get_data(), param1_escaped.c_str(), param2_escaped.c_str() );
  formatted.append_from_buf( t );
  
  if( incoming->write_data_block( formatted.get_data(), formatted.get_data_length() ) == formatted.get_data_length() )
  {    
    incoming->close();
    return true;
  }
  else
  {
    jdk_log_debug1("(%d) %s Error sending error response code %d to client", my_id, incoming_addr.c_str() , http_code );
    incoming->close();
    return false;
  }
}

bool if2k_http_proxy_worker::receive_server_response()
{
  http_response.clear();
  while( outgoing->is_open_for_reading() )
  {
    buf_len = outgoing->read_data( buf, sizeof(buf)-1 );
    if( buf_len>0 )
    {
      if( http_response.parse_buffer( buf, buf_len ) )	     
      {
        // done parsing! Did it succeed?
        jdk_log( JDK_LOG_DEBUG2, "(%d) %s server header parsed, is_valid=%d", my_id, incoming_addr.c_str() , http_response.is_valid() );
        return http_response.is_valid();
      }
    }
    else
    {
      if( buf_len==0 )
      {
        jdk_log_debug1("(%d) %s read of server returned 0", my_id, incoming_addr.c_str()  );
      }
      if( buf_len<0 )
      {
        jdk_log_debug1( "(%d) %s read of server returned -1", my_id, incoming_addr.c_str()  );
        return false;
      }
    }
  }
  jdk_log_debug1( "(%d) %s Socket closed while reading http header from server", my_id, incoming_addr.c_str()  );
  return false;
}

bool if2k_http_proxy_worker::handle_generated()
{	
  // first see if our built in generators can handle it
  jdk_dynbuf full_post_buf;
  bool is_internal_request=false;
  
  const jdk_http_request_header::value_t *proxy_id_entry=0;
  
  http_request.find_entry( "X-if2002-id:", &proxy_id_entry );
  if( proxy_id_entry )
  {
    jdk_log_debug3( "request has x-if2002-id: %s - my id: %s", proxy_id_entry->c_str(), shared.my_proxy_id.c_str() );
    if( proxy_id_entry->icmp( shared.my_proxy_id )==0 )
    {
      jdk_log_debug2( "request is internal: %s", proxy_id_entry->c_str() );
      is_internal_request=true;
    }
  }
  else
  {
    jdk_log_debug4( "request has no x-if2002-id" );
  }
  
  if( !is_internal_request 
      && (
        http_request.get_url().host.icmp(shared.my_host_name)==0 
        || http_request.get_url().host.icmp(shared.my_interface)==0
        )
      && http_request.get_url().port==shared.my_interface_port )
  {
    is_internal_request=true;
  }
  
  if( !is_internal_request 
      && (
        http_request.get_url().host.icmp("localhost")==0 
        || http_request.get_url().host.icmp("127.0.0.1")==0
        )
      && http_request.get_url().port==shared.my_interface_port )
  {
    is_internal_request=true;
  }
  
  if( is_internal_request )
  {	  
    post_data_content_length=0;
    
    const jdk_http_request_header::value_t *v;
    if( http_request.find_entry("content-length:", &v )!=-1 )
    {
      post_data_content_length = v->strtol(10);
      
      {						
        if( post_data_content_length<1000000 )
        {    
          full_post_buf.resize( post_data_content_length );
          remaining_post_data_content_length=post_data_content_length;
          
          int pos = http_request.get_buf_pos();
          int len = buf_len-pos;
          
          jdk_log_debug1( "Content length is %d, already read %d", post_data_content_length, len  );				
          
          if( len>post_data_content_length )
            len=post_data_content_length;
          
          if( len>0 )
          {
            full_post_buf.append_from_data( 
              &buf[ http_request.get_buf_pos() ],
              len
              );
            
            remaining_post_data_content_length=post_data_content_length-len;					
          }
          
          
          
          while( remaining_post_data_content_length>0 )
          {
            int additional_len = incoming->read_data( 
              full_post_buf.get_data()+len, 
              remaining_post_data_content_length 
              );
            if( additional_len<=0 )
              return true;
            len += additional_len;
            remaining_post_data_content_length -=additional_len;
            
            jdk_log_debug1( "read %d more bytes of post, %d to go", additional_len, remaining_post_data_content_length );
          }    
          
          
          while(1)
          {
            jdk_select_manager sel;
            sel.add_read( incoming );
            sel.set_timeout( 1 );
            int r=sel.do_select();
            if( r==0 )
            {
              break;
            }
            if( r<0 )
            {
              return true;
            }
            if( r>0 )
            {
              if( sel.can_read(incoming) )
              {
                char throwaway[128];
                int throwaway_len = incoming->read_data( throwaway, sizeof(throwaway) );
                if( throwaway_len<0 )
                {
                  jdk_log_debug1( "throwaway error" );								
                  return true;
                }
                if( throwaway_len==0 )
                {
                  jdk_log_debug1( "throwaway closed" );
                  break;
                }
                if( throwaway_len>0 )
                {
                  jdk_log_debug1( "throwaway %d bytes", throwaway_len );
                }
              }
            }
          }
          
          
          full_post_buf.set_data_length( post_data_content_length );
          //				fprintf( stderr, "Incoming POST:\n" );
          //				full_post_buf.extract_to_stream( stderr );
        }
        else
        {
          remaining_post_data_content_length = 0;
          post_data_content_length = 0;							    
        }
        
      }		
    }
    
    
    for( int i=0; i<shared.generators->count(); ++i )
    {
      jdk_dynbuf full_response;			
      jdk_http_response_header response;	
      bool r=false;
      {
        jdk_http_server_generator *g;		    
        g=shared.generators->get(i);
        if( g )
        {
          //jdk_log_debug4( "trying generator %d", i );
          r=g->handle_request(
            http_request,
            full_post_buf,
            response,
            full_response,
            incoming_addr
            );			
          //if( r )
          //	jdk_log_debug3( "generator %d succeeded", i );
        }
      }
      if( r )
      {											
        
        // now flatten and send the response back!
        jdk_str<64> len;
        len.form("%ld",full_response.get_data_length());
        response.add_entry("Content-Length:", len );
        http_transformed_response.clear();
        if( response.flatten( http_transformed_response ) )
        {
          
          if( incoming->write_data_block( 
                http_transformed_response.get_data(),
                http_transformed_response.get_data_length()
                ) == http_transformed_response.get_data_length()
            )
          {									
            if( incoming->write_data_block(
                  full_response.get_data(),
                  full_response.get_data_length()
                  ) == full_response.get_data_length() )
            {
              incoming->close();
              jdk_log( JDK_LOG_DEBUG1, "(%d) sent generated response to %s", my_id, http_request.get_url().unexplode().c_str() );
              return true;
            }
          }
          incoming->close();
          jdk_log( JDK_LOG_DEBUG1, "(%d) Unable to send generated response", my_id );								
          return true;
        }
        else
        {
          jdk_log( JDK_LOG_ERROR, "(%d) Unable to flatten generated response", my_id );
          return false;
        }								
        
      }
    }		
    return true; // always return true for requests that reflected back to us
  }
  
  return false; // no generator found for this request
}

bool if2k_http_proxy_worker::handle_https_mode()
{
  jdk_log( JDK_LOG_DEBUG1, "HTTPS CONNECT to %s", current_url.c_str() );
  
#if 0
  if2k_http_proxy_xfer_processor xfer( shared, incoming_addr, current_url, http_request, http_response, false );
#endif	
  
  cached_destination_port = -1;	
  // TODO: currently we ignore transformed_proxy?
  int tries=0;
  for( ; tries<shared.max_retries; tries++ )
  {
    if( tries!=0 )
    {
      jdk_log_debug1( "(%d) %s retry #%d", my_id, incoming_addr.c_str() , tries );
      // couldnt make connection. sleep a bit and try again
      jdk_sleepms( shared.retry_delay );
    }
    
    // connect to the server
    if( make_cached_connection() )
    {
      break;
    }
  }
  
  if( tries>=shared.max_retries )
  {
    jdk_log_debug1( "(%d) %s connection failed to server for %s", my_id, incoming_addr.c_str(), current_url.c_str() );
    
    respond_with_connection_error_page( current_url );
    
    return false; // failure to connect
  }
  
  
  {
    if( use_proxy )
    {
      write_http_header();
      jdk_log_debug3( "(%d) wrote http request header to https proxy", my_id );
    }
    else
    {
      incoming->write_string_block( "HTTP/1.0 200 Connection established\r\n\r\n" );
      jdk_log_debug3( "(%d) wrote http response header to https client", my_id );
    }
    
    // we may still have data in buf[] that needs to be sent as well.
    int pos = http_request.get_buf_pos();
    int len = buf_len - pos;
    if( len>0 )
    {
      jdk_log_debug2( "(%d) %s pre-sending %d bytes of CONNECT data", my_id, incoming_addr.c_str() , len );
      if( outgoing->write_data_block( &buf[pos], len )!=len )
      {
        jdk_log_debug2( "(%d) error writing connect data to server", my_id );
        return false;
      }
    }
  }
  
  jdk_socket_transfer(
    my_id,
    incoming,
    outgoing
    );
  
#if 0	
  jdk_socket_transfer_with_processing(
    my_id,
    incoming,
    outgoing,
    &xfer
    );
#endif	
  jdk_log( JDK_LOG_DEBUG1, "HTTP CONNECT finished for %s", current_url.c_str() );
  
  return true;
}

bool if2k_http_proxy_worker::handle_http_mode()
{
  if( make_retried_cached_connection_with_request() )
  {
    if( transform_server_response()  )
    {
      
      if( transfer_from_server() )
      {
        jdk_log_debug1("(%d) %s http request transferred successfully: %s", my_id, incoming_addr.c_str() ,current_url.c_str() );
        return true;
      }
      else
      {
        jdk_log_debug1("(%d) %s http request transfer failed: %s", my_id, incoming_addr.c_str() , current_url.c_str() );
        return false;
      }
    }
    else
    {
      jdk_log_debug1( "(%d) %s invalid response from server for %s", my_id, incoming_addr.c_str() , current_url.c_str() );
      
      respond_with_connection_error_page( current_url );
      return false;
    }
  }
  else
  {
    jdk_log_debug1( "(%d) %s connection failed to server for %s", my_id, incoming_addr.c_str(), current_url.c_str() );
    
    respond_with_connection_error_page( current_url );
    return false;
  }
}


bool if2k_http_proxy_worker::handle_gopher_mode()
{
  jdk_log( JDK_LOG_DEBUG1, "HTTP gopher to %s", transformed_url.c_str() );
  
  // TODO: currently we ignore transformed_proxy?
  int tries=0;
  for( ; tries<shared.max_retries; tries++ )
  {
    if( tries!=0 )
    {
      jdk_log_debug1( "(%d) %s retry #%d", my_id, incoming_addr.c_str() , tries );
      // couldnt make connection. sleep a bit and try again
      jdk_sleepms( shared.retry_delay );
    }
    
    // connect to the server
    if( make_cached_connection() )
    {
      break;
    }
  }
  
  if( tries>=shared.max_retries )
  {
    jdk_log_debug1( "(%d) %s connection failed to server for %s", my_id, incoming_addr.c_str(), current_url.c_str() );
    
    respond_with_connection_error_page( current_url );
    
    return false; // failure to connect
  }
  
  
  jdk_string_url unescaped_path;
  
  unescaped_path.cpy( http_request.get_url().path.c_str() + 1 ); // skip the leading '/'
  
  jdk_cgi_unescape( unescaped_path );
  
  outgoing->print( "%s\r\n", unescaped_path.c_str() );
  incoming->write_string_block( "HTTP/1.0 200 Connection established\r\n\r\n" );
  
  if2k_http_proxy_xfer_processor xfer( shared, incoming_addr, current_url, http_request, http_response, nofilter );
  
  
  jdk_socket_transfer_with_processing(
    my_id,
    incoming,
    outgoing,
    &xfer
    );
  
  jdk_log( JDK_LOG_DEBUG1, "HTTP gopher finished for %s", current_url.c_str() );
  
  return true;
  
}

bool if2k_http_proxy_worker::handle_ftp_mode()
{
  respond_with_redirect_page( shared.further_proxy );
  return true;
}


bool if2k_http_proxy_worker::transfer_from_server()
{
  bool sent_initial_response=false;
  //bool prepended_initial_response=false;
  bool direction=false;
  
  bool has_content_length=false;
  int content_length=0;
  int total_received=0;
  
  const jdk_http_response_header::value_t *con_len_str;
  
  if( http_response.find_entry("content-length:",&con_len_str)!=-1 )
  {
    has_content_length=true;
    content_length=con_len_str->strtol();
    jdk_log_debug1( "server response has content-length %d", content_length );
  }
  
  // any response to a HEAD request has no payload, even though it has a content length field
  if( http_request.get_request_type() == jdk_http_request_header::REQUEST_HEAD )
  {
    has_content_length=true;
    content_length=0;
    jdk_log_debug3( "Response is from HEAD request. No content expected" );
  }
  
  if( !has_content_length )
  {
    if(
      http_response.get_http_response_code()==304 
      || http_response.get_http_response_code()==204
      || (http_response.get_http_response_code()>=100
          && http_response.get_http_response_code()<=199)
      )
    {
      // these messages always have 0 content length, even if the server doesn't say so.
      has_content_length=true;
      content_length=0;
      jdk_log_debug3( "Response has no content-length but code is %d", http_response.get_http_response_code() );
    }
  }
  
#if 0	
  if( settings.get_long("tunnel_mode")==1 )
  {
    int pre_len = buf_len-http_response.get_buf_pos();
    if( pre_len>0 )
    {
      incoming->write_data_block( & buf[ http_response.get_buf_pos() ], pre_len );
    }
#if 0
    if( content_length>=pre_len )
    {
      content_length -= pre_len;
      
      while( content_length >0 )
      {
        int thiscount = 
          jdk_socket_transfer_one_block(
            my_id,
            outgoing,
            incoming,
            content_length,
            false
            );		
        if( thiscount<=0 )
        {
          jdk_log_debug2( "error transferring before content length complete" );
          break;
        }
        else
        {
          content_length-=thiscount;
          jdk_log_debug2( "transferred: %ld, %ld to go", thiscount, content_length );
        }
      }
    }
    else
#endif
    {
      jdk_log_debug2("unknown left" );
      jdk_socket_transfer(
        my_id,
        outgoing,
        incoming
        );		
    }
  }
  else
#endif
    while( incoming->is_open_for_writing() && outgoing->is_open_for_reading() )
    {
      // copy data from server (outgoing) to client (incoming)
      if2k_http_proxy_xfer_processor xfer( shared, incoming_addr, current_url, http_request, http_response, nofilter );
      
      unsigned char xfer_buf[16384];
      
      int xfer_buf_byte_count=0;
      int xfer_buf_todo=sizeof(xfer_buf);
      
      if( has_content_length )
      {
        if( xfer_buf_todo>content_length - total_received )
        {
          xfer_buf_todo=content_length - total_received;
        }
      }
      
      // is this our first buffer?
      if( !sent_initial_response )
      {
        // yes, first prefix the initial data that was read
        int pre_len = buf_len-http_response.get_buf_pos();
        if( pre_len>0 )
        {
          if( pre_len>=(int)sizeof(xfer_buf) )
          {
            jdk_log( JDK_LOG_ERROR, "(%d) pre_len is too big (%d)", my_id, pre_len );
            return false;
          } 
          // prepend initial data into buf
          memcpy( xfer_buf, buf + http_response.get_buf_pos(), pre_len );
          xfer_buf_byte_count=pre_len;
          xfer_buf_todo-=pre_len;
          jdk_log_debug1("(%d) prepending %d bytes", my_id, pre_len );
        }
      }
      
      // try load up the full xfer_buf
      int readlen=0;
      while( xfer_buf_todo>0 )
      {
        readlen = outgoing->read_data(xfer_buf+xfer_buf_byte_count, xfer_buf_todo );
        if( readlen==0 || readlen==-1)
        {	
          break;
        }
        xfer_buf_todo -= readlen;
        xfer_buf_byte_count += readlen;
      }
      
      // scan it
      if( xfer_buf_byte_count>0 )
      {
        jdk_log_debug3( "*** (%d) Received %d bytes, ready to scan.", my_id, xfer_buf_byte_count );
        
        int len = xfer.scan_outgoing( xfer_buf, xfer_buf_byte_count, sizeof( xfer_buf ) );
        
        if( len<=0 )
        {
          // scanner says stop the transfer!
          if( sent_initial_response )
          {
            // but we already sent the first chunk so all we can do is abort it.
            jdk_log_debug1( "(%d) aborting page due to match in data", my_id );
            return false;
          }					
          else
          { 					
            // luckily we did not send the initial response yet, so we can redirect!
            do_redirect=true;                    
            keep_alive=false;
            jdk_log_debug1( "(%d) redirecting page due to match in data", my_id );
            
            jdk_string_url redirect_url;
            
            if( shared.filter->generate_blocked_page_url(
                  redirect_url, 
                  http_request,
                  incoming_addr, 
                  3, 
                  xfer.match_phrase
                  ) ) 
            {
              respond_with_redirect_page( redirect_url );							
            }
            
            return false;
          }                
        }            
      }
      
      // ok, time to send the initial response if we havent yet
      if( !sent_initial_response )
      {				
        jdk_log_debug1("(%d) sending server response after %d bytes read from server data", my_id, xfer_buf_byte_count );
        if( !send_server_response() )
        {
          jdk_log(JDK_LOG_DEBUG3,"(%d) unable to send server response to client", my_id );
          keep_alive=false;
          return false;
        }
        sent_initial_response=true;
      }
      
      // now send the client the data that we received from the server
      if( xfer_buf_byte_count>0 )
      {
        int bytes_sent = incoming->write_data_block( xfer_buf, xfer_buf_byte_count );
        if( bytes_sent==xfer_buf_byte_count )
        {
          jdk_log( JDK_LOG_DEBUG4, "(%d) %d bytes written to %s", my_id, xfer_buf_byte_count, direction ? "server" : "client" );
        }
        else
        {
          jdk_log( JDK_LOG_DEBUG4, "(%d) failed writing %d bytes to %s", my_id, xfer_buf_byte_count, direction ? "server" : "client" );
          keep_alive=false;
          return false;
        }
      }
      
      // if there was no data, then we are done
      if( xfer_buf_byte_count<=0 )
      {
        // no sense for keep alive, this eof was not expected.
        keep_alive=false;
        jdk_log( JDK_LOG_DEBUG4, "(%d) server closed socket, total_received=%d", my_id, total_received );
        if( has_content_length && total_received<content_length )
        {
          jdk_log( JDK_LOG_DEBUG3, "(%d) premature close. I wanted %d, got %d", my_id, content_length, total_received );
        }
        return true;
      }
      
      // and count up how many bytes we have done
      
      total_received += xfer_buf_byte_count;
      
      if( has_content_length && total_received>=content_length )
      {
        jdk_log_debug3( "(%d) all content transferred total_received=%d", my_id, total_received );
        //incoming->write_string_block( "\r\n" );
        
        return true; 	// completed the transfer! Yay!
      }        
      
    }
  return true;
}

bool if2k_http_proxy_worker::transfer_to_server()
{
  if( remaining_post_data_content_length==-1 )
  {
    // unknown content length.
    if2k_http_proxy_xfer_processor xfer( shared, incoming_addr, current_url, http_request, http_response, nofilter );
    
    int byte_count =jdk_socket_transfer_one_block_with_processing(
      my_id,
      incoming,
      outgoing,
      -1,
      false,
      &xfer
      );
    
    if( byte_count<=0 )
    {
      // Normally we would do: outgoing->shutdown_write();
      // here but many other servers fuck up because of non-standard compliance
      return true;
    }
  }
  else
  {		
    while( outgoing->is_open_for_writing()
           && incoming->is_open_for_reading()
           && remaining_post_data_content_length>0
      )
    {
      jdk_log_debug2( "(%d) %s remaining post data length=%d", my_id, incoming_addr.c_str() , remaining_post_data_content_length );
      
      // copy data from server (outgoing) to client (incoming)
      
      if2k_http_proxy_xfer_processor xfer( shared, incoming_addr, current_url, http_request, http_response, nofilter );
      
      
      int byte_count = jdk_socket_transfer_one_block_with_processing(
        my_id,
        incoming,
        outgoing,
        remaining_post_data_content_length,
        true,
        &xfer
        );
      
      if( byte_count<0 )
      {
        return false;
      }
      
      if( byte_count==0 )
      {
        jdk_log( JDK_LOG_DEBUG1, "client closed socket during POST, %d bytes remaining", remaining_post_data_content_length );
        // yup, we are done. so close the outgoing socket for writing
        //outgoing->shutdown_write();
        break;
      }
      
      
      if( post_data_content_length>0 )
      {
        remaining_post_data_content_length -= byte_count;
        
        if( remaining_post_data_content_length == 0 )
        {
          // we are done sending all post data!
          //outgoing->write_string_block("\r\n\r\n");
          break;
        }
      }
      
    }
  }		
  return true;	
}

bool if2k_http_proxy_worker::transform_http_request()
{
  // extract the full URL for future reference
  current_url = http_request.get_url().unexplode();
  
//	jdk_log( JDK_LOG_INFO, "(%d) %s Requests %s", my_id, incoming_addr.c_str() , current_url.c_str() );
//	jdk_log( JDK_LOG_INFO, "(%d) unexploded: protocol %s ", my_id, http_request.get_url().protocol.c_str() );
//	jdk_log( JDK_LOG_INFO, "(%d) unexploded: host %s ", my_id, http_request.get_url().host.c_str() );
//	jdk_log( JDK_LOG_INFO, "(%d) unexploded: port %d ", my_id, http_request.get_url().port );
//	jdk_log( JDK_LOG_INFO, "(%d) unexploded: path %s ", my_id, http_request.get_url().path.c_str() );
//	jdk_log( JDK_LOG_INFO, "(%d) unexploded: file %s ", my_id, http_request.get_url().file.c_str() );
  jdk_log_debug2( "(%d) %s Request type: %d", my_id, incoming_addr.c_str() , (int)http_request.get_request_type() );
  
  do_redirect=false;    // default to no redirect
  
  jdk_str<1024> match_phrase;
  if( !nofilter )
  {	
    jdk_synchronized( shared.hack_mutex );						
    do_redirect = shared.filter->test_url( http_request, current_url, transformed_url, transformed_proxy, incoming_addr, match_phrase );
  }
  
  
  if( do_redirect )
  {
    jdk_log( JDK_LOG_DEBUG1, "(%d) %s Redirected %s to %s",
             my_id,
             incoming_addr.c_str(),
             current_url.c_str(),
             transformed_url.c_str()
      );
    // are we to use a further proxy as well?
    
    if( !shared.further_proxy.is_clear() )
    {
      jdk_synchronized( shared.hack_mutex );							
      transformed_proxy=shared.further_proxy;
      use_proxy=true;
    }
    else
    {
      use_proxy=false;
    }
    
    return true;
  }
  
  // did the filter request that this URL be accessed via a special proxy?
  
  if( transformed_proxy.len()>3 )
  {
    // yup. use redirected_proxy.
    use_proxy=true;
    
    jdk_log( JDK_LOG_DEBUG1, "(%d) %s Using proxy %s for %s",
             my_id,
             incoming_addr.c_str(),
             transformed_proxy.c_str(),
             current_url.c_str()
      );
  }
  else
  {
    // no, the filter doesnt care.
    // are we supposed to use a proxy anyways?
    if( !shared.further_proxy.is_clear() )
    {
      // yes, use it then.
      jdk_synchronized( shared.hack_mutex );							
      transformed_proxy=shared.further_proxy;
      use_proxy = true;
    }
    else
    {
      use_proxy = false;
    }
  }
  
  
  // take away the original connection flags for the proxy
  http_request.delete_entry( "proxy-connection:" );
  http_request.delete_entry( "Connection:" );
  http_request.delete_entry( "Keep-Alive:" );
  //http_request.delete_entry( "Accept-Encoding:" );
  
  // TODO: there are other Proxy- items to keep track of.
  
  // add a special tag so we can recognize loopbacks to ourselves
  http_request.set_entry( "X-if2002-id:", shared.my_proxy_id );
  
  if( settings.get_long("use_keep_alive")==1 )
  {
    http_request.set_entry( "Connection:", "Keep-Alive" );
  }
  else
  {
    http_request.set_entry( "Connection:", "close" );
  } 
  
  if( use_proxy )
  {
    http_request.set_entry( "Proxy-connection:", "close" );
  }
  
  http_request.set_http_version( jdk_str<64>("HTTP/1.0") ); 	// HACK! forcing http to use version 1.0
  
  // change the url in the http_request to our transformed one
  if( http_request.get_request_type()!=jdk_http_request_header::REQUEST_CONNECT )
  {
    jdk_url new_url;
    new_url.explode( transformed_url );
    http_request.set_url( new_url );
  }
  
  http_transformed_request.clear();
  http_request.flatten( http_transformed_request, use_proxy );  
  
  // was it a http post request?
  const jdk_http_request_header::value_t *v;
  
  if( http_request.find_entry("content-length:", &v )!=-1 )
  {
    // find how much total post data we have
    post_data_content_length = v->strtol();
    remaining_post_data_content_length = post_data_content_length;
    
    // yup, we may still have data in buf[] that needs to be sent as well.
    int pos = http_request.get_buf_pos();
    int len = buf_len - pos;
    if( len>0 )
    {
      jdk_dynbuf dummy;
      //jdk_str<1024> match_phrase;
      bool r=true;
      
      if( !nofilter )
      {
        jdk_synchronized( shared.hack_mutex );
        r = shared.filter->test_client_send_packet( current_url, http_request, (unsigned char *)&buf[pos], len, dummy, incoming_addr, match_phrase );
      }
      
      if( r==false )
      {
        jdk_string_url redirect_url;
        
        {
          jdk_synchronized( shared.hack_mutex );
          r = shared.filter->generate_blocked_page_url(
            redirect_url, 
            http_request,
            incoming_addr, 
            3, 
            match_phrase
            );
        }
        
        if( r )
        {
          respond_with_redirect_page( redirect_url );							
        }				
        return 0;
      }																   			
      
      jdk_log_debug2( "(%d) %s pre-sending %d bytes of POST data", my_id, incoming_addr.c_str() , len );
//		    jdk_log_debug4( "(%d) %s POST data is:\n%s\n", my_id, incoming_addr.c_str() , &buf[pos] );
      
      http_transformed_request.append_from_data(
        &buf[pos],
        len
        );
    }
    else
    {
      jdk_log_debug3( "(%d) %s no pre-sending of POST data: pos=%d buf_len=%d", my_id, incoming_addr.c_str(), pos, buf_len  );
    }
    post_data_presend_count = len;
  }
  
  return true;
}


bool if2k_http_proxy_worker::transform_server_response()
{
  // TODO: extract Content-Length and 'Transfer-Encoding: chunked' and deal with it
  bool r=http_response.flatten( http_transformed_response );
  if( r )
  {
    jdk_log_debug2( "(%d) %s http_response.flatten succeeded.", my_id, incoming_addr.c_str()  );
    
    if( http_response.get_http_version().icmp("HTTP/0.9")==0 )
    {
      keep_alive=false;
    }
    else
    {
      const jdk_http_response_header::value_t *v;
      if( http_response.find_entry("connection:",&v)!=-1 )
      {
        if( v->icmp("close")==0 )
        {
          keep_alive=false;
        }
      }
      if( http_response.find_entry("content-length:",&v)==-1 )
      {
        keep_alive=false;
      }                
    }
    
    
  }
  else
  {
    jdk_log_debug1( "(%d) http_response.flatten failed.", my_id, incoming_addr.c_str()  );
  }    
  return r;
}

bool if2k_http_proxy_worker::send_server_response()
{
  bool r=false;
  
  
  if( incoming->write_data_block( 
        http_transformed_response
        )==http_transformed_response.get_data_length() )
  {
    jdk_log_debug2( "(%d) %s write to client of response header succeeded.", my_id, incoming_addr.c_str()  );    
    
    r=true;
  }	 
  else
  {
    jdk_log_debug1( "(%d) %s write to client of response header failed.", my_id, incoming_addr.c_str()  );    
  }
  
  return r;   					
}


bool if2k_http_proxy_worker::make_retried_cached_connection_with_request()
{
  for( int tries=0; tries<shared.max_retries; tries++ )
  {
    if( tries!=0 )
    {
      jdk_log_debug1( "(%d) %s retry #%d", my_id, incoming_addr.c_str() , tries );
      // couldnt make connection. sleep a bit and try again
      jdk_sleepms( shared.retry_delay );
    }
    
    // connect to the server
    if( make_cached_connection() )
    {
      jdk_log_debug1( "(%d) %s made connection to %s %d attempts", my_id, incoming_addr.c_str() , outgoing_addr.c_str() , tries+1 );
      
      // we have a connection to the server. Try send the request to it. The
      // server may have closed the connection in the meantime so we would have to retry.
      
      if( write_http_header() )
      {
        jdk_log_debug1( "(%d) %s wrote request to server successfully", my_id, incoming_addr.c_str()  );
        
        // was it a request with a content-length payload?
        if( http_request.find("content-length:")!=-1 )
        {
          // yup, then we still may have more stuff to send to the server.
          
          if( transfer_to_server() )
          {
            if( receive_server_response() )
            {
              if( tries!=0 )
              {
                jdk_log( JDK_LOG_DEBUG1, "(%d) %s Negotiated %s after %d retries", my_id, incoming_addr.c_str() , current_url.c_str(), tries );					
              }
              return true;
            }
            else
            {
              jdk_log_debug1( "(%d) %s unable to receive complete server response after POST", my_id, incoming_addr.c_str()  );
              outgoing->close();
            }
          }
          else
          {
            jdk_log_debug1( "(%d) %s unable to write complete request", my_id, incoming_addr.c_str()  );
            outgoing->close();
          }
        }
        else
        {
          if( receive_server_response() )
          {
            if( tries!=0 )
            {
              jdk_log( JDK_LOG_DEBUG1, "(%d) %s Negotiated %s after %d retries", my_id, incoming_addr.c_str() , current_url.c_str(), tries );					
            }
            
            return true;
          }
          else
          {
            jdk_log_debug1( "(%d) %s unable to receive complete server response from %s", my_id, incoming_addr.c_str() , current_url.c_str() );
            outgoing->close();
          }
        }
      }
      else
      {
        // no go, try make the connection again.
        jdk_log_debug1( "(%d) %s unable to write request to to %s", my_id, incoming_addr.c_str() , current_url.c_str() );
        // force the cached connection to be closed.
        outgoing->close();
        // and we will loop through again hopefully
      }
    }
  }
  
  jdk_log( JDK_LOG_DEBUG1, "(%d) %s failed connection for %s after %d retries", my_id, incoming_addr.c_str() , current_url.c_str(), shared.max_retries );
  return false;
}


bool if2k_http_proxy_worker::make_cached_connection()
{
  
  // figure out what server we really want to be connected to
  jdk_string_host real_host;
  int real_port;
  
  jdk_url url;
  url.explode( transformed_url );
  
  jdk_url proxy_url;
  proxy_url.explode( transformed_proxy );
  
  if( use_proxy )
  {
    real_host = proxy_url.host;
    real_port = proxy_url.port;
  }
  else
  {
    real_host = url.host;
    real_port = url.port;
  }
  
  if( real_port==-1 )
    real_port = 80;
  
  // Were we already connected to the server we want?
  if( real_host == cached_destination_host && real_port == cached_destination_port)
  {
    if( outgoing->is_open_for_writing() )
    { 
      jdk_log_debug3( "(%d) %s Already connected to desired host and port, %s:%d", my_id, incoming_addr.c_str() , real_host.c_str(), real_port );
      // yup! then we are done!
      return true;
    }
    else
    {
      jdk_log_debug3( "(%d) %s:%d WAS already connected but not anymore", my_id, real_host.c_str(), real_port);
    }    
  }
  
  
  {
    jdk_log_debug2( "(%d) %s About to make real connection to %s:%d", my_id, incoming_addr.c_str() , real_host.c_str(), real_port );
    
    // no, it wasn't. We need to make a new connection
    if( outgoing->make_connection( real_host.c_str(), real_port, 0, false ) ) // no dns cache
    {
      // yay we made a connection. Remember the host and port, and get the real remote address
      cached_destination_host = real_host;
      cached_destination_port = real_port;
      jdk_log_debug3( "(%d) %s connected to %s:%d", my_id, incoming_addr.c_str(),
                      real_host.c_str(), real_port );
      
      outgoing->get_remote_addr( outgoing_addr.c_str(), outgoing_addr.getmaxlen() );
      
      return true;	// success
    }
  }
  return false;
}


