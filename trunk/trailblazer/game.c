/***************************************************************************
                          game.c  -  description
                             -------------------
    begin                : Sun Jun 9 2002
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

#include "pspctrl.h"
#include "trail.h"

int GAMERun(ROAD *r,int Level)
{
	int done = 0;
	SDL_Event event;

    ROADCalculate(r);                       // Calculate all the details for the road.
    r->Level = Level;
    r->ThisMap = MAPGet(Level);
    ROADPaintStart(r,r->ThisMap->Data);     // Paint the initial value

	while (r->RoadDone == 0) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_JOYBUTTONUP: 
			case SDL_JOYBUTTONDOWN:
				if(event.jbutton.button == 7) r->Control ^= CTRL_LEFT;
				if(event.jbutton.button == 9) r->Control ^= CTRL_RIGHT;
				if(event.jbutton.button == 8) r->Control ^= CTRL_UP;
				if(event.jbutton.button == 6) r->Control ^= CTRL_DOWN;
				if(event.jbutton.button == 2) r->Control ^= CTRL_JUMP;
				break;
			case SDL_KEYDOWN: /* for PC only */
				if(event.key.keysym.sym == SDLK_SPACE)
				break;
			default:
				break;
			}
		}
        GAMEExecute(r);
	}

    if (r->RoadDone &&                      // Update best time
            r->CurrentTime < r->ThisMap->Time) r->ThisMap->Time = r->CurrentTime;
    if (r->RoadDone != 0 ||                 // Space required ?
                r->CurrentTime >= 9999)
    {
        GAMEPrint(r,4,2,"Press  Space",0,255,255);
        GFXUpdate();
//        while (GFXPollKeyboard(),GFXIsKeyPressed(SDLK_SPACE) == 0) {}
    }
    return 0;
}

//
//
//                                              Read a controller
//
//
void GAMEControls(int Player,int *CByte)
{
    *CByte = 0;
    if (GFXIsKeyPressed(SDLK_z)) *CByte |= CTRL_LEFT;
    if (GFXIsKeyPressed(SDLK_x)) *CByte |= CTRL_RIGHT;
    if (GFXIsKeyPressed(SDLK_k)) *CByte |= CTRL_UP;
    if (GFXIsKeyPressed(SDLK_m)) *CByte |= CTRL_DOWN;
    if (GFXIsKeyPressed(SDLK_l)) *CByte |= CTRL_JUMP;
}

//
//
//                                          String Printer (in game text)
//
//
void GAMEPrint(ROAD *r,int x,int y,char *Msg,int rc,int g,int b)
{
    LFONT f;
    f = r->Font;f.r = rc;f.g = g;f.b = b;
    f.w = f.w - f.lw;
    f.h = f.h - f.lw;
    while (*Msg != '\0')
    {
        GFXLChar(&f,
                 r->rc.x+f.lw/2+r->Font.w*x,
                 r->rc.y+f.lw/2+r->Font.h*y,
                 *Msg++);
        x++;
    }
}

//
//
//                                  Execute one game "loop"
//
//
void GAMEExecute(ROAD *r)
{
    int i,Cell,TimeDelay;
    char szChar[2],szTemp[32];

//  printf("%d:%d ",r->yVelocity,r->yBall);

    r->CurrentTime =                        // Time in centiseconds
                  (SDL_GetTicks()-r->StartTime)/10;
    if ((++r->Frame) % 4 == 0)              // Update time every fourth frame
    {
        sprintf(szTemp,"%02d-%02d",r->CurrentTime/100%100,r->CurrentTime%100);
        for (i = 0;i < 5;i++)
            if (szTemp[i] != r->LastTime[i])
            {
              szChar[0] = szTemp[i];szChar[1] = '\0';
              GAMEPrint(r,15+i,0,szChar,255,255,0);
            }
        strcpy(r->LastTime,szTemp);
    }

//    GAMEControls(r->PlayerID,           // Get the keyboard controls
//                        &(r->Control));

    if (r->yBall <= 0 && r->Sink == 0)  // Landed ?
    {
      r->yBall = 0;                     // On ground, dy = 0
      r->yVelocity = 0;
      r->Launch = 0;                    // Not launched any more
    }


    if (r->yBall == 0)                  // Not on the ground
    {
        Cell =                          // Find out what cell we are on.
          r->MapData[r->xBall*WIDTH/BALLXMAX];

        if (r->yBall == 0 &&            // Time to jump
            r->yVelocity == 0)
            if (Cell == SQ_JUMP || (((r->Control & CTRL_JUMP) != 0) && r->Jumps > 0 && Cell != SQ_HOLE))
        {
            r->yVelocity = JUMPVMULT*INITSPEED // Calculate velocity
                                  / r->zVelocity+JUMPVBASE;
            if (r->Control & CTRL_JUMP && r->Jumps > 0)
            {
                r->Jumps--;
                GAMEPrint(r,19,1,szTemp,255,255,0);
            }
        }
        if (Cell == SQ_REVERSE)         // Handle auto reversing
                            r->Reverse = -1;
        if (Cell == SQ_UNREVERSE)
                            r->Reverse = 1;
        if (Cell == SQ_ACCEL)           // Handle speed up/slow down
                            r->zVelocity = MAXSPEED;
        if (Cell == SQ_DEACCEL)
                            r->zVelocity = MINSPEED;
        if (Cell == SQ_HOLE)            // If falling down initiate that
        {
            r->Sink = 1;r->yBall = -1;
        }
    }

    if (r->Sink > 0)                    // falling down hole
    {
        r->yBall -= 300/FRAMERATE;      // go further down
        if (r->yBall < -1000)           // time to pop up ?
        {
            r->Launch = 1;              // do so !
            r->yVelocity = LAUNCHYV;
            r->Sink = 0;
            r->yBall = 0;
        }
    }

    if (r->yVelocity!=0 || r->yBall>0)  // If in air or about to jump
    {
      r->yBall = r->yBall+              // Add velocity to position
                  r->yVelocity*100/FRAMERATE;
      r->yVelocity = r->yVelocity -     // Deduct gravity from acceleration
                            GRAVITY/FRAMERATE;
    }

    if (r->zVelocity > MINSPEED)        // Check speed within limits
              r->zVelocity = MINSPEED;
    if (r->zVelocity < MAXSPEED)
              r->zVelocity = MAXSPEED;

    ROADCalculateVideoCtrl(r,           // Recalculate stuff
                           (r->Launch) ? LAUNCHSPEED
                                       : r->zVelocity,
                           &(r->ScrollCount),&TimeDelay);


    if (r->Sink == 0)                   // If not sinking
    {
        ROADScroll(r,r->ScrollCount);
        if (r->Control & CTRL_LEFT)
                    r->xBall -= r->Reverse*20*50/FRAMERATE;
        if (r->Control & CTRL_RIGHT)
                    r->xBall += r->Reverse*20*50/FRAMERATE;
        if (r->Control & CTRL_DOWN)
                    r->zVelocity = r->zVelocity * 11/10;
        if (r->Control & CTRL_UP)
                    r->zVelocity = r->zVelocity * 10/11;
    }
    ROADUpdate(r,TimeDelay);            // Synchronise and update if time available
}
