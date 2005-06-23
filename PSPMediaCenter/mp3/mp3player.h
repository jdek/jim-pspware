// modplayer.h: headers for psp modplayer code
//
// All public functions for modplayer
//
//////////////////////////////////////////////////////////////////////
#ifndef _MP3PLAYER_H_
#define _MP3PLAYER_H_
#include <mad.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Function prototypes for public functions
void MP3Play_Init(int channel);
int MP3Play_Play();
int MP3Play_Stop();
void MP3Play_End();
int MP3Play_Load(char *filename);
void MP3Play_Tick();
void MP3Play_Close();

#ifdef __cplusplus
}
#endif

#endif
