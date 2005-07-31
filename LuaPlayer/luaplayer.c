#include <stdlib.h>
#include <unistd.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>

#include "graphics.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define MAX_IMAGES 1024

static lua_State *L;

Image* images[MAX_IMAGES];
static int currentImageHandle = 0;

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
		int imageHandle = luaL_checkint(L, 3);
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
	if (argc != 4) return luaL_error(L, "wrong number of arguments");
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	const char* text = luaL_checkstring(L, 3);
	int color = luaL_checkint(L, 4);
	if (argc == 4) {
		printTextScreen(x, y, text, color);
	} else {
		int imageHandle = luaL_checkint(L, 3);
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
	lua_register(L, "dir", lua_dir);
	lua_register(L, "getCurrentDirectory", lua_getCurrentDirectory);
	lua_register(L, "setCurrentDirectory", lua_setCurrentDirectory);

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
