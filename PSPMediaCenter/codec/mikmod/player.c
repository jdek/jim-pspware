// player.c: MIKMOD Player Implementation in C for Sony PSP
//
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

#include "mikmod.h"


#define FALSE 0
#define TRUE !FALSE
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

// The following variables are maintained and updated by the tracker during playback
static int isPlaying;		// Set to true when a mod is being played

//////////////////////////////////////////////////////////////////////
// These are the public functions
//////////////////////////////////////////////////////////////////////
static int myChannel;

/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

extern int _mm_errno;
extern BOOL _mm_critical;
extern char *_mm_errmsg[];

void my_error_handler(void)
{
    printf("_mm_critical %d\n", _mm_critical);
    printf("_mm_errno %d\n", _mm_errno);
    printf("%s\n", _mm_errmsg[_mm_errno]);
    return;
}

UNIMOD *mf;
int maxchan = 255;


void MIKMODsetStubs(codecStubs * stubs)
{
    stubs->init = MIKMOD_Init;
    stubs->load = MIKMOD_Load;
    stubs->play = MIKMOD_Play;
    stubs->pause = MIKMOD_Pause;
    stubs->stop = MIKMOD_Stop;
    stubs->end = MIKMOD_End;
    stubs->time = MIKMOD_GetTimeString;
    stubs->tick = NULL;
    stubs->eos = MIKMOD_EndOfStream;
    memcpy(stubs->extension, "mod\0"
	   ".xm\0" ".it\0" "s3m\0" "stm\0" "mtm\0" "669\0" "far\0" "dsm\0" "med\0" "ult\0" "uni\0" "\0\0\0\0", 13 * 4);
    // mod,xm,it,s3m,stm,mtm,669,far,dsm,med,ult,uni
}

void MIKMOD_Init(int channel)
{
    static inited = 0;
    myChannel = channel;
    isPlaying = FALSE;
    if (inited == 0) {
	inited = 1;
	_mm_RegisterErrorHandler(my_error_handler);
	MikMod_RegisterAllLoaders();
	MikMod_RegisterAllDrivers();
    }
    //md_mode = DMODE_16BITS|DMODE_STEREO|DMODE_SOFT_SNDFX|DMODE_SOFT_MUSIC; 
    MikMod_Init();
}


void MIKMOD_FreeTune()
{
    MikMod_FreeSong(mf);
}


void MIKMOD_End()
{
    MIKMOD_Stop();
    Player_Stop();
    MIKMOD_FreeTune();
    MikMod_Exit();
}

//////////////////////////////////////////////////////////////////////
// Functions - Local and not public
//////////////////////////////////////////////////////////////////////
int size;
unsigned char *ptr;

void MIKMOD_DisplayInfo(UNIMOD * ptr)
{
    int count;
    int x, y;
    printf("\n");
    printf("Song Name   : %s\n", ptr->songname);
    printf("Composer    : %s\n", ptr->composer);
    printf("Comment     : %s\n", ptr->comment);
    printf("Module Type : %s\n", ptr->modtype);
    printf("Channels    : %d\n", ptr->numchn);
    //printf("Instruments : %d\n",ptr->numins);
    //printf("Samples     : %d\n",ptr->numsmp);
    //printf("Patterns    : %d\n",ptr->numpat);

    x = pspDebugScreenGetX();
    y = pspDebugScreenGetY();
    for (count = 1; count < ptr->numsmp; count++) {
	if (count == ((ptr->numsmp + 1) / 2)) {
	    x += 30;
	    y -= (count - 1);
	}
	pspDebugScreenSetXY(x, y + count);
	printf("  %02d - %s", count, mf->samples[count].samplename);
    }
}

int MIKMOD_Load(char *filename)
{
    isPlaying = FALSE;

    mf = MikMod_LoadSong(filename, maxchan);
    if (mf != 0) {
	Player_Start(mf);
	MIKMOD_DisplayInfo(mf);
	return 1;
    }
    return 0;
}

// This function initialises for playing, and starts
int MIKMOD_Play()
{
    // See if I'm already playing
    if (isPlaying)
	return FALSE;

    isPlaying = TRUE;
    return TRUE;
}

void MIKMOD_Pause()
{
    isPlaying = !isPlaying;
    if (isPlaying)
	MikMod_EnableOutput();
    else
	MikMod_DisableOutput();
}

int MIKMOD_Stop()
{
    //stop playing
    isPlaying = FALSE;

    return TRUE;
}

void MIKMOD_GetTimeString(char *dest)
{
    sprintf(dest, "%02d:%02d:%02d", mf->sngpos, mf->patpos, mf->vbtick);
}

//extern BOOL MikMod_Playing(void);
extern BOOL Player_Active(void);
int MIKMOD_EndOfStream()
{
    if (!Player_Active())
	return 1;
    return 0;
}
