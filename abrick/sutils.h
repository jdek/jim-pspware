#ifndef SUTILS_H
#define SUTILS_H
#include <string>
#include <map>
#include "SDL.h"
//-----------------------------------------------------------------------------
Uint32 getpixel(SDL_Surface *surface, int x, int y);
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
bool LoadImage(SDL_Surface **surface, const char *filename);
bool RGB2OneColor(SDL_Surface *surface, Uint8 r, Uint8 g, Uint8 b);
void PatternFill(SDL_Surface *pattern, SDL_Surface *surface);
void NjamSetRect(SDL_Rect& rect, int x, int y, int w=0, int h=0);
SDLKey NjamGetch(bool Wait);
int NjamRandom(int MaxValue);
void Box(SDL_Surface *surface, SDL_Rect& rect, int border, Uint8 r, Uint8 g, Uint8 b);

typedef enum { fxBlackWhite, fxDarken, fxDarkenAlot } tEffect;
bool SurfaceEffect(SDL_Surface *surface, SDL_Rect& r, tEffect Effect = fxBlackWhite);
//-----------------------------------------------------------------------------
//! exception class
class Exiter
{
public:
	std::string message;
	Exiter(std::string m);
};
//-----------------------------------------------------------------------------
#endif

