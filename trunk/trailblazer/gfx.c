/***************************************************************************
                          gfx.c  -  graphics and I/O
                             -------------------
    begin                : Wed Jun 5 2002
    copyright            : (C) 2002 by Paul Robson
    email                : autismuk@aol.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "trail.h"

SDL_Surface *Display;                       // The display surface
Uint32 LastSync;                            // Last Sync time
SDL_Surface *BallSprite;                    // Ball Sprite space
int BallSize;                               // Diameter of ball
int KeyPress[256];                          // Keys pressed on keyboard

static void _GFXCreateBall(SDL_Surface *s,int Size);

//
//
//                              Initialise the Graphics etc.
//
//
int  GFXInitialise(void)
{
    int i;
    char Temp[32];
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_JOYSTICK)<0) return(-1);
    atexit(SDL_Quit);                       // Tidy up on exit
    Display = SDL_SetVideoMode(480,272,     // Create a display screen
                            16,SDL_SWSURFACE|SDL_ANYFORMAT);
    if (Display == NULL) return (-2);
    sprintf(Temp,"TrailBlazer v%s",VERSION);
    SDL_WM_SetCaption(Temp,NULL);
    BallSize = Display->h / 7;              // Height of Ball
    BallSprite = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCCOLORKEY,
                                BallSize*16,BallSize,Display->format->BitsPerPixel,0,0,0,0);
    if (BallSprite == NULL) return -2;
    _GFXCreateBall(BallSprite,BallSize);
    for (i = 0;i < 256;i++) KeyPress[i]=0;  // No keys pressed
    return 0;
}

//
//
//                                 Tidy up after finishing
//
//
void GFXTerminate(void)
{
    SDL_FreeSurface(BallSprite);
}

//
//
//      Array mapping characters to line bitmasks.
//
//
static int _CharMap[] =
    {   ' ',0x00,   '0',0x3F,   '1',0x0C,   '2',0x76,   '3',0x5E,   '4',0x4D,   '5',0x5B,
        '6',0x7B,   '7',0x0E,   '8',0x7F,   '9',0x5F,
        'A',0x6F,   'B',0x1DE,  'C',0x33,   'D',0x19E,  'E',0x73,   'F',0x63,   'G',0x3B,
        'H',0x6D,   'I',0x192,  'J',0x3E,   'K',0xE9,   'L',0x31,   'M',0x1AF,  'N',0x2F,
        'O',0x3F,   'P',0x67,   'Q',0x4F,   'R',0x167,  'S',0x5B,   'T',0x182,  'U',0x3D,
        'V',0x3D,   'W',0x1BD,  'X',0x6D,   'Y',0x145,  'Z',0x76,
        '"',0x05,   '\'',0x04,  '+',0x1C0,  '-',0x40,   '?',0x66,

        0,0 };

//
//
//              The straight-line-font function. A simple scalable font
//
//
void GFXLChar(LFONT *f,int x,int y,int ch)
{
    SDL_Rect r;
    int i = 0,b = -1;
    Uint32 Col;
    Col = SDL_MapRGB(Display->format,f->r,f->g,f->b);
    ch = toupper(ch);                       // We don't do lower case
    r.x = x;r.y = y;r.w = f->w;r.h = f->h;  // Erase the background
    SDL_FillRect(Display,&r,SDL_MapRGB(Display->format,0,0,0));
    while (_CharMap[i] != ch)               // Look up the character
    {
        if (_CharMap[i] == 0) return;
        i = i+2;
    }
    b = _CharMap[i+1];
    r.x = x;r.y = y;r.w = f->lw;r.h = f->h/2;
    if (b & 0x01) SDL_FillRect(Display,&r,Col);
    r.h = f->lw;r.w = f->w;
    if (b & 0x02) SDL_FillRect(Display,&r,Col);
    r.x = x+f->w-f->lw;r.w = f->lw;r.h = f->h/2;
    if (b & 0x04) SDL_FillRect(Display,&r,Col);
    r.y = y+f->h-r.h;
    if (b & 0x08) SDL_FillRect(Display,&r,Col);
    r.x = x;
    if (b & 0x20) SDL_FillRect(Display,&r,Col);
    r.y = y+f->h-f->lw;r.w = f->w;r.h = f->lw;
    if (b & 0x10) SDL_FillRect(Display,&r,Col);
    r.y = y+f->h/2-f->lw/2;
    if (b & 0x40) SDL_FillRect(Display,&r,Col);
    r.x = x+f->w/2-f->lw/2;r.y = y;r.w = f->lw;r.h = f->h/2;
    if (b & 0x80) SDL_FillRect(Display,&r,Col);
    r.y = y+f->h-r.h;
    if (b & 0x100) SDL_FillRect(Display,&r,Col);
}

//
//
//  A function to draw horizontal lines on the display
//
//

void GFXVLine(int x,int y,int w,Uint32 Colour)
{
    int bpp = Display->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)Display->pixels + y * Display->pitch + x * bpp;

    if (x < 0 || y < 0 || w <= 0 ||
                x+w >= Display->w || y >= Display->h) return;

    switch(bpp)
    {
    case 1:
        while (w-- > 0) *p++ = Colour;
        break;

    case 2:
        while (w-- > 0)
        {
            *(Uint16 *)p = Colour;
            p += 2;
        }
        break;

    case 3:
        while (w-- > 0)
        {
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p[0] = (Colour >> 16) & 0xff;
                p[1] = (Colour >> 8) & 0xff;
                p[2] = Colour & 0xff;
            }
            else
            {
                p[0] = Colour & 0xff;
                p[1] = (Colour >> 8) & 0xff;
                p[2] = (Colour >> 16) & 0xff;
            }
            p = p + 3;
        }
        break;

    case 4:
        while (w-- > 0)
        {
            *(Uint32 *)p = Colour;
            p = p + 4;
        }
        break;
    }
}

//
//
//                                      Colour mapping
//
//
Uint32 GFXColour(int r,int g,int b)
{
    return SDL_MapRGB(Display->format,r,g,b);
}

//
//
//                                  Update the whole display
//
//
void GFXUpdate(void)
{
    SDL_UpdateRect(Display,0,0,Display->w,Display->h);
}

//
//
//                                  Get the screen rectangle
//
//
void GFXScreenRect(SDL_Rect *rc)
{
    rc->x = 0;rc->y = 0;
    rc->w = Display->w;rc->h = Display->h;
}

//
//
//                      Convert Map Tile Number to the appropriate colour
//
//
Uint32 GFXMapToColour(int Map)
{
    Uint32 c;
    switch(Map)
    {
        case SQ_HOLE:       c = SDL_MapRGB(Display->format,0,0,0);break;        // Black
        case SQ_NORMAL1:    c = SDL_MapRGB(Display->format,0,128,255);break;    // Blue
        case SQ_ACCEL:      c = SDL_MapRGB(Display->format,0,255,0);break;      // Green
        case SQ_REVERSE:    c = SDL_MapRGB(Display->format,255,0,255);break;    // Purple
        case SQ_UNREVERSE:  c = SDL_MapRGB(Display->format,255,128,0);break;    // Orange
        case SQ_NORMAL2:    c = SDL_MapRGB(Display->format,0,0,255);break;      // Darker Blue
        case SQ_DEACCEL:    c = SDL_MapRGB(Display->format,255,0,0);break;      // Red
        case SQ_JUMP:       c = SDL_MapRGB(Display->format,255,255,255);break;  // White
        case SQ_END:        c = SDL_MapRGB(Display->format,80,80,80);break;     // End (two grays)
        case SQ_END+1:      c = SDL_MapRGB(Display->format,160,160,160);break;
        default:c = SDL_MapRGB(Display->format,128,128,128);break;
    }
    return c;
}

//
//
//          Sync to the internal timer ; update the screen if there is time to do so.
//
//

void GFXSyncAndUpdate(Uint32 Speed,int x,int y,int Sprite)
{
    SDL_Rect rc,rc2,rc3;
    if (SDL_GetTicks() < LastSync+Speed)    // Is there time for an update ???
    {
        x = x - BallSize/2;y = y-BallSize;  // Adjust for sprite top left corner
        rc.x = x;rc.y = y;                  // Calculate screen rectangle
        rc.w = rc.h = BallSize;
        rc2.x = Sprite*BallSize;rc2.y = 0;  // Calculate sprite rectangle
        rc2.w = rc2.h = BallSize;
        rc3.x = 8*BallSize;rc3.y = 0;       // Calculate background save rectangle
        rc3.w = rc3.h = BallSize;
        SDL_BlitSurface(Display,&rc,BallSprite,&rc3);
        SDL_BlitSurface(BallSprite,&rc2,Display,&rc);
        GFXUpdate();
        SDL_BlitSurface(BallSprite,&rc3,Display,&rc);
    }
    while (SDL_GetTicks() < LastSync) {}    // Wait for time out
    LastSync = LastSync+Speed;              // Add speed.
    if (SDL_GetTicks() > LastSync)          // Has time been lost, if so, resynchronise
            LastSync = SDL_GetTicks()+Speed;
}

//
//
//                              Draw the balls into the buffer
//
//
static void _GFXCreateBall(SDL_Surface *s,int Size)
{
    int i,x,y,j,c;
    Uint32 Back = SDL_MapRGB(s->format,128,128,128);
    double a;
    SDL_Rect rc,rc2;
    SDL_SetColorKey(s,                      // Set the transparent key
                    SDL_SRCCOLORKEY,Back);
    for (i = 0;i < 8;i++)                   // 8 balls, rolling state changes
    {
        rc.x = Size * i;rc.y = 0;           // Ball rectangle
        rc.w = rc.h = Size;
        SDL_FillRect(s,&rc,                 // Fill with yellow
                SDL_MapRGB(s->format,255,255,0));
        for (y = 0;y < Size;y++)            // Now make the chequerboard pattern.
        {
            rc2.x = rc.x+((y*8/Size & 2) ?  // Work out position of horizontal first chequer
                                        0 : Size/4);
            rc2.y = y+(7-i)*Size/8/2;       // Work out vertical, adjust for rolling
            if (rc2.y >= Size) rc2.y -= Size;
            rc2.w = Size/4;rc2.h = 1;       // Size, width
            for (j = 0;j < 2;j++)           // Draw 3 of them
            {
                SDL_FillRect(s,&rc2,SDL_MapRGB(s->format,255,0,0));
                rc2.x += Size/2;
            }
        }
        for (y = 0;y < Size;y++)            // Now trim it to a circle shape
        {
            x = (Size/2-y);                 // Work out the outside size ; its filling the border
            if (x < 0) x = -x;              // in not the circle
            x = Size/2 - (int)sqrt(Size*Size/4-x*x)+1;
            rc2.x = rc.x;rc2.y = rc.y+y;    // Left side trim
            rc2.w = x;rc2.h = 1;
            SDL_FillRect(s,&rc2,Back);
            rc2.x = rc2.x + Size-rc2.w;     // Right side trim
            SDL_FillRect(s,&rc2,Back);
        }
        for (j = Size/2-1;j <= Size/2;j++)  // Black border around it. Some primitives in SDL would
            for (c = 0;c < 360;c++)         // be nice. Not to allegro, but basic line, circle stuff
            {                               // I know there are libs but I only want to link SDL
                a = (double)c /360.0 * 2 * 3.14;
                rc2.x = rc.x+Size/2+(int)(cos(a)*j);
                rc2.y = rc.y+Size/2+(int)(sin(a)*j);
                rc2.w = rc2.h = 1;
                SDL_FillRect(s,&rc2,SDL_MapRGB(s->format,0,0,0));
            }
    }
}

//
//
//                              Return the diameter of the ball
//
//
int  GFXBallSize(void)
{
    return BallSize;
}

//
//
//                      Poll keyboard for keys, and Close button
//
//
void GFXPollKeyboard(void)
{
    SDL_Event ev;
    int n;
    while (SDL_PollEvent(&ev))
    {
        if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP)
        {
            n = ev.key.keysym.sym;
            KeyPress[n] = (ev.type == SDL_KEYDOWN);
        }
        if (ev.type == SDL_QUIT)
            KeyPress[SDLK_ESCAPE] = 1;
    }
}

//
//
//                              Key checking function
//
//
int GFXIsKeyPressed(int Key)
{
    return KeyPress[Key];
}

//
//                          Clear an area of the screen
//
void GFXClearRect(SDL_Rect *rc)
{
    SDL_FillRect(Display,rc,SDL_MapRGB(Display->format,0,0,0));
}
