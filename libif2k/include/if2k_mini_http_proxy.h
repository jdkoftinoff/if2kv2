#ifndef __IF2K_MINI_HTTP_PROXY_H
#define __IF2K_MINI_HTTP_PROXY_H

#include "if2k_mini_config.h"
#include "if2k_mini_server.h"
#include "if2k_mini_tunnel.h"

#include "jdk_http.h"


class if2k_mini_http_proxy : public if2k_mini_server
{
  if2k_mini_http_proxy( const if2k_mini_http_proxy &o );
  const if2k_mini_http_proxy & operator = ( const if2k_mini_http_proxy &o );
  
public:
  explicit if2k_mini_http_proxy( const jdk_settings &settings_, if2k_mini_kernel &kernel_ );
  virtual ~if2k_mini_http_proxy();
  
  virtual void load_settings();
  
  virtual bool handle_connection( jdk_client_socket &sock );
  
  
  bool is_request_local( const jdk_http_request_header &request_header );
  bool is_request_blocked( 
    const jdk_http_request_header &request_header, 
    jdk_string &redir_url,
    jdk_settings &redirection_info,
    if2k_mini_kernel_request &if2k_request
    );
  
  bool redirect_if_blocked_image( 
    const jdk_string &normalized_url, 
    jdk_string &redir_url
    );
  
  bool make_redir_response( 
    jdk_dynbuf &response_buf, 
    const jdk_string_url &redirection_destination,
    const jdk_settings &redirection_cgi_params
    ) const;
  
  
  jdk_string_url runtime_id;
  jdk_string_url http_proxy;
  bool http_proxy_auth_basic_enable;
  jdk_string_url http_proxy_auth_basic_username;
  jdk_string_url http_proxy_auth_basic_password;
  jdk_string_url proxy_auth;
  jdk_string_url httpfilter_interface;
  jdk_string_url remote_update_password;
  bool remote_update_enable;

  bool blocking_enable;
  bool blocking_referer_enable;
  bool block_bad_images;
  bool block_bad;
  bool override_enable;

  jdk_string_url blockpage_url;
  jdk_string_url block_image_jpg_url;
  jdk_string_url block_image_gif_url;
  jdk_string_url block_image_png_url;
  
  int httpfilter_max_retries;
  int httpfilter_retry_delay;
  
  jdk_string_filename httpfilter_server_webdir;
  jdk_string_url httpfilter_server_error_404;
  jdk_string_url httpfilter_server_error_connection;
  
  long scanner_queue_size;
};

class if2k_http_gen_manager
{
public:
  if2k_http_gen_manager(
    if2k_mini_http_proxy &parent_, 
    jdk_client_socket &client_sock_,
    jdk_http_request_header &request_header_,
    jdk_dynbuf &request_additional_data_
    )
    :
  parent( parent_ ),
    client_sock( client_sock_ ),
    request_header( request_header_ ),
    request_additional_data( request_additional_data_ ),
    request_url( request_header.get_url().unexplode() )
  {
  }
  
  virtual ~if2k_http_gen_manager()
  {
  }
  
  virtual bool process();
protected:
  
  bool handle_get();
  bool handle_post();
  
  bool handle_get_local_file();
  bool handle_get_cgi(); 
  bool handle_post_cgi();
  
  bool handle_get_override();
  bool handle_post_override();

  bool handle_get_settings();
  bool handle_post_settings();
  
  bool handle_get_dbfile();
  bool handle_post_dbfile();
  
  bool handle_get_predbfile();
  bool handle_post_predbfile();
  
  bool handle_get_restart();
  bool handle_get_precompile();

  jdk_string_filename get_local_filename( const jdk_url &url, jdk_settings &get_params );
  const char *get_mime_type_for_file( const char *extension, bool &do_template );
  
  if2k_mini_http_proxy &parent;
  jdk_client_socket &client_sock;
  jdk_http_request_header &request_header;
  jdk_dynbuf &request_additional_data;
  jdk_string_url request_url;
  
  jdk_string_filename content_path;
  jdk_settings cgi_params;
  jdk_http_response_header response_header;
  jdk_dynbuf response_content;
};

class if2k_mini_xfer_manager
{
public:
  if2k_mini_xfer_manager( 
    if2k_mini_http_proxy &parent_, 
    jdk_client_socket &client_sock_,
    jdk_http_request_header &request_header_,
    jdk_dynbuf &request_additional_data_,
    if2k_mini_kernel_request &kernel_request_,
    bool license_valid_
    )
    : 
    parent( parent_ ),
    client_sock( client_sock_ ),
    request_header( request_header_ ),
    request_additional_data( request_additional_data_ ),
    request_url( request_header.get_url().unexplode() ),
    kernel_request( kernel_request_ ),
    license_valid( license_valid_ )
  {         
  }
  
  virtual ~if2k_mini_xfer_manager()
  {
  }
  
  virtual bool process() = 0;
  
protected:
  if2k_mini_http_proxy &parent;
  jdk_client_socket &client_sock;
  jdk_http_request_header &request_header;
  jdk_dynbuf &request_additional_data;
  jdk_string_url request_url;         
  if2k_mini_kernel_request &kernel_request; 
  bool license_valid;
};

class if2k_http_xfer_manager : public if2k_mini_xfer_manager
{
public:
  if2k_http_xfer_manager( 
    if2k_mini_http_proxy &parent_, 
    jdk_client_socket &client_sock_,
    jdk_http_request_header &request_header_,
    jdk_dynbuf &request_additional_data_,
    if2k_mini_kernel_request &kernel_request_,
    bool license_valid_
    )
    : 
    if2k_mini_xfer_manager( 
      parent_, 
      client_sock_,
      request_header_,
      request_additional_data_,
      kernel_request_,
      license_valid_
      )
  {         
  }
  
  virtual ~if2k_http_xfer_manager()
  {
  }
  
  virtual bool process();
  
};

class if2k_https_xfer_manager : public if2k_mini_xfer_manager
{
public:
  if2k_https_xfer_manager( 
    if2k_mini_http_proxy &parent_, 
    jdk_client_socket &client_sock_,
    jdk_http_request_header &request_header_,
    jdk_dynbuf &request_additional_data_,
    if2k_mini_kernel_request &kernel_request_,
    bool license_valid_
    )
    : 
    if2k_mini_xfer_manager( 
      parent_, 
      client_sock_,
      request_header_,
      request_additional_data_,
      kernel_request_,
      license_valid_
      )
  {         
  }
  
  virtual ~if2k_https_xfer_manager()
  {
  }
  
  virtual bool process();
};


class if2k_http_redirect_manager
{
public:
  if2k_http_redirect_manager(
    const if2k_mini_http_proxy &parent_, 
    jdk_client_socket &client_sock_,
    const jdk_http_request_header &request_header_,
    const jdk_dynbuf &request_additional_data_,
    const jdk_string_url &redirection_destination_,
    const jdk_settings &redirection_cgi_params_
    )
    :
  parent( parent_ ),
    client_sock( client_sock_ ),
    request_header( request_header_ ),
    request_additional_data( request_additional_data_ ),
    redirection_destination( redirection_destination_ ),
    redirection_cgi_params( redirection_cgi_params_ ),
    request_url( request_header.get_url().unexplode() )
  {
  }
  
  virtual ~if2k_http_redirect_manager()
  {
  }
  
  virtual bool process();
protected:
  const if2k_mini_http_proxy &parent;
  jdk_client_socket &client_sock;
  const jdk_http_request_header &request_header;
  const jdk_dynbuf &request_additional_data;
  const jdk_string_url &redirection_destination;
  const jdk_settings &redirection_cgi_params;
  jdk_string_url request_url;          
  
  static char *http_redirect;
};

class if2k_mini_scanning_tunnel : public if2k_mini_tunnel
{
public:
  if2k_mini_scanning_tunnel(
    jdk_client_socket &incoming_,
    jdk_client_socket &outgoing_,
    if2k_mini_http_proxy &parent_,
    if2k_mini_kernel_request request_,
    jdk_http_request_header &request_header_,
    int queue_size,
    bool license_valid_
    ) 
    : 
    if2k_mini_tunnel(incoming_, outgoing_, queue_size,queue_size,0),
    parent( parent_ ),
    request( request_ ),
    request_header( request_header_ ),
    blocking_applied( false ),
    license_valid( license_valid_ )
  {
  }
  
  const if2k_mini_kernel_request &get_blocked_status() const
  {
    return request;
  }
  
protected:
  virtual bool handle_client_incoming_data( jdk_buf &data );
  virtual bool handle_server_incoming_data( jdk_buf &data );
  
  
  if2k_mini_http_proxy &parent;
  if2k_mini_kernel_request request;
  jdk_http_request_header &request_header;
  
  bool blocking_applied;
  bool license_valid;
};




#endif
