/***************************************************************************
                          main.c  -  TrailBlazer main program
                             -------------------
    begin                : Sat Jun  8 11:52:59 BST 2002
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

#include <pspkernel.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <string.h>
#include "trail.h"

static int _MAIKey(void);
static void _MAIText(int x,int y,int r,int g,int b,char *Txt);

int main(int argc, char *argv[])
{
    ROAD r;
    int i;
	SDL_Joystick *joystick = NULL;

	i = GFXInitialise();
    if (i) {
    	printf ("GFXInitialise ret: %d\n", i);
		exit(1);
    }
    i = MAPRead();                          // Attempt to load the map file
    if (i != 0)                             // It failed ?
    {
        printf("Map read failed : %d\n",i);
		exit(1);
    }

	r.Control = 0;
    if(SDL_NumJoysticks()) {
		joystick = SDL_JoystickOpen(0);
	} else {
		printf("No joystick detected\n");
	}

    while (i = MAIMainScreen(&r),i > 0)     // Display times, get selected
    {
        GFXScreenRect(&(r.rc));             // Get the rectangle for the whole screen
        r.PlayerID = 0;                     // Player ID
        i--;
        if (i >= 0 && i < MAPMAX)
            if (MAPGet(i)->Data != NULL)
                        GAMERun(&r,i);      // Run the game
    }
    GFXTerminate();
    MAPDispose();

	if(joystick)
		SDL_JoystickClose(joystick);

    return EXIT_SUCCESS;
}

//
//
//                      Display Selection Screen
//
//
int MAIMainScreen(ROAD *r)
{
    SDL_Rect rc;
    int i,x,y,Key = 1;
    char _Temp[32];
    MAP *m;
    GFXScreenRect(&rc);                     // Clear screen
    GFXClearRect(&rc);
    _MAIText(-1,1,255,255,0,"TrailBlazer");
    _MAIText(-1,2,0,255,0,"By Paul Robson");
    _MAIText(-1,15,255,0,0,"ported by rinco");
    for (i = 0;i < MAPMAX;i++)              // Display Best times
    {
        m = MAPGet(i);
        if (m->Data != NULL)
        {
            x = (i % 2) * 11 + 2;y = i/2+4;
            sprintf(_Temp,"%c -",i+65);
            _MAIText(x,y,0,255,255,_Temp);
            sprintf(_Temp,"%02d-%02d",m->Time/100%100,m->Time%100);
            if (m->Time == 9999) strcpy(_Temp,"To do");
            _MAIText(x+4,y,0,255,0,_Temp);
        }
    }

    GFXUpdate();
    SDL_Delay(2000);
    return Key;
}

//
//
//                  Get a selected track - or quit
//
//
static int _MAIKey(void)
{
    int i;
    GFXPollKeyboard();
    if (GFXIsKeyPressed(SDLK_ESCAPE)) return -1;
    for (i = 0;i < MAPMAX;i++)
        if (GFXIsKeyPressed(SDLK_a+i)) return i+1;
    return 0;
}

//
//
//                      Print text, main screen
//
//
static void _MAIText(int x,int y,int r,int g,int b,char *Txt)
{
    LFONT f;
    SDL_Rect rc;
    GFXScreenRect(&rc);
    if (x < 0) x = 12-strlen(Txt)/2;
    f.w = rc.w/24;f.h = rc.h/16;f.lw = f.w/8;
    f.r = r;f.g = g;f.b = b;
    f.w -= f.lw*2;f.h -= f.lw*2;
    while (*Txt != '\0')
    {
        GFXLChar(&f,x*rc.w/24,y*rc.h/16,*Txt);
        Txt++;x++;
    }
}
