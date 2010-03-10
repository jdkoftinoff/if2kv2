#ifndef __IF2_LUA_KERNEL_H
#define __IF2_LUA_KERNEL_H

#include "jdk_lua.h"
#include "if2_kernel.h"


class if2_lua_kernel : public jdk_lua_type_thunker_base<if2_lua_kernel>
{
   static void constructor(lua_State *L );
   static void destructor(lua_State *L );
   static int verify_url(lua_State *L);
   static int verify_post_data(lua_State *L);   
   static int verify_received_data(lua_State *L);   
public:
   static void register_type( lua_State *L);
   if2_lua_kernel( const jdk_settings &settings )
	   : k(settings)
   {
	   
   }
   
   virtual ~if2_lua_kernel()
   {
   }
   
private:
   if2_kernel k;
};

#endif
