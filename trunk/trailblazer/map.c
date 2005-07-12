/***************************************************************************
                     map.c  -  read and store maps
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

#include "trail.h"

static MAP Map[MAPMAX];                     // The actual maps

//
//
//  Read file trail.dat into memory. It can either be in the cwd or in /usr/share/trailblazer
//
//
int MAPRead(void)
{
    FILE *f;
    int Line,i,j,MapPtr = 0;
    char Buffer[256];
    for (i = 0;i < MAPMAX;i++)              // Erase all maps
    {
        Map[i].Data = NULL;
        Map[i].Time = 9999;                 // Default time (9999 cs)
    }

    f = fopen("ms0:/PSP/GAME/TRAILBLAZER/trail.dat","r");         // Try to open in cwd
    if (f == NULL)                          // If not there, try to open in cwd
    	f = fopen("trail.dat","r");
    if (f == NULL) return -1;               // Not found (Error -1)
    i = 0;Line = 0;
    while (fgets(Buffer,                    // Keep reading
                    sizeof(Buffer),f) != NULL)
    {
        Line++;                             // Next line
        if (strncmp(Buffer,"LEVEL",5) == 0) // LEVEL = n selects the buffer to read
        {
            i = atoi(Buffer+6)-1;           // -1 because the first one is Level 0
            if (i < 0 || i >= MAPMAX) return Line;
            if (Map[i].Data != NULL) return Line;
            Map[i].Data = (unsigned char *)malloc(DEFMAPSIZE);
            if (Map[i].Data == NULL) return -4;
            for (j = 0;j < DEFMAPSIZE;j++)  // Fill it full of end character
                Map[i].Data[j] = SQ_END+(((j%5)&1) ^ (j/5&1));
            sprintf(Map[i].Name,"(unnamed)");
            MapPtr = 0;                     // Start at the beginning
        }
        if (isdigit(Buffer[0]))             // Map definition
        {
            for (j = 0;j < WIDTH;j++)
                Map[i].Data[MapPtr++] = Buffer[j]-'0';
        }
    }
    fclose(f);                              // Close the file
    f = fopen("trail.time","r");
    if (f != NULL)
    {
        for (i = 0;i < MAPMAX;i++)
            fscanf(f,"%d",&(Map[i].Time));
        fclose(f);
    }
    return 0;
}

//
//
//                          Throw away all allocated map memory
//
//
void MAPDispose(void)
{
    int i;
    FILE *f;
    for (i = 0;i < MAPMAX;i++)
    {
        if (Map[i].Data != NULL) free(Map[i].Data);
        Map[i].Data = NULL;
    }
    f = fopen("/usr/share/trailblazer/trail.time","w");
    if (f != NULL)
    {
        for (i = 0;i < MAPMAX;i++)
            fprintf(f,"%d ",Map[i].Time);
        fclose(f);
    }
}

//
//
//                                Return map information
//
//
MAP *MAPGet(int n)
{
    if (n < 0 || n >= MAPMAX) return NULL;
    return &(Map[n]);
}
