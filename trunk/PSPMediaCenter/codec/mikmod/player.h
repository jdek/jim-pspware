// player.h: headers for psp MIKMOD player code
//
// All public functions for MIKMOD player
//
//////////////////////////////////////////////////////////////////////
#ifndef _MIKMODPLAYER_H_
#define _MIKMODPLAYER_H_

#include <mad.h>
#include "../../codec.h"

#ifdef __cplusplus
extern "C" {
#endif

//  Function prototypes for public functions
    void MIKMODsetStubs(codecStubs * stubs);

//private functions
    void MIKMOD_Init(int channel);
    int MIKMOD_Play();
    void MIKMOD_Pause();
    int MIKMOD_Stop();
    void MIKMOD_End();
    int MIKMOD_Load(char *filename);
    void MIKMOD_GetTimeString(char *dest);
    int MIKMOD_EndOfStream();

#ifdef __cplusplus
}
#endif
#endif
