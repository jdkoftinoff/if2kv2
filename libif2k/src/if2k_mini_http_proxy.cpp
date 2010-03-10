#include "jdk_world.h"
#include "if2k_mini_config.h"
#include "if2k_mini_http_proxy.h"
#include "jdk_http.h"
#include "jdk_http_server.h"
#include "jdk_html_template.h"
#include "jdk_socketutil.h"
#include "jdk_base64.h"
#include "jdk_sha1.h"

#if 0
char *if2k_http_redirect_manager::http_redirect =
"<html>\r\n"
"<head><title>Moved Temporarily</title></head>\r\n"
"<body bgcolor=\"white\" text=\"black\">\r\n"
"<font size=+1><a href=\"%s\">Moved Temporarily</a></font>\r\n"
"</html>\r\n";
#endif

if2k_mini_http_proxy::if2k_mini_http_proxy( const jdk_settings &settings_, if2k_mini_kernel &kernel_ )
  : if2k_mini_server( settings_, kernel_ )
{
}

if2k_mini_http_proxy::~if2k_mini_http_proxy()
{
}

void if2k_mini_http_proxy::load_settings()
{
  runtime_id = settings.get("runtime.id");
  http_proxy = settings.get("http.proxy");
  http_proxy_auth_basic_enable = settings.get_bool("http.proxy.auth.basic.enable");
  http_proxy_auth_basic_username = settings.get("http.proxy.auth.basic.username");
  http_proxy_auth_basic_password =settings.get("http.proxy.auth.basic.password");

  if( http_proxy_auth_basic_enable )
  {
    jdk_string_url pre_proxy_auth;
    pre_proxy_auth.cat( http_proxy_auth_basic_username );
    pre_proxy_auth.cat( ":" );
    pre_proxy_auth.cat( http_proxy_auth_basic_password );
    char base64_proxy_auth[4096];
    jdk_base64_encode( pre_proxy_auth.c_str(), pre_proxy_auth.len(), base64_proxy_auth, sizeof( base64_proxy_auth ) - 1 );
    proxy_auth.cpy( "Basic " );
    proxy_auth.cat( base64_proxy_auth );
  }

  httpfilter_interface = settings.get("httpfilter.visible.interface");
  if( httpfilter_interface.is_clear() )
    httpfilter_interface = settings.get("httpfilter.interface");

  remote_update_password = settings.get("kernel.remote.update.password");

  remote_update_enable = settings.get_bool("kernel.remote.update.enable" );
  override_enable = settings.get_bool("kernel.override.allow" );
  blockpage_url = settings.get("blockpage.url");
  block_image_jpg_url = settings.get("kernel.blocking.image.redirect.jpg.url");
  block_image_gif_url = settings.get("kernel.blocking.image.redirect.gif.url");
  block_image_png_url = settings.get("kernel.blocking.image.redirect.png.url");

  blocking_enable = settings.get_bool("kernel.blocking.enable");
  blocking_referer_enable = settings.get_bool( "kernel.blocking.referer.enable" );
  block_bad = settings.get_bool("kernel.blocking.block.bad");
  block_bad_images = settings.get_bool("kernel.blocking.image.redirect.enable");

  httpfilter_max_retries = settings.get_long("httpfilter.max_retries");
  httpfilter_retry_delay = settings.get_long("httpfilter.retry.delay");
  httpfilter_server_webdir = settings.get("httpfilter.server.webdir");
  httpfilter_server_error_404 = settings.get("httpfilter.server.error.404");
  httpfilter_server_error_connection = settings.get("httpfilter.server.error.connection");
  scanner_queue_size = settings.get_long("httpfilter.scanner.queue_size");
  if( scanner_queue_size < 4096 )
    scanner_queue_size = 4096;
}

bool if2k_mini_http_proxy::is_request_local( const jdk_http_request_header &request_header )
{
  jdk_string_host host_and_port(  request_header.get_url().get_host_and_port() );

  return host_and_port.icmp( httpfilter_interface )==0;
}

bool if2k_mini_http_proxy::redirect_if_blocked_image( 
  const jdk_string &normalized_url,
  jdk_string &redir_url
  )
{
  bool r=false;
  const char *last_dot = normalized_url.rchr('.');
  if( last_dot && block_bad_images && blocking_enable )
  {
    if( jdk_stricmp( last_dot, ".jpg" )==0 || jdk_stricmp( last_dot, ".jpeg" )==0 )
    {
      redir_url.cpy( block_image_jpg_url );
      jdk_log( JDK_LOG_DEBUG2, "Redirecting jpg image to %s from %s", redir_url.c_str(), normalized_url.c_str() );
      r=true;
    }
    else if( jdk_stricmp( last_dot, ".gif" )==0 )
    {
      redir_url.cpy( block_image_gif_url );
      jdk_log( JDK_LOG_DEBUG2, "Redirecting gif image to %s from %s", redir_url.c_str(), normalized_url.c_str() );
      r=true;
    }
    else if( jdk_stricmp( last_dot, ".png" )==0 )
    {
      redir_url.cpy( block_image_png_url );
      jdk_log( JDK_LOG_DEBUG2, "Redirecting png image to %s from %s", redir_url.c_str(), normalized_url.c_str() );
      r=true;
    }
  }
  return r;
}

bool if2k_mini_http_proxy::is_request_blocked( 
  const jdk_http_request_header &request_header, 
  jdk_string &redir_url, 
  jdk_settings &redirection_info,
  if2k_mini_kernel_request &if2k_request
)
{
  jdk_url referer_url( request_header.get("referer:") );
  jdk_url normalized_url( request_header.get_url() );
  normalized_url.unescape();

  jdk_string_url full_url( normalized_url.unexplode() );
  jdk_string_url full_referer_url( referer_url.unexplode() );


  if2k_mini_kernel_request referer_request( if2k_request );
  referer_request.set_url(full_referer_url.c_str() );
  kernel.verify_url( referer_request );
  jdk_log( JDK_LOG_DEBUG2, "Kernel referer url verify: %d", referer_request.is_blocked );

  jdk_log( JDK_LOG_DEBUG2, "Kernel target url verify:" );
  if2k_mini_kernel_request target_request( if2k_request );
  target_request.set_url(full_url.c_str() );
  kernel.verify_url( target_request );
  jdk_log( JDK_LOG_DEBUG2, "Kernel referer url verify: %d", target_request.is_blocked );

  if( blocking_referer_enable && referer_request.is_good && !target_request.is_postbad )
  {
    if2k_request.set_url( full_url.c_str() );
    return false;
  }  

  if2k_request = target_request;
  
  if( if2k_request.is_blocked )
  {
    if( !redirect_if_blocked_image( full_url, redir_url ) )
    {
      if( block_bad )
      {
        redirection_info.merge( if2k_request.info );
        redir_url.cpy(blockpage_url);
      }
    }
  }
  else
  {
    redirection_info.clear();
    redir_url.clear();
  }

  return if2k_request.is_blocked;

}

bool if2k_mini_http_proxy::handle_connection( jdk_client_socket &sock )
{
  bool repeat_flag=true;
  bool use_keepalive=false;
  jdk_string_url client_addr;
  sock.get_remote_addr( client_addr );
  jdk_log( JDK_LOG_DEBUG1, "httpfilter connection from :%s", client_addr.c_str() );

  sock.setup_socket_blocking( false );
  
  jdk_http_request_header request_header;
  jdk_dynbuf request_additional_data;
  
  while( request_header.parse_socket(sock, request_additional_data ) )
  {
    jdk_settings redirection_info;
    jdk_string_url current_url( request_header.get_url().unexplode() );
    jdk_string_url redir_url;
    jdk_log( JDK_LOG_DEBUG1, "Request for url: %s", current_url.c_str() );

    bool use_generator=is_request_local( request_header );
    
    bool license_valid = true;


    if2k_mini_kernel_request kernel_request;
    bool blocked = is_request_blocked( request_header, redir_url, redirection_info, kernel_request );


    if( use_generator )
    {
      if2k_http_gen_manager gen( *this, sock, request_header, request_additional_data );

      jdk_log( JDK_LOG_DEBUG2, "Generating url: %s", current_url.c_str() );      
      gen.process();
      if( !use_keepalive )
        break;
    }
    else if( blocked )
    {
      jdk_log( JDK_LOG_DEBUG1, "Redirecting url: %s", current_url.c_str() );      
      if2k_http_redirect_manager redir( *this, sock, request_header, request_additional_data, redir_url, redirection_info );
      redir.process();
      if( !use_keepalive )
        break;
    }
    else    
    {
      if( request_header.get_request_type() == jdk_http_request_header::REQUEST_CONNECT )
      {
        if2k_https_xfer_manager xfer( 
          *this, 
          sock, 
          request_header, 
          request_additional_data, 
          kernel_request,
          license_valid
          );
      
        jdk_log( JDK_LOG_DEBUG2, "Transferring HTTPS url: %s with %d prebytes request data", current_url.c_str(), request_additional_data.get_data_length() );      

        xfer.process();

        if( !use_keepalive )
          break;
      }
      else
      {
        if2k_http_xfer_manager xfer( 
          *this, 
          sock, 
          request_header, 
          request_additional_data, 
          kernel_request, 
          license_valid 
          );
      
        jdk_log( JDK_LOG_DEBUG2, "Transferring url: %s with %d prebytes request data", current_url.c_str(), request_additional_data.get_data_length() );      
        xfer.process();
        if( !use_keepalive )
          break;
      }
    }
  }
  
  return repeat_flag;
}

jdk_string_filename if2k_http_gen_manager::get_local_filename(
                                                                               const jdk_url &url, 
                                                                               jdk_settings &get_params 
                                                                               )
{
  jdk_string_filename content_path( parent.httpfilter_server_webdir );

  jdk_string_filename content_suffix_path( url.path );

  char * question_mark_pos = content_suffix_path.chr('?');
  if( question_mark_pos )
  {
    jdk_cgi_loadsettings( &get_params, question_mark_pos+1 );
    *question_mark_pos='\0';
  }
  else
  {
    get_params.clear();
  }

  
  if( content_suffix_path.get(0)!='/' )
  {
    content_suffix_path.prepend('/');
  }
  
  if( content_suffix_path.cmp("/")==0 )
  {
    content_suffix_path.cat("index.html");
  }
  else
  {
    // do security checks on path
    if( content_suffix_path.str("..")!=0 )
    {
      jdk_log( JDK_LOG_INFO, "blocking URL with '..': %s", content_suffix_path.c_str() );
      content_path.cat( "/index.html" );
      return content_path;
    }
    size_t len = content_suffix_path.len();
    for( size_t i=0; i<len; ++i )
    {
      char c = content_suffix_path.get(i);
      if( !jdk_isalnum(c) && c!='&' && c!='?' && c!='=' && c!='/' && c!='.' && c!=';' )
      {
        jdk_log( JDK_LOG_INFO, "blocking URL with bad character: 0x%02x", c );
        content_path.cat( "/index.html" );
        return content_path;        
      }
    }
  }
  content_path.cat( content_suffix_path );
  return content_path;
}

const char * if2k_http_gen_manager::get_mime_type_for_file( const char *extension, bool &do_template )
{
  const char *ext = strrchr((char *)extension,'.');
  const char *mime="application/octet-stream";
  
  do_template=false;
  if( ext )
  {
    ++ext;
    if( strcmp( ext, "jpg")==0 )
      mime="image/jpeg";
    else if( strcmp( ext, "gif")==0 )
      mime="image/gif";
    else if( strcmp( ext, "html")==0 )
    {
      mime="text/html; charset=UTF-8";
      do_template=true;
    }
    else if( strcmp( ext, "txt")==0 )
      mime="text/plain; charset=UTF-8";
    else if( strcmp( ext, "css")==0 )
      mime="text/css; charset=UTF-8";
  } 
  return mime;
}

bool if2k_http_gen_manager::process()
{
  bool r=false;
  bool connection_close=true;

  response_header.set_http_version(jdk_str<16>("HTTP/1.1"));

  content_path = get_local_filename(request_header.get_url(),cgi_params);

  if( request_header.find_entry( "content-length:", 0 )>0 )
  {
    jdk_str<64> content_length_str( request_header.get( "content-length:" ));

    size_t content_length = content_length_str.strtoul();

    size_t todo = content_length - request_additional_data.get_data_length();

    if( todo>0 )
    {
      jdk_dynbuf b(todo);
      if( !client_sock.read_data_block( b, todo ) )
      {
        return false;
      }
      request_additional_data.append_from_buf( b );
    }
  }

  if( request_header.get_request_type() == jdk_http_request_header::REQUEST_GET )
  {
    r = handle_get();
  }
  else if( request_header.get_request_type() == jdk_http_request_header::REQUEST_POST )
  {
    r = handle_post();
  }

  if( r )
  {
    response_header.set_http_response_code(200);
  }
  else
  {
    content_path = parent.httpfilter_server_webdir;
    content_path.cat('/');
    content_path.cat( parent.httpfilter_server_error_404 );
    response_header.set_http_response_code(404);
    response_header.set_entry("Content-type:","text/html");
    response_content.append_from_file( content_path );
  }


  response_header.set_valid(true);
  jdk_str<256> content_length;
  content_length.form("%d",response_content.get_data_length());
  response_header.set_entry( "Content-length:", content_length );
  if( connection_close )
    response_header.set_entry( "Connection:", "close" );

  if( response_header.send_to_socket( client_sock ) )
  {
    size_t len = client_sock.write_data_block( response_content );
    if( len == response_content.get_data_length() )
    {
      jdk_log( JDK_LOG_DEBUG3, "Wrote content, length: %d", response_content.get_data_length() );
      r=!connection_close;
    }
    else
    {
      jdk_log( JDK_LOG_DEBUG2, "Error writing content, tried length: %d, wrote %d", response_content.get_data_length(), len );
    }
  }
  return r;
}

bool if2k_http_gen_manager::handle_get()
{
  if( request_header.get_url().path.ncmp( "/cgi/", 5 )==0 )
  {
    return handle_get_cgi();
  }
  else
  {
    return handle_get_local_file();
  }
}


bool if2k_http_gen_manager::handle_post()
{
  jdk_string_filename path( request_header.get_url().path );

  if( parent.override_enable && path.ncmp("/cgi/override.cgi?", 18)==0 )
  {
    jdk_log( JDK_LOG_DEBUG1, "Remote override_now post request" );

    return handle_post_override();
  }  

  if( cgi_params.find("p")<0 )
  {
    jdk_log( JDK_LOG_INFO, "Post request refused: No password" );
    return false;
  }
  jdk_str<256> praw( cgi_params.get("p") );
  jdk_str<512> p;
  if2k_hash_for_password( p, praw );

  //jdk_log( JDK_LOG_INFO, "praw=%s, p=%s, actual=%s", praw.c_str(), p.c_str(), parent.remote_update_password.c_str() );

  if( parent.remote_update_enable )
  {
    if( p.cmp( parent.remote_update_password )==0 )
    {
      if( path.ncmp( "/cgi/settings.cgi?", 18 )==0 )
      {
        jdk_log( JDK_LOG_DEBUG1, "Remote settings post request" );

        return handle_post_settings();
      }  
  
      if( path.ncmp( "/cgi/db.cgi?", 12 )==0 )
      {
        jdk_log( JDK_LOG_DEBUG1, "Remote db file post request" );

        return handle_post_dbfile();
      }

      if( path.ncmp( "/cgi/predb.cgi?", 15 )==0 )
      {
        jdk_log( JDK_LOG_DEBUG1, "Remote predb file post request" );

        return handle_post_predbfile();
      }
    }
    else
    {
      jdk_log( JDK_LOG_INFO, "Post request refused: Bad password" );
    }
  }
  else
  {
    jdk_log( JDK_LOG_INFO, "Post request refused. Remote update disabled." );
  }

  return false;
}

bool if2k_http_gen_manager::handle_get_cgi()
{
  jdk_string_filename path( request_header.get_url().path );

  if( parent.override_enable && path.ncmp("/cgi/override.cgi?", 18)==0 )
  {
    jdk_log( JDK_LOG_DEBUG1, "Remote override_now read request" );

    return handle_get_override();
  }

  if( cgi_params.find("p")<0 )
  {
    jdk_log( JDK_LOG_INFO, "Get request refused: No password" );
    return false;
  }

  jdk_str<256> praw( cgi_params.get("p") );
  jdk_str<512> p;
  if2k_hash_for_password( p, praw );

  //jdk_log( JDK_LOG_INFO, "praw=%s, p=%s, actual=%s", praw.c_str(), p.c_str(), parent.remote_update_password.c_str() );

  if( parent.remote_update_enable )
  {
    if( p.cmp( parent.remote_update_password )==0 )
    {
      if( path.ncmp( "/cgi/settings.cgi?", 18 )==0 )
      {
        jdk_log( JDK_LOG_DEBUG1, "Remote settings request" );
        return handle_get_settings();
      }
      if( path.ncmp( "/cgi/db.cgi?", 12 )==0 )
      {
        jdk_log( JDK_LOG_DEBUG1, "Remote db request" );
        return handle_get_dbfile();
      }
      if( path.ncmp( "/cgi/predb.cgi?", 15 )==0 )
      { 
        jdk_log( JDK_LOG_DEBUG1, "Remote predb request" );
        return handle_get_predbfile();
      }
      if( path.ncmp( "/cgi/restart.cgi?", 17 )==0 )
      {
        return handle_get_restart();
      } 
      if( path.ncmp( "/cgi/precompile.cgi?", 20 )==0 )
      {
        return handle_get_precompile();
      }
    }
    else
    {
      jdk_log( JDK_LOG_INFO, "Get request refused: Bad password" );
    }
  }
  else
  {
    jdk_log( JDK_LOG_INFO, "Get request refused. Remote update disabled." );
  }

  return false;
}


bool if2k_http_gen_manager::handle_get_local_file()
{
  bool r=false;
  jdk_dynbuf raw_content;
  if( raw_content.append_from_file( content_path ) )
  {
    jdk_log( JDK_LOG_DEBUG2, "Serving local file: %s", content_path.c_str() );
    response_header.set_http_response_code(200);

    bool do_template = false;

    response_header.set_entry("Content-type:",get_mime_type_for_file( content_path.c_str(), do_template ) );

    if( do_template )
    {
      jdk_html_template( response_content, raw_content, &parent.settings, &cgi_params );
    }
    else
    {
      response_content.append_from_buf( raw_content );
    }
    r=true;
  }
  return r;
}

bool if2k_http_gen_manager::handle_get_settings()
{
  bool r=false;

  if( cgi_params.find("f")<0 )
  {
    jdk_log( JDK_LOG_INFO, "Get settings refused, no file specified" );
    return false;
  }
  jdk_string_filename f(cgi_params.get("f"));

  if( f.cmp( "additional" )==0 )
  {
    r=parent.kernel.get_additional_settings().save_buf( &response_content );
  }
  else if( f.cmp( "install" )==0 )
  {
    r=parent.kernel.get_install_settings().save_buf( &response_content );
  }
  else if( f.cmp( "license" )==0 )
  {
    r=parent.kernel.get_license_settings().save_buf( &response_content );
  }
  else
  {
    jdk_log( JDK_LOG_INFO, "Get settings refused, unknown file specified" );
  }
  return r;
}

bool if2k_http_gen_manager::handle_post_settings()
{
  bool r=false;

  if( cgi_params.find("f")<0 )
  {
    jdk_log( JDK_LOG_INFO, "Post settings refused, no file specified" );
    return false;
  }

  jdk_string_filename f(cgi_params.get("f"));

  jdk_settings s;

  if( f.cmp( "additional" )==0 )
  {
    s.merge( parent.kernel.get_additional_settings() );
    if( s.load_buf( request_additional_data ) )
    {
#if JDK_IS_DEBUG
      fprintf( stderr, "------raw-buf------------\n" );
      request_additional_data.extract_to_file( stderr );
      fprintf( stderr, "------settings-----------\n" );
      s.save_file( stderr );
      fprintf( stderr, "-------------------------\n" );
#endif
      r = parent.kernel.update_additional_settings( s );
    }
    jdk_log( JDK_LOG_DEBUG1, "updated additional settings, data size=%d, success=%d", 
     request_additional_data.get_data_length(),
      (int)r
      );
  }
  else if( f.cmp( "install" )==0 )
  {
    s.merge( parent.kernel.get_install_settings() );
    if( s.load_buf( request_additional_data ) )
    {
      r = parent.kernel.update_install_settings( s );    
    }
    jdk_log( JDK_LOG_DEBUG1, "updated install settings, data size=%d, success=%d", 
      request_additional_data.get_data_length(),
      (int)r
      );

  }
  else if( f.cmp( "license" )==0 )
  {
    s.merge( parent.kernel.get_license_settings() );
    if( s.load_buf( request_additional_data ) )
    {
      r = parent.kernel.update_license_settings( s );    
    }
    jdk_log( JDK_LOG_DEBUG1, "updated license settings, data size=%d, success=%d", 
      request_additional_data.get_data_length(),
      (int)r
      );
  }
  else
  {
    jdk_log( JDK_LOG_INFO, "Post settings refused, unknown file specified" );
  }
  
  return r;
}

bool if2k_http_gen_manager::handle_get_override()
{
  jdk_settings s;
  s.set_long( "override", parent.kernel.is_in_override_mode() );
  s.save_buf( &response_content );
  jdk_log( JDK_LOG_DEBUG1, "override_now value = %d", parent.kernel.is_in_override_mode() );
  return true;
}

bool if2k_http_gen_manager::handle_post_override()
{
  jdk_settings s;
  s.load_buf( request_additional_data );
  parent.kernel.set_override_mode( s.get_bool("override") );

  jdk_log( JDK_LOG_DEBUG1, "updated override mode (%d)",
      (int)s.get_bool("override")
      );

  return true;
}


bool if2k_http_gen_manager::handle_get_dbfile()
{
  if( cgi_params.find("f")<0 )
  {
    jdk_log( JDK_LOG_INFO, "dbfile request missing filename" );
    return false;
  }

  jdk_string_filename f(cgi_params.get("f"));
  parent.kernel.get_db_file( f, response_content );
  return true;
}

bool if2k_http_gen_manager::handle_post_dbfile()
{
  bool r;

  if( cgi_params.find("f")<0 )
  {
    jdk_log( JDK_LOG_INFO, "post dbfile request missing filename" );
    return false;
  }
  jdk_string_filename f(cgi_params.get("f"));
  r=parent.kernel.set_db_file( f, request_additional_data );

  jdk_log( JDK_LOG_DEBUG1, "db file '%s' updated. success=%d", f.c_str(), r );
  return r;
}

bool if2k_http_gen_manager::handle_get_predbfile()
{
  if( cgi_params.find("f")<0 )
  {
    jdk_log( JDK_LOG_INFO, "predbfile request missing filename" );
    return false;
  }
  jdk_string_filename f(cgi_params.get("f"));
  parent.kernel.get_predb_file( f, response_content );
  return true;
}

bool if2k_http_gen_manager::handle_post_predbfile()
{
  bool r;

  if( cgi_params.find("f")<0 )
  {
    jdk_log( JDK_LOG_INFO, "post predbfile request missing filename" );
    return false;
  }
  jdk_string_filename f(cgi_params.get("f"));
  r=parent.kernel.set_predb_file( f, request_additional_data );
  jdk_log( JDK_LOG_DEBUG1, "predb file '%s' updated. success=%d", f.c_str(), r );
  return r;
}

bool if2k_http_gen_manager::handle_get_restart()
{
  bool r = parent.kernel.trigger_restart();
  jdk_log( JDK_LOG_DEBUG1, "Restart triggered (%d)", (int)r );
  return r;
}

bool if2k_http_gen_manager::handle_get_precompile()
{
  bool r = parent.kernel.trigger_precompile();
  jdk_log( JDK_LOG_DEBUG1, "Precompile triggered (%d)", (int)r );
  return r;
}


bool if2k_http_xfer_manager::process()
{
  bool r=false;

  jdk_string_host server_host_and_port( request_header.get_url().get_host_and_port() );

  jdk_inet_client_socket outgoing_socket;

  jdk_http_request_header transformed_request( request_header );
  
  transformed_request.delete_entry( "connection:" );
  transformed_request.delete_entry( "proxy-connection:" );
  transformed_request.delete_entry( "accept-encoding:" );
  transformed_request.delete_entry( "te:" );
  transformed_request.delete_entry( "keep-alive:" );
  transformed_request.set_entry( "Connection:", "close" );
  if( !parent.runtime_id.is_clear() )
    transformed_request.set_entry( "X-if2k-id:", parent.runtime_id );
  transformed_request.set_http_version(jdk_str<16>("HTTP/1.0"));
  
#if 0
  {
    jdk_log( JDK_LOG_DEBUG4, "Transformed request:" );
    jdk_dynbuf b;
    transformed_request.flatten(b,false);
    b.extract_to_stream( stderr );
  }
#endif

  size_t content_length_to_send=0;
  size_t total_request_content_length=0;
  
  if( transformed_request.find( "content-length:" ) )
  {
    total_request_content_length=transformed_request.get("content-length:").strtol();
    jdk_log( JDK_LOG_DEBUG3, "Total to send: %d", content_length_to_send );
    jdk_log( JDK_LOG_DEBUG3, "Already sent: %d", request_additional_data.get_data_length() );

    content_length_to_send=total_request_content_length-request_additional_data.get_data_length();
    if( content_length_to_send<0 )
    {
      content_length_to_send = 0;
    }
  }

#if 0
  {
    jdk_dynbuf x;
    transformed_request.flatten(x,false);
    x.extract_to_stream( stderr );
  }
#endif

  bool use_proxy=false;
  jdk_string_url host_and_port_to_connect_to;
  if( parent.http_proxy.is_clear() )
  {
    host_and_port_to_connect_to = server_host_and_port;
  }
  else
  {
    host_and_port_to_connect_to = parent.http_proxy;
    use_proxy=true;
  }

  if( use_proxy )
  {
    if( parent.http_proxy_auth_basic_enable )
    {
      transformed_request.set_entry("Proxy-Authorization:", parent.proxy_auth );
    }
  }

  if( outgoing_socket.make_connection( host_and_port_to_connect_to.c_str(), 0, false ) )
  {
    jdk_log( JDK_LOG_DEBUG1, "Made connection to : %s", server_host_and_port.c_str() );       
  
    if( transformed_request.send_to_socket( outgoing_socket, use_proxy ) )
    {
      if( total_request_content_length>0 )
      {
        client_sock.setup_socket_blocking( false );
        outgoing_socket.setup_socket_blocking( false );
        
        jdk_log( JDK_LOG_DEBUG2, "Starting data transaction %d,%d for %s", content_length_to_send, 0, request_url.c_str() );
        if2k_mini_tunnel tunnel( client_sock, outgoing_socket );
        tunnel.handle_transaction( 0, 0, &request_additional_data, 0 );
        jdk_log( JDK_LOG_DEBUG2, "Ended data transaction %d,%d %s", total_request_content_length, 0, request_url.c_str() );       
      }
      else
      {
        jdk_http_response_header response;
        jdk_dynbuf response_additional_data;
        
        if( response.parse_socket( outgoing_socket, response_additional_data ) )
        {        
#if 0
          {
            jdk_dynbuf x;
            response.flatten(x);
            x.extract_to_stream( stderr );
          }
#endif
          
          response.delete_entry( "proxy-authenticate:" );
          long content_length_to_receive=0;
          
          jdk_dynbuf response_header_flattened;
          response.flatten( response_header_flattened );
          response_header_flattened.append_from_buf( response_additional_data );
          
          {
            client_sock.setup_socket_blocking( false );
            outgoing_socket.setup_socket_blocking( false );
            
            jdk_log( JDK_LOG_DEBUG2, "Starting transaction %d,%d for %s", content_length_to_send, content_length_to_receive, request_url.c_str() );
            
            bool enable_scanner = false;

            if( !kernel_request.is_good && response.find( "content-type:" )>=0 )
            {
              jdk_str<4096> content_type(response.get("content-type:"));
              
              if( content_type.str("text") )
              {
                enable_scanner = true;
              }

              if( content_type.str("script"))
              {
                enable_scanner = false;
              }
            }

             if( enable_scanner )
            {
              jdk_log( JDK_LOG_DEBUG2, "Starting scanning tunnel" );
              if2k_mini_kernel_request request;
              request.set_url( request_url );
              
              if2k_mini_scanning_tunnel tunnel(
                  client_sock, 
                  outgoing_socket, 
                  parent,request,request_header,
                  parent.scanner_queue_size,
                  license_valid
                  );
              if( tunnel.handle_transaction( 
                (jdk_int64)content_length_to_send, 
                (jdk_int64)content_length_to_receive,
                &request_additional_data,
                &response_header_flattened
                    ) )
              {
#if 0
                tunnel.get_blocked_status();
                if( blocked_status.is_blocked )
                {
                  if( tunnel.get_actual_data_count_received()==0 )
                  {
                    jdk_dynbuf request_additional_data;

                    http_redirect_manager redir( 
                      parent, 
                      client_sock, 
                      request_header, 
                      request_additional_data, 
                      parent.blockpage_url, 
                      blocked_status.info
                      );
                    
                  }
                }
#endif 
              }
            }
            else
            {
              if2k_mini_tunnel tunnel(client_sock, outgoing_socket);
              tunnel.handle_transaction( 
                content_length_to_send, 
                content_length_to_receive,
                &request_additional_data,
                &response_header_flattened
                );
            }
            
            jdk_log( JDK_LOG_DEBUG2, "Ended transaction %d,%d %s", content_length_to_send, content_length_to_receive, request_url.c_str() );
          }
        }
      }
    }
  }
  else
  {
    jdk_log( JDK_LOG_DEBUG1, "Unable to connect to : %s", server_host_and_port.c_str() );
  }
  
  return r;
}


bool if2k_https_xfer_manager::process()
{
  bool r=false;

  jdk_string_host server_host_and_port( request_header.get_url().get_host_and_port() );

  bool use_proxy=false;
  jdk_string_url host_and_port_to_connect_to;
  if( parent.http_proxy.is_clear() )
  {
    host_and_port_to_connect_to = server_host_and_port;
  }
  else
  {
    host_and_port_to_connect_to = parent.http_proxy;
    use_proxy=true;
  }


  if( use_proxy )
  {
    if( parent.http_proxy_auth_basic_enable )
    {
      request_header.set_entry("Proxy-Authorization:", parent.proxy_auth );
    }
  }

  jdk_inet_client_socket outgoing_socket;
  if( outgoing_socket.make_connection( host_and_port_to_connect_to.c_str(), 0, false ) )
  {
    jdk_log( JDK_LOG_DEBUG1, "Made https connection to : %s", server_host_and_port.c_str() );       
    if( use_proxy )
    {
      if( request_header.send_to_socket( outgoing_socket ) )
      {      
        jdk_socket_transfer( 0, &client_sock, &outgoing_socket );        
      }
      else
      {
        jdk_log( JDK_LOG_DEBUG1, "Error sending header to https proxy" );
      }
    }
    else
    {
      client_sock.write_string_block( "HTTP/1.0 200 Connection established\r\n\r\n" );
      jdk_socket_transfer( 0, &client_sock, &outgoing_socket );
    }
  }
  else
  {
    jdk_log( JDK_LOG_DEBUG1, "Unable to https connect to : %s", server_host_and_port.c_str() );
  }
  
  return r;
}

bool if2k_mini_http_proxy::make_redir_response( 
  jdk_dynbuf &response_buf, 
  const jdk_string_url &redirection_destination,
  const jdk_settings &redirection_cgi_params
  ) const
{
  bool fancy_cgi=false;
  int http_code = 302;

  jdk_string_url full_blockpage_url(redirection_destination);

  if( full_blockpage_url.get(0)!='h' )
  {
    fancy_cgi=true;
    full_blockpage_url.form( 
      "http://%s/%s", 
      httpfilter_interface.c_str(), 
      redirection_destination.c_str() 
      );
  }

  if( fancy_cgi && redirection_cgi_params.count()>0 )
  {
    jdk_str<4096> text_params;
    if( jdk_cgi_savesettings( redirection_cgi_params, text_params.c_str(), text_params.getmaxlen()-1 ) )
    {
      if( !text_params.is_clear() )
      {
        full_blockpage_url.cat('?');
        full_blockpage_url.cat( text_params );
        jdk_log( JDK_LOG_DEBUG2, "Redirection cgi params: '%s'", text_params.c_str() );
      }
    }
  }

  response_buf.append_form( "HTTP/1.0 %d Code\r\n", http_code );
  response_buf.append_form( "Connection: close\r\n" );
  response_buf.append_form( "Pragma: no-cache\r\n" );
  response_buf.append_form( "Expires: -1\r\n" );	
  response_buf.append_form( "Cache-Control: no-cache\r\n" );			
  response_buf.append_form( "Content-length: 0\r\n" );
  response_buf.append_form( "Location: %s\r\n\r\n", full_blockpage_url.c_str() );

  jdk_log( JDK_LOG_DEBUG2, "Made redirection response to %s", full_blockpage_url.c_str() );
  return true;
}

bool if2k_http_redirect_manager::process()
{
  jdk_dynbuf response;
  if( parent.make_redir_response( 
        response,
        redirection_destination,
        redirection_cgi_params
        ) )
  {
    client_sock.write_data_block( response );
  }
  return true;
}


bool if2k_mini_scanning_tunnel::handle_client_incoming_data( jdk_buf &data )
{
  parent.kernel.verify_transmitted_data( request, data );

  if( request.is_blocked && !blocking_applied)
  {
    if( actual_sent_to_client_byte_count == 0)
    {
      jdk_dynbuf redir_response;
      if( parent.make_redir_response( 
            redir_response,
            parent.blockpage_url,
            request.info
            ) )
      {
        if2k_mini_tunnel::handle_server_incoming_data( redir_response );
        outgoing_shutdown=true;
        blocking_applied=true;
        return false;
      }
    }
    return true;
  }

  return if2k_mini_tunnel::handle_client_incoming_data( data );
}

bool if2k_mini_scanning_tunnel::handle_server_incoming_data( jdk_buf &data )
{
  if( license_valid )
  {
    parent.kernel.verify_received_data( request, data );
  }
  if( request.is_blocked && !blocking_applied)
  {
    if( actual_sent_to_client_byte_count == 0)
    {
      jdk_dynbuf redir_response;
      if( parent.make_redir_response( 
            redir_response,
            parent.blockpage_url,
            request.info
            ))
      {
        if2k_mini_tunnel::handle_server_incoming_data( redir_response );
        outgoing_shutdown=true;
        blocking_applied=true;
        return false;
      }
       
    }
    return true;
  }
  
  return if2k_mini_tunnel::handle_server_incoming_data( data );
}
