
#include "jdk_world.h"
#include "if2k_lua_scanner.h"

#if 0

class if2_lua_scanner_event : public if2_scanner_event<if2_tree_traits_alphanumeric>
{
public:
  if2_lua_scanner_event(
    lua_State *L_, 
    const jdk_string &func_, 
    const jdk_string &context_code_ 
    )
    : L(L_), func(func_), context_code( context_code_ )
  {
  }
  
  void operator() (
    const jdk_tree<if2_tree_traits_alphanumeric> &tree, 
    if2_tree_traits_alphanumeric::index_t match_item 
    )
  {
    int p=lua_gettop(L);
    lua_getglobal(L,func.c_str());	   
    jdk_lua_push( L, context_code );	 	   	   	   
    jdk_lua_push( L, match_item );	   
    lua_call(L,2,0);
    lua_settop(L,p);
  }
  
private:
  lua_State *L;
  const jdk_string &func;
  const jdk_string &context_code;
};

int jdk_lua_type_thunker_base<if2_lua_scanner>::otag = 0;

void if2_lua_scanner::register_type( lua_State *L )
{
  otag = lua_newtag( L );
  lua_pushcfunction( L, destructor );
  lua_settagmethod( L, otag, "gc" );
  lua_pushcfunction(L,constructor );
  lua_setglobal(L,"if2_scanner");
}

int if2_lua_scanner::destructor( lua_State *L )
{
  if2_lua_scanner *o = static_cast<if2_lua_scanner*>(lua_touserdata(L,-1));
  delete o;
  return 0;
}

int if2_lua_scanner::constructor( lua_State *L )
{
  jdk_map< jdk_str<256>, jdk_str<4096> > settings;
  jdk_str<1024> precompiled_setting;
  jdk_str<1024> setting_prefix;
  int num_files;
  
  jdk_lua_get( L, num_files );
  jdk_lua_get( L, setting_prefix );
  jdk_lua_get( L, precompiled_setting );
  jdk_lua_get( L, settings );
  if2_lua_scanner *o = new if2_lua_scanner(settings,precompiled_setting,setting_prefix,num_files);
  lua_newtable(L);
  lua_pushnumber(L,0); // userdata
  lua_pushusertag(L,(void *)o,otag );
  lua_settable(L,-3);
  
  construct_method(find);
  construct_method(extract);
  construct_method(load);
  construct_method(save);	
  construct_method(add);
  return 0;	
}

int if2_lua_scanner::find(lua_State *L)
{
  if2_lua_scanner *o=get_this(L);
  const char *data;
  int data_len;
  jdk_str<1024> event_callback;	
  
  jdk_lua_get( L, event_callback );
  data = lua_tostring(L,-1);
  data_len = lua_strlen(L,-1);
  lua_pop(L,1);
  if2_lua_scanner_event event( L, event_callback );
  int r=o->k.find(data, data_len, event);
  jdk_lua_push(L,r);
  return 1;	
}

int if2_lua_scanner::verify_post_data(lua_State *L)
{
  if2_lua_scanner *o=get_this(L);
  const char *client_address;
  const char *url;
  const char *data;
  int data_len;
  
  data = lua_tostring(L,-1);
  data_len = lua_strlen(L,-1);
  lua_pop(L,1);
  url = lua_tostring(L,-1);
  lua_pop(L,1);	
  client_address = lua_tostring(L,-1);
  lua_pop(L,1);
  int r=o->k.verify_post_data(client_address,url,data,data_len);
  jdk_lua_push(L,r);
  return 1;	
  
}

int if2_lua_scanner::verify_received_data(lua_State *L)
{
  if2_lua_scanner *o=get_this(L);
  const char *client_address;
  const char *url;
  const char *data;
  int data_len;
  
  data = lua_tostring(L,-1);
  data_len = lua_strlen(L,-1);
  lua_pop(L,1);
  url = lua_tostring(L,-1);
  lua_pop(L,1);	
  client_address = lua_tostring(L,-1);
  lua_pop(L,1);
  int r=o->k.verify_received_data(client_address,url,data,data_len);
  jdk_lua_push(L,r);
  return 1;		
}

#endif
