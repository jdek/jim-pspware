/***************************************************************************
                          trail.h  -  description
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <SDL/SDL.h>

#define VERSION "0.9"

#define SQ_HOLE         (0)                 // Constants for the map
#define SQ_NORMAL1      (1)
#define SQ_ACCEL        (2)
#define SQ_UNREVERSE    (3)
#define SQ_REVERSE      (4)
#define SQ_NORMAL2      (5)
#define SQ_DEACCEL      (6)
#define SQ_JUMP         (7)
#define SQ_END          (0x40)

#define  CTRL_LEFT      (0x01)              // Control bits
#define  CTRL_RIGHT     (0x02)
#define  CTRL_UP        (0x04)
#define  CTRL_DOWN      (0x08)
#define  CTRL_JUMP      (0x10)

#define  KEY_EXIT   (-1)                    // Close clicked
#define  WIDTH      (5)                     // Road Width
#define  DEPTH      (9)                     // Number of visible segments ahead
#define  MAXLINES   (128)                   // Max number of pixels between road segments
#define  BALLXMAX   (WIDTH*256)             // Max X value of ball
#define  BALLYMAX   (1024)                  // Max Y value of ball
#define  MAPMAX     (26)                    // Max Number of Maps
#define  DEFMAPSIZE (2048)                  // Default size of the map

#define  FRAMERATE  (50)                    // Frame rate
#define  MINSPEED   (1800)                  // Slowest
#define  MAXSPEED   (85)                    // Fastest
#define  INITSPEED  (130)                   // Starting speed
#define  LAUNCHSPEED (100)                  // Speed when being launched
#define  LAUNCHYV   (65)                    // Vertical velocity of initial launch
#define  GRAVITY    (200)                   // Gravity value
#define  JUMPVMULT  (30)                    // Jump v speed multiplier
#define  JUMPVBASE  (8)                     // Jump v base value
#define  INITJUMPS  (3)                     // Initial number of player jumps

typedef struct _LFont                       // Line font structure
{
    int w,h;                                // width and height
    int r,g,b;                              // colour
    int lw;                                 // Line width
} LFONT;

typedef struct _RoadLine                    // A single pixel-high line on the road
{
    int x[WIDTH];                           // The start position of each line
    int y;                                  // Horizontal position
    int w;                                  // The width of each line
} ROADLINE;

typedef struct _RoadSeg                     // A segment of the road
{
    int y;                                  // y position of the top
    int h;                                  // Number of pixel lines high
    ROADLINE Line[MAXLINES];                // The line data for the segment.
} ROADSEG;

typedef struct _Map                         // Map structure
{
    unsigned char *Data;                    // Data
    int Time;                               // Completion time in seconds
    char Name[64];                          // Map Name
} MAP;

typedef struct _Road                        // The complete road display descriptor.
{
    SDL_Rect rc;                            // The display space for this road
    int yTop,yBottom,yHeight;               // Whole road
    ROADSEG Seg[DEPTH];                     // The segments
    unsigned char *MapData;                 // The data for the map.
    int Level;                              // Level number
    int SegDone;                            // Number of completed segments
    int RoadDone;                           // Nonzero if level complete
    int yOffset;                            // Pixel offset in road (counts in segment 0)
    int xBall;                              // Ball position (256 = 1 square)
    int yBall;                              // Ball height (range 0..BALLYMAX)
    int Launch;                             // Launching flag (1 when catapulted out)
    int yVelocity;                          // Velocity up screen
    int zVelocity;                          // Velocity into screen (in milliseconds per scrollunit)
    int xPMin,xPMax;                        // Minimum/Maximum pixel range
    int PlayerID;                           // PlayerID (0-3)
    int Reverse;                            // Reverse multiplier
    int ScrollCount;                        // Scrolls per frame (related to speed)
    int Sink;                               // Sink counter (fell in hole)
    int Control;                            // Controls
    int Jumps;                              // Available jumps
    char LastTime[6];                       // Last time (as text)
    int StartTime,CurrentTime;              // Start time (ticks), Current time (centiseconds)
    int Frame;                              // Frame Counter
    LFONT Font;                             // Screen font metrics
    MAP *ThisMap;                           // The current map
} ROAD;

int  GFXInitialise(void);
void GFXTerminate(void);
void GFXLChar(LFONT *f,int x,int y,int ch);
void GFXVLine(int x,int y,int w,Uint32 Colour);
Uint32 GFXColour(int r,int g,int b);
void GFXUpdate(void);
void GFXScreenRect(SDL_Rect *rc);
Uint32 GFXMapToColour(int Map);
void GFXSyncAndUpdate(Uint32 Speed,int x,int y,int Sprite);
int  GFXBallSize(void);
int GFXIsKeyPressed(int Key);
void GFXPollKeyboard(void);
void GFXClearRect(SDL_Rect *rc);

void ROADCalculate(ROAD *r);
void ROADPaintStart(ROAD *r,unsigned char *Map);
void ROADScroll(ROAD *r,int n);
void ROADCalculateVideoCtrl(ROAD *r,int MsPerSegment,int *pNumScroll,int *pDelayTime);
void ROADUpdate(ROAD *r,int Delay);

int  MAPRead(void);
void MAPDispose(void);
MAP *MAPGet(int n);

int  MAIMainScreen(ROAD *r);

int  GAMERun(ROAD *r,int Level);
void GAMEControls(int Player,int *CByte);
void GAMEPrint(ROAD *r,int x,int y,char *Msg,int rc,int g,int b);
void GAMEExecute(ROAD *r);

#define printf	pspDebugScreenPrintf
