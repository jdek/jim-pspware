#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>

#include "luaplayer.h"


static lua_State *L;

extern "C" {
	lua_State * getLuaState()
	{
		return L;
	}
}

const char * runScript(const char* script, bool isStringBuffer )
{
	L = lua_open();
	
	// Standard libraries
	luaopen_io(L);
	luaopen_base(L);
	luaopen_table(L);
	luaopen_string(L);
	luaopen_math(L);
	luaopen_loadlib(L);
	
	// Modules
	luaSound_init(L);
	luaControls_init(L);
	luaGraphics_init(L);
	lua3D_init(L);
	luaTimer_init(L);
	luaSystem_init(L);
	luaWlan_init(L);
	
	int s = 0;
	const char * errMsg = NULL;

	if(!isStringBuffer) 
		s = luaL_loadfile(L, script);
	else 
		s = luaL_loadbuffer(L, script, strlen(script), NULL);
		
	if (s == 0) {
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
	}
	if (s) {
		errMsg = lua_tostring(L, -1);
		printf("error: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1); // remove error message
	}
	lua_close(L);
	
	return errMsg;
}
