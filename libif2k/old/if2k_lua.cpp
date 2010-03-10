
#include "jdk_world.h"
#include "if2k_lua.h"

#if IF2K_ENABLE_LUA


// url_encode takes a url string in and returns it properly % encoded

struct lua_url_encode
{
  lua_url_encode( lua_State *L ) {}
  inline jdk_string_url operator()( jdk_string_url &s )
  {
    jdk_string_url ret;
    jdk_cgi_escape_with_amp( s, ret );
    return ret;
  }
};

struct lua_url_decode
{
  lua_url_decode( lua_State *L ) {}
  inline jdk_string_url operator()( jdk_string_url &s )
  {
    jdk_string_url ret;
    jdk_cgi_unescape( s, ret );
    return ret;
  }
};

struct lua_html_encode
{
  lua_html_encode( lua_State *L ) {}
  inline jdk_dynbuf operator()( jdk_dynbuf &in )
  {
    // TODO: this sucks
    //if( jdk_html_requires_escaping(in) )
    //{
    jdk_dynbuf out( in.get_data_length() * 3 /2 );
    jdk_html_amp_escape_text(
      (const char *)in.get_data(),
      (char *)out.get_data(),
      out.get_buf_length()
      );
    out.set_data_length( strlen( (char *)out.get_data() ) );
    return out;
    //}
    //else
    //{
    //	return in;
    //}
  }
};

struct lua_html_decode
{
  lua_html_decode( lua_State *L ) {}
  inline jdk_dynbuf operator()( jdk_dynbuf &in )
  {
    // TODO: this sucks
    
    //if( jdk_html_requires_unescaping(in) )
    //{
    jdk_dynbuf out( in.get_data_length() * 3 /2 );
    jdk_html_amp_unescape_text(
      (const char *)in.get_data(),
      (char *)out.get_data(),
      out.get_buf_length()
      );
    out.set_data_length( strlen( (char *)out.get_data() ) );
    return out;
    //}
    //else
    //{
    //	return in;
    //}
  }
};

struct lua_if_parse_builtin
{
  if2k_lua_interpreter *interpreter;
  
  lua_if_parse_builtin( lua_State *L )
    :
    interpreter( (if2k_lua_interpreter *)jdk_get_interpreter(L) )
  {
  }
  
  inline int operator()(const jdk_string &fname)
  {
    return interpreter->parse_builtin( fname.c_str() );
  }
};


struct lua_if_reload
{
  if2k_lua_interpreter *interpreter;
  
  lua_if_reload( lua_State *L )
    :
    interpreter( (if2k_lua_interpreter *)jdk_get_interpreter(L) )
  {
  }
  
  inline void operator()()
  {
    interpreter->trigger_reload_settings();
  }
};


struct lua_if_get_settings
{
  if2k_lua_interpreter *interpreter;
  
  lua_if_get_settings( lua_State *L )
    :
    interpreter( (if2k_lua_interpreter *)jdk_get_interpreter(L) )
  {
  }
  
  inline jdk_dynbuf operator()()
  {
    jdk_dynbuf buf;
    global_settings->save_buf( &buf );
    return buf;
  }
};


struct lua_if_update_settings
{
  if2k_lua_interpreter *interpreter;
  
  lua_if_update_settings( lua_State *L )
    :
    interpreter( (if2k_lua_interpreter *)jdk_get_interpreter(L) )
  {
  }
  
  inline int operator()(const jdk_dynbuf &settings_text )
  {
    jdk_settings_text parsed_new_settings(settings_text);
    
    global_settings->merge( parsed_new_settings );
    
    interpreter->trigger_reload_settings();
    return 0;
  }
};

struct lua_if_parse_url
{
  if2k_lua_interpreter *interpreter;
  
  lua_if_parse_url( lua_State *L )
    :
    interpreter( (if2k_lua_interpreter *)jdk_get_interpreter(L) )
  {
  }
  
  inline int operator()(const jdk_string_url &url )
  {
    return interpreter->if_parse_url(url.c_str());
  }
};

struct lua_if_parse_cached_url
{
  if2k_lua_interpreter *interpreter;
  
  lua_if_parse_cached_url( lua_State *L )
    :
    interpreter( (if2k_lua_interpreter *)jdk_get_interpreter(L) )
  {
  }
  
  inline int operator()(
    const jdk_string_url &url, 
    const jdk_string &localfile, 
    int ignore_remote_if_local_exists 
    )
  {
    return interpreter->if_parse_cached_url(
      url.c_str(),
      localfile.c_str(),
      ignore_remote_if_local_exists
      );
  }
};



// TODO: functions to create, use, and destroy if2_scanner

if2k_lua_interpreter::if2k_lua_interpreter(
  const jdk_settings &settings_, 
  int stack_size_
  )
  : 
  jdk_lua_interpreter(stack_size_),	
  settings(settings_),
  reload_settings_trigger(false)
{
  jdk_debug_block( "if2k_lua_interpreter" );
  jdk_synchronized( mutex );
  jdk_lua_regfunc(
    get_l(), 
    "url_encode", 
    jdk_lua_thunk1<lua_url_encode,jdk_string_url,jdk_string_url > 
    );
  
  jdk_lua_regfunc(
    get_l(),
    "url_decode",
    jdk_lua_thunk1<lua_url_decode,jdk_string_url,jdk_string_url >
    );
  
  jdk_lua_regfunc(
    get_l(),
    "html_encode",
    jdk_lua_thunk1<lua_html_encode,jdk_dynbuf,jdk_dynbuf >
    );
  
  jdk_lua_regfunc(
    get_l(),
    "html_decode",
    jdk_lua_thunk1<lua_html_decode,jdk_dynbuf,jdk_dynbuf >
    );
  
  jdk_lua_regfunc(
    get_l(),
    "if_reload",
    jdk_lua_thunk0_noreturn<lua_if_reload>
    );
  
  jdk_lua_regfunc(
    get_l(),
    "if_get_settings",
    jdk_lua_thunk0<lua_if_get_settings,jdk_dynbuf>
    );	
  
  jdk_lua_regfunc(
    get_l(),
    "if_update_settings",
    jdk_lua_thunk1<lua_if_update_settings,int,jdk_dynbuf>
    );	
  
  jdk_lua_regfunc(
    get_l(),
    "if_parse_builtin",
    jdk_lua_thunk1<lua_if_parse_builtin,int,jdk_str<256> >
    );
  
  jdk_lua_regfunc(
    get_l(),
    "if_parse_url",
    jdk_lua_thunk1<lua_if_parse_url,int,jdk_string_url >
    );
  
  jdk_lua_regfunc(
    get_l(),
    "if_parse_cached_url",
    jdk_lua_thunk3<lua_if_parse_cached_url,int,jdk_string_url,jdk_string_url,int  >
    );
  
}

if2k_lua_interpreter::~if2k_lua_interpreter()
{
}


bool if2k_lua_interpreter::parse_builtin( const char *fname )
{
  jdk_bindir *bin = jdk_bindir_find( "if2k_lua_files", fname );
  if( bin )
  {
    jdk_log_debug2( "found builtin %s", fname );
    jdk_synchronized(mutex);	
    int oldtop = lua_gettop(get_l());
    parse_buf( bin->data, bin->length );
    lua_settop(get_l(),oldtop);
    return true;
  }
  else
  {
    jdk_log( JDK_LOG_ERROR, "can't find builtin script %s", fname);
    return false;
  }	
}

bool if2k_lua_interpreter::if_parse_url( const char *url )
{
  jdk_dynbuf response;
  jdk_http_response_header response_header;
  
  if( jdk_http_get(
        url,
        &response,
        1024*512,
        &response_header,
        settings.get("further_proxy").c_str(),
        false
        )==200 )
  {
    jdk_synchronized(mutex);	
    
    int oldtop = lua_gettop(get_l());
    bool r = parse_buf( response )==0;
    lua_settop(get_l(),oldtop);
    return r;
  }
  
  return false;
}

bool if2k_lua_interpreter::if_parse_cached_url(
  const char *url, 
  const char *cachefile,
  bool ignore_remote_if_local_exists
  )
{
  jdk_remote_buf remote_lua_code(
    cachefile,
    url,
    settings.get("further_proxy"), 
    1024*1024, 
    ignore_remote_if_local_exists
    );
  
  if( remote_lua_code.check_and_grab() )
  {
    jdk_synchronized(mutex);	
    int oldtop = lua_gettop(get_l());
    bool r = parse_buf( remote_lua_code.get_buf() )==0;
    lua_settop(get_l(),oldtop);
    return r;
  }
  
  return false;
}


void if2k_lua_interpreter::run()
{
  jdk_debug_block( "if2k_lua_interpreter::run" );
  update();
}

void if2k_lua_interpreter::update()
{
  jdk_debug_block( "if2k_lua_interpreter::update()" );
  
  {
    jdk_synchronized( mutex );
    jdk_lua_setglobal( get_l(), "settings", settings );
  }
  jdk_log_debug2( "if2k_lua_interpreter settings set" );
  {
    jdk_synchronized( mutex);
    parse_builtin( "if2k_lua_init.luac" );
    
    int oldtop = lua_gettop( get_l());
    jdk_lua_call( get_l(), "if_run" );
    lua_settop( get_l(),oldtop);
  }
  jdk_log_debug2( "if2k_lua_interpreter if_run() complete" );		
  
  jdk_str<128> lua_builtin_init = settings.get("lua_builtin_init");
  if( !lua_builtin_init.is_clear() )
  {
    jdk_log_debug1( "if2k_lua_interpreter parsing builtin: %s", lua_builtin_init.c_str() );
    jdk_synchronized( mutex);
    parse_builtin( lua_builtin_init.c_str() );
  }
  
  jdk_remote_buf remote_lua_code( settings, "lua_init", 1024*1024, false );
  
  if( remote_lua_code.check_and_grab() )
  {
    jdk_synchronized( mutex);	
    int oldtop = lua_gettop( get_l());
    parse_buf( remote_lua_code.get_buf() );
    lua_settop( get_l(),oldtop);
    jdk_log_debug1( "if2k_lua_interpreter remote buf parsed" );		
  }
  
  jdk_str<4096> lua_command = settings.get("lua_command");
  if( !lua_command.is_clear() )
  {
    jdk_synchronized( mutex);		
    
    jdk_log_debug2( "if2k_lua_interpreter parsing lua_command: %s", lua_builtin_init.c_str() );
    
    int oldtop = lua_gettop( get_l());
    parse_string( lua_command.c_str() );
    lua_settop( get_l(),oldtop);
  }	
  
  
}

bool if2k_lua_interpreter::if_webserve(
  jdk_dynbuf &ret, 
  const jdk_string &client_ip, 
  const jdk_http_request_header &request_header 
  )
{
  jdk_debug_block( "if2k_lua_interpreter::if_webserve()" );
  ret.clear();
  jdk_synchronized(mutex);
  jdk_lua_call(
    get_l(), 
    "if_webserve", 
    ret, 
    client_ip, 
    request_header 
    );
  return ret.get_data_length()>0;
}

bool if2k_lua_interpreter::if_cgi_get(
  const jdk_string &cmd,
  if2k_lua_web_response  &ret,
  const jdk_string &client_ip, 
  const jdk_http_request_header &request_header, 
  const jdk_settings_text &cgiparams 
  )
{
  jdk_debug_block( "if2k_lua_interpreter::if_cgi_get()" );
  ret.clear();
  jdk_str<1024> func_name = "if_cgi_get_";
  func_name.cat( cmd );
  jdk_synchronized(mutex);
  jdk_lua_call(
    get_l(), 
    func_name.c_str(), 
    ret, 
    client_ip, 
    request_header, 
    cgiparams 
    );
  return ret.header.is_valid();
}

bool if2k_lua_interpreter::if_cgi_post(
  const jdk_string &cmd,
  if2k_lua_web_response  &ret,
  const jdk_string &client_ip,
  const jdk_http_request_header &request_header,
  const jdk_dynbuf &postparams
  )
{
  jdk_debug_block( "if2k_lua_interpreter::if_cgi_post()" );
  ret.clear();
  jdk_str<1024> func_name = "if_cgi_post_";
  func_name.cat( cmd );
  jdk_synchronized(mutex);
  jdk_lua_call(
    get_l(),
    func_name.c_str(),
    ret,
    client_ip,
    request_header,
    postparams
    );
  return ret.header.is_valid();
}

bool if2k_lua_interpreter::if_cgi_put(
  const jdk_string &cmd,
  if2k_lua_web_response  &ret,
  const jdk_string &client_ip,
  const jdk_http_request_header &request_header,
  const jdk_dynbuf &putcontents,
  const jdk_settings_text &cgiparams
  )
{
  jdk_debug_block( "if2k_lua_interpreter::if_cgi_put()" );
  ret.clear();
  jdk_str<1024> func_name = "if_cgi_put_";
  func_name.cat( cmd );
  jdk_log( JDK_LOG_DEBUG4, "%s", func_name.c_str() );
  jdk_synchronized(mutex);
  jdk_lua_call(
    get_l(),
    func_name.c_str(),
    ret,
    client_ip,
    request_header,
    putcontents,
    cgiparams
    );
  return ret.header.is_valid();
}


bool if2k_lua_interpreter::if_url_test(
  jdk_buf &ret, 
  const jdk_string &client_ip,
  const jdk_http_request_header &request_header 
  )
{
  jdk_debug_block( "if2k_lua_interpreter::if_url_test()" );
  ret.clear();
  jdk_synchronized(mutex);
  jdk_lua_call(
    get_l(),
    "if_url_test",
    ret,
    client_ip,
    request_header 
    );
  return ret.get_data_length()>0;
}

bool if2k_lua_interpreter::if_url_accessed(
  jdk_buf &ret, 
  const jdk_string &client_ip,
  const jdk_http_request_header &request_header 
  )
{
  jdk_debug_block( "if2k_lua_interpreter::if_url_accessed()" );
  ret.clear();
  jdk_synchronized(mutex);
  jdk_lua_call(
    get_l(), 
    "if_url_accessed", 
    ret, 
    client_ip,
    request_header 
    );
  return ret.get_data_length()>0;
}

bool if2k_lua_interpreter::if_url_blocked(
  jdk_string_url &redirect_url, 
  const jdk_string &client_ip,
  const jdk_http_request_header &request_header,
  int reason,
  const jdk_string &match
  )
{
  jdk_debug_block( "if2k_lua_interpreter::if_url_blocked()" );
  redirect_url.clear();
  jdk_synchronized(mutex);
  jdk_lua_call(
    get_l(), 
    "if_url_blocked", 
    redirect_url, 
    client_ip,
    request_header,
    reason,
    match
    );
  return redirect_url.len()>0;
}


bool if2k_lua_interpreter::if_url_override(
  jdk_buf &ret, 
  const jdk_string &client_ip,
  const jdk_http_request_header &request_header 
  )
{
  jdk_debug_block( "if2k_lua_interpreter::if_url_override()" );
  ret.clear();
  jdk_synchronized(mutex);
  jdk_lua_call(
    get_l(),
    "if_url_override", 
    ret, 
    client_ip,
    request_header 
    );
  return ret.get_data_length()>0;
}

bool if2k_lua_interpreter::if_url_error(
  const jdk_string &error_name,
  if2k_lua_web_response &ret,
  const jdk_string &client_ip,
  const jdk_http_request_header &request_header 
  )
{
  jdk_str<1024> lua_func("if_url_error_");
  lua_func.cat( error_name );
  
  jdk_debug_block( "if2k_lua_interpreter::if_url_error()" );
  ret.clear();
  jdk_synchronized(mutex);
  jdk_lua_call(
    get_l(),
    lua_func.c_str(), 
    ret, 
    client_ip,
    request_header 
    );
  return ret.contents.get_data_length()>0;
}

#endif





