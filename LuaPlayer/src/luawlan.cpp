#ifndef LUAPLAYER_USERMODE

#include <pspsdk.h>
#include <psputility.h>
#include <pspnet_apctl.h>
#include "luaplayer.h"

extern "C" {
#include "my_socket.h"
}

#define MAX_PICK 5

typedef struct
{
	SOCKET sock;
	struct sockaddr_in addrTo;
	bool serverSocket;
} Socket;

UserdataStubs(Socket, Socket*)

static const char* wlanNotInitialized = "WLAN not initialized.";
static bool wlanInitialized = false;

static int Wlan_init(lua_State* L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
	int err = pspSdkInetInit();
	if (err != 0) return luaL_error(L, "pspSdkInetInit failed.");
	wlanInitialized = true;
	return 0;
}

static int Wlan_getConnectionConfigs(lua_State* L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");

	lua_newtable(L);

	struct
	{
		int index;
		char name[64];
	} picks[MAX_PICK];
	int pick_count = 0;
	
	int iNetIndex;
	for (iNetIndex = 1; iNetIndex < 100; iNetIndex++) // skip the 0th connection
	{
		if (sceUtilityCheckNetParam(iNetIndex) != 0) break;  // no more
		sceUtilityGetNetParam(iNetIndex, 0, (netData*) picks[pick_count].name);
		picks[pick_count].index = iNetIndex;
		pick_count++;
		lua_pushnumber(L, pick_count);
		lua_pushstring(L, picks[pick_count - 1].name);
		lua_settable(L, -3);
		if (pick_count >= MAX_PICK) break;  // no more room
	}

	return 1;  // table is already on top
}

static int Wlan_useConnectionConfig(lua_State* L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "Argument error: index to connection config expected."); 
	
	int connectionConfig = luaL_checkint(L, 1);
	int err = sceNetApctlConnect(connectionConfig);
    	if (err != 0) return luaL_error(L, "sceNetApctlConnect error");
	return 0;
}

static int Wlan_getIPAddress(lua_State* L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L); 
	if (argc != 0) return luaL_error(L, "no arguments expected.");

	char szMyIPAddr[32];
	if (sceNetApctlGetInfo(8, szMyIPAddr) != 0) {
		lua_pushstring(L, szMyIPAddr);
		return 1;
	}
	return luaL_error(L, "unknown IP address");
}

static int Socket_free(lua_State *L)
{
	Socket* socket = *toSocket(L, 1);
	sceNetInetClose(socket->sock);
	free(socket);
	return 0;
}

unsigned short htons(unsigned short wIn)
{
    u8 bHi = (wIn >> 8) & 0xFF;
    u8 bLo = wIn & 0xFF;
    return ((unsigned short)bLo << 8) | bHi;
}

int setSockNoBlock(SOCKET s, u32 val)
{ 
    return sceNetInetSetsockopt(s, SOL_SOCKET, 0x1009, (const char*)&val, sizeof(u32));
}

static int Socket_connect(lua_State *L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L); 
	if (argc != 2) return luaL_error(L, "host and port expected."); 
	
	Socket** luaSocket = pushSocket(L);
	Socket* socket = (Socket*) malloc(sizeof(Socket));
	*luaSocket = socket;
	socket->serverSocket = false;
	
	const char *host = luaL_checkstring(L, 1);
	int port = luaL_checkint(L, 2);
	u32 ip = sceNetInetInetAddr(host);  // TODO: resolv non-IP addresses

	int err;
	socket->sock = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
	if (socket->sock & 0x80000000) {
		Socket_free(L);
		return luaL_error(L, "invalid socket."); 
	}
	
	socket->addrTo.sin_family = AF_INET;
	socket->addrTo.sin_port = htons(port);
	socket->addrTo.sin_addr[0] = ip & 0xff;
	socket->addrTo.sin_addr[1] = (ip >> 8) & 0xff;
	socket->addrTo.sin_addr[2] = (ip >> 16) & 0xff;
	socket->addrTo.sin_addr[3] = (ip >> 24) & 0xff;
	
	setSockNoBlock(socket->sock, 1);
	err = sceNetInetConnect(socket->sock, &socket->addrTo, sizeof(socket->addrTo));
	Socket_free(L);
	
	if (err == -1 && sceNetInetGetErrno() != 0x77) {
		Socket_free(L);
		return luaL_error(L, "connection failed."); 
	}

	return 1;
}

static int Socket_isConnected(lua_State *L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 1) return luaL_error(L, "no argument expected.");

	Socket* socket = *toSocket(L, 1);

	// try connect again, which should always fail
	// look at why it failed to figure out if it is connected
	//REVIEW: a conceptually cleaner way to poll this?? (accept?)
	int err = sceNetInetConnect(socket->sock, &socket->addrTo, sizeof(socket->addrTo));
	if (err == 0 || (err == -1 && sceNetInetGetErrno() == 0x7F)) {
		// now connected - I hope
		lua_pushboolean(L, 1);
		return 1;
	}
	lua_pushboolean(L, 0);
	return 1;
}

static int Socket_recv(lua_State *L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 1) return luaL_error(L, "no argument expected.");

	Socket* socket = *toSocket(L, 1);

	char data[256];
	int count = sceNetInetRecv(socket->sock, (u8*) &data, 256, 0);
	if (count > 0) {
		lua_pushlstring(L, data, count);
	} else {
		lua_pushstring(L, "");
	}
	return 1;
}

static int Socket_send(lua_State *L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 2) return luaL_error(L, "one argument expected.");

	Socket* socket = *toSocket(L, 1);

	size_t size;
	const char *string = luaL_checklstring(L, 2, &size);
	if (!string) return luaL_error(L, "Socket:write expected a string.");
	int result = sceNetInetSend(socket->sock, string, size, 0);
	lua_pushnumber(L, result);
	return 1;
}

static int Socket_close(lua_State *L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 1) return luaL_error(L, "no argument expected.");

	Socket* socket = *toSocket(L, 1);
	sceNetInetClose(socket->sock);
	return 0;
}

static int Socket_tostring (lua_State *L)
{
	Socket* socket = *toSocket(L, 1);
	lua_pushfstring(L, "socket: %p, destination IP: %i.%i.%i.%i",
		socket,
		socket->addrTo.sin_addr[0],
		socket->addrTo.sin_addr[1],
		socket->addrTo.sin_addr[2],
		socket->addrTo.sin_addr[3]);
	return 1;
}

static const luaL_reg Socket_methods[] = {
	{"connect", Socket_connect},
	{"isConnected", Socket_isConnected},
	{"send", Socket_send},
	{"recv", Socket_recv},
	{"close", Socket_close},
	{0,0}
};

static const luaL_reg Socket_meta[] = {
	{"__gc", Socket_free},
	{"__tostring", Socket_tostring},
	{0,0}
};

UserdataRegister(Socket, Socket_methods, Socket_meta)

static const luaL_reg Wlan_functions[] = {
	{"init", Wlan_init},
	{"getConnectionConfigs", Wlan_getConnectionConfigs},
	{"useConnectionConfig", Wlan_useConnectionConfig},
	{"getIPAddress", Wlan_getIPAddress},
	{0, 0}
};

void luaWlan_init(lua_State *L)
{
	luaL_openlib(L, "Wlan", Wlan_functions, 0);
	Socket_register(L);
}

#endif
