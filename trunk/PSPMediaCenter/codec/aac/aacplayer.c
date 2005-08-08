// mp3player.c: MP3 Player Implementation in C for Sony PSP
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
#include "aacplayer.h"

#define FALSE 0
#define TRUE !FALSE
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
//#define OUTPUT_BUFFER_SIZE    2048    /* Must be an integer multiple of 4. */

#ifdef AAC_ENABLE_SBR
#define SBR_MUL		2
#else
#define SBR_MUL		1
#endif

#define AAC_BUFFER_SIZE AAC_MAX_NCHANS * AAC_MAX_NSAMPS * SBR_MUL
HAACDecoder *hAACDecoder;
short AACDecodeBuffer[AAC_BUFFER_SIZE];
short AACOutputBuffer[AAC_BUFFER_SIZE];

/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

u8 *AACSourceBuffer;
u8 *readPtr;
int AACSourceBufferSize;


// The following variables are maintained and updated by the tracker during playback
static int isPlaying;		// Set to true when a mod is being played

//////////////////////////////////////////////////////////////////////
// These are the public functions
//////////////////////////////////////////////////////////////////////
static int myChannel;

/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

void AACsetStubs(codecStubs * stubs)
{
    stubs->init = AAC_Init;
    stubs->load = AAC_Load;
    stubs->play = AAC_Play;
    stubs->pause = AAC_Pause;
    stubs->stop = AAC_Stop;
    stubs->end = AAC_End;
    stubs->time = NULL;
    stubs->tick = NULL;
    stubs->eos = AAC_EndOfStream;
    memcpy(stubs->extension, "aac\0" "\0\0\0\0", 2 * 4);
}


static void AACCallback(short *_buf, unsigned long numSamples)
{
    static short tempmixbuf[PSP_NUM_AUDIO_SAMPLES * 2 * 2] __attribute__ ((aligned(64)));
    static unsigned long tempmixleft = 0;
    AACFrameInfo aacFrameInfo;
    static int bytesDecoded;
    if (isPlaying == TRUE) {	// Playing , so mix up a buffer
	while (tempmixleft < numSamples) {	//  Not enough in buffer, so we must mix more
	    unsigned long bytesRequired = (numSamples - tempmixleft) * 4;	// 2channels, 16bit = 4 bytes per sample
	    unsigned long ret = AACDecode(hAACDecoder, &readPtr, &AACSourceBufferSize, &tempmixbuf[tempmixleft * 2]);
	    if (ret) {
		printf("decoder threw error %d\n", ret);
	    } else {
	    	 //get stats from last frame
	    	AACGetLastFrameInfo(hAACDecoder, &aacFrameInfo);
	    	bytesDecoded = aacFrameInfo.bitsPerSample / 8 * aacFrameInfo.outputSamps;
		}
	    tempmixleft += aacFrameInfo.outputSamps;	// back down to sample num
	}
	if (tempmixleft >= numSamples) {	//  Buffer has enough, so copy across
	    int count, count2;
	    short *_buf2;
	    for (count = 0; count < numSamples; count++) {
		count2 = count + count;
		_buf2 = _buf + count2;
		// Double up for stereo
		*(_buf2) = tempmixbuf[count2];
		*(_buf2 + 1) = tempmixbuf[count2 + 1];
	    }
	    //  Move the pointers
	    tempmixleft -= numSamples;
	    //  Now shuffle the buffer along
	    for (count = 0; count < tempmixleft; count++)
		tempmixbuf[count] = tempmixbuf[numSamples + count];
	}

    } else {			//  Not Playing , so clear buffer
	int count;
	for (count = 0; count < numSamples * 2; count++)
	    *(_buf + count) = 0;
    }
}

void AAC_Init(int channel)
{
    myChannel = channel;
    isPlaying = FALSE;
    pspAudioSetChannelCallback(myChannel, AACCallback);
    hAACDecoder = (HAACDecoder *) AACInitDecoder();
}


void AAC_FreeTune()
{
    /* The input file was completely read; the memory allocated by our
     * reading module must be reclaimed.
     */
    if (AACSourceBuffer) {
	free(AACSourceBuffer);
	AACSourceBuffer = NULL;
    }
    if (hAACDecoder) {
	AACFreeDecoder(hAACDecoder);
	hAACDecoder = NULL;
    }
}


void AAC_End()
{
    AAC_Stop();
    pspAudioSetChannelCallback(myChannel, 0);
    AAC_FreeTune();
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
int AAC_Load(char *filename)
{
    int fd;
    //psp_stats pstat;
    //sceIoGetstat(filename, &pstat);
    if ((fd = sceIoOpen(filename, PSP_O_RDONLY, 0777)) > 0) {
	//  opened file, so get size now
	AACSourceBufferSize = sceIoLseek32(fd, 0, PSP_SEEK_END);
	sceIoLseek(fd, 0, PSP_SEEK_SET);
	AACSourceBuffer = (unsigned char *) malloc(AACSourceBufferSize + 8);
	memset(AACSourceBuffer, 0, AACSourceBufferSize + 8);
	if (AACSourceBuffer != 0) {		// Read file in
	    sceIoRead(fd, AACSourceBuffer, AACSourceBufferSize);
	    readPtr = AACSourceBuffer;
	} else {
	    printf("Error allocing\n");
	    sceIoClose(fd);
	    return 0;
	}
	// Close file
	sceIoClose(fd);
    } else {
	return 0;
    }

    isPlaying = FALSE;
    return 1;
}

// This function initialises for playing, and starts
int AAC_Play()
{
    // See if I'm already playing
    if (isPlaying)
	return FALSE;

    isPlaying = TRUE;
    return TRUE;
}

void AAC_Pause()
{
    isPlaying = !isPlaying;
}

int AAC_Stop()
{
    //stop playing
    isPlaying = FALSE;

    //clear buffer
//    memset(OutputBuffer, 0, OUTPUT_BUFFER_SIZE);
    //OutputPtr = (unsigned char *) OutputBuffer;

    return TRUE;
}

int AAC_EndOfStream()
{
    return 0;
}
