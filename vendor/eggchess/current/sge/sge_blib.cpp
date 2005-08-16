/*
*	SDL Graphics Extension
*	Triangles of every sort
*
*	Started 000428
*
*	License: LGPL v2+ (see the file LICENSE)
*	(c)2000-2001 Anders Lindström & Johan E. Thélin
*/

/*********************************************************************
 *  This library is free software; you can redistribute it and/or    *
 *  modify it under the terms of the GNU Library General Public      *
 *  License as published by the Free Software Foundation; either     *
 *  version 2 of the License, or (at your option) any later version. *
 *********************************************************************/

/*
*  This is written by Johan E. Thélin and me.
*/

#include <SDL/SDL.h>
#include "sge_surface.h"
#include "sge_primitives.h"
#include "sge_blib.h"

#define SWAP(x,y,temp) temp=x;x=y;y=temp

/* Globals used for sge_Update/sge_Lock (defined in sge_surface) */
extern Uint8 _sge_update;
extern Uint8 _sge_lock;

/* We need some internal functions */
extern void _Line(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
extern void _HLine(SDL_Surface *Surface, Sint16 x1, Sint16 x2, Sint16 y, Uint32 Color);
extern void _HLineAlpha(SDL_Surface *Surface, Sint16 x1, Sint16 x2, Sint16 y, Uint32 Color, Uint8 alpha);

/* Macro to inline RGB mapping */
#define MapRGB(format, r, g, b)\
	(r >> format->Rloss) << format->Rshift\
		| (g >> format->Gloss) << format->Gshift\
		| (b >> format->Bloss) << format->Bshift


//==================================================================================
// Draws a horisontal line, fading the colors
//==================================================================================
void _FadedLine(SDL_Surface *dest,Sint16 x1,Sint16 x2,Sint16 y,Uint8 r1,Uint8 g1,Uint8 b1,Uint8 r2,Uint8 g2,Uint8 b2)
{
	Sint16 x;
	Uint8 t;
	
	/* Fix coords */
	if ( x1 > x2 ) {
		SWAP(x1,x2,x);
		SWAP(r1,r2,t);
		SWAP(g1,g2,t);
		SWAP(b1,b2,t);
	}	
	
	/* We use fixedpoint math */
	Sint32 R = r1<<16;
	Sint32 G = g1<<16;
	Sint32 B = b1<<16;
	
	/* Color step value */
	Sint32 rstep = Sint32((r2-r1)<<16) / Sint32(x2-x1+1);
	Sint32 gstep = Sint32((g2-g1)<<16) / Sint32(x2-x1+1);
	Sint32 bstep = Sint32((b2-b1)<<16) / Sint32(x2-x1+1);
	
	
	/* Clipping */
	if(x2<sge_clip_xmin(dest) || x1>sge_clip_xmax(dest) || y<sge_clip_ymin(dest) || y>sge_clip_ymax(dest))
		return;
	if (x1 < sge_clip_xmin(dest)){
		/* Update start colors */
		R += (sge_clip_xmin(dest)-x1)*rstep;
		G += (sge_clip_xmin(dest)-x1)*gstep;
		B += (sge_clip_xmin(dest)-x1)*bstep;
  		x1 = sge_clip_xmin(dest);
	}
	if (x2 > sge_clip_xmax(dest))
  		x2 = sge_clip_xmax(dest);

	
	switch (dest->format->BytesPerPixel) {
		case 1: { /* Assuming 8-bpp */
			Uint8 *pixel;
			Uint8 *row = (Uint8 *)dest->pixels + y*dest->pitch;
			
			for (x = x1; x <= x2; x++){
				pixel = row + x;
				
				*pixel = SDL_MapRGB( dest->format, R>>16, G>>16, B>>16 );
		
				R += rstep;
				G += gstep;
				B += bstep;
			}
		}
		break;

		case 2: { /* Probably 15-bpp or 16-bpp */
			Uint16 *pixel;
			Uint16 *row = (Uint16 *)dest->pixels + y*dest->pitch/2;
			
			for (x = x1; x <= x2; x++){
				pixel = row + x;
				
				*pixel = MapRGB( dest->format, R>>16, G>>16, B>>16 );
		
				R += rstep;
				G += gstep;
				B += bstep;
			}
		}
		break;

		case 3: { /* Slow 24-bpp mode, usually not used */
			Uint8 *pixel;
			Uint8 *row = (Uint8 *)dest->pixels + y*dest->pitch;
			
			Uint8 rshift8=dest->format->Rshift/8;
			Uint8 gshift8=dest->format->Gshift/8;
			Uint8 bshift8=dest->format->Bshift/8;
			
			for (x = x1; x <= x2; x++){
				pixel = row + x*3;
		
				*(pixel+rshift8) = R>>16;
  				*(pixel+gshift8) = G>>16;
  				*(pixel+bshift8) = B>>16;
		
				R += rstep;
				G += gstep;
				B += bstep;
			}
		}
		break;

		case 4: { /* Probably 32-bpp */
			Uint32 *pixel;
			Uint32 *row = (Uint32 *)dest->pixels + y*dest->pitch/4;
			
			for (x = x1; x <= x2; x++){
				pixel = row + x;
				
				*pixel = MapRGB( dest->format, R>>16, G>>16, B>>16 );
		
				R += rstep;
				G += gstep;
				B += bstep;
			}
		}
		break;
	}
}

void sge_FadedLine(SDL_Surface *dest,Sint16 x1,Sint16 x2,Sint16 y,Uint8 r1,Uint8 g1,Uint8 b1,Uint8 r2,Uint8 g2,Uint8 b2)
{
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		if ( SDL_LockSurface(dest) < 0 )
			return;

	_FadedLine(dest,x1,x2,y,r1,g1,b1,r2,g2,b2);
	
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		SDL_UnlockSurface(dest);
		
	if(_sge_update!=1){return;}
	if ( x1 > x2 )
		sge_UpdateRect(dest, x1, y, x1-x2+1, 1);
	else
		sge_UpdateRect(dest, x1, y, x2-x1+1, 1);	
}


//==================================================================================
// Draws a horisontal, textured line
//==================================================================================
void _TexturedLine(SDL_Surface *dest,Sint16 x1,Sint16 x2,Sint16 y,SDL_Surface *source,Sint16 sx1,Sint16 sy1,Sint16 sx2,Sint16 sy2)
{
	Sint16 x;
	
	/* Fix coords */
	if ( x1 > x2 ) {
		SWAP(x1,x2,x);
		SWAP(sx1,sx2,x);
		SWAP(sy1,sy2,x);
	}	
	
	/* Fixed point texture starting coords */
	Sint32 srcx = sx1<<16;
	Sint32 srcy = sy1<<16;
	
	/* Texture coords stepping value */
	Sint32 xstep = Sint32((sx2-sx1)<<16) / Sint32(x2-x1+1);
	Sint32 ystep = Sint32((sy2-sy1)<<16) / Sint32(x2-x1+1);
	
	
	/* Clipping */
	if(x2<sge_clip_xmin(dest) || x1>sge_clip_xmax(dest) || y<sge_clip_ymin(dest) || y>sge_clip_ymax(dest))
		return;
	if (x1 < sge_clip_xmin(dest)){
		/* Fix texture starting coord */
		srcx += (sge_clip_xmin(dest)-x1)*xstep;
		srcy += (sge_clip_xmin(dest)-x1)*ystep;
  		x1 = sge_clip_xmin(dest);
	}
	if (x2 > sge_clip_xmax(dest))
  		x2 = sge_clip_xmax(dest);

	
	if(dest->format->BytesPerPixel == source->format->BytesPerPixel){
		/* Fast mode. Just copy the pixel */
	
		switch (dest->format->BytesPerPixel) {
			case 1: { /* Assuming 8-bpp */
				Uint8 *pixel;
				Uint8 *row = (Uint8 *)dest->pixels + y*dest->pitch;
			
				for (x = x1; x <= x2; x++){
					pixel = row + x;
				
					*pixel = *((Uint8 *)source->pixels + (srcy>>16)*source->pitch + (srcx>>16));
		
					srcx += xstep;
					srcy += ystep;
				}
			}
			break;

			case 2: { /* Probably 15-bpp or 16-bpp */
				Uint16 *pixel;
				Uint16 *row = (Uint16 *)dest->pixels + y*dest->pitch/2;
			
				Uint16 pitch = source->pitch/2;
			
				for (x = x1; x <= x2; x++){
					pixel = row + x;
				
					*pixel = *((Uint16 *)source->pixels + (srcy>>16)*pitch + (srcx>>16));
		
					srcx += xstep;
					srcy += ystep;
				}
			}
			break;

			case 3: { /* Slow 24-bpp mode, usually not used */
				Uint8 *pixel, *srcpixel;
				Uint8 *row = (Uint8 *)dest->pixels + y*dest->pitch;
			
				Uint8 rshift8=dest->format->Rshift/8;
				Uint8 gshift8=dest->format->Gshift/8;
				Uint8 bshift8=dest->format->Bshift/8;
			
				for (x = x1; x <= x2; x++){
					pixel = row + x*3;
					srcpixel = (Uint8 *)source->pixels + (srcy>>16)*source->pitch + (srcx>>16)*3;
		
					*(pixel+rshift8) = *(srcpixel+rshift8);
  					*(pixel+gshift8) = *(srcpixel+gshift8);
  					*(pixel+bshift8) = *(srcpixel+bshift8);
		
					srcx += xstep;
					srcy += ystep;
				}	
			}
			break;

			case 4: { /* Probably 32-bpp */
				Uint32 *pixel;
				Uint32 *row = (Uint32 *)dest->pixels + y*dest->pitch/4;
			
				Uint16 pitch = source->pitch/4;
			
				for (x = x1; x <= x2; x++){
					pixel = row + x;
				
					*pixel = *((Uint32 *)source->pixels + (srcy>>16)*pitch + (srcx>>16));
		
					srcx += xstep;
					srcy += ystep;
				}
			}
			break;
		}
	}else{
		/* Slow mode. We must translate every pixel color! */
	
		Uint8 r=0,g=0,b=0;
	
		switch (dest->format->BytesPerPixel) {
			case 1: { /* Assuming 8-bpp */
				Uint8 *pixel;
				Uint8 *row = (Uint8 *)dest->pixels + y*dest->pitch;
			
				for (x = x1; x <= x2; x++){
					pixel = row + x;
				
					SDL_GetRGB(sge_GetPixel(source, srcx>>16, srcy>>16), source->format, &r, &g, &b);
					*pixel = SDL_MapRGB( dest->format, r, g, b );
		
					srcx += xstep;
					srcy += ystep;
				}
			}
			break;

			case 2: { /* Probably 15-bpp or 16-bpp */
				Uint16 *pixel;
				Uint16 *row = (Uint16 *)dest->pixels + y*dest->pitch/2;
			
				for (x = x1; x <= x2; x++){
					pixel = row + x;
					
					SDL_GetRGB(sge_GetPixel(source, srcx>>16, srcy>>16), source->format, &r, &g, &b);
					*pixel = MapRGB( dest->format, r, g, b );
		
					srcx += xstep;
					srcy += ystep;
				}
			}
			break;

			case 3: { /* Slow 24-bpp mode, usually not used */
				Uint8 *pixel, *srcpixel;
				Uint8 *row = (Uint8 *)dest->pixels + y*dest->pitch;
			
				Uint8 rshift8=dest->format->Rshift/8;
				Uint8 gshift8=dest->format->Gshift/8;
				Uint8 bshift8=dest->format->Bshift/8;
			
				for (x = x1; x <= x2; x++){
					pixel = row + x*3;
					srcpixel = (Uint8 *)source->pixels + (srcy>>16)*source->pitch + (srcx>>16)*3;
		
					SDL_GetRGB(sge_GetPixel(source, srcx>>16, srcy>>16), source->format, &r, &g, &b);
					
					*(pixel+rshift8) = r;
  					*(pixel+gshift8) = g;
  					*(pixel+bshift8) = b;
		
					srcx += xstep;
					srcy += ystep;
				}	
			}
			break;

			case 4: { /* Probably 32-bpp */
				Uint32 *pixel;
				Uint32 *row = (Uint32 *)dest->pixels + y*dest->pitch/4;
			
				for (x = x1; x <= x2; x++){
					pixel = row + x;
				
					SDL_GetRGB(sge_GetPixel(source, srcx>>16, srcy>>16), source->format, &r, &g, &b);
					*pixel = MapRGB( dest->format, r, g, b );
		
					srcx += xstep;
					srcy += ystep;
				}
			}
			break;
		}
	}
}

void sge_TexturedLine(SDL_Surface *dest,Sint16 x1,Sint16 x2,Sint16 y,SDL_Surface *source,Sint16 sx1,Sint16 sy1,Sint16 sx2,Sint16 sy2)
{
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		if ( SDL_LockSurface(dest) < 0 )
			return;
	if ( SDL_MUSTLOCK(source) && _sge_lock )
		if ( SDL_LockSurface(source) < 0 )
			return;

	_TexturedLine(dest,x1,x2,y,source,sx1,sy1,sx2,sy2);
	
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		SDL_UnlockSurface(dest);
	if ( SDL_MUSTLOCK(source) && _sge_lock )
		SDL_UnlockSurface(source);
		
	if(_sge_update!=1){return;}
	if ( x1 > x2 )
		sge_UpdateRect(dest, x1, y, x1-x2+1, 1);
	else
		sge_UpdateRect(dest, x1, y, x2-x1+1, 1);	
}


//==================================================================================
// Draws a trigon
//==================================================================================
void sge_Trigon(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint32 color)
{
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		if ( SDL_LockSurface(dest) < 0 )
			return;

	_Line(dest,x1,y1,x2,y2,color);
	_Line(dest,x1,y1,x3,y3,color);
	_Line(dest,x3,y3,x2,y2,color);
	
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		SDL_UnlockSurface(dest);
		
	if(_sge_update!=1){return;}
	
	Sint16 xmax=x1, ymax=y1, xmin=x1, ymin=y1;
	xmax= (xmax>x2)? xmax : x2;  ymax= (ymax>y2)? ymax : y2;
	xmin= (xmin<x2)? xmin : x2;  ymin= (ymin<y2)? ymin : y2;
	xmax= (xmax>x3)? xmax : x3;  ymax= (ymax>y3)? ymax : y3;
	xmin= (xmin<x3)? xmin : x3;  ymin= (ymin<y3)? ymin : y3;
	
	sge_UpdateRect(dest, xmin, ymin, xmax-xmin+1, ymax-ymin+1);
}

void sge_Trigon(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint8 R, Uint8 G, Uint8 B)
{
	sge_Trigon(dest,x1,y1,x2,y2,x3,y3, SDL_MapRGB(dest->format, R,G,B));
}


//==================================================================================
// Draws a trigon (alpha)
//==================================================================================
void sge_TrigonAlpha(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint32 color, Uint8 alpha)
{
	Uint8 update = _sge_update;
	Uint8 lock = _sge_lock;
	_sge_update = 0;
	_sge_lock = 0;
	
	if (SDL_MUSTLOCK(dest) && lock)
		if (SDL_LockSurface(dest) < 0)
			return;
	
	sge_LineAlpha(dest,x1,y1,x2,y2,color,alpha);
	sge_LineAlpha(dest,x1,y1,x3,y3,color,alpha);
	sge_LineAlpha(dest,x3,y3,x2,y2,color,alpha);
	
	
	if (SDL_MUSTLOCK(dest) && lock) {
		SDL_UnlockSurface(dest);
	}
	
	_sge_update = update;
	_sge_lock = lock;
	
	if(_sge_update!=1){return;}
	
	Sint16 xmax=x1, ymax=y1, xmin=x1, ymin=y1;
	xmax= (xmax>x2)? xmax : x2;  ymax= (ymax>y2)? ymax : y2;
	xmin= (xmin<x2)? xmin : x2;  ymin= (ymin<y2)? ymin : y2;
	xmax= (xmax>x3)? xmax : x3;  ymax= (ymax>y3)? ymax : y3;
	xmin= (xmin<x3)? xmin : x3;  ymin= (ymin<y3)? ymin : y3;
	
	sge_UpdateRect(dest, xmin, ymin, xmax-xmin+1, ymax-ymin+1);
}

void sge_TrigonAlpha(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint8 R, Uint8 G, Uint8 B, Uint8 alpha)
{
	sge_TrigonAlpha(dest,x1,y1,x2,y2,x3,y3, SDL_MapRGB(dest->format, R,G,B), alpha);
}


//==================================================================================
// Draws an AA trigon (alpha)
//==================================================================================
void sge_AATrigonAlpha(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint32 color, Uint8 alpha)
{
	Uint8 update = _sge_update;
	Uint8 lock = _sge_lock;
	_sge_update = 0;
	_sge_lock = 0;
	
	if (SDL_MUSTLOCK(dest) && lock)
		if (SDL_LockSurface(dest) < 0)
			return;
	
	sge_AALineAlpha(dest,x1,y1,x2,y2,color,alpha);
	sge_AALineAlpha(dest,x1,y1,x3,y3,color,alpha);
	sge_AALineAlpha(dest,x3,y3,x2,y2,color,alpha);
	
	
	if (SDL_MUSTLOCK(dest) && lock) {
		SDL_UnlockSurface(dest);
	}
	
	_sge_update = update;
	_sge_lock = lock;
	
	if(_sge_update!=1){return;}
	
	Sint16 xmax=x1, ymax=y1, xmin=x1, ymin=y1;
	xmax= (xmax>x2)? xmax : x2;  ymax= (ymax>y2)? ymax : y2;
	xmin= (xmin<x2)? xmin : x2;  ymin= (ymin<y2)? ymin : y2;
	xmax= (xmax>x3)? xmax : x3;  ymax= (ymax>y3)? ymax : y3;
	xmin= (xmin<x3)? xmin : x3;  ymin= (ymin<y3)? ymin : y3;
	
	sge_UpdateRect(dest, xmin, ymin, xmax-xmin+1, ymax-ymin+1);
}

void sge_AATrigonAlpha(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint8 R, Uint8 G, Uint8 B, Uint8 alpha)
{
	sge_AATrigonAlpha(dest,x1,y1,x2,y2,x3,y3, SDL_MapRGB(dest->format, R,G,B), alpha);
}

void sge_AATrigon(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint32 color)
{
	sge_AATrigonAlpha(dest,x1,y1,x2,y2,x3,y3, color, 255);
}

void sge_AATrigon(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint8 R, Uint8 G, Uint8 B)
{
	sge_AATrigonAlpha(dest,x1,y1,x2,y2,x3,y3, SDL_MapRGB(dest->format, R,G,B), 255);
}


//==================================================================================
// Draws a filled trigon
//==================================================================================
void sge_FilledTrigon(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint32 color)
{
	Sint16 y;

	if( y1==y3 )
		return;

	/* Sort coords */
	if ( y1 > y2 ) {
		SWAP(y1,y2,y);
		SWAP(x1,x2,y);
	}
	if ( y2 > y3 ) {
		SWAP(y2,y3,y);
		SWAP(x2,x3,y);
	}
	if ( y1 > y2 ) {
		SWAP(y1,y2,y);
		SWAP(x1,x2,y);
	}
	
	/*
	 * How do we calculate the starting and ending x coordinate of the horizontal line
	 * on each y coordinate?  We can do this by using a standard line algorithm but
	 * instead of plotting pixels, use the x coordinates as start and stop
	 * coordinates for the horizontal line.
	 * So we will simply trace the outlining of the triangle; this will require 3 lines.
	 * Line 1 is the line between (x1,y1) and (x2,y2)
	 * Line 2 is the line between (x1,y1) and (x3,y3)
	 * Line 3 is the line between (x2,y2) and (x3,y3)
	 *
	 * We can divide the triangle into 2 halfs. The upper half will be outlined by line
	 * 1 and 2. The lower half will be outlined by line line 2 and 3.
	*/
	
	
	/* Starting coords for the three lines */
	Sint32 xa = Sint32(x1<<16);
	Sint32 xb = xa;
	Sint32 xc = Sint32(x2<<16);

	/* Lines step values */
	Sint32 m1 = 0;
	Sint32 m2 = Sint32((x3 - x1)<<16)/Sint32(y3 - y1);
	Sint32 m3 = 0;
	
	/* Upper half of the triangle */
	if( y1==y2 )
		_HLine(dest, x1, x2, y1, color);
	else{
		m1 = Sint32((x2 - x1)<<16)/Sint32(y2 - y1);
		
		for ( y = y1; y <= y2; y++) {
			_HLine(dest, xa>>16, xb>>16, y, color);
				
			xa += m1;
			xb += m2;
		}
	}
	
	/* Lower half of the triangle */
	if( y2==y3 )
		_HLine(dest, x2, x3, y2, color);
	else{
		m3 = Sint32((x3 - x2)<<16)/Sint32(y3 - y2);
		
		for ( y = y2+1; y <= y3; y++) {
			_HLine(dest, xb>>16, xc>>16, y, color);

			xb += m2;
			xc += m3;
		}
	}
	
	
	if(_sge_update!=1){return;}
	
	Sint16 xmax=x1, xmin=x1;
	xmax= (xmax>x2)? xmax : x2;
	xmin= (xmin<x2)? xmin : x2;
	xmax= (xmax>x3)? xmax : x3;
	xmin= (xmin<x3)? xmin : x3;
	
	sge_UpdateRect(dest, xmin, y1, xmax-xmin+1, y3-y1+1);
}

void sge_FilledTrigon(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint8 R, Uint8 G, Uint8 B)
{
	sge_FilledTrigon(dest,x1,y1,x2,y2,x3,y3, SDL_MapRGB(dest->format, R,G,B));
}


//==================================================================================
// Draws a filled trigon (alpha)
//==================================================================================
void sge_FilledTrigonAlpha(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint32 color, Uint8 alpha)
{
	Sint16 y;

	if( y1==y3 )
		return;

	/* Sort coords */
	if ( y1 > y2 ) {
		SWAP(y1,y2,y);
		SWAP(x1,x2,y);
	}
	if ( y2 > y3 ) {
		SWAP(y2,y3,y);
		SWAP(x2,x3,y);
	}
	if ( y1 > y2 ) {
		SWAP(y1,y2,y);
		SWAP(x1,x2,y);
	}

	Sint32 xa = Sint32(x1<<16);
	Sint32 xb = xa;
	Sint32 xc = Sint32(x2<<16);

	Sint32 m1 = 0;
	Sint32 m2 = Sint32((x3 - x1)<<16)/Sint32(y3 - y1);
	Sint32 m3 = 0;
	
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		if ( SDL_LockSurface(dest) < 0 )
			return;
	
	/* Upper half of the triangle */
	if( y1==y2 )
		_HLineAlpha(dest, x1, x2, y1, color, alpha);
	else{
		m1 = Sint32((x2 - x1)<<16)/Sint32(y2 - y1);
		
		for ( y = y1; y <= y2; y++) {
			_HLineAlpha(dest, xa>>16, xb>>16, y, color, alpha);
				
			xa += m1;
			xb += m2;
		}
	}
	
	/* Lower half of the triangle */
	if( y2==y3 )
		_HLineAlpha(dest, x2, x3, y2, color, alpha);
	else{
		m3 = Sint32((x3 - x2)<<16)/Sint32(y3 - y2);
		
		for ( y = y2+1; y <= y3; y++) {
			_HLineAlpha(dest, xb>>16, xc>>16, y, color, alpha);

			xb += m2;
			xc += m3;
		}
	}
	
	
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		SDL_UnlockSurface(dest);
	
	
	if(_sge_update!=1){return;}
	
	Sint16 xmax=x1, xmin=x1;
	xmax= (xmax>x2)? xmax : x2;
	xmin= (xmin<x2)? xmin : x2;
	xmax= (xmax>x3)? xmax : x3;
	xmin= (xmin<x3)? xmin : x3;
	
	sge_UpdateRect(dest, xmin, y1, xmax-xmin+1, y3-y1+1);
}

void sge_FilledTrigonAlpha(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint8 R, Uint8 G, Uint8 B, Uint8 alpha)
{
	sge_FilledTrigonAlpha(dest,x1,y1,x2,y2,x3,y3, SDL_MapRGB(dest->format, R,G,B),alpha);
}


//==================================================================================
// Draws a gourand shaded trigon
//==================================================================================
void sge_FadedTrigon(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint32 c1,Uint32 c2,Uint32 c3)
{
	Sint16 y;

	if( y1==y3 )
		return;
		
	Uint8 c=0;
	SDL_Color col1;
	SDL_Color col2;
	SDL_Color col3;
	
	col1 = sge_GetRGB(dest,c1);
	col2 = sge_GetRGB(dest,c2);
	col3 = sge_GetRGB(dest,c3);

	/* Sort coords */
	if ( y1 > y2 ) {
		SWAP(y1,y2,y);
		SWAP(x1,x2,y);
		SWAP(col1.r,col2.r,c);
		SWAP(col1.g,col2.g,c);
		SWAP(col1.b,col2.b,c);
	}
	if ( y2 > y3 ) {
		SWAP(y2,y3,y);
		SWAP(x2,x3,y);
		SWAP(col2.r,col3.r,c);
		SWAP(col2.g,col3.g,c);
		SWAP(col2.b,col3.b,c);
	}
	if ( y1 > y2 ) {
		SWAP(y1,y2,y);
		SWAP(x1,x2,y);
		SWAP(col1.r,col2.r,c);
		SWAP(col1.g,col2.g,c);
		SWAP(col1.b,col2.b,c);
	}

	/*
	 * We trace three lines exactly like in sge_FilledTrigon(), but here we
	 * must also keep track of the colors. We simply calculate how the color
	 * will change along the three lines.
	*/

	/* Starting coords for the three lines */
	Sint32 xa = Sint32(x1<<16);
	Sint32 xb = xa;
	Sint32 xc = Sint32(x2<<16);
	
	/* Starting colors (rgb) for the three lines */
	Sint32 r1 = Sint32(col1.r<<16);
	Sint32 r2 = r1;
	Sint32 r3 = Sint32(col2.r<<16);
	
	Sint32 g1 = Sint32(col1.g<<16);
	Sint32 g2 = g1;
	Sint32 g3 = Sint32(col2.g<<16);
	
	Sint32 b1 = Sint32(col1.b<<16);
	Sint32 b2 = b1;
	Sint32 b3 = Sint32(col2.b<<16);
	
	/* Lines step values */
	Sint32 m1 = 0;
	Sint32 m2 = Sint32((x3 - x1)<<16)/Sint32(y3 - y1);
	Sint32 m3 = 0;
	
	/* Colors step values */
	Sint32 rstep1 = 0;
	Sint32 rstep2 = Sint32((col3.r - col1.r) << 16) / Sint32(y3 - y1);
	Sint32 rstep3 = 0;
	
	Sint32 gstep1 = 0;
	Sint32 gstep2 = Sint32((col3.g - col1.g) << 16) / Sint32(y3 - y1);
	Sint32 gstep3 = 0;
	
	Sint32 bstep1 = 0;
	Sint32 bstep2 = Sint32((col3.b - col1.b) << 16) / Sint32(y3 - y1);
	Sint32 bstep3 = 0;
	
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		if ( SDL_LockSurface(dest) < 0 )
			return;
	
	/* Upper half of the triangle */
	if( y1==y2 )
		_FadedLine(dest, x1, x2, y1, col1.r, col1.g, col1.b, col2.r, col2.g, col2.b);
	else{
		m1 = Sint32((x2 - x1)<<16)/Sint32(y2 - y1);
		
		rstep1 = Sint32((col2.r - col1.r) << 16) / Sint32(y2 - y1);
		gstep1 = Sint32((col2.g - col1.g) << 16) / Sint32(y2 - y1);
		bstep1 = Sint32((col2.b - col1.b) << 16) / Sint32(y2 - y1);
		
		for ( y = y1; y <= y2; y++) {
			_FadedLine(dest, xa>>16, xb>>16, y, r1>>16, g1>>16, b1>>16, r2>>16, g2>>16, b2>>16);
				
			xa += m1;
			xb += m2;
			
			r1 += rstep1;
			g1 += gstep1;
			b1 += bstep1;
			
			r2 += rstep2;
			g2 += gstep2;
			b2 += bstep2;
		}
	}
	
	/* Lower half of the triangle */
	if( y2==y3 )
		_FadedLine(dest, x2, x3, y2, col2.r, col2.g, col2.b, col3.r, col3.g, col3.b);
	else{
		m3 = Sint32((x3 - x2)<<16)/Sint32(y3 - y2);
		
		rstep3 = Sint32((col3.r - col2.r) << 16) / Sint32(y3 - y2);
		gstep3 = Sint32((col3.g - col2.g) << 16) / Sint32(y3 - y2);
		bstep3 = Sint32((col3.b - col2.b) << 16) / Sint32(y3 - y2);
		
		for ( y = y2+1; y <= y3; y++) {
			_FadedLine(dest, xb>>16, xc>>16, y, r2>>16, g2>>16, b2>>16, r3>>16, g3>>16, b3>>16);

			xb += m2;
			xc += m3;
			
			r2 += rstep2;
			g2 += gstep2;
			b2 += bstep2;
			
			r3 += rstep3;
			g3 += gstep3;
			b3 += bstep3;
		}
	}
	
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		SDL_UnlockSurface(dest);
	
	if(_sge_update!=1){return;}
	
	Sint16 xmax=x1, xmin=x1;
	xmax= (xmax>x2)? xmax : x2;
	xmin= (xmin<x2)? xmin : x2;
	xmax= (xmax>x3)? xmax : x3;
	xmin= (xmin<x3)? xmin : x3;
	
	sge_UpdateRect(dest, xmin, y1, xmax-xmin+1, y3-y1+1);
}


//==================================================================================
// Draws a texured trigon (fast)
//==================================================================================
void sge_TexturedTrigon(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,SDL_Surface *source,Sint16 sx1,Sint16 sy1,Sint16 sx2,Sint16 sy2,Sint16 sx3,Sint16 sy3)
{
	Sint16 y;

	if( y1==y3 )
		return;

	/* Sort coords */
	if ( y1 > y2 ) {
		SWAP(y1,y2,y);
		SWAP(x1,x2,y);
		SWAP(sx1,sx2,y);
		SWAP(sy1,sy2,y);
	}
	if ( y2 > y3 ) {
		SWAP(y2,y3,y);
		SWAP(x2,x3,y);
		SWAP(sx2,sx3,y);
		SWAP(sy2,sy3,y);
	}
	if ( y1 > y2 ) {
		SWAP(y1,y2,y);
		SWAP(x1,x2,y);
		SWAP(sx1,sx2,y);
		SWAP(sy1,sy2,y);
	}

	/*
	 * Again we do the same thing as in sge_FilledTrigon(). But here we must keep track of how the 
	 * texture coords change along the lines.
	*/

	/* Starting coords for the three lines */
	Sint32 xa = Sint32(x1<<16);
	Sint32 xb = xa;
	Sint32 xc = Sint32(x2<<16);

	/* Lines step values */
	Sint32 m1 = 0;
	Sint32 m2 = Sint32((x3 - x1)<<16)/Sint32(y3 - y1);
	Sint32 m3 = 0;

	/* Starting texture coords for the three lines */	
	Sint32 srcx1 = Sint32(sx1<<16);
	Sint32 srcx2 = srcx1;
	Sint32 srcx3 = Sint32(sx2<<16);
	
	Sint32 srcy1 = Sint32(sy1<<16);
	Sint32 srcy2 = srcy1;
	Sint32 srcy3 = Sint32(sy2<<16);
	
	/* Texture coords stepping value */
	Sint32 xstep1 = 0;
	Sint32 xstep2 = Sint32((sx3 - sx1) << 16) / Sint32(y3 - y1);
	Sint32 xstep3 = 0;
	
	Sint32 ystep1 = 0;
	Sint32 ystep2 = Sint32((sy3 - sy1) << 16) / Sint32(y3 - y1);
	Sint32 ystep3 = 0;
	
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		if ( SDL_LockSurface(dest) < 0 )
			return;
	if ( SDL_MUSTLOCK(source) && _sge_lock )
		if ( SDL_LockSurface(source) < 0 )
			return;
	
	/* Upper half of the triangle */
	if( y1==y2 )
		_TexturedLine(dest,x1,x2,y1,source,sx1,sy1,sx2,sy2);
	else{
		m1 = Sint32((x2 - x1)<<16)/Sint32(y2 - y1);
		
		xstep1 = Sint32((sx2 - sx1) << 16) / Sint32(y2 - y1);
		ystep1 = Sint32((sy2 - sy1) << 16) / Sint32(y2 - y1);
		
		for ( y = y1; y <= y2; y++) {
			_TexturedLine(dest, xa>>16, xb>>16, y, source, srcx1>>16, srcy1>>16, srcx2>>16, srcy2>>16);
				
			xa += m1;
			xb += m2;
			
			srcx1 += xstep1;
			srcx2 += xstep2;
			srcy1 += ystep1;
			srcy2 += ystep2;
		}
	}
	
	/* Lower half of the triangle */
	if( y2==y3 )
		_TexturedLine(dest,x2,x3,y2,source,sx2,sy2,sx3,sy3);
	else{
		m3 = Sint32((x3 - x2)<<16)/Sint32(y3 - y2);
		
		xstep3 = Sint32((sx3 - sx2) << 16) / Sint32(y3 - y2);
		ystep3 = Sint32((sy3 - sy2) << 16) / Sint32(y3 - y2);
		
		for ( y = y2+1; y <= y3; y++) {
			_TexturedLine(dest, xb>>16, xc>>16, y, source, srcx2>>16, srcy2>>16, srcx3>>16, srcy3>>16);

			xb += m2;
			xc += m3;
			
			srcx2 += xstep2;
			srcx3 += xstep3;
			srcy2 += ystep2;
			srcy3 += ystep3;
		}
	}
	
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		SDL_UnlockSurface(dest);
	if ( SDL_MUSTLOCK(source) && _sge_lock )
		SDL_UnlockSurface(source);
	
	if(_sge_update!=1){return;}
	
	Sint16 xmax=x1, xmin=x1;
	xmax= (xmax>x2)? xmax : x2;
	xmin= (xmin<x2)? xmin : x2;
	xmax= (xmax>x3)? xmax : x3;
	xmin= (xmin<x3)? xmin : x3;
	
	sge_UpdateRect(dest, xmin, y1, xmax-xmin+1, y3-y1+1);
}


//==================================================================================
// Draws a texured *RECTANGLE*
//==================================================================================
void sge_TexturedRect(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Sint16 x4,Sint16 y4,SDL_Surface *source,Sint16 sx1,Sint16 sy1,Sint16 sx2,Sint16 sy2,Sint16 sx3,Sint16 sy3,Sint16 sx4,Sint16 sy4)
{
	Sint16 y;
	
	if( y1==y3 || y1 == y4 || y4 == y2 )
		return;
	
	/* Sort the coords */
	if ( y1 > y2 ) {
		SWAP(x1,x2,y);
		SWAP(y1,y2,y);
		SWAP(sx1,sx2,y);
		SWAP(sy1,sy2,y);
	}
	if ( y2 > y3 ) {
		SWAP(x3,x2,y);
		SWAP(y3,y2,y);
		SWAP(sx3,sx2,y);
		SWAP(sy3,sy2,y);
	}
	if ( y1 > y2 ) {
		SWAP(x1,x2,y);
		SWAP(y1,y2,y);
		SWAP(sx1,sx2,y);
		SWAP(sy1,sy2,y);
	}
	if ( y3 > y4 ) {
		SWAP(x3,x4,y);
		SWAP(y3,y4,y);
		SWAP(sx3,sx4,y);
		SWAP(sy3,sy4,y);
	}
	if ( y2 > y3 ) {
		SWAP(x3,x2,y);
		SWAP(y3,y2,y);
		SWAP(sx3,sx2,y);
		SWAP(sy3,sy2,y);
	}
	if ( y1 > y2 ) {
		SWAP(x1,x2,y);
		SWAP(y1,y2,y);
		SWAP(sx1,sx2,y);
		SWAP(sy1,sy2,y);
	}

	/*
	 * We do this exactly like sge_TexturedTrigon(), but here we must trace four lines.
	*/

	Sint32 xa = Sint32(x1<<16);
	Sint32 xb = xa;
	Sint32 xc = Sint32(x2<<16);
	Sint32 xd = Sint32(x3<<16);

	Sint32 m1 = 0;
	Sint32 m2 = Sint32((x3 - x1)<<16)/Sint32(y3 - y1);
	Sint32 m3 = Sint32((x4 - x2)<<16)/Sint32(y4 - y2);
	Sint32 m4 = 0;
	
	Sint32 srcx1 = Sint32(sx1<<16);
	Sint32 srcx2 = srcx1;
	Sint32 srcx3 = Sint32(sx2<<16);
	Sint32 srcx4 = Sint32(sx3<<16);
	
	Sint32 srcy1 = Sint32(sy1<<16);
	Sint32 srcy2 = srcy1;
	Sint32 srcy3 = Sint32(sy2<<16);
	Sint32 srcy4 = Sint32(sy3<<16);
	
	Sint32 xstep1 = 0;
	Sint32 xstep2 = Sint32((sx3 - sx1) << 16) / Sint32(y3 - y1);
	Sint32 xstep3 = Sint32((sx4 - sx2) << 16) / Sint32(y4 - y2);
	Sint32 xstep4 = 0;
	
	Sint32 ystep1 = 0;
	Sint32 ystep2 = Sint32((sy3 - sy1) << 16) / Sint32(y3 - y1);
	Sint32 ystep3 = Sint32((sy4 - sy2) << 16) / Sint32(y4 - y2);
	Sint32 ystep4 = 0;
	
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		if ( SDL_LockSurface(dest) < 0 )
			return;
	
	/* Upper bit of the rectangle */
	if( y1==y2 )
		_TexturedLine(dest,x1,x2,y1,source,sx1,sy1,sx2,sy2);
	else{
		m1 = Sint32((x2 - x1)<<16)/Sint32(y2 - y1);
		
		xstep1 = Sint32((sx2 - sx1) << 16) / Sint32(y2 - y1);
		ystep1 = Sint32((sy2 - sy1) << 16) / Sint32(y2 - y1);
		
		for ( y = y1; y <= y2; y++) {
			_TexturedLine(dest, xa>>16, xb>>16, y, source, srcx1>>16, srcy1>>16, srcx2>>16, srcy2>>16);
				
			xa += m1;
			xb += m2;
			
			srcx1 += xstep1;
			srcx2 += xstep2;
			srcy1 += ystep1;
			srcy2 += ystep2;
		}
	}
	
	/* Middle bit of the rectangle */	
	for ( y = y2+1; y <= y3; y++) {
		_TexturedLine(dest, xb>>16, xc>>16, y, source, srcx2>>16, srcy2>>16, srcx3>>16, srcy3>>16);

		xb += m2;
		xc += m3;
			
		srcx2 += xstep2;
		srcx3 += xstep3;
		srcy2 += ystep2;
		srcy3 += ystep3;
	}
	
	/* Lower bit of the rectangle */
	if( y3==y4 )
		_TexturedLine(dest,x3,x4,y3,source,sx3,sy3,sx4,sy4);
	else{
		m4 = Sint32((x4 - x3)<<16)/Sint32(y4 - y3);
		
		xstep4 = Sint32((sx4 - sx3) << 16) / Sint32(y4 - y3);
		ystep4 = Sint32((sy4 - sy3) << 16) / Sint32(y4 - y3);
		
		for ( y = y3+1; y <= y4; y++) {
			_TexturedLine(dest, xc>>16, xd>>16, y, source, srcx3>>16, srcy3>>16, srcx4>>16, srcy4>>16);

			xc += m3;
			xd += m4;
			
			srcx3 += xstep3;
			srcx4 += xstep4;
			srcy3 += ystep3;
			srcy4 += ystep4;
		}
			
	}
	
	
	if ( SDL_MUSTLOCK(dest) && _sge_lock )
		SDL_UnlockSurface(dest);
	
	if(_sge_update!=1){return;}
	
	Sint16 xmax=x1, xmin=x1;
	xmax= (xmax>x2)? xmax : x2;
	xmin= (xmin<x2)? xmin : x2;
	xmax= (xmax>x3)? xmax : x3;
	xmin= (xmin<x3)? xmin : x3;
	xmax= (xmax>x4)? xmax : x4;
	xmin= (xmin<x4)? xmin : x4;
	
	sge_UpdateRect(dest, xmin, y1, xmax-xmin+1, y4-y1+1);
}

