#include "jdk_world.h"
#include "jdk_httprequest.h"
#include "jdk_http.h"
#include "jdk_dynbuf.h"

#include "if2k_client.h"

if2k_client::if2k_client() : proxy_ip(), server_ip(), pass()
{
  
}

if2k_client::~if2k_client()
{
}

void if2k_client::set_connection(
  const jdk_string &server_ip_,
  const jdk_string &proxy_ip_,
  const jdk_string &pass_
  )
{
  server_ip = server_ip_;
  proxy_ip = proxy_ip_;
  pass = pass_;
}

bool if2k_client::execute_script(
  const char *request_type,
  const jdk_str<128> &script,
  const jdk_settings_text &params,
  const jdk_buf &additional,
  jdk_buf &response
  )
{
  // form the beginning part of the url
  jdk_string_url full_url;
  full_url.form( "http://%s/cgi/%s?", server_ip.c_str(), script.c_str() );
  
  // append any cgi variables to it
  jdk_string_url cgi_params;
  jdk_cgi_savesettings( params, cgi_params.c_str(), cgi_params.getmaxlen() );
  full_url.cat( cgi_params );
  
  // form the if2002 password header string
  jdk_str<4096> password_header;
  password_header.form("x-if2002-pass: %s\r\n", pass.c_str() );
  
  // declare variables which hold the http request info
  jdk_str<512> connect_host;
  int connect_port;
  jdk_dynbuf request;
  
  // form the request
  bool r;
  r=jdk_http_form_request(
    connect_host,  // will be filled in with host name to connect to
    connect_port,  // will be filled in with port to connect to
    request,       // will be filled in with raw data to send
    full_url.c_str(), 	// raw unescaped url to request
    request_type,   	// "GET" or "HEAD" or "POST", etc
    proxy_ip.c_str(), 	// proxy url/ip
    additional.get_data_length(),  // for POST or PUT, or -1 for none
    "text/plain",      // for POST or PUT, blank or null for none
    password_header.c_str()  // single multi-line string to insert in headers
    );
  
  
  if( r )
  {
    // request was formed properly. Now do the request
    jdk_http_response_header response_header;
    
    int response_code =
      jdk_http_do_request(
        request_type, // "GET" or "POST" etc
        full_url.c_str(),
        &response,
        8*1024*1024,
        &response_header,
        proxy_ip.c_str(),
        password_header.c_str(),
        &additional,
        "text/plain",
        "1.0",
        false
        );
    
    if( response_code != 200 )
    {
      // something went wrong
      response.clear();
      r=false;
    }
  }
  
  return r;
}

bool if2k_client::load_settings( 
  if2k_client_settings &settings
  )
{
  bool r;
  
  jdk_staticbuf<64> additional;
  
  // get the current settings map
  {
    jdk_dynbuf response;
    jdk_settings_text params;
    
    r = execute_script(
      "GET",
      "settings",
      params,
      additional,
      response
      );
    if( !r )
    {
      settings.clear();
      return false;
    }
    
    // parse the settings map
    r = settings.settings.load_buf( response );
    
    if( !r )
    {
      settings.clear();
      return false;
    }
  }
  
  
  // get all the lists
  jdk_dynbuf response;
  jdk_settings_text params;
  
  for( int i=0; i<if2k_client_settings::num_lists; ++i )
  {
    for( int j=0; j<if2k_client_settings::num_categories; ++j )
    {
      if( settings.get_list_category_file(i,j).len()>0 )
      {
        jdk_str<256> val( settings.name_base[i][j] );
        val.cat( "file" );
        
        params.clear();
        params.set(
          jdk_setting::key_t("category"),
          jdk_setting::value_t(val)
          );
        
        settings.list[i][j].clear();	
        r = r & execute_script(
          "GET",
          "list",
          params,
          additional,
          settings.list[i][j]
          );
        if( !r )
        {
          //			settings.clear();
          return false;
        }
      }
    }
  }
  
  return r;
}

bool if2k_client::apply_setting(
  const jdk_settings_text &map
  )
{
  jdk_staticbuf<64> response;
  
  jdk_dynbuf additional;
  jdk_settings_text params;
  
  map.save_buf( &additional );
  
  return execute_script(
    "PUT",
    "settings",
    params,
    additional,
    response
    );
}


bool if2k_client::save_settings( 
  const if2k_client_settings &settings
  )
{
  bool r;
  jdk_staticbuf<64> response;
  
  // convert and store the current settings map
  {
    jdk_dynbuf additional;
    jdk_settings_text params;
    
    settings.settings.save_buf( &additional );
    
    r = execute_script(
      "PUT",
      "settings",
      params,
      additional,
      response
      );
    
    if( !r )
    {
      return false;
    }
  }
  
  
  // store all the lists
  jdk_settings_text params;
  
  for( int i=0; i<if2k_client_settings::num_lists; ++i )
  {
    for( int j=0; j<if2k_client_settings::num_categories; ++j )
    {
      if( settings.get_list_category_file(i,j).len()>0 )
      {
        jdk_str<256> val( settings.name_base[i][j] );
        val.cat( "file" );
        
        params.clear();
        params.set(
          jdk_setting::key_t("category"),
          val
          );
        
        r = r & execute_script(
          "PUT",
          "list",
          params,
          settings.list[i][j],
          response
          );
        if( !r )
        {
          return false;
        }
      }
    }
  }
  return r;
}

bool if2k_client::trigger_reload()
{
  jdk_staticbuf<64> response;
  jdk_staticbuf<64> additional;
  jdk_settings_text params;
  
  return execute_script(
    "GET",
    "reload",
    params,
    additional,
    response
    );
}

bool if2k_client::add_script( 
  const jdk_buf &script
  )
{
  jdk_staticbuf<64> response;
  jdk_settings_text params;
  
  return execute_script(
    "PUT",
    "parse",
    params,
    script,
    response
    );
}

