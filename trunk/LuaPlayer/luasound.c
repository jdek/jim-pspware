#include <stdlib.h>
#include <unistd.h>
#include "luaplayer.h"

#include "sound.h"

// Forward declaration
static Voice* pushVoice(lua_State *L);


// ===================================
// Sound through mikmodlib and sound.*
// ===================================


// Music
// ------------------------------

static int lua_loadAndPlayMusicFile(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
	
	const char *path = luaL_checkstring(L, 1);
	BOOL loop = false;
	if(argc == 2) loop = lua_toboolean(L, 2);
	
	char fullpath[512];
	getcwd(fullpath, 256);
	mystrcat(fullpath, "/");
	mystrcat(fullpath, path);
	
	loadAndPlayMusicFile(fullpath, loop);
	
	return 0;
}

static int lua_stopAndUnloadMusic(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	
	stopAndUnloadMusic();
	
	return 0;
}

static int lua_musicIsPlaying(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	
	BOOL result = musicIsPlaying();

	lua_pushboolean(L, result);
	return 1;
}

static int lua_setMusicVolume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 ) return luaL_error(L, "wrong number of arguments");
	int arg = luaL_checknumber(L, 1);

	setMusicVolume(arg);

	return 0;
}


static const luaL_reg Music_functions[] = {
  {"playFile",          lua_loadAndPlayMusicFile},
  {"stop",           	lua_stopAndUnloadMusic},
  {"playing", 		lua_musicIsPlaying},
  {"volume", 			lua_setMusicVolume},
  {0, 0}
};


// Utility functions
// ------------------------------

static int lua_setSFXVolume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 ) return luaL_error(L, "wrong number of arguments");
	int arg = luaL_checknumber(L, 1);

	setSFXVolume(arg);

	return 0;
}
static int lua_setReverb(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 ) return luaL_error(L, "wrong number of arguments");
	int arg = luaL_checknumber(L, 1);

	setReverb(arg);

	return 0;
}
static int lua_setPanSep(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 ) return luaL_error(L, "wrong number of arguments");
	int arg = luaL_checknumber(L, 1);

	setPanSep(arg);

	return 0;
}

static const luaL_reg SoundSystem_functions[] = {
  {"SFXVolume",          		lua_setSFXVolume},
  {"reverb",           			lua_setReverb},
  {"panoramicSeparation", 		lua_setPanSep},
  {0, 0}
};


// The "Sound" userdata object.
// ------------------------------
// note: To implement a usedata object, see 
// http://lua-users.org/wiki/UserDataExample

UserdataStubs(Sound, Sound*)

static int Sound_new(lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 1 )
		return luaL_error(L, "Need a file path argument as only argument.");
	
	// Load variables
	char fullpath[512];
	const char *path = luaL_checkstring(L, 1);
	if(!path) return luaL_error(L, "Argument must be a file path.");
	getcwd(fullpath, 256);
	mystrcat(fullpath, "/");
	mystrcat(fullpath, path);
	
	// Create the user object
	Sound** newsound = pushSound(L);
	*newsound = loadSound(fullpath);
	
	if (!*newsound) return luaL_error(L, "error loading sound");
	
	// Note: a userdata object has already been pushed.
	return 1;
}

static int Sound_gc(lua_State *L) // garbage collect
{
	Sound** handle= toSound(L, 1);
	unloadSound(*handle);
	return 0;
}

static int Sound_tostring (lua_State *L)
{
  char buff[32];
  sprintf(buff, "%p", *toSound(L, 1));
  lua_pushfstring(L, "Sound (%s)", buff);
  return 1;
}


static int Sound_play(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 ) return luaL_error(L, "Sound:play() takes no arguments. Also, call it with a colon, not a dot.");
	
	Sound** handle = toSound(L, 1);
	
	Voice *voice = pushVoice(L);
	*voice = playSound(*handle);
	
	// the Voice is already pushed
	return 1;
}




static const luaL_reg Sound_methods[] = {
  {"load",          Sound_new},
  {"play", 			Sound_play},
  {0, 0}
};
static const luaL_reg Sound_meta[] = {
  {"__gc",       Sound_gc},
  {"__tostring", Sound_tostring},
  {0, 0}
};

UserdataRegister(Sound, Sound_methods, Sound_meta)

// The "Voice" userdata object.
// ------------------------------
// note: To implement a usedata object, see 
// http://lua-users.org/wiki/UserDataExample

UserdataStubs(Voice, Voice)

static int Voice_tostring (lua_State *L)
{
  lua_pushfstring(L, "Voice (%d)", *toVoice(L, 1));
  return 1;
}


static int Voice_stop(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 ) return luaL_error(L, "Voice:stop() takes no arguments. Also, call it with a colon, not a dot.");
	
	unsigned handle = *toVoice(L, 1);

	stopSound(handle);

	return 0;
}

static int Voice_setVolume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2 )
		return luaL_error(L, "Voice:setVolume() takes one argument.");
	
	unsigned handle = *toVoice(L, 1);
	int arg = luaL_checknumber(L, 2);

	setVoiceVolume(handle, arg);

	return 0;
}

static int Voice_setPanning(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2 )
		return luaL_error(L, "Voice:setPanning() takes one argument.");
	
	unsigned handle = *toVoice(L, 1);
	int arg = luaL_checknumber(L, 2);
	
	setVoicePanning(handle, arg);

	return 0;
}

static int Voice_setFrequency(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2 )
		return luaL_error(L, "Voice:setFrequency() takes one argument.");
	
	unsigned handle = *toVoice(L, 1);
	int arg = luaL_checknumber(L, 2);

	
	setVoiceFrequency(handle, arg);

	return 0;
}

static int Voice_playing(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 )
		return luaL_error(L, "Voice:playing() takes no arguments. Also, call it with a colon, not a dot.");
	
	unsigned handle = *toVoice(L, 1);
	
	BOOL result = voiceIsPlaying(handle);

	lua_pushboolean(L, result);
	return 1;
}



static const luaL_reg Voice_methods[] = {
  {"stop",          Voice_stop},
  {"volume", 		Voice_setVolume},
  {"pan", 			Voice_setPanning},
  {"frequency", 	Voice_setFrequency},
  {"playing", 	 	Voice_playing},
  {0, 0}
};
static const luaL_reg Voice_meta[] = {
  // Doesn't need to be gc'd, mikmod deallocates voice when it has stopped
  {"__tostring", Voice_tostring},
  {0, 0}
};
UserdataRegister(Voice, Voice_methods, Voice_meta)



void luaSound_init(lua_State *L) {
	luaL_openlib(L, "Music", Music_functions, 0);
	luaL_openlib(L, "SoundSystem", SoundSystem_functions, 0);
	Sound_register(L);
	Voice_register(L);
}

