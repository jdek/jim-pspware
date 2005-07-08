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
long AACSourceBufferSize;
long samplesInOutput = 0;

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
    stubs->play = AAC3_Play;
    stubs->pause = AAC_Pause;
    stubs->stop = AAC_Stop;
    stubs->end = AAC_End;
    stubs->time = NULL;
    stubs->tick = NULL;
    memcpy(stubs->extension, "aac\0", 4);
}


static void AACCallback(short *_buf, unsigned long numSamples)
{
    unsigned long samplesOut = 0;
    AACFrameInfo aacFrameInfo;
    int err;
    int bytesDecoded;

    if (isPlaying == TRUE) {	//  Playing , so mix up a buffer
	if (samplesInOutput > 0) {
	    if (samplesInOutput > numSamples) {
		memcpy((char *) _buf, (char *) AACOutputBuffer, numSamples * 2 * 2);
		samplesOut = numSamples;
		samplesInOutput -= numSamples;
	    } else {
		memcpy((char *) _buf, (char *) AACOutputBuffer, samplesInOutput * 2 * 2);
		samplesOut = samplesInOutput;
		samplesInOutput = 0;
	    }
	}
	while (samplesOut < numSamples) {
	    //decode samples into AACDecodeBuffer
	    err = AACDecode(hAACDecoder, &readPtr, &bytesLeft, AACDecodeBuffer);
	    if (err) {
		printf("decoder threw error %d\n", err);
	    }
	    //get stats from last frame
	    AACGetLastFrameInfo(hAACDecoder, &aacFrameInfo);
	    pirntf("bitspersample: %d (%d bytes), samps: %d\n", aacFrameInfo.bitsPerSample,
		   aacFrameInfo.bitsPerSample / 8, aacFrameInfo.outputSamps);
	    bytesDecoded = aacFrameInfo.bitsPerSample / 8, aacFrameInfo.outputSamps;
	    //move # of requested samples into audio buffer
	    if (samplesOut + aacFrameInfo.outputSamps <= numSamples * 2) {
		memcpy(&_buf[samplesOut * 2 * 2], (char *) AACDecodeBuffer, bytesDecoded);
		samplesOut += aacFrameInfo.outputSamps;
	    } else {
		memcpy(&_buf[samplesOut * 2 * 2], (char *) AACDecodeBuffer, (numSamples - samplesOut) * 2 * 2);

		for (i = 0; i < aacFrameInfo.outputSamps - (numSamples - samplesOut); i++)
		    AACOutputBuffer[i] = AACDecodeBuffer[(numSamples - samplesOut) + i];

		samplesOut += numSamples - samplesOut;
		samplesInOutput += aacFrameInfo.outputSamps - (numSamples - samplesOut);
	    }


	}
    } else {			//  Not Playing , so clear buffer
	{
	    int count;
	    for (count = 0; count < numSamples * 2; count++)
		*(_buf + count) = 0;
	}
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
	AACSourceBufferSize = sceIoLseek(fd, 0, PSP_SEEK_END);
	sceIoLseek(fd, 0, PSP_SEEK_SET);
	AACSourceBuffer = (unsigned char *) malloc(AACSourceBufferSize + 8);
	memset(AACSourceBuffer, 0, size + 8);
	if (ptr != 0) {		// Read file in
	    sceIoRead(fd, AACSourceBuffer, AACSourceBufferSize);
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
    memset(OutputBuffer, 0, OUTPUT_BUFFER_SIZE);
    OutputPtr = (unsigned char *) OutputBuffer;

    return TRUE;
}
