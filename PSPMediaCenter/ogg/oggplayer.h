// modplayer.h: headers for psp modplayer code
//
// All public functions for modplayer
//
//////////////////////////////////////////////////////////////////////
#ifndef _OGGPLAYER_H_
#define _OGGPLAYER_H_

#include <ivorbiscodec.h>
#include <ivorbisfile.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Function prototypes for public functions
void OGGPlay_Init(int channel);
int OGGPlay_Play();
int OGGPlay_Stop();
void OGGPlay_End();
int OGGPlay_Load(char *filename);
void OGGPlay_Tick();
void OGGPlay_Close();

#ifdef __cplusplus
}
#endif

#endif
