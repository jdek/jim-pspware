#include <stdlib.h>
#include <unistd.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "luaplayer.h"


static lua_State *L;

void runScript(const char* filename)
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
	luaSystem_init(L);
	

	int s = luaL_loadfile(L, filename);
	if (s == 0) {
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
	}
	if (s) {
		printf("error: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1); // remove error message
	}
	lua_close(L);
}
