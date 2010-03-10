
#if 0
# include "jdk_world.h"
#include "if2_lua_kernel.h"

int jdk_lua_type_thunker_base<if2_kernel>::otag = 0;

void if2_lua_kernel::register_type( lua_State *L )
{
  otag = lua_newtag( L );
  lua_pushcfunction( L, destructor );
  lua_settagmethod( L, otag, "gc" );
  lua_pushcfunction(L,constructor );
  lua_setglobal(L,"if2_kernel");
}


int if2_lua_kernel::destructor( lua_State *L )
{
  if2_lua_kernel *o = static_cast<if2_lua_kernel*>(lua_touserdata(L,-1));
  delete o;
  return 0;
}

void if2_lua_kernel::constructor( lua_State *L )
{
  jdk_settings settings;
  jdk_lua_get( L, settings );
  if2_lua_kernel *o = new if2_lua_kernel(settings);
  lua_newtable(L);
  lua_pushnumber(L,0); // userdata
  lua_pushusertag(L,(void *)o,otag );
  lua_settable(L,-3);
  
  construct_method(verify_url);
  construct_method(verify_post_data);
  construct_method(verify_received_data);
}

int if2_lua_kernel::verify_url(lua_State *L)
{
  if2_lua_kernel *o=get_this(L);
  const char *client_address;
  const char *url;
  url = lua_tostring(L,-1);
  lua_pop(L,1);
  client_address = lua_tostring(L,-1);
  lua_pop(L,1);
  int r=o->k.verify_url(client_address,url);
  jdk_lua_push(L,r);
  return 1;	
}

int if2_lua_kernel::verify_post_data(lua_State *L)
{
  if2_lua_kernel *o=get_this(L);
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

int if2_lua_kernel::verify_received_data(lua_State *L)
{
  if2_lua_kernel *o=get_this(L);
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
