#include "jdk_world.h"
#include "jdk_log.h"
#include "jdk_httprequest.h"
#include "jdk_cgi.h"
#include "jdk_dynbuf.h"
#include "jdk_base64.h"

bool jdk_http_form_request(
  jdk_string &connect_host,        // will be filled in with host name to connect to
  int &connect_port,               // will be filled in with port to connect to
  jdk_buf &request,                // will be filled in with raw data to send
  const char *raw_url,             // raw unescaped url to request
  const char *http_request_type,   // "GET" or "HEAD" or "POST", etc
  const char *proxy,               // raw unescaped proxy address/url, blank or null for none
  int content_length,              // for POST or PUT, or -1 for none
  const char *content_type,        // for POST or PUT, blank or null for none
  const char *additional_headers,  // single multi-line string to insert in headers,
  const char *http_version,         // http version code to use,
  const char *proxy_basic_auth_username,
  const char *proxy_basic_auth_password
  )
{
  bool use_proxy=false;
  
  jdk_log_debug4( "httprequest: forming request header for '%s'", raw_url );	
  // clean up the url and escape it
  jdk_string_url pre_cleaned_url( raw_url );
  pre_cleaned_url.strip_endws();
  pre_cleaned_url.strip_begws();
  jdk_string_url cleaned_url;
  jdk_cgi_escape( pre_cleaned_url, cleaned_url );
  jdk_log_debug4( "httprequest: point 1" );
  
  // explode the request url
  jdk_url exploded_url;
  if( exploded_url.explode(cleaned_url)!=1 )
  {
    jdk_log_debug2( "httprequest: bad url '%s'", raw_url );
    return false;
  }
  jdk_log_debug4( "httprequest: point 2" );
  // explode the proxy location (if any)
  jdk_url exploded_proxy;
  jdk_string_url pre_cleaned_proxy;

  if( proxy && *proxy )
  {
    pre_cleaned_proxy.cpy( proxy );
    pre_cleaned_proxy.strip_endws();
    pre_cleaned_proxy.strip_begws();
  }
  jdk_log_debug4( "httprequest: point 3" );
  // do we have valid proxy info?
  if( !pre_cleaned_proxy.is_clear() )
  {
    // yup, escape it
    jdk_string_url cleaned_proxy;
    jdk_cgi_escape( pre_cleaned_proxy, cleaned_proxy );
    
    // can we extract the info?
    if( exploded_proxy.explode(cleaned_proxy)==1 )
    {
      // yes, we have a valid proxy setting, so use it
      use_proxy=true;
    }
    else
    {
      jdk_log_debug2( "httprequest: bad proxy url '%s'", proxy );
      // invalid proxy setting
      return false;
    }

  }

  jdk_string_url proxy_auth;
  if( use_proxy )
  {  
    if( proxy_basic_auth_username && proxy_basic_auth_password )
    {
      jdk_string_url pre_proxy_auth;
      pre_proxy_auth.cat( proxy_basic_auth_username );
      pre_proxy_auth.cat( ":" );
      pre_proxy_auth.cat( proxy_basic_auth_password );
      char base64_proxy_auth[4096];
      jdk_base64_encode( pre_proxy_auth.c_str(), pre_proxy_auth.len(), base64_proxy_auth, sizeof( base64_proxy_auth ) - 1 );
      proxy_auth.cpy( "Basic " );
      proxy_auth.cat( base64_proxy_auth );      
    }
  }

  // set the connect_host and connect_port
  if( use_proxy )
  {
    // from the proxy info
    connect_host.cpy( exploded_proxy.host );
    if( exploded_proxy.port==-1 )
      connect_port = 80;
    else
      connect_port = exploded_proxy.port;		
  }
  else
  {
    // from the host info
    connect_host.cpy( exploded_url.host );
    if( exploded_url.port==-1 )
      connect_port = 80;
    else
      connect_port = exploded_url.port;
  }
  
  // clear the request buffer
  request.clear();
  
  // do the first request line
  if( use_proxy )
  {
    // form the first request line with full url info
    request.append_form( "%s %s HTTP/%s\r\n", http_request_type, cleaned_url.c_str(), http_version );
  }
  else
  {
    // form the first request line with a non-proxy request
    request.append_form( 
      "%s %s HTTP/%s\r\n", 
      http_request_type,
      exploded_url.path.c_str(),
      http_version
      );
  }
  
  // add the 'Host:' line if port != 80
  jdk_log_debug4( "httprequest: point 4" );
  if( exploded_url.port!=80 && exploded_url.port!=-1 )
  {
    request.append_form( 
      "Host: %s:%d\r\n", 
      exploded_url.host.c_str(),
      exploded_url.port
      );
  }
  else
  {
    request.append_form( 
      "Host: %s\r\n", 
      exploded_url.host.c_str()
      );
  }
    

  // accept every mime type
  request.append_from_string( "Accept: */*\r\n" );
  jdk_log_debug4( "httprequest: point 5" );
  // tell connection to close
  if( use_proxy )
  {
    // for proxy, no cache either
    request.append_from_string( "Pragma: no-cache\r\n" );
    request.append_from_string( "Proxy-Connection: Close\r\n" );
    if( proxy_auth.len() > 0 )
    {
      jdk_log_debug4( "httprequest: proxy-authorization is %s", proxy_auth.c_str() );
      request.append_form( "Proxy-Authorization: %s\r\n", proxy_auth.c_str() );
    }
  }
  else
  {
    // simple for non-proxy
    request.append_from_string( "Connection: close\r\n" );
  }
  jdk_log_debug4( "httprequest: point 6" );	
  // do we have a content type header to send?
  if( content_type && *content_type )
  {
    // yes!
    request.append_form( "Content-type: %s\r\n", content_type );
  }
  jdk_log_debug4( "httprequest: point 7" );
  // do we have a content length header to send?
  if( content_length !=-1 && jdk_strcmp(http_request_type,"GET")!=0)
  {
    // yes!
    request.append_form( "Content-length: %d\r\n", content_length );
  }
  jdk_log_debug4( "httprequest: point 8" );
  // do we have any additional headers to send?
  if( additional_headers && *additional_headers )
  {
    // yes!
    request.append_from_string( additional_headers );
  }
  
  // we have finished forming our request. Polish it off with the extra crlf
  request.append_from_string( "\r\n" );
  jdk_log_debug3( "httprequest: success forming request for '%s'", raw_url );
  
  jdk_str<4096> dumpline;
  int pos=0;
  while( (pos=request.extract_to_string( dumpline, pos ))>0 )
  {
    jdk_log_debug4("httprequest: %s", dumpline.c_str() );
  }
  
  
  return true;
}


// read until socket is closed or max_response_size bytes read

int jdk_http_receive_buf( jdk_inet_client_socket &s, jdk_buf &full_response, size_t max_response_size )
{
  while( s.is_open_for_reading() )
  {
    char buf[16384];
    int len;
    
    len = s.read_data( buf, sizeof(buf) );
    if( len<0 )
    {
      jdk_log_debug2( "http_receive_buf: error reading data - so far %d", full_response.get_data_length() );				
      // error reading the data!
      return -1;
    }
    
    if( len==0 )
    {
      // socket is closed
      break;
    }
    
    if( full_response.get_data_length() + len > max_response_size )
    {
      // too much data!
      jdk_log_debug2( "http_receive_buf: too much data received so far %d", full_response.get_data_length() + len );				
      return -1;
    }
    
    // just right, append the data
    full_response.append_from_data( buf, len );
    
    if( full_response.get_data_length() == max_response_size )
    {
      // got the complete buffer
      jdk_log_debug3( "http_receive_buf: received exact size as requested: %d", full_response.get_data_length() );						   
      return (int)full_response.get_data_length();
    }
  }
  jdk_log_debug3( "http_receive_buf: received : %d", full_response.get_data_length() );
  return (int)full_response.get_data_length();
}


int jdk_http_do_request(
  const char *http_request_type, // "GET" or "POST"
  const char *url,
  jdk_buf *response,
  long max_response_size,
  jdk_http_response_header *response_header,
  const char *proxy,
  const char *additional_headers, // additional headers, 0 or blank for none
  const jdk_buf *send_content,  // data for POST or PUT, null for none
  const char *send_content_type, // POST or PUT content type, null or blank for none
  const char *http_version,
  bool use_ssl,
  const char *proxy_basic_auth_username,
  const char *proxy_basic_auth_password
  )
{
  int content_length=-1;
  // do we have content?
  if( send_content )
  {
    // yes! find out the length
    content_length = int(send_content->get_data_length());
  }
  
  jdk_str<256> connect_host;
  int connect_port;
  jdk_dynbuf request_buf;
  
  if( 		
    !jdk_http_form_request(
      connect_host, 
      connect_port,
      request_buf,
      url,
      http_request_type,
      proxy,
      content_length, 
      send_content_type,  
      additional_headers, 
      http_version,
      proxy_basic_auth_username,
      proxy_basic_auth_password
      ))
  {
    // error forming request! return failure
    jdk_log_debug2( "http_do_request: error forming request for %s", url );
    return -1;
  }
  
  jdk_inet_client_socket s;
  if( !s.make_connection( connect_host.c_str(), connect_port, 0, use_ssl ) )
  {
    // error connecting! return failure
    jdk_log_debug2( "http_do_request: connect error to %s:%d", connect_host.c_str(), connect_port );		
    return -1;
  }
  
  // send request		
  if( s.write_data_block( request_buf ) != (int)request_buf.get_data_length() )
  {
    // error sending request. return failure
    jdk_log_debug2( "http_do_request: error sending request to %s:%d", connect_host.c_str(), connect_port );			
    return -1;
  }
  
  if( content_length>0 && send_content )
  {
    if( s.write_data_block( *send_content ) != (int)send_content->get_data_length() )
    {
      jdk_log_debug2( "http_do_request: error sending request content to %s:%d", connect_host.c_str(), connect_port );			
      return -1;
    }
  }
  
  // now grab entire response
  jdk_dynbuf full_response;
  
  if( jdk_http_receive_buf( s, full_response, max_response_size )<0 )
  {
    // error receiving response. return failure
    jdk_log_debug2( "http_do_request: error receiving response from %s:%d", connect_host.c_str(), connect_port );  	   
    return -1;
  }
  
  // parse the response header
  if( !response_header->parse_buffer( full_response ) )
  {
    // error parsing the response. return failure
    jdk_log_debug2( "http_do_request: error parsing response for %s from %s:%d", url, connect_host.c_str(), connect_port );  	   		
    return -1;
  }
  
  // is it valid?
  if( !response_header->is_valid() )
  {
    // not valid. return failure
    jdk_log_debug2( "http_do_request: response header not valid for %s from %s:%d", url, connect_host.c_str(), connect_port );  	   			   
    return -1;
  }
  
  // we have a valid response, as far as the header goes.
  
  // store any response data after the header in the supplied buffer
  response->clear();
  response->append_from_buf( full_response, int(response_header->get_buf_pos()) );
  
  // TODO: double check content-length if it exists and double check the actual length   
  
  jdk_log_debug3( "http_do_request: response code %d for %s from %s:%d", 
                  response_header->get_http_response_code(), 
                  url, 
                  connect_host.c_str(), 
                  connect_port 
    );
  
  return response_header->get_http_response_code();	
}


int jdk_http_get(
  const char *url,
  jdk_buf *response,
  long max_response_size,
  jdk_http_response_header *response_header,
  const char *proxy,
  bool use_ssl,
  const char *additional_request_headers,
  const char *proxy_basic_auth_username,
  const char *proxy_basic_auth_password  
  )
{
  return
    jdk_http_do_request(
      "GET",
      url,
      response,
      max_response_size,
      response_header,
      proxy,
      additional_request_headers, 
      0, // no send content
      0, // no send content mime type
      "1.0", // http protocol version,
      use_ssl,
      proxy_basic_auth_username,
      proxy_basic_auth_password
      );
  
}

int jdk_http_head(
  const char *url,
  jdk_http_response_header *response_header,
  const char *proxy,
  bool use_ssl,
  const char *additional_request_headers,
  const char *proxy_basic_auth_username,
  const char *proxy_basic_auth_password 
  )
{
  jdk_dynbuf response;
  return
    jdk_http_do_request(
      "HEAD",
      url,
      &response,
      10*1024, // 10k max response for head content - there shouldn't be any anyway
      response_header,
      proxy,
      additional_request_headers, 
      0, // no send content
      0, // no send content mime type
      "1.0", // http protocol version,
      use_ssl,
      proxy_basic_auth_username,
      proxy_basic_auth_password
      );
}

int jdk_http_post(
  const char *url,
  jdk_buf &post_data,
  const char *post_data_mimetype,				  
  jdk_buf *response,
  long max_response_size,
  jdk_http_response_header *response_header,
  const char *proxy,
  bool use_ssl,
  const char *additional_request_headers,
  const char *proxy_basic_auth_username,
  const char *proxy_basic_auth_password 
  )
{
  return
    jdk_http_do_request(
      "POST",
      url,
      response,
      max_response_size,
      response_header,
      proxy,
      additional_request_headers,
      &post_data, // send content
      post_data_mimetype, // no send content mime type
      "1.0", // http protocol version
      use_ssl,
      proxy_basic_auth_username,
      proxy_basic_auth_password
      );
  
}


int jdk_http_put(
  const char *url,
  jdk_buf &put_data,
  const char *put_data_mimetype,				  
  jdk_buf *response,
  long max_response_size,
  jdk_http_response_header *response_header,
  const char *proxy,
  bool use_ssl,
  const char *additional_request_headers,
  const char *proxy_basic_auth_username,
  const char *proxy_basic_auth_password 
  )
{
  return
    jdk_http_do_request(
      "PUT",
      url,
      response,
      max_response_size,
      response_header,
      proxy,
      additional_request_headers,
      &put_data, // send content
      put_data_mimetype, // no send content mime type
      "1.0", // http protocol version
      use_ssl,
      proxy_basic_auth_username,
      proxy_basic_auth_password
      );
  
}


