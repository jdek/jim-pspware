#include <stdlib.h>
#include <pspkernel.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <psppower.h>
#include "luaplayer.h"

#include <unistd.h>

static int usbActivated = 0;


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


// Move g_dir to the stack and MEET CERTAIN DOOM! If the SceIoDirent is found on the STACK instead of among the globals, the PSP WILL CRASH IN A VERY WEIRD WAY. You have been WARNED.
SceIoDirent g_dir;

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

static int LoadStartModule(char *path)
{
    u32 loadResult;
    u32 startResult;
    int status;

    loadResult = sceKernelLoadModule(path, 0, NULL);
    if (loadResult & 0x80000000)
		return -1;
    else
		startResult =
	    sceKernelStartModule(loadResult, 0, NULL, &status, NULL);

    if (loadResult != startResult)
	return -2;

    return 0;
}

static int lua_usbActivate(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	if (usbActivated) return 0;
	
	static int modulesLoaded = 0;
	if (!modulesLoaded) {
		//start necessary drivers 
		LoadStartModule("flash0:/kd/semawm.prx"); 
		LoadStartModule("flash0:/kd/usbstor.prx"); 
		LoadStartModule("flash0:/kd/usbstormgr.prx"); 
		LoadStartModule("flash0:/kd/usbstorms.prx"); 
		LoadStartModule("flash0:/kd/usbstorboot.prx"); 
	
		//setup USB drivers 
		int retVal = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0); 
		if (retVal != 0) { 
			printf("Error starting USB Bus driver (0x%08X)\n", retVal); 
			sceKernelSleepThread(); 
		}
		retVal = sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0); 
		if (retVal != 0) { 
			printf("Error starting USB Mass Storage driver (0x%08X)\n", retVal); 
			sceKernelSleepThread(); 
		} 
		retVal = sceUsbstorBootSetCapacity(0x800000); 
		if (retVal != 0) { 
			printf("Error setting capacity with USB Mass Storage driver (0x%08X)\n", retVal); 
			sceKernelSleepThread(); 
		} 
		retVal = 0; 
		modulesLoaded = 1;
	}
	sceUsbActivate(0x1c8);
	usbActivated = 1;
	return 0;
}

static int lua_usbDeactivate(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	if (!usbActivated) return 0;

	sceUsbDeactivate();
	usbActivated = 0;
	return 0;
}

// battery functions
 
static int lua_powerIsPowerOnline(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushboolean(L, scePowerIsPowerOnline());
	return 1;
}

static int lua_powerIsBatteryExist(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushboolean(L, scePowerIsBatteryExist());
	return 1;
}

static int lua_powerIsBatteryCharging(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushboolean(L, scePowerIsBatteryCharging());
	return 1;
}

static int lua_powerGetBatteryChargingStatus(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushnumber(L, scePowerGetBatteryChargingStatus());
	return 1;
}

static int lua_powerIsLowBattery(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushboolean(L, scePowerIsLowBattery());
	return 1;
}

static int lua_powerGetBatteryLifePercent(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushnumber(L, scePowerGetBatteryLifePercent());
	return 1;
}

static int lua_powerGetBatteryLifeTime(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushnumber(L, scePowerGetBatteryLifeTime());
	return 1;
}

static int lua_powerGetBatteryTemp(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushnumber(L, scePowerGetBatteryTemp());
	return 1;
}

static int lua_powerGetBatteryVolt(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushnumber(L, scePowerGetBatteryVolt());
	return 1;
}

static const luaL_reg System_functions[] = {
  {"currentDirectory",              lua_curdir},
  {"listDirectory",           	    lua_dir},
  {"usbDiskModeActivate",           lua_usbActivate},
  {"usbDiskModeDeactivate",    	    lua_usbDeactivate},
  {"powerIsPowerOnline",            lua_powerIsPowerOnline},
  {"powerIsBatteryExist",           lua_powerIsBatteryExist},
  {"powerIsBatteryCharging",        lua_powerIsBatteryCharging},
  {"powerGetBatteryChargingStatus", lua_powerGetBatteryChargingStatus},
  {"powerIsLowBattery",             lua_powerIsLowBattery},
  {"powerGetBatteryLifePercent",    lua_powerGetBatteryLifePercent},
  {"powerGetBatteryLifeTime",       lua_powerGetBatteryLifeTime},
  {"powerGetBatteryTemp",           lua_powerGetBatteryTemp},
  {"powerGetBatteryVolt",           lua_powerGetBatteryVolt},
  {0, 0}
};
void luaSystem_init(lua_State *L) {
	luaL_openlib(L, "System", System_functions, 0);
}

