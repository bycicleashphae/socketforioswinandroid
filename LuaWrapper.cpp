#include "eng_socket.h"
#include "LuaMsg.h"
 
#include "LuaWrapper.h"

extern "C" void RegisterLuaSocketObject()
{
	lua::RegisteClassToScript();
	lua::RegisteGlobalFunctions();
}

void lua::RegisteClassToScript() {
	ext_lua_socket(_L);
	LuaPlus<CLuaMsg>::Register(_L); 
}
   
 
void lua::RegisteGlobalFunctions() {
	const luaL_reg global_functions[] = {
		{ "RegisteSocketClass", lua::LuaPlus<SocketObject>::RegisteSocketClassL }, 
		{ NULL, NULL }
	};
	int __recordStackIndex = lua_gettop((_L));
	luaL_register(_L, "eng", global_functions);	
	lua_pop(_L, 1);
	while (lua_gettop((_L)) > (__recordStackIndex + (0))) lua_remove((_L), __recordStackIndex + 1);
}
  