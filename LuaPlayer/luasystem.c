#include <stdlib.h>
#include <pspkernel.h>
#include "luaplayer.h"

#include <unistd.h>



static int lua_getCurrentDirectory(lua_State *L)
{
	char path[256];
	getcwd(path, 256);
	lua_pushstring(L, path);
	
	return 1;
}

static int lua_setCurrentDirectory(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	if(!path) return luaL_error(L, "Argument error: System.currentDirectory(file) takes a filename as string as argument.");

	lua_getCurrentDirectory(L);
	chdir(path);
	
	return 1;
}

static int lua_curdir(lua_State *L) {
	int argc = lua_gettop(L);
	if(argc == 0) return lua_getCurrentDirectory(L);
	if(argc == 1) return lua_setCurrentDirectory(L);
	return luaL_error(L, "Argument error: System.currentDirectory([file]) takes zero or one argument.");
}




static int lua_dir(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 0 && argc != 1) return luaL_error(L, "Argument error: System.listDirectory([path]) takes zero or one argument.");

	const char *path = "";
	if (argc == 0) {
		path = "";
	} else {
		path = luaL_checkstring(L, 1);
	}
	int fd = sceIoDopen(path);
	SceIoDirent g_dir;
	if (fd < 0) {
		lua_pushnil(L);  /* return nil */
		return 1;
	}
	lua_newtable(L);
	int i = 1;
	while (sceIoDread(fd, &g_dir) > 0) {
		lua_pushnumber(L, i++);  /* push key for file entry */

		lua_newtable(L);

		lua_pushstring(L, "name");
		lua_pushstring(L, g_dir.d_name);
		lua_settable(L, -3);

		lua_pushstring(L, "size");
		lua_pushnumber(L, g_dir.d_stat.st_size);
		lua_settable(L, -3);

		lua_pushstring(L, "directory");

		lua_pushboolean(L, g_dir.d_stat.st_attr & FIO_SO_IFDIR);
		lua_settable(L, -3);

		lua_settable(L, -3);
	}

	sceIoDclose(fd);

	return 1;  /* table is already on top */
}

static const luaL_reg System_functions[] = {
  {"currentDirectory",          lua_curdir},
  {"listDirectory",           	lua_dir},
  {0, 0}
};
void luaSystem_init(lua_State *L) {
	luaL_openlib(L, "System", System_functions, 0);
}

