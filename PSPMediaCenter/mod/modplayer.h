// modplayer.h: headers for psp modplayer code
//
// All public functions for modplayer
//
//////////////////////////////////////////////////////////////////////
#ifndef _MODPLAYER_GENERAL_H
#define _MODPLAYER_GENERAL_H

#ifdef __cplusplus
extern "C" {
#endif

//  Function prototypes for public functions
void ModPlay_Init(int channel,unsigned char *data);
int ModPlay_Play();
int ModPlay_Stop();
void ModPlay_End();
void ModPlayer_Load(char *filename,char *data);
void ModPlay_Tick();
void ModPlay_Close();

#ifdef __cplusplus
}
#endif

#endif
