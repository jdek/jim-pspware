// modplayer.c: Module Player Implementation in C for Sony PSP
//
// "PSP ModPlayer v1.0" by adresd
//
// Much of the information in this file (particularly the code used to do 
// the effects) came from Brett Paterson's MOD Player Tutorial.
// Also contains some code by Mark Feldman, which is not subject to a license
// of any kind.
//
// This is not the most efficient bit of code in the world and there are a lot
// of optimisations that could be done, but it is released as a working 
// modplayer for PSP which can be used and expanded upon by others.
// I would ask that anyone who expands or improves it considers releasing
// an updated version of the source, as a courtesy.
//
// This code is released with no implied warranty or assurance that it works
// it is not subject to any GPL or suchlike license, so use and enjoy.
//
//                   -- adresd
////////////////////////////////////////////////////////////////////////////

#include <kernel.h>
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <audio.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <display.h>
#include <ctype.h>
#include <audiolib.h>
#include "oggplayer.h"

#define FALSE 0
#define TRUE !FALSE
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define INPUT_BUFFER_SIZE	(5*8192)
#define OUTPUT_BUFFER_SIZE	2048	/* Must be an integer multiple of 4. */
/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

OggVorbis_File vf;
int eof = 0;
int current_section;
char **oggComments;
vorbis_info *vi;
int errno, __errno;
static int done_init;
FILE *fp;
static int isPlaying;		// Set to true when a mod is being played
static int myChannel;

#define PREBUFFER	2097152
static short outputBuffer[PREBUFFER];
u32 outputBufferLevel;

void OGGsetStubs(codecStubs * stubs)
{
    memcpy(validExtensions[CODEC_OGG], "ogg", 3);
    stubs->init = OGG_Init;
    stubs->load = OGG_Load;
    stubs->play = OGG_Play;
    stubs->pause = OGG_Pause;
    stubs->stop = OGG_Stop;
    stubs->end = OGG_End;
    stubs->tick = NULL;
}

static void OGGCallback(short *_buf, unsigned long numSamples)
{
    unsigned long samplesOut = 0;
    u8 justStarted = 1;
    unsigned long bytesRequired = numSamples * 2 * 2;
    int underruns = 0;
    unsigned long ret = 0;

    if (isPlaying == TRUE) {	//  Playing , so mix up a buffer
	//do decoding here
	//if out buffer has more than requested, copy it out
	if (outputBufferLevel > bytesRequired) {
	    printf("%d > %d\n", outputBufferLevel, bytesRequired);
	    memcpy((char *) _buf, (char *) outputBuffer, bytesRequired);
	    memmove(outputBuffer, &outputBuffer[bytesRequired], outputBufferLevel - bytesRequired);
	    outputBufferLevel -= bytesRequired;
	}
	//while (bytesRequired > 0) {
	/*if (outputBufferLevel < PREBUFFER*2) {
	   ret=ov_read(&vf,(unsigned char*)outputBuffer, PREBUFFER*2-outputBufferLevel,&current_section);
	   outputBufferLevel += ret;
	   /*if (ret != bytesRequired) {
	   underruns++;
	   printf("requested %d got %d, underrun %d\n", bytesRequired, ret, underruns);
	   } */
	//bytesRequired -= ret;
	//}

    } else {			//  Not Playing , so clear buffer
	int count;
	for (count = 0; count < numSamples * 2; count++)
	    *(_buf + count) = 0;
	//decode a little more
	if (done_init && outputBufferLevel < PREBUFFER * 2 - 1) {
	    ret = ov_read(&vf, (unsigned char *) outputBuffer, PREBUFFER * 2 - outputBufferLevel, &current_section);
	    outputBufferLevel += ret;
	}

    }
}

//////////////////////////////////////////////////////////////////////
// Functions - Local and not public
//////////////////////////////////////////////////////////////////////

//  This is the initialiser and module loader
//  This is a general call, which loads the module from the 
//  given address into the modplayer
//
//  It basically loads into an internal format, so once this function
//  has returned the buffer at 'data' will not be needed again.
int OGG_Load(char *filename)
{
    int bytesRequired = PREBUFFER * 2;
    unsigned long ret = 0;
    isPlaying = 0;

    done_init = 0;
    //sceIoGetstat(filename,&pstat);
    fp = fopen(filename, "r");
    if (!fp) {
	printf("could not open file %s\n", filename);
	sceDisplayWaitVblankStart();
	sceDisplayWaitVblankStart();
	sceKernelDelayThread(500000);
	return 0;
    }

    printf("opening oggfile\n");
    sceDisplayWaitVblankStart();
    sceDisplayWaitVblankStart();
    if (ov_open(fp, &vf, NULL, 0) < 0) {
	printf("Input does not appear to be an Ogg bitstream.\n");
	sceDisplayWaitVblankStart();
	sceDisplayWaitVblankStart();
	sceKernelDelayThread(500000);
	return 0;
    } else {
	printf("here is ogg info:\n");
	sceDisplayWaitVblankStart();
	sceDisplayWaitVblankStart();
	oggComments = ov_comment(&vf, -1)->user_comments;
	vi = ov_info(&vf, -1);
	printf("\nBitstream is %d channel, %ldHz\n", vi->channels, vi->rate);
	printf("\nDecoded length: %ld samples\n", (long) ov_pcm_total(&vf, -1));
	printf("Encoded by: %s\n\n", ov_comment(&vf, -1)->vendor);
    }

    while (bytesRequired > 0) {
	ret = ov_read(&vf, (char *) outputBuffer, bytesRequired, &current_section);
	/*if (ret != bytesRequired) {
	   underruns++;
	   printf("requested %d got %d, underrun %d\n", bytesRequired, ret, underruns);
	   } */
	bytesRequired -= ret;
	outputBufferLevel += ret;
	printf("PreBuffer %0.2f pc full (added %d samples)\n",
	       (float) (outputBufferLevel + 1) / (float) (PREBUFFER * 2) * 100, ret);
    }
    done_init = 1;
    printf("Done prebuffering.\n");
    sceDisplayWaitVblankStart();
    sceDisplayWaitVblankStart();
    sceKernelDelayThread(500000);
    return 1;
}

void OGG_Init(int channel)
{
    myChannel = channel;
    isPlaying = FALSE;
    AudioSetChannelCallback(myChannel, OGGCallback);
}

// This function initialises for playing, and starts
int OGG_Play()
{
    // See if I'm already playing
    if (isPlaying)
	return FALSE;

    isPlaying = TRUE;
    return TRUE;
}

int OGG_Stop()
{
    //stop playing
    isPlaying = FALSE;
    OGG_FreeTune();
    //seek to beginning of file
    //sceIoLseek(BstdFile->fd, 0, SEEK_SET);

    return TRUE;
}


void OGG_Pause()
{
    isPlaying = !isPlaying;
}

void OGG_FreeTune()
{
    ov_clear(&vf);
    if (fp)
	fclose(fp);
}

void OGG_End()
{
    OGG_Stop();
    AudioSetChannelCallback(myChannel, 0);
    OGG_FreeTune();
}
