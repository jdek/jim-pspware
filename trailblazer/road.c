/***************************************************************************
               road.c  -  road drawing/scrolling/setup routines
                             -------------------
    begin                : Sat Jun 8 2002
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

static int _ROADCalculateY(ROAD *r,int h);

//
//
//              Calculate all coordinates and lengths ; assumes Road rectangle set up.
//
//

void ROADCalculate(ROAD *r)
{
    int n,h,i,j,k,x,y,x1,x2;

    n = _ROADCalculateY(r,256);             // Make initial calculation assuming first one is 256 high.
    h = (r->rc.h) * 2 / 3;                  // This is how big we want it to be.
    n = 256 * h / n;                        // Work out the scaling from the first calculation to correct
    if (n > MAXLINES) n = MAXLINES;         // Can only have this many.... this is a constant in trail.h
    r->yHeight = _ROADCalculateY(r,n);      // Calculate segment heights scaled correctly
    r->yTop = r->Seg[DEPTH-1].y;            // Top of road
    r->yBottom = r->rc.y+r->rc.h;           // Bottom of road
    x1 = r->rc.w / 7;                       // The minimum width (at the top)
    x2 = r->rc.w * 10/10;                   // The maximum width (at the bottom)
    r->xPMin = r->rc.x + r->rc.w/2 - x2/2;  // Pixel range
    r->xPMax = r->xPMin + x2;
    r->xPMin+= GFXBallSize()/2;             // Adjust for min/max ball
    r->xPMax-= GFXBallSize()/2;
    for (i = 0;i < DEPTH;i++)               // Calculate data for each line
        for (j = 0;j < r->Seg[i].h;j++)
        {
            y = r->Seg[i].y + j;            // The vertical position of the line

            n = (x2-x1)*(y-r->yTop) /       // Calculate the width of the whole line
                            (r->yBottom-r->yTop)+x1;
            x = r->rc.x+r->rc.w/2-n/2;      // Start point of the line
            r->Seg[i].Line[j].w = n/WIDTH+1;// Width of each segment
            r->Seg[i].Line[j].y = y;        // Vertical position
            for (k = 0;k < WIDTH;k++)       // Calculate the segments
                r->Seg[i].Line[j].x[k] = x + n * k / WIDTH;
        }
}

//
//
//  For a given height (of the first road bit) calculate the heights of all the other road segments
//  Returns the total height of the road.
//
//
static int _ROADCalculateY(ROAD *r,int h)
{
    int i,y,c = 0;
    y = r->rc.y+r->rc.h;                    // Bottom of display area
    for (i = 0;i < DEPTH;i++)
    {
        r->Seg[i].h = h;                    // Set height
        c = c + h;                          // Add to total
        y = y - h;                          // Top of section
        r->Seg[i].y = y;                    // Y position
        h = h * 3 / 4;                      // Perspective effect on height
    }
    return c;
}

//
//
//                      Paint the road in its initial state, and initialise it.
//
//
void ROADPaintStart(ROAD *r,unsigned char *Map)
{
    int i,j,k,c;
    char szTemp[32];
    GFXClearRect(&(r->rc));
    for (i = 0;i < DEPTH;i++)               // Work through each segment
        for (j = 0;j < r->Seg[i].h;j++)     // Each line within the segment
            for (k = 0;k < WIDTH;k++)       // Each line across horizontally
            {
                c = GFXMapToColour(Map[i*WIDTH+k]);
                GFXVLine(r->Seg[i].Line[j].x[k],r->Seg[i].Line[j].y,r->Seg[i].Line[j].w,c);
            }
    r->MapData = Map;                       // Store map pointer
    r->yOffset = 0;                         // Offset position
    r->SegDone = 0;                         // No of completed segments
    r->RoadDone = 0;                        // Not complete
    r->xBall = BALLXMAX/2;                  // Ball is centred, on the ground
    r->yBall = 0;
    r->zVelocity = INITSPEED;               // Into screen velocity
    r->Reverse = 1;                         // Reverse multiplier
    r->Jumps = INITJUMPS;                   // Number of jumps
    r->Font.w = (r->rc.w)/20;               // Set size of font
    r->Font.h = (r->rc.h)/12;
    r->Font.lw = (r->Font.w / 8) & 0xFE;    // Line width and colour
    if (r->Font.w == 0) r->Font.w = 0;
    r->Font.r = r->Font.g = r->Font.b = 255;
    r->Launch = 1;                          // The initial launch
    r->yVelocity = LAUNCHYV;
    r->Sink = 0;
    r->StartTime = SDL_GetTicks();          // Record the start time
    r->CurrentTime = r->Frame = 0;
    GAMEPrint(r,0,0,"Level",0,255,0);
    sprintf(szTemp,"%c",r->Level+'A');
    GAMEPrint(r,7,0,szTemp,255,255,0);
    GAMEPrint(r,10,0,"Time",0,255,0);
    strcpy(r->LastTime,"00-00");
    GAMEPrint(r,15,0,r->LastTime,255,255,0);
    GAMEPrint(r,10,1,"Jump",0,255,0);
    sprintf(szTemp,"%d",r->Jumps);
    GAMEPrint(r,19,1,szTemp,255,255,0);
}

//
//
//                              Scroll the road n pixels
//
//  Basically, this just draws one line in each segment per scroll, moving the border down
//  by one pixel, so we don't have to continually redraw the road. Of course, the slow bit
//  is SDL_UpdateRect() so this isn't really necessary. But why waste cycles.
//
//
void ROADScroll(ROAD *r,int n)
{
    int i,j,y;
    unsigned char *MapData;
    if (r->MapData[0] >= SQ_END)            // Completed the level
    {
        r->RoadDone = 1;
        return;
    }
    while (n-- > 0)                         // For however many times....
    {
        MapData = r->MapData;               // Get the map data
        MapData += WIDTH;                   // Because we are scrolling initially 1 into 0
        for (i = 0;i < DEPTH;i++)           // Draw new line for each part, starting at the bottom
        {
            y = r->Seg[i].h *               // Work out sub-position in that segment
                        r->yOffset / r->Seg[0].h;
            for (j = 0;j < WIDTH;j++)       // Do each square
            {
                GFXVLine(r->Seg[i].Line[y].x[j],
                         r->Seg[i].Line[y].y,
                         r->Seg[i].Line[y].w,
                         GFXMapToColour(*MapData));
                MapData++;
            }
        }
        r->yOffset++;                       // Advance scrolling counter
        if (r->yOffset == r->Seg[0].h)      // Scrolled one whole segment ?
        {
            r->MapData += WIDTH;            // Go on to the next map square.
            r->yOffset = 0;
            r->SegDone++;
        }
    }
}

//
//
//  Calculate sync and timing information. Given a number of Milliseconds per Segment
//  Calculate the ScrollsPerUpdate and the DelayTime
//
//
void ROADCalculateVideoCtrl(ROAD *r,int MsPerSegment,int *pNumScroll,int *pDelayTime)
{
    int UsPerSegment = 1000 *               // Microseconds per segment
                    MsPerSegment / r->Seg[0].h;
    int UsPerDelay;

    *pDelayTime = 1000/FRAMERATE;           // Time in ms per frame
    UsPerDelay = *pDelayTime * 1000;        // Microseconds per delay time
    *pNumScroll=UsPerDelay/UsPerSegment;    // Number to scroll
    if (*pNumScroll == 0) *pNumScroll = 1;  // This is a minimum
}

//
//
//                  Copy the ball onto the screen, update the road
//
//

void ROADUpdate(ROAD *r,int Delay)
{
    int x,y,n;
    if (r->xBall < 0) r->xBall = 0;         // Force into range
    if (r->xBall >= BALLXMAX) r->xBall = BALLXMAX-1;
    x = (r->xPMax-r->xPMin) *               // Calculate lower centre position of ball
                r->xBall / BALLXMAX + r->xPMin;
    y = r->rc.y + r->rc.h - r->rc.h/3 * r->yBall / BALLYMAX;

    n = r->yOffset * 256 / r->Seg[0].h;     // Sub-segment in range 0-255
    n = n + r->SegDone * 256;               // Overall position in map
    GFXSyncAndUpdate(Delay,x,y,(n >> 5) & 7);
}

