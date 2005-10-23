#include <stdlib.h>
#include <malloc.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <string.h>
#include "luaplayer.h"

#include "graphics.h"
#ifndef BOOL
#define BOOL int
#endif

UserdataStubs(Color, Color)

struct FloatVertex
{
	float u, v;
	unsigned int color;
	float x,y,z;
};

/// screen.*
/// ====================
static int lua_waitVblankStart(lua_State *L)
{
	int argc = lua_gettop(L), t = 0;
	if (argc != 0 && argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments"); // can be called as both screen.wait...() and screen:wait...()
	if (argc) t = lua_type(L, 1);
	if (argc == 0 || t != LUA_TNUMBER) {
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
	if (w <= 0 || h <= 0 || w > 512 || h > 512) return luaL_error(L, "invalid size");
	luaC_collectgarbage(L);
	Image* image = createImage(w, h);
	if (!image) return luaL_error(L, "can't create image");
	Image** luaImage = pushImage(L);
	*luaImage = image;
	return 1;
}
static int Image_load (lua_State *L) {
	if (lua_gettop(L) != 1) return luaL_error(L, "Argument error: Image.load(filename) takes one argument.");
	luaC_collectgarbage(L);
	Image* image = loadImage(luaL_checkstring(L, 1));
	if(!image) return luaL_error(L, "Image.load: Error loading image.");
	Image** luaImage = pushImage(L);
	*luaImage = image;
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
	Color color = (argc==2)?*toColor(L, 2):0;

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
	Color color = (argc==6)?*toColor(L, 5):0;
	
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
	Color color = (argc==6) ? *toColor(L, 5) : 0;
	
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
	Color color = (argc == 4)?*toColor(L, 3):0;
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
	Color color = (argc == 5)?*toColor(L, 4):0xFF000000;
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
		saveImage(filename, dest->data, dest->imageWidth, dest->imageHeight, dest->textureWidth, 1);
	} else {
		saveImage(filename, getVramDisplayBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT, PSP_LINE_SIZE, 0);
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
	
	Color *color = pushColor(L);
	
	unsigned r = CLAMP(luaL_checkint(L, 1), 0, 255); 
	unsigned g = CLAMP(luaL_checkint(L, 2), 0, 255); 
	unsigned b = CLAMP(luaL_checkint(L, 3), 0, 255);
	unsigned a;
	if (argc == 4) {
		a = CLAMP(luaL_checkint(L, 4), 0, 255);
	} else {
		a = 255;
	}

	//*color = ((b>>3)<<10) | ((g>>3)<<5) | (r>>3) | (a == 255 ? 0x8000 : 0);
	*color = a << 24 | b << 16 | g << 8 | r;
	
	return 1;
}
static int Color_colors (lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 1) return luaL_error(L, "Argument error: color:colors() takes no arguments, and it must be called from an instance with a colon.");
	Color color = *toColor(L, 1);
	int r = R(color); 
	int g = G(color);
	int b = B(color);
	int a = A(color);
	
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
	Color a = *toColor(L, 1);
	Color b = *toColor(L, 2);
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



#define CONSTANT(name)\
   lua_pushstring(L, #name);\
   lua_pushnumber(L, name);\
   lua_settable(L, LUA_GLOBALSINDEX);

static int lua_sceGuClearColor(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuClearColor(*toColor(L, 1));
	return 0;
}

static int lua_sceGuClearDepth(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuClearDepth(luaL_checkint(L, 1));
	return 0;
}

static int lua_sceGuClear(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuClear(luaL_checkint(L, 1));
	return 0;
}

static int lua_sceGumMatrixMode(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGumMatrixMode(luaL_checkint(L, 1));
	return 0;
}

static int lua_sceGumLoadIdentity(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 0) return luaL_error(L, "wrong number of arguments"); 
	sceGumLoadIdentity();
	return 0;
}
static int lua_sceGumPerspective(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 4) return luaL_error(L, "wrong number of arguments"); 
	sceGumPerspective(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4));
	return 0;
}
	
static int lua_sceGumTranslate(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 3) return luaL_error(L, "wrong number of arguments"); 
	ScePspFVector3 v;
	v.x = luaL_checknumber(L, 1);
	v.y = luaL_checknumber(L, 2);
	v.z = luaL_checknumber(L, 3);
	sceGumTranslate(&v);
	return 0;
}

static int lua_sceGumRotateXYZ(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 3) return luaL_error(L, "wrong number of arguments"); 
	ScePspFVector3 v;
	v.x = luaL_checknumber(L, 1);
	v.y = luaL_checknumber(L, 2);
	v.z = luaL_checknumber(L, 3);
	sceGumRotateXYZ(&v);
	return 0;
}

static int lua_sceGuTexImage(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	Image* image = *toImage(L, 1);
	sceGuTexImage(0, image->textureWidth, image->textureHeight, image->textureWidth, image->data);

	return 0;
}

static int lua_sceGuTexFunc(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 2) return luaL_error(L, "wrong number of arguments"); 
	sceGuTexFunc(luaL_checkint(L, 1), luaL_checkint(L, 2));
	return 0;
}

static int lua_sceGuTexEnvColor(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuTexEnvColor(*toColor(L, 1));
	return 0;
}

static int lua_sceGuTexFilter(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 2) return luaL_error(L, "wrong number of arguments"); 
	sceGuTexFilter(luaL_checkint(L, 1), luaL_checkint(L, 2));
	return 0;
}

static int lua_sceGuTexScale(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 2) return luaL_error(L, "wrong number of arguments"); 
	sceGuTexScale(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	return 0;
}

static int lua_sceGuTexOffset(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 2) return luaL_error(L, "wrong number of arguments"); 
	sceGuTexOffset(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	return 0;
}

static int lua_sceGuAmbientColor(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 1) return luaL_error(L, "wrong number of arguments"); 
	sceGuAmbientColor(*toColor(L, 1));
	return 0;
}

static int lua_sceGumDrawArray(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc != 3) return luaL_error(L, "wrong number of arguments");

	int prim = luaL_checkint(L, 1);
	int vtype = luaL_checkint(L, 2);
	if (lua_type(L, 3) != LUA_TTABLE) return luaL_error(L, "vertices table missing");
	int n = luaL_getn(L, 3);

	// hack for testing
	// array must be available until guFinish. Perhaps a user object would be better, with cached vertices data
	static struct FloatVertex* vertices = NULL;
	if (vertices == NULL) {
		vertices = memalign(16, n * sizeof(struct FloatVertex));
		int i;
		for (i = 1; i <= n; ++i) {
			struct FloatVertex* vertice = &vertices[i - 1];
			
			// get vertice table
			lua_rawgeti(L, 3, i);
			lua_pushnil(L);  /* first key */
			while (lua_next(L, -2) != 0) {
				// 'key' is at index -2 and 'value' at index -1
				const char* key = luaL_checkstring(L, -2);
				if (strcmp(key, "u") == 0) {
					vertice->u = luaL_checknumber(L, -1);
				} else if (strcmp(key, "v") == 0) {
					vertice->v = luaL_checknumber(L, -1);
				} else if (strcmp(key, "color") == 0) {
					vertice->color = *toColor(L, -1);
				} else if (strcmp(key, "x") == 0) {
					vertice->x = luaL_checknumber(L, -1);
				} else if (strcmp(key, "y") == 0) {
					vertice->y = luaL_checknumber(L, -1);
				} else if (strcmp(key, "z") == 0) {
					vertice->z = luaL_checknumber(L, -1);
				}
				lua_pop(L, 1);  // removes 'value'; keeps 'key' for next iteration
			}
	
			// remove vertice table
			lua_pop(L, 1);
		}
	}
	
	sceGumDrawArray(prim, vtype, n, NULL, vertices);
	//free(vertices);
	return 0;
}

static int lua_sceGuFinish(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 0) return luaL_error(L, "wrong number of arguments"); 
	sceGuFinish();
	return 0;
}

static int lua_sceGuSync(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 2) return luaL_error(L, "wrong number of arguments"); 
	sceGuSync(luaL_checkint(L, 1), luaL_checkint(L, 2));
	return 0;
}

static int lua_sceGuStart(lua_State *L) {
	int argc = lua_gettop(L); 
	if (argc != 0) return luaL_error(L, "wrong number of arguments"); 
	guStart();
	return 0;
}

void luaGraphics_init(lua_State *L) {
	Image_register(L);
	Color_register(L);
	
	luaL_openlib(L, "screen", Screen_functions, 0);
	luaL_openlib(L, "screen", Image_methods, 0); // Basically just an ugly hack. What I'd really want to say is metatable(screen).__index = Image, but my lua powress is failing me there.

	// sceGu* and sceGum* stuff

	CONSTANT(GU_COLOR_BUFFER_BIT)
	CONSTANT(GU_DEPTH_BUFFER_BIT)
	CONSTANT(GU_PROJECTION)
	CONSTANT(GU_VIEW)
	CONSTANT(GU_MODEL)
	CONSTANT(GU_PI)
	CONSTANT(GU_TFX_ADD)
	CONSTANT(GU_TCC_RGB)
	CONSTANT(GU_LINEAR)
	CONSTANT(GU_LINEAR)
	CONSTANT(GU_TRIANGLES)
	CONSTANT(GU_TEXTURE_32BITF)
	CONSTANT(GU_COLOR_8888)
	CONSTANT(GU_VERTEX_32BITF)
	CONSTANT(GU_TRANSFORM_3D)
	
	lua_register(L, "sceGuClearColor", lua_sceGuClearColor);
	lua_register(L, "sceGuClearDepth", lua_sceGuClearDepth);
	lua_register(L, "sceGuClear", lua_sceGuClear);
	lua_register(L, "sceGumMatrixMode", lua_sceGumMatrixMode);
	lua_register(L, "sceGumLoadIdentity", lua_sceGumLoadIdentity);
	lua_register(L, "sceGumPerspective", lua_sceGumPerspective);
	lua_register(L, "sceGumTranslate", lua_sceGumTranslate);
	lua_register(L, "sceGumRotateXYZ", lua_sceGumRotateXYZ);
	lua_register(L, "sceGuTexImage", lua_sceGuTexImage);
	lua_register(L, "sceGuTexFunc", lua_sceGuTexFunc);
	lua_register(L, "sceGuTexEnvColor", lua_sceGuTexEnvColor);
	lua_register(L, "sceGuTexFilter", lua_sceGuTexFilter);
	lua_register(L, "sceGuTexScale", lua_sceGuTexScale);
	lua_register(L, "sceGuTexOffset", lua_sceGuTexOffset);
	lua_register(L, "sceGuAmbientColor", lua_sceGuAmbientColor);
	lua_register(L, "sceGumDrawArray", lua_sceGumDrawArray);
	lua_register(L, "sceGuFinish", lua_sceGuFinish);
	lua_register(L, "sceGuSync", lua_sceGuSync);
	lua_register(L, "sceGuStart", lua_sceGuStart);
}

