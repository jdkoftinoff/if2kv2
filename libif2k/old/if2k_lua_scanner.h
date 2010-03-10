#ifndef __IF2_LUA_SCANNER_H
#define __IF2_LUA_SCANNER_H

#if 0

#include "jdk_lua.h"
#include "jdk_tree.h"
#include "if2_pattern_expander.h"
#include "if2_scanner.h"

class if2_lua_scanner : public jdk_lua_type_thunker_base<if2_lua_scanner>
{
   static int constructor(lua_State *L );
   static int destructor(lua_State *L );
   static int find(lua_State *L );
   static int extract(lua_State *L );
   static int load( lua_State *L );
   static int save( lua_State *L );
   static int add( lua_State *L );
public:
   static void register_type( lua_State *L);
   explicit if2_lua_scanner( 
					const jdk_settings &settings, 
					const jdk_string &precompiled_setting,
					const jdk_string &setting_prefix,
					int num_files,
					int code_offset=0,
					const jdk_str<64> &pattern_prefix=""
					)
	   : k(
		settings,
		precompiled_setting,
		setting_prefix,
		expander,
		num_files,
		code_offset, 
		pattern_prefix 
		)
   {
	   
   }

private:
   if2_pattern_expander_standard expander;
   if2_scanner_alphanumeric k;
};
#endif

#endif
