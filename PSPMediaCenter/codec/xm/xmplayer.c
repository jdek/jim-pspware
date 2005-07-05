// xmplayer.c: XM Player Implementation in C for Sony PSP
//
// PSP port     : adresd
// PS2 port     : Arnaud Storq (norecess@planet-d.net)                        
// Base code    : Bruno Schodlbauer                                           
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

#include "xmplayer.h"

#define REMOVE_TYPEDEF
#include "vbf/vbf_std.h"
#include "mpl/mx_std.h"
#include "mpl/ml_xm.h"
#include "mpl/sys/mem.h"

typedef struct Format
{
  int nChannels;
  int nSamplesPerSec;
  int wBitsPerSample;
  int nBlockAlign;

  int dwBufferBytes;
} Format;
static Format format;
static mpl__mixer_t *mixah;

static short *buffer;
static int bufferSize;

static int myChannel;
static unsigned char *filedataptr;

#define printf pspDebugScreenPrintf

#define FALSE 0
#define TRUE !FALSE


static int
cmd_cm(void *internal, mpl__snd_mixer_t **mixer, mpl__snd_mixer_opt_t opt)
{
  mpl__mixer_opt_t mopt;
  mpl__mixer_t *mx;
  mpl__snd_dev_output_t out;

  mpl__mx_std_create(&mx);

  mixah = mx;

  mopt.ch_num = opt.ch_num;
  mopt.mem_usage = 0;
  mopt.qual = MPL__MX_HQ;
  mopt.smp_num = opt.smp_num;

  mpl__mixer_set_options(mx, &mopt);
  mpl__mixer_set_vol(mx, 1.0f, 0.5f, 0.5f);

  out.channels = 2;
  out.format = MPL__SND_DEV_FMT_16BIT;
  out.freq = 44100; // 48000
  out.latency = 0;

  mpl__mixer_set_output_options(mx, &out, 100);

  mpl__mixer_get_interface(mx, mixer);

  return 0;
}

static int
cmd_dm(void *internal, mpl__snd_mixer_t *mixer)
{
  return 0;
}

static int
cmd_goo(void *internal, mpl__snd_dev_output_t *opt)
{
  opt->channels = 2;
  opt->format = 2;
  opt->freq = 44100;
  opt->latency = 0;

  return 0;
}

static void
setparams(int ch, int freq, int bits)
{
  format.nChannels = ch;
  format.nSamplesPerSec = freq;
  format.wBitsPerSample = bits;
  format.nBlockAlign = bits*ch>>3;

  format.dwBufferBytes = format.nBlockAlign*(int)(freq*0.1f);

  bufferSize = (format.dwBufferBytes/format.nBlockAlign)>>2;

  if (mpl__mem_alloc(format.nBlockAlign*bufferSize, &buffer)<=MPL__ERR_GENERIC)
  {
    printf("XMplay: Memory Allocation failed: %d\n", format.nBlockAlign*bufferSize);
  }
}

// XMCallback
static volatile int m_bPlaying;
static void XMPlayCallback(short *_buf, unsigned long length)
{
  if (m_bPlaying == 1) {
    mpl__mixer_mix(mixah, (unsigned char *)_buf, length);
  }
  else {
    int count;
    for(count=0;count<length*2;count++)
      *(_buf+count) = 0;;
  }
}

void XMsetStubs(codecStubs * stubs)
{
  stubs->init = XM_Init;
  stubs->load = XM_Load;
  stubs->play = XM_Play;
  stubs->pause = XM_Pause;
  stubs->stop = XM_Stop;
  stubs->end = XM_End;
  stubs->time = XM_GetTimeString;
  stubs->tick = NULL;
  memcpy(stubs->extension, ".xm\0", 4);
}

void XM_Init(int channel)
{
  filedataptr = 0;
  myChannel = channel;
  m_bPlaying = 0;
  pspAudioSetChannelCallback(myChannel, XMPlayCallback);
}
void XM_End()
{
  XM_Stop();
  pspAudioSetChannelCallback(myChannel, 0);
  if (filedataptr != 0) {
    free(filedataptr);
    filedataptr = 0;
  }
}

void XM_GetTimeString(char *dest)
{
  *dest = '\0';
  //HH:MM:SS
  sprintf(dest,"%02d:%02d:%02d",0,0,0);
}

int XM_InitTune(unsigned char *ptr, long size)
{
  mpl__xm_t xm;
  mpl__mp_t mp;
  vbf_t file;
  mpl__snd_dev_t dev;

  mpl__mem_init();
  dev.create_mixer = cmd_cm;
  dev.destroy_mixer = cmd_dm;
  dev.get_output_options = cmd_goo;

  vbf_std_open(&file, ptr,size);

  if (mpl__xm_load(&file, &xm)!=MPL__ERR_OK)	{
    printf("XMplay: Module Parsing failed\n");
    return 0;
  }

  if (mpl__mp_xm_construct(&xm, &mp, 0)<MPL__ERR_OK)	{
    printf("XMplay: Module Construction failed\n");
    return 0;
  }

  if (mpl__mp_set_dev(&mp, &dev)<MPL__ERR_OK)	{
    printf("XMplay: Device Setup failed\n");
    return 0;
  }

  if (mpl__mp_set_vol(&mp, 1.0f)<MPL__ERR_OK)	{
    printf("XMplay: Volume Setup failed\n");
    return 0;
  }

  if (mpl__mp_play(&mp)<MPL__ERR_OK) {
    printf("XMplay: Play failed\n");
    return 0;
  }
  mpl__mp_set_loop(&mp, 0);
  printf("XMplay: Module loaded ok\n");
  setparams(2, 44100, 16);
  m_bPlaying = 0;
  return 1;
}

//  This is the initialiser and module loader
//  This is a general call, which loads the module from the 
//  given address into the modplayer
//
//  It basically loads into an internal format, so once this function
//  has returned the buffer at 'data' will not be needed again.
int XM_Load(char *filename)
{
  int fd;
  unsigned char *ptr;
  long size;
  if ((fd = sceIoOpen(filename, PSP_O_RDONLY, 0777)) > 0) {
    //  opened file, so get size now
    size = sceIoLseek(fd, 0, PSP_SEEK_END);
    sceIoLseek(fd, 0, PSP_SEEK_SET);

    if (filedataptr != 0) {
      free(filedataptr);
      filedataptr = 0;
    }

    ptr = (unsigned char *) malloc(size + 8);
    if (ptr != 0) {		// Read file in
      filedataptr = ptr;
      memset(ptr, 0, size + 8);
      sceIoRead(fd, ptr, size);
      // ok, read the file in, now do our stuff
      return XM_InitTune(ptr,size);
    } else {
      printf("Error allocing\n");
      sceIoClose(fd);
      return 0;
    }
    // Close file
    sceIoClose(fd);
  } 
  m_bPlaying = 0;
  return 0;
}

// This function initialises for playing, and starts
int XM_Play()
{
  // See if I'm already playing
  if (m_bPlaying==1)
    return FALSE;

  m_bPlaying = 1;
  return TRUE;
}
void XM_Pause()
{
  if (m_bPlaying == 1)
    m_bPlaying = 0;
  else
    m_bPlaying = 1;
}
int XM_Stop()
{
  //stop playing
  m_bPlaying = 0;

  return TRUE;
}


