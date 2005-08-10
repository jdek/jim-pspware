#include <stdlib.h>
#include <pspdisplay.h>
#include "luaplayer.h"

#include "graphics.h"
#ifndef BOOL
#define BOOL int
#endif

typedef unsigned Color;

UserdataStubs(Color, Color)


/// screen.*
/// ====================
static int lua_waitVblankStart(lua_State *L)
{
	int argc = lua_gettop(L), t = 0;
	if (argc != 0 && argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments"); // can be called as both screen.wait...() and screen:wait...()
	if (argc) t = lua_type(L, 1);
	if (argc == 0 || t != LUA_TNUMBER) {
		sceDisplayWaitVblankStart();
	} else {
		int count = (t == LUA_TNUMBER)?luaL_checkint(L, 1):luaL_checkint(L, 2);
		int i;
		for (i = 0; i < count; i++) sceDisplayWaitVblankStart();
	}
	return 0;
}

static int lua_flipScreen(lua_State *L)
{
	flipScreen();
	return 0;
}

/// Utility
/// ====================

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
		*sx -= *dx; 
		*dx = 0; 
		if (*sx >= destinationWidth) return 0; 
	} 
	if (*dy < 0) { 
		*height += *dy; 
		if (*height <= 0) return 0; 
		*sy -= *dy; 
		*dy = 0; 
		if (*sy >= destinationHeight) return 0; 
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





UserdataStubs(Image, Image*) //==========================
static int Image_createEmpty(lua_State *L)
{
	if (lua_gettop(L) != 2) return luaL_error(L, "Argument error: Image.createEmpty(w, h) takes two arguments.");
	int w = luaL_checkint(L, 1);
	int h = luaL_checkint(L, 2);
	if (w <= 0 || h <= 0 || w > SCREEN_WIDTH || h > SCREEN_HEIGHT) return luaL_error(L, "invalid size");
	Image** image = pushImage(L);
	*image = createImage(w, h);
	if (!*image) return luaL_error(L, "can't create image");
	return 1;
}
static int Image_load (lua_State *L) {
	if (lua_gettop(L) != 1) return luaL_error(L, "Argument error: Image.load(filename) takes one argument.");
	Image** image = pushImage(L);
	*image = loadImage(luaL_checkstring(L, 1));
	if(!*image) return luaL_error(L, "Image.load: Error loading image.");
	return 1;
}



#define SETDEST \
	Image *dest = NULL; \
	{ \
		int type = lua_type(L, 1); \
		if (type == LUA_TTABLE) lua_remove(L, 1); \
		else if (type == LUA_TUSERDATA) { \
			dest = *toImage(L, 1); \
			lua_remove(L, 1); \
		} else return luaL_error(L, "Method must be called with a colon!"); \
	}


static int Image_blit (lua_State *L) {
	int argc = lua_gettop(L);
	
	if (argc != 4 && argc != 5 && argc != 8 && argc != 9) return luaL_error(L, "Argument error: image:blit() takes 3, 4, 7 or 8 arguments, and MUST be called with a colon.");
	
	BOOL alpha = (argc==5 || argc==9)?lua_toboolean(L, -1):true; 
	if(argc==5 || argc==9) lua_pop(L, 1);
	
	SETDEST
		
	int dx = luaL_checkint(L, 1);
	int dy = luaL_checkint(L, 2);
	Image* source = *toImage(L, 3);
	
	BOOL rect = (argc ==8 || argc == 9) ;
	int sx = rect? luaL_checkint(L, 4) : 0;
	int sy = rect? luaL_checkint(L, 5) : 0;
	int width = rect? luaL_checkint(L, 6) : source->imageWidth;
	int height = rect? luaL_checkint(L, 7) : source->imageHeight;
	
	if (!dest) {
		if (!adjustBlitRectangle(width, height, SCREEN_WIDTH, SCREEN_HEIGHT, &sx, &sy, &width, &height, &dx, &dy)) return 0;
		alpha?
			blitAlphaImageToScreen(sx, sy, width, height, source, dx, dy) :
			blitImageToScreen(sx, sy, width, height, source, dx, dy);
	} else {
		if (!adjustBlitRectangle(width, height, dest->imageWidth, dest->imageHeight, &sx, &sy, &width, &height, &dx, &dy)) return 0;
		alpha?
			blitAlphaImageToImage(sx, sy, width, height, source, dx, dy, dest) :
			blitImageToImage(sx, sy, width, height, source, dx, dy, dest);
	}
	return 0;
}
static int Image_clear (lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 1 && argc != 2) return luaL_error(L, "Argument error: Image:clear([color]) zero or one argument.");
	unsigned color = (argc==2)?*toColor(L, 2):0;

	SETDEST
	if(dest)
		clearImage(color, dest);
	else
		clearScreen(color);
	return 0;
}
static int Image_fillRect (lua_State *L) {
	int argc = lua_gettop(L);
	if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments");
	SETDEST

	int x0 = luaL_checkint(L, 1);
	int y0 = luaL_checkint(L, 2);
	int width = luaL_checkint(L, 3);
	int height = luaL_checkint(L, 4);
	int color = (argc==6)?*toColor(L, 5):0;
	
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
	if (!dest) {
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
		if (x0 >= dest->imageWidth || y0 >= dest->imageHeight) return 0;
		if (x0 + width >= dest->imageWidth) {
			width = dest->imageWidth - x0;
			if (width <= 0) return 0;
		}
		if (y0 + height >= dest->imageHeight) {
			height = dest->imageHeight - y0;
			if (height <= 0) return 0;
		}
		fillImageRect(color, x0, y0, width, height, dest);
	}
	return 0;
	
}
static int Image_drawLine (lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments"); 
	SETDEST
	int x0 = luaL_checkint(L, 1); 
	int y0 = luaL_checkint(L, 2); 
	int x1 = luaL_checkint(L, 3); 
	int y1 = luaL_checkint(L, 4); 
	int color = (argc==6) ? *toColor(L, 5) : 0;
	
	// TODO: better clipping
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (!dest) {
		if (x0 >= SCREEN_WIDTH) x0 = SCREEN_WIDTH - 1;
		if (x1 >= SCREEN_WIDTH) x1 = SCREEN_WIDTH - 1;
		if (y0 >= SCREEN_HEIGHT) y0 = SCREEN_HEIGHT - 1;
		if (y1 >= SCREEN_HEIGHT) y1 = SCREEN_HEIGHT - 1;
		drawLineScreen(x0, y0, x1, y1, color);
	} else {

		if (x0 >= dest->imageWidth) x0 = dest->imageWidth - 1;
		if (x1 >= dest->imageWidth) x1 = dest->imageWidth - 1;
		if (y0 >= dest->imageHeight) y0 = dest->imageHeight - 1;
		if (y1 >= dest->imageHeight) y1 = dest->imageHeight - 1;
		drawLineImage(x0, y0, x1, y1, color, dest);
	}
	return 0;
}
static int Image_pixel (lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 3 && argc != 4) return luaL_error(L, "Image:pixel(x, y, [color]) takes two or three arguments, and must be called with a colon.");
	SETDEST
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	int color = (argc == 4)?*toColor(L, 3):0;
	if(dest) {
		if (x >= 0 && y >= 0 && x < dest->imageWidth && y < dest->imageHeight) {
			if(argc==3) {
				*pushColor(L) = getPixelImage(x, y, dest);
				return 1;
			} else {
				putPixelImage(color, x, y, dest);
				return 0;
			}
		}
	} else {
		if (x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
			if(argc==3) {
				*pushColor(L) = getPixelScreen(x, y);
				return 1;
			} else {
				putPixelScreen(color, x, y);
				return 0;
			}
		}
	}

	return luaL_error(L, "An argument was incorrect.");
}
static int Image_print (lua_State *L) {
	int argc = lua_gettop(L);
	if (argc != 4 && argc != 5) return luaL_error(L, "wrong number of arguments");
	SETDEST
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	const char* text = luaL_checkstring(L, 3);
	int color = (argc == 5)?*toColor(L, 4):0x8000;
	if (!dest) {
		printTextScreen(x, y, text, color);
	} else {
		printTextImage(x, y, text, color, dest);
	}
	return 0;
}
static int Image_width (lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 1) return luaL_error(L, "Argument error: Image:width() must be called with a colon, and takes no arguments.");
	SETDEST
	if(dest) lua_pushnumber(L, dest->imageWidth);
	else lua_pushnumber(L, SCREEN_WIDTH);
	return 1;
}
static int Image_height (lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 1) return luaL_error(L, "Argument error: Image:width() must be called with a colon, and takes no arguments.");
	SETDEST
	if(dest) lua_pushnumber(L, dest->imageHeight);
	else lua_pushnumber(L, SCREEN_HEIGHT);
	return 1;
}
static int Image_save (lua_State *L) {
	if (lua_gettop(L) != 2) return luaL_error(L, "wrong number of arguments");
	const char *filename = luaL_checkstring(L, 2);
	SETDEST
	if (dest) {
		saveImage(filename, dest->data, dest->imageWidth, dest->imageHeight, dest->textureWidth);
	} else {
		saveImage(filename, getVramDisplayBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT, PSP_LINE_SIZE);
	}
	return 0;
}

static int Image_free(lua_State *L) {
	freeImage(*toImage(L, 1));
	return 0;
}

static int Image_tostring (lua_State *L) {
	Image_width(L);
	int w = luaL_checkint(L, 2); lua_pop(L, 1);
	Image_height(L);
	int h = luaL_checkint(L, 2); lua_pop(L, 1);

	char buff[32];
	sprintf(buff, "%p", *toImage(L, 1));
	lua_pushfstring(L, "Image (%s) [%d, %d]", buff, w, h);
	return 1;
}
static const luaL_reg Image_methods[] = {
	{"createEmpty", Image_createEmpty},
	{"load", Image_load},
	{"blit", Image_blit},
	{"clear", Image_clear},
	{"fillRect", Image_fillRect},
	{"drawLine", Image_drawLine},
	{"pixel", Image_pixel},
	{"print", Image_print},
	{"width", Image_width},
	{"height", Image_height},
	{"save", Image_save},
	{0,0}
};
static const luaL_reg Image_meta[] = {
	{"__gc", Image_free},
	{"__tostring", Image_tostring},
	{0,0}
};
UserdataRegister(Image, Image_methods, Image_meta)




static int Color_new (lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 3 && argc != 4) return luaL_error(L, "Argument error: Color.new(r, g, b, [a]) takes either three color arguments or three color arguments and an alpha value."); 
	
	unsigned *color = pushColor(L);
	
	unsigned r = CLAMP(luaL_checkint(L, 1), 0, 255); 
	unsigned g = CLAMP(luaL_checkint(L, 2), 0, 255); 
	unsigned b = CLAMP(luaL_checkint(L, 3), 0, 255);
	unsigned a;
	if (argc == 4) {
		a = CLAMP(luaL_checkint(L, 3), 0, 255);
	} else {
		a = 255;
	}

	*color = ((b>>3)<<10) | ((g>>3)<<5) | (r>>3) | (a == 255 ? 0x8000 : 0);
	
	return 1;
}
static int Color_colors (lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 1) return luaL_error(L, "Argument error: color:colors() takes no arguments, and it must be called from an instance with a colon.");
	unsigned color = *toColor(L, 1);
	unsigned r = (color & 0x1f) << 3; 
	unsigned g = ((color >> 5) & 0x1f) << 3 ;
	unsigned b = ((color >> 10) & 0x1f) << 3 ;
	unsigned a = color & 0x8000 ? 0xff : 0; 
	
	lua_newtable(L);
	lua_pushstring(L, "r"); lua_pushnumber(L, r); lua_settable(L, -3);
	lua_pushstring(L, "g"); lua_pushnumber(L, g); lua_settable(L, -3);
	lua_pushstring(L, "b"); lua_pushnumber(L, b); lua_settable(L, -3);
	lua_pushstring(L, "a"); lua_pushnumber(L, a); lua_settable(L, -3);

	return 1;
}

static int Color_tostring (lua_State *L) {
	Color_colors(L);
	lua_pushstring(L, "r"); lua_gettable(L, -2); int r = luaL_checkint(L, -1); lua_pop(L, 1);
	lua_pushstring(L, "g"); lua_gettable(L, -2); int g = luaL_checkint(L, -1); lua_pop(L, 1);
	lua_pushstring(L, "b"); lua_gettable(L, -2); int b = luaL_checkint(L, -1); lua_pop(L, 1);
	lua_pushstring(L, "a"); lua_gettable(L, -2); int a = luaL_checkint(L, -1); lua_pop(L, 1);
	lua_pop(L, 1); // pop the table
	lua_pushfstring(L, "Color (r %d, g %d, b %d, a %d)", r, g, b, a);
	return 1;
}

static int Color_equal(lua_State *L) {
	unsigned a = *toColor(L, 1);
	unsigned b = *toColor(L, 2);
	lua_pushboolean(L, a == b);
	return 1;
}
static const luaL_reg Color_methods[] = {
	{"new", Color_new},
	{"colors", Color_colors},
	{0,0}
};
static const luaL_reg Color_meta[] = {
	{"__tostring", Color_tostring},
	{"__eq", Color_equal},
	{0,0}
};
UserdataRegister(Color, Color_methods, Color_meta)


static const luaL_reg Screen_functions[] = {
	{"flip", lua_flipScreen},
	{"waitVblankStart", lua_waitVblankStart},
	{0,0}
};





void luaGraphics_init(lua_State *L) {
	Image_register(L);
	Color_register(L);
	
	luaL_openlib(L, "screen", Screen_functions, 0);
	luaL_openlib(L, "screen", Image_methods, 0); // Basically just an ugly hack. What I'd really want to say is metatable(screen).__index = Image, but my lua powress is failing me there.
}

