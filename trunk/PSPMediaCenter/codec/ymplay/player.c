// player.c: YM Player Implementation in C for Sony PSP
//
// Uses the port of StSound (YM Player) on the forums.pspdev.org site
// by Jim
////////////////////////////////////////////////////////////////////////////

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <pspaudiolib.h>
#include "player.h"

#define _WIN32
#include "StSoundLibrary.h"


#define FALSE 0
#define TRUE !FALSE
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

// The following variables are maintained and updated by the tracker during playback
static int isPlaying;		// Set to true when a mod is being played
static int eos;
//////////////////////////////////////////////////////////////////////
// These are the public functions
//////////////////////////////////////////////////////////////////////
static int myChannel;
static YMMUSIC *pMusic;
void *mf;
ymMusicInfo_t info;



/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

void YMPLAYsetStubs(codecStubs * stubs)
{
    stubs->init = YMPLAY_Init;
    stubs->load = YMPLAY_Load;
    stubs->play = YMPLAY_Play;
    stubs->pause = YMPLAY_Pause;
    stubs->stop = YMPLAY_Stop;
    stubs->end = YMPLAY_End;
    stubs->time = YMPLAY_GetTimeString;
    stubs->tick = NULL;
    stubs->eos = YMPLAY_EndOfStream;
    memcpy(stubs->extension, ".ym\0" "\0\0\0\0", 2 * 4);
}


static void YMPLAYCallback(void *_buf2, unsigned int numSamples, void *pdata)
{
  short *_buf = (short *)_buf2;
    unsigned long samplesOut = 0;
    //      u8 justStarted = 1;

    if (isPlaying == TRUE) {	//  Playing , so mix up a buffer
	short *s = (short *) _buf;
	short *d = (short *) _buf;

	if (!ymMusicCompute((void *) pMusic, (ymsample *) _buf, numSamples))
	    eos = 1;

	s += numSamples - 1;
	d += numSamples * 2 - 1;
	while (numSamples--) {
	    *d = *s;
	    *(d - 1) = *s;
	    s--;
	    d -= 2;
	}
    } else {			//  Not Playing , so clear buffer
	int count;
	for (count = 0; count < numSamples * 2; count++)
	    *(_buf + count) = 0;
    }
}

void YMPLAY_Init(int channel)
{
    myChannel = channel;
    isPlaying = FALSE;
    pspAudioSetChannelCallback(myChannel, YMPLAYCallback,0);
    pMusic = 0;
}


void YMPLAY_FreeTune()
{
    ymMusicDestroy(pMusic);
}


void YMPLAY_End()
{
    YMPLAY_Stop();
    pspAudioSetChannelCallback(myChannel, 0,0);
    ymMusicStop(pMusic);
    YMPLAY_FreeTune();
}

//////////////////////////////////////////////////////////////////////
// Functions - Local and not public
//////////////////////////////////////////////////////////////////////
int size;
unsigned char *ptr;

int YMPLAY_Load(char *filename)
{
    eos = 0;
    pMusic = ymMusicCreate();
    mf = ymMusicLoad(pMusic, filename);
    if (mf) {
	ymMusicGetInfo(pMusic, &info);
	ymMusicSetLoopMode(pMusic, YMFALSE);
	ymMusicPlay(pMusic);

	printf("\n\n");
	printf("Name.....: %s\n", info.pSongName);
	printf("Author...: %s\n", info.pSongAuthor);
	printf("Comment..: %s\n", info.pSongComment);
	printf("Duration.: %d:%02d\n", (int) info.musicTimeInSec / 60, (int) info.musicTimeInSec % 60);
	return 1;
    } else
	return 0;
}

// This function initialises for playing, and starts
int YMPLAY_Play()
{
    // See if I'm already playing
    if (isPlaying)
	return FALSE;

    isPlaying = TRUE;
    return TRUE;
}

void YMPLAY_Pause()
{
    isPlaying = !isPlaying;
}

int YMPLAY_Stop()
{
    //stop playing
    isPlaying = FALSE;

    return TRUE;
}

void YMPLAY_GetTimeString(char *dest)
{
    unsigned long time = ymMusicGetPos(pMusic) / 1000;
    sprintf(dest, "%d:%02d", (int) (time / 60), (int) (time % 60));
}

int YMPLAY_EndOfStream()
{
    if (eos)
	return 1;
    return 0;
}
