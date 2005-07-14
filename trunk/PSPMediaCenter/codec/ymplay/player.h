// player.h: headers for psp YM player code
//
// All public functions for YM player
//
//////////////////////////////////////////////////////////////////////
#ifndef _YMPLAYER_H_
#define _YMPLAYER_H_

#include <mad.h>
#include "../../codec.h"

#ifdef __cplusplus
extern "C" {
#endif

//  Function prototypes for public functions
    void YMPLAYsetStubs(codecStubs * stubs);

//private functions
    void YMPLAY_Init(int channel);
    int YMPLAY_Play();
    void YMPLAY_Pause();
    int YMPLAY_Stop();
    void YMPLAY_End();
    int YMPLAY_Load(char *filename);
    void YMPLAY_GetTimeString(char *dest);
    int YMPLAY_EndOfStream();

#ifdef __cplusplus
}
#endif
#endif
