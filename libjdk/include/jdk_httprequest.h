#ifndef _JDK_HTTP_REQUEST_H
#define _JDK_HTTP_REQUEST_H

#include "jdk_world.h"
#include "jdk_thread.h"
#include "jdk_socket.h"
#include "jdk_url.h"
#include "jdk_buf.h"
#include "jdk_string.h"
#include "jdk_http.h"


int jdk_http_get( 
				 const char *url, 
				 jdk_buf *response,
         long max_len,
         jdk_http_response_header *response_header, 
				 const char *proxy,
				 bool use_ssl,
				 const char *additional_request_headers = 0,
         const char *proxy_basic_auth_username = 0,
         const char *proxy_basic_auth_password = 0  
  );


int jdk_http_head(
				 const char *url,
				 jdk_http_response_header *response_header,
				 const char *proxy,
				 bool use_ssl,
				 const char *additional_request_headers = 0,
         const char *proxy_basic_auth_username = 0,
         const char *proxy_basic_auth_password = 0 
				 );

int jdk_http_post( 
				 const char *url,
				 jdk_buf &post_data,
				 const char *post_data_mimetype,
				 jdk_buf *response,
         long max_len,
         jdk_http_response_header *response_header, 
				 const char *proxy,
				 bool use_ssl,
				 const char *additional_request_headers = 0,
         const char *proxy_basic_auth_username = 0,
         const char *proxy_basic_auth_password = 0 
				 );

int jdk_http_put( 
				 const char *url,
				 jdk_buf &post_data,
				 const char *post_data_mimetype,
				 jdk_buf *response,
         long max_len,
         jdk_http_response_header *response_header, 
				 const char *proxy,
				 bool use_ssl,
				 const char *additional_request_headers = 0,
         const char *proxy_basic_auth_username = 0,
         const char *proxy_basic_auth_password = 0 
				 );


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
	const char *http_version = "1.0", // http version code to use
  const char *proxy_basic_auth_username = 0,
  const char *proxy_basic_auth_password = 0
	);

int jdk_http_receive_buf( jdk_inet_client_socket &s, jdk_buf &full_response, size_t max_response_size );

int jdk_http_do_request(
	const char *http_request_type, // "GET" or "POST" etc
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
  const char *proxy_basic_auth_username = 0,
  const char *proxy_basic_auth_password = 0  
	);

#endif
