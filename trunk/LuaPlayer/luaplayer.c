#include <stdlib.h>
#include <unistd.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>

#include "graphics.h"
#include "sound.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define MAX_IMAGES 1024

#define true 1
#define false 0

#define SOUNDHANDLE "Sound"
#define VOICEHANDLE "Voice"

static lua_State *L;

Image* images[MAX_IMAGES];


static int currentImageHandle = 0;


// predefines
static Voice* pushVoice(lua_State *L);


static int lua_ctrlRead(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	SceCtrlData pad;
	sceCtrlReadBufferPositive(&pad, 1); 
	lua_pushnumber(L, (int)pad.Buttons);
	return 1;
}

#define CHECK_CTRL(name, bit) \
static int name(lua_State *L) \
{ \
	if (lua_gettop(L) != 1) return luaL_error(L, "wrong number of arguments"); \
	lua_pushboolean(L, ((int)luaL_checkint(L, 1) & bit) == bit); \
	return 1; \
}

CHECK_CTRL(lua_isCtrlSelect, PSP_CTRL_SELECT)
CHECK_CTRL(lua_isCtrlStart, PSP_CTRL_START)
CHECK_CTRL(lua_isCtrlUp, PSP_CTRL_UP)
CHECK_CTRL(lua_isCtrlRight, PSP_CTRL_RIGHT)
CHECK_CTRL(lua_isCtrlDown, PSP_CTRL_DOWN)
CHECK_CTRL(lua_isCtrlLeft, PSP_CTRL_LEFT)
CHECK_CTRL(lua_isCtrlLTrigger, PSP_CTRL_LTRIGGER)
CHECK_CTRL(lua_isCtrlRTrigger, PSP_CTRL_RTRIGGER)
CHECK_CTRL(lua_isCtrlTriangle, PSP_CTRL_TRIANGLE)
CHECK_CTRL(lua_isCtrlCircle, PSP_CTRL_CIRCLE)
CHECK_CTRL(lua_isCtrlCross, PSP_CTRL_CROSS)
CHECK_CTRL(lua_isCtrlSquare, PSP_CTRL_SQUARE)
CHECK_CTRL(lua_isCtrlHome, PSP_CTRL_HOME)
CHECK_CTRL(lua_isCtrlHold, PSP_CTRL_HOLD)

static int lua_waitVblankStart(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 0 && argc != 1) return luaL_error(L, "wrong number of arguments");
	if (argc == 0) {
		sceDisplayWaitVblankStart();
	} else {
		int count = luaL_checkint(L, 1);
		int i;
		for (i = 0; i < count; i++) sceDisplayWaitVblankStart();
	}
	return 0;
}

static int lua_flipScreen(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	flipScreen();
	return 0;
}

// returns 0, if nothing to blit
static int adjustBlitRectangle(
	int sourceWidth, int sourceHeight,
	int destinationWidth, int destinationHeight,
	int* sx, int* sy,
	int* width, int* height,
	int* dx, int* dy)
{
	if (*width <= 0 || *height <= 0) return 0;  // zero area, nothing to blit
	if (*sx < 0 || *sy < 0) return 0;  // illegal, source is not clipped
	if (*dx < 0) {
		*width += *dx;
		if (*width <= 0) return 0;
		*dx = 0;
	}
	if (*dy < 0) {
		*height += *dy;
		if (*height <= 0) return 0;
		*dy = 0;
	}
	if (*dx + *width > destinationWidth) {
		*width = destinationWidth - *dx;
		if (*width <= 0) return 0;
	}
	if (*dy + *height > destinationHeight) {
		*height = destinationHeight - *dy;
		if (*height <= 0) return 0;
	}
	return 1;
}
	
static int lua_blitImage(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 3 && argc != 4) return luaL_error(L, "wrong number of arguments");
	int dx = luaL_checkint(L, 1);
	int dy = luaL_checkint(L, 2);
	int imageHandle = luaL_checkint(L, 3);
	if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong source image handle");
	Image* image = images[imageHandle];
	int width = image->imageWidth;
	int height = image->imageHeight;
	int sx = 0;
	int sy = 0;
	if (argc == 3) {
		if (!adjustBlitRectangle(width, height, SCREEN_WIDTH, SCREEN_HEIGHT, &sx, &sy, &width, &height, &dx, &dy)) return 0;
		blitImageToScreen(sx, sy, width, height, image, dx, dy);
	} else {
		int destHandle = luaL_checkint(L, 4);
		if (destHandle < 1 || destHandle > currentImageHandle) return luaL_error(L, "wrong destination image handle");
		Image* destImage = images[destHandle];
		if (!adjustBlitRectangle(width, height, destImage->imageWidth, destImage->imageHeight, &sx, &sy, &width, &height, &dx, &dy)) return 0;
		blitImageToImage(sx, sy, width, height, image, dx, dy, destImage);
	}
	return 0;
}

static int lua_blitAlphaImage(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 3 && argc != 4) return luaL_error(L, "wrong number of arguments");
	int dx = luaL_checkint(L, 1);
	int dy = luaL_checkint(L, 2);
	int imageHandle = luaL_checkint(L, 3);
	if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong source image handle");
	Image* image = images[imageHandle];
	int width = image->imageWidth;
	int height = image->imageHeight;
	int sx = 0;
	int sy = 0;
	if (argc == 3) {
		if (!adjustBlitRectangle(width, height, SCREEN_WIDTH, SCREEN_HEIGHT, &sx, &sy, &width, &height, &dx, &dy)) return 0;
		blitAlphaImageToScreen(sx, sy, width, height, image, dx, dy);
	} else {
		int destHandle = luaL_checkint(L, 4);
		if (destHandle < 1 || destHandle > currentImageHandle) return luaL_error(L, "wrong destination image handle");
		Image* destImage = images[destHandle];
		if (!adjustBlitRectangle(width, height, destImage->imageWidth, destImage->imageHeight, &sx, &sy, &width, &height, &dx, &dy)) return 0;
		blitAlphaImageToImage(sx, sy, width, height, image, dx, dy, destImage);
	}
	return 0;
}

static int lua_blitImageRect(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 7 && argc != 8) return luaL_error(L, "wrong number of arguments");
	int sx = luaL_checkint(L, 1);
	int sy = luaL_checkint(L, 2);
	int width = luaL_checkint(L, 3);
	int height = luaL_checkint(L, 4);
	int imageHandle = luaL_checkint(L, 5);
	int dx = luaL_checkint(L, 6);
	int dy = luaL_checkint(L, 7);
	if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong source image handle");
	Image* image = images[imageHandle];
	if (argc == 7) {
		if (!adjustBlitRectangle(width, height, SCREEN_WIDTH, SCREEN_HEIGHT, &sx, &sy, &width, &height, &dx, &dy)) return 0;
		blitImageToScreen(sx, sy, width, height, image, dx, dy);
	} else {
		int destHandle = luaL_checkint(L, 8);
		if (destHandle < 1 || destHandle > currentImageHandle) return luaL_error(L, "wrong destination image handle");
		Image* destImage = images[destHandle];
		if (!adjustBlitRectangle(width, height, destImage->imageWidth, destImage->imageHeight, &sx, &sy, &width, &height, &dx, &dy)) return 0;
		blitImageToImage(sx, sy, width, height, image, dx, dy, destImage);
	}
	return 0;
}

static int lua_blitAlphaImageRect(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 7 && argc != 8) return luaL_error(L, "wrong number of arguments");
	int sx = luaL_checkint(L, 1);
	int sy = luaL_checkint(L, 2);
	int width = luaL_checkint(L, 3);
	int height = luaL_checkint(L, 4);
	int imageHandle = luaL_checkint(L, 5);
	int dx = luaL_checkint(L, 6);
	int dy = luaL_checkint(L, 7);
	if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong source image handle");
	Image* image = images[imageHandle];
	if (argc == 7) {
		if (!adjustBlitRectangle(width, height, SCREEN_WIDTH, SCREEN_HEIGHT, &sx, &sy, &width, &height, &dx, &dy)) return 0;
		blitAlphaImageToScreen(sx, sy, width, height, image, dx, dy);
	} else {
		int destHandle = luaL_checkint(L, 8);
		if (destHandle < 1 || destHandle > currentImageHandle) return luaL_error(L, "wrong destination image handle");
		Image* destImage = images[destHandle];
		if (!adjustBlitRectangle(width, height, destImage->imageWidth, destImage->imageHeight, &sx, &sy, &width, &height, &dx, &dy)) return 0;
		blitAlphaImageToImage(sx, sy, width, height, image, dx, dy, destImage);
	}
	return 0;
}

int lua_fillRect(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments");
	int color = luaL_checkint(L, 1);
	int x0 = luaL_checkint(L, 2);
	int y0 = luaL_checkint(L, 3);
	int width = luaL_checkint(L, 4);
	int height = luaL_checkint(L, 5);
	if (width <= 0 || height <= 0) return 0;
	if (x0 < 0) {
		width += x0;
		if (width <= 0) return 0;
		x0 = 0;
	}
	if (y0 < 0) {
		height += y0;
		if (height <= 0) return 0;
		y0 = 0;
	}
	if (argc == 5) {
		if (width <= 0 || height <= 0) return 0;
		if (x0 >= SCREEN_WIDTH || y0 >= SCREEN_HEIGHT) return 0;
		if (x0 + width >= SCREEN_WIDTH) {
			width = SCREEN_WIDTH - x0;
			if (width <= 0) return 0;
		}
		if (y0 + height >= SCREEN_HEIGHT) {
			height = SCREEN_HEIGHT - y0;
			if (height <= 0) return 0;
		}
		fillScreenRect(color, x0, y0, width, height);
	} else {
		int imageHandle = luaL_checkint(L, 6);
		if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong destination image handle");
		Image* image = images[imageHandle];
		if (x0 >= image->imageWidth || y0 >= image->imageHeight) return 0;
		if (x0 + width >= image->imageWidth) {
			width = image->imageWidth - x0;
			if (width <= 0) return 0;
		}
		if (y0 + height >= image->imageHeight) {
			height = image->imageHeight - y0;
			if (height <= 0) return 0;
		}
		fillImageRect(color, x0, y0, width, height, image);
	}
	return 0;
}

static int lua_createImage(lua_State *L)
{
	if (lua_gettop(L) != 2) return luaL_error(L, "wrong number of arguments");
	int w = luaL_checkint(L, 1);
	int h = luaL_checkint(L, 2);
	if (w <= 0 || h <= 0 || w > SCREEN_WIDTH || h > SCREEN_HEIGHT) return luaL_error(L, "invalid size");
	if (currentImageHandle == MAX_IMAGES - 1) return luaL_error(L, "two many image handles");
	Image* image = createImage(w, h);
	if (!image) return luaL_error(L, "can't create image");
	images[++currentImageHandle] = image;
	lua_pushnumber(L, currentImageHandle);
	return 1;
}

static int lua_clear(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
	int color = luaL_checkint(L, 1);
	if (argc == 1) {
		clearScreen(color);
	} else {
		int imageHandle = luaL_checkint(L, 2);
		if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong image handle");
		Image* image = images[imageHandle];
		clearImage(color, image);
	}
	return 0;
}

static int lua_screenshot(lua_State *L)
{
	if (lua_gettop(L) != 1) return luaL_error(L, "wrong number of arguments");
	const char *filename = luaL_checkstring(L, 1);
	screenshot(filename);
	return 0;
}

static int lua_loadImage(lua_State *L)
{
	if (lua_gettop(L) != 1) return luaL_error(L, "wrong number of arguments");
	if (++currentImageHandle == MAX_IMAGES) return luaL_error(L, "too many images");
	Image* image = loadImage(luaL_checkstring(L, 1));
	if (!image) return luaL_error(L, "error loading image");
	images[currentImageHandle] = image;
	lua_pushnumber(L, currentImageHandle);
	return 1;
}

static int lua_getImageWidth(lua_State *L)
{
	if (lua_gettop(L) != 1) return luaL_error(L, "wrong number of arguments");
	int imageHandle = luaL_checkint(L, 1);
	if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong image handle");
	lua_pushnumber(L, images[imageHandle]->imageWidth);
	return 1;
}

static int lua_getImageHeight(lua_State *L)
{
	if (lua_gettop(L) != 1) return luaL_error(L, "wrong number of arguments");
	int imageHandle = luaL_checkint(L, 1);
	if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong image handle");
	lua_pushnumber(L, images[imageHandle]->imageHeight);
	return 1;
}

static int lua_putPixel(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 3 && argc != 4) return luaL_error(L, "wrong number of arguments");
	int color = luaL_checkint(L, 1);
	int x = luaL_checkint(L, 2);
	int y = luaL_checkint(L, 3);
	if (argc == 3) {
		if (x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
			putPixelScreen(color, x, y);
		}
	} else {
		int imageHandle = luaL_checkint(L, 4);
		if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong image handle");
		Image* image = images[imageHandle];
		if (x >= 0 && y >= 0 && x < image->imageWidth && y < image->imageHeight) {
			putPixelImage(color, x, y, image);
		}
	}
	return 0;
}

static int lua_getPixel(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 2 && argc != 3) return luaL_error(L, "wrong number of arguments");
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	if (argc == 2) {
		if (x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
			lua_pushnumber(L, getPixelScreen(x, y));
			return 1;
		}
	} else {
		int imageHandle = luaL_checkint(L, 3);
		if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong image handle");
		Image* image = images[imageHandle];
		if (x >= 0 && y >= 0 && x < image->imageWidth && y < image->imageHeight) {
			lua_pushnumber(L, getPixelImage(x, y, image));
			return 1;
		}
	}
	return luaL_error(L, "wrong arguments");
}

static int lua_printDecimal(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 4 && argc != 5) return luaL_error(L, "wrong number of arguments");
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	int value = luaL_checkint(L, 3);
	int color = luaL_checkint(L, 4);
	Image* image = NULL;
	if (argc == 5) {
		int imageHandle = luaL_checkint(L, 5);
		if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong source image handle");
		image = images[imageHandle];
	}
	int i;
	int digits[6];
	for (i = 5; i >= 0; i--) {
		digits[i] = value%10;
		value /= 10;
	}
	i = 0;
	while (i < 5 && digits[i] == 0) digits[i++] = -1;
	for (i = 0; i < 6; i++) {
		int digit = digits[i];
		if (digit >= 0) {
			if (x >= 0 && y >= 0 && x < SCREEN_WIDTH - 4 && y < SCREEN_HEIGHT - 5 && digit <= 9) {
				if (argc == 4) {
					print7SegmentScreen(x, y, digits[i], color);
				} else {
					print7SegmentImage(x, y, digits[i], color, image);
				}
			}
			x += 6;
		}
	}
	return 0;
}

static int lua_printText(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 4 && argc != 5) return luaL_error(L, "wrong number of arguments");
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	const char* text = luaL_checkstring(L, 3);
	int color = luaL_checkint(L, 4);
	if (argc == 4) {
		printTextScreen(x, y, text, color);
	} else {
		int imageHandle = luaL_checkint(L, 5);
		if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong source image handle");
		Image* image = images[imageHandle];
		printTextImage(x, y, text, color, image);
	}
	return 0;
}

static int lua_getColorNumber(lua_State *L) 
{ 
	int argc = lua_gettop(L); 
	if (argc != 3) return luaL_error(L, "wrong number of arguments"); 
	
	int r = luaL_checkint(L, 1); 
	int g = luaL_checkint(L, 2); 
	int b = luaL_checkint(L, 3); 
	
	if (r > 255)  r = 255; 
	if (g > 255)  g = 255; 
	if (b > 255)  b = 255; 
	if (r < 0)  r = 0; 
	if (g < 0)  g = 0; 
	if (b < 0)  b = 0; 
	
	u16 rgb = ((b>>3)<<10) | ((g>>3)<<5) | (r>>3) | 0x8000;
	
	lua_pushnumber(L, rgb); 
	return 1; 
}

static int lua_drawLine(lua_State *L) 
{ 
	int argc = lua_gettop(L); 
	if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments"); 
	
	int x0 = luaL_checkint(L, 1); 
	int y0 = luaL_checkint(L, 2); 
	int x1 = luaL_checkint(L, 3); 
	int y1 = luaL_checkint(L, 4); 
	int color = luaL_checkint(L, 5); 
	
	// TODO: better clipping
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (argc == 5) {
		if (x0 >= SCREEN_WIDTH) x0 = SCREEN_WIDTH - 1;
		if (x1 >= SCREEN_WIDTH) x1 = SCREEN_WIDTH - 1;
		if (y0 >= SCREEN_HEIGHT) y0 = SCREEN_HEIGHT - 1;
		if (y1 >= SCREEN_HEIGHT) y1 = SCREEN_HEIGHT - 1;
		drawLineScreen(x0, y0, x1, y1, color);
	} else {
		int imageHandle = luaL_checkint(L, 6);
		if (imageHandle < 1 || imageHandle > currentImageHandle) return luaL_error(L, "wrong source image handle");
		Image* image = images[imageHandle];
		if (x0 >= image->imageWidth) x0 = image->imageWidth - 1;
		if (x1 >= image->imageWidth) x1 = image->imageWidth - 1;
		if (y0 >= image->imageHeight) y0 = image->imageHeight - 1;
		if (y1 >= image->imageHeight) y1 = image->imageHeight - 1;
		drawLineImage(x0, y0, x1, y1, color, image);
	}
	return 0;
}


SceIoDirent g_dir;

static int lua_dir(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 0 && argc != 1) return luaL_error(L, "wrong number of arguments");

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

static int lua_getCurrentDirectory(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 0) return luaL_error(L, "wrong number of arguments");

	char path[256];
	getcwd(path, 256);
	lua_pushstring(L, path);
	
	return 1;
}

static int lua_setCurrentDirectory(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 1) return luaL_error(L, "wrong number of arguments");

	const char *path = luaL_checkstring(L, 1);
	chdir(path);
	
	return 0;
}



// ===================================
// Helper functions
// ===================================


void mystrcat(char * s, const char * b) {
	while(*s != 0) s++;
	while(*b != 0) *(s++) = *(b++);
	*s = 0;
}

void stackDump (lua_State *L) {
  int i;
  int top = lua_gettop(L);
  for (i = 1; i <= top; i++) {  /* repeat for each level */
	int t = lua_type(L, i);
	switch (t) {

	  case LUA_TSTRING:  /* strings */
		printf("`%s'", lua_tostring(L, i));
		break;

	  case LUA_TBOOLEAN:  /* booleans */
		printf(lua_toboolean(L, i) ? "true" : "false");
		break;

	  case LUA_TNUMBER:  /* numbers */
		printf("%g", lua_tonumber(L, i));
		break;

	  default:  /* other values */
		printf("%s", lua_typename(L, t));
		break;

	}
	printf("  ");  /* put a separator */
  }
  printf("\n");  /* end the listing */
}

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
	if(argc == 2) loop = luaL_checknumber(L, 2);
	
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

static Sound** toSound (lua_State *L, int index)
{
  Sound** handle  = (Sound **)lua_touserdata(L, index);
  if (handle == NULL) luaL_typerror(L, index, SOUNDHANDLE);
  return handle;
}



static Sound** pushSound(lua_State *L) {
	Sound ** sound = (Sound**)lua_newuserdata(L, sizeof(Sound*)); // stack:
	luaL_getmetatable(L, SOUNDHANDLE); // sound, sound.meta
	lua_pushvalue(L, -1); // sound, sound.meta, sound.meta
	lua_setmetatable(L, -3); // sound, sound.meta
	lua_pushstring(L, "__index");	//sound, sound.meta, __index
	lua_pushstring(L, SOUNDHANDLE); // sound, sound.meta, __index, "Sound"
	lua_gettable(L, LUA_GLOBALSINDEX);//sound, sound.meta, __index, Sound
	lua_settable(L, -3); // sound.meta.__index = Sound
					   // stack is sound, sound.meta
	lua_pop(L, 1); // stack is sound
	return sound;
}

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
  {"new",           Sound_new},
  {"play", 			Sound_play},
  {0, 0}
};
static const luaL_reg Sound_meta[] = {
  {"__gc",       Sound_gc},
  {"__tostring", Sound_tostring},
  {0, 0}
};
int Sound_register(lua_State *L) {
	luaL_newmetatable(L, SOUNDHANDLE);
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3); // metatable.__index = metatable
	
	
	luaL_openlib(L, 0, Sound_meta, 0);
	luaL_openlib(L, SOUNDHANDLE, Sound_methods, 0);
	
	lua_pushstring(L, SOUNDHANDLE);
	lua_gettable(L, LUA_GLOBALSINDEX);
	luaL_getmetatable(L, SOUNDHANDLE);
	lua_setmetatable(L, -2);
	return 1;
}


// The "Voice" userdata object.
// ------------------------------
// note: To implement a usedata object, see 
// http://lua-users.org/wiki/UserDataExample


static Voice *toVoice (lua_State *L, int index)
{
  Voice* handle  = (Voice*)lua_touserdata(L, index);
  if (handle == NULL) luaL_typerror(L, index, VOICEHANDLE);
  return handle;
}

static Voice* pushVoice(lua_State *L) {
	Voice * voice = (Voice*)lua_newuserdata(L, sizeof(Voice));
	luaL_getmetatable(L, VOICEHANDLE);
	lua_pushvalue(L, -1);
	lua_setmetatable(L, -3);
	
	lua_pushstring(L, "__index");
	lua_pushstring(L, VOICEHANDLE);
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_settable(L, -3); // voice.__index = Voice

	lua_pop(L, 1);
	return voice;
}


static int Voice_tostring (lua_State *L)
{
  char buff[32];
  sprintf(buff, "%d", *toVoice(L, 1));
  lua_pushfstring(L, "Voice (%s)", buff);
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
int Voice_register(lua_State *L) {
	luaL_newmetatable(L, VOICEHANDLE);
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3); // metatable.__index = metatable
	
	
	luaL_openlib(L, 0, Voice_meta, 0);
	luaL_openlib(L, VOICEHANDLE, Voice_methods, 0);
	
	lua_pushstring(L, VOICEHANDLE);
	lua_gettable(L, LUA_GLOBALSINDEX);
	luaL_getmetatable(L, VOICEHANDLE);
	lua_setmetatable(L, -2);
	return 1;
}


// ===================================
// LuaPlayer code
// ===================================



void runScript(const char* filename)
{
	L = lua_open();
	
	luaopen_io(L);
	luaopen_base(L);
	luaopen_table(L);
	luaopen_string(L);
	luaopen_math(L);
	luaopen_loadlib(L);
	
	lua_register(L, "ctrlRead", lua_ctrlRead);
	lua_register(L, "isCtrlSelect", lua_isCtrlSelect);
	lua_register(L, "isCtrlStart", lua_isCtrlStart);
	lua_register(L, "isCtrlUp", lua_isCtrlUp);
	lua_register(L, "isCtrlRight", lua_isCtrlRight);
	lua_register(L, "isCtrlDown", lua_isCtrlDown);
	lua_register(L, "isCtrlLeft", lua_isCtrlLeft);
	lua_register(L, "isCtrlUp", lua_isCtrlUp);
	lua_register(L, "isCtrlLTrigger", lua_isCtrlLTrigger);
	lua_register(L, "isCtrlRTrigger", lua_isCtrlRTrigger);
	lua_register(L, "isCtrlTriangle", lua_isCtrlTriangle);
	lua_register(L, "isCtrlCircle", lua_isCtrlCircle);
	lua_register(L, "isCtrlCross", lua_isCtrlCross);
	lua_register(L, "isCtrlSquare", lua_isCtrlSquare);
	lua_register(L, "isCtrlHome", lua_isCtrlHome);
	lua_register(L, "isCtrlHold", lua_isCtrlHold);
	lua_register(L, "waitVblankStart", lua_waitVblankStart);
	lua_register(L, "flipScreen", lua_flipScreen);
	lua_register(L, "blitImage", lua_blitImage);
	lua_register(L, "blitAlphaImage", lua_blitAlphaImage);
	lua_register(L, "blitImageRect", lua_blitImageRect);
	lua_register(L, "blitAlphaImageRect", lua_blitAlphaImageRect);
	lua_register(L, "fillRect", lua_fillRect);
	lua_register(L, "createImage", lua_createImage);
	lua_register(L, "clear", lua_clear);
	lua_register(L, "screenshot", lua_screenshot);
	lua_register(L, "loadImage", lua_loadImage);
	lua_register(L, "getImageWidth", lua_getImageWidth);
	lua_register(L, "getImageHeight", lua_getImageHeight);
	lua_register(L, "putPixel", lua_putPixel);
	lua_register(L, "getPixel", lua_getPixel);
	lua_register(L, "printDecimal", lua_printDecimal);
	lua_register(L, "printText", lua_printText);
	lua_register(L, "getColorNumber", lua_getColorNumber);
	lua_register(L, "drawLine", lua_drawLine);
	lua_register(L, "dir", lua_dir);
	lua_register(L, "getCurrentDirectory", lua_getCurrentDirectory);
	lua_register(L, "setCurrentDirectory", lua_setCurrentDirectory);
	
	//// Mikmod
	luaL_openlib(L, "Music", Music_functions, 0);
	luaL_openlib(L, "SoundSystem", SoundSystem_functions, 0);
	Sound_register(L);
	Voice_register(L);

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
