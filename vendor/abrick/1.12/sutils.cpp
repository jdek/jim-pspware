#include <stdlib.h>
#include "SDL.h"
#include "SDL_image.h"
#include "sutils.h"

//#define UTILS_USE_SDL_IMAGE
//-----------------------------------------------------------------------------
//! exception class
Exiter::Exiter(std::string m)
{
	message = m;
};
//-----------------------------------------------------------------------------
// Return the pixel value at (x, y)
// NOTE: The surface must be locked before calling this!
Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp)
	{
		case 1:	        return *p;
		case 2:	        return *(Uint16 *)p;
		case 3:
			#pragma warn -ccc
			#pragma warn -rch
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
			#pragma warn +ccc
			#pragma warn +rch

		case 4:	        return *(Uint32 *)p;
		default:        return 0;       /* shouldn't happen, but avoids warnings */
    }
}
//-----------------------------------------------------------------------------
// Set the pixel at (x, y) to the given value
// NOTE: The surface must be locked before calling this!
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp)
	{
		case 1:        *p = pixel;        			break;
		case 2:        *(Uint16 *)p = pixel;        break;
		case 3:
			#pragma warn -ccc
			#pragma warn -rch
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				p[0] = (pixel >> 16) & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = pixel & 0xff;
			} else {
				p[0] = pixel & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = (pixel >> 16) & 0xff;
			}
			break;
			#pragma warn +ccc
			#pragma warn +rch

		case 4:			*(Uint32 *)p = pixel;		break;
    }
}
//-----------------------------------------------------------------------------
bool LoadImage(SDL_Surface **surface, const char *filename)
{
 	SDL_Surface *temp;
 	printf("Loading image: %s ...", filename);
	#ifdef UTILS_USE_SDL_IMAGE
 	temp = IMG_Load(filename);
	#else
	temp = SDL_LoadBMP(filename);
	#endif
 	if (!temp)
 	{
 		printf("FAILED.\nError: %s\n", SDL_GetError());
 		return false;
 	}
 	printf("OK.\n");

	// Convert image to video format
	*surface = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);
	if ( *surface == NULL )
	{
		printf("Couldn't convert image: %s\n",	SDL_GetError());
		return false;
	}

	return true;
}
//-----------------------------------------------------------------------------
bool RGB2OneColor(SDL_Surface *surface, Uint8 r, Uint8 g, Uint8 b)
{
    int x, y;
    Uint32 color = SDL_MapRGB(surface->format, r, g, b);

	if ( SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) < 0 )
	{
		fprintf(stderr, "sutils.cpp: Can't lock surface: %s\n", SDL_GetError());
		return false;
	}

	Uint8 r1, g1, b1;
	for (int x = 0; x < surface->w; x++)
	{
		for (int y = 0; y < surface->h; y++)
		{
			SDL_GetRGB(getpixel(surface, x, y), surface->format, &r1, &g1, &b1);
			if (r1+g1+b1 > 50)
				putpixel(surface, x, y, color);
		}
	}

    if ( SDL_MUSTLOCK(surface) )
        SDL_UnlockSurface(surface);

	return true;
}
//-----------------------------------------------------------------------------
void PatternFill(SDL_Surface *pattern, SDL_Surface *surface)
{
	int w1, w2, h1, h2;

	w2 = surface->w;
	h2 = surface->h;
	w1 = pattern->w;
	h1 = pattern->h;

	SDL_Rect to;
	for (int x=0; x < w2/w1 + 1; x++)
	{
		for (int y=0; y < h2/h1 + 1; y++)
		{
			NjamSetRect(to, x * w1, y * h1);
			SDL_BlitSurface(pattern, NULL, surface, &to);
		}
	}
}
//-----------------------------------------------------------------------------
void NjamSetRect(SDL_Rect& rect, int x, int y, int w, int h)
{
 	rect.x = x;
 	rect.y = y;
 	rect.w = w;
 	rect.h = h;
}
//-----------------------------------------------------------------------------
SDLKey NjamGetch(bool Wait)
{
    do
    {
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN)
				return event.key.keysym.sym;
			if (event.type == SDL_QUIT)
				throw Exiter("Close button of window clicked");
		}
    }
    while (Wait);

    return SDLK_LAST;	// this is probably never used by SDL, so it denotes that key isn't pressed
}
//-----------------------------------------------------------------------------
int NjamRandom(int MaxValue)
{
 	return ((int)((double)MaxValue*rand()/(RAND_MAX+1.0)));	// according to rand man page
}
//-----------------------------------------------------------------------------
bool SurfaceEffect(SDL_Surface *surface, SDL_Rect& r, tEffect Effect)
{
    int x, y;
	if ( SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) < 0 )
	{
		fprintf(stderr, "njamutils.cpp: Can't lock surface: %s\n", SDL_GetError());
		return false;
	}

	Uint8 r1, g1, b1;
	for (int x = r.x; x < r.w+r.x; x++)
	{
		for (int y = r.y; y < r.h+r.y; y++)
		{
			SDL_GetRGB(getpixel(surface, x, y), surface->format, &r1, &g1, &b1);
			int val = (r1 + g1 + b1) / 3;
			Uint32 color;

			if (Effect == fxBlackWhite)
				color = SDL_MapRGB(surface->format, (unsigned char)(val/1.1), (unsigned char)val, (unsigned char)(val/1.1));
			else if (Effect == fxDarken)
				color = SDL_MapRGB(surface->format, r1/2, g1/2, b1/2);
			else if (Effect == fxDarkenAlot)
				color = SDL_MapRGB(surface->format, r1/5, g1/5, b1/5);
			putpixel(surface, x, y, color);
		}
	}

    if ( SDL_MUSTLOCK(surface) )
        SDL_UnlockSurface(surface);

	return true;
}
//-----------------------------------------------------------------------------
void Box(SDL_Surface *surface, SDL_Rect& rect, int border, Uint8 r, Uint8 g, Uint8 b)
{
	SDL_Rect rc;
	NjamSetRect(rc, rect.x, rect.y, rect.w, border);	// upper
	SDL_FillRect(surface, &rc, SDL_MapRGB(surface->format, r, g, b));

	NjamSetRect(rc, rect.x, rect.y+rect.h-border, rect.w, border);	// lower
	SDL_FillRect(surface, &rc, SDL_MapRGB(surface->format, r, g, b));

	NjamSetRect(rc, rect.x, rect.y, border, rect.h);	// left
	SDL_FillRect(surface, &rc, SDL_MapRGB(surface->format, r, g, b));

	NjamSetRect(rc, rect.x+rect.w-border, rect.y, border, rect.h);	// right
	SDL_FillRect(surface, &rc, SDL_MapRGB(surface->format, r, g, b));
}
//-----------------------------------------------------------------------------
