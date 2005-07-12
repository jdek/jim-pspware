// xmplayer.h: headers for psp XM player code
//
// All public functions for xmplayer
//
//////////////////////////////////////////////////////////////////////
#ifndef _XMPLAYER_H_
#define _XMPLAYER_H_

#include "../../codec.h"

#ifdef __cplusplus
extern "C" {
#endif

//  Function prototypes for public functions
    void XMPLAYsetStubs(codecStubs * stubs);

//private functions
    void XMPLAY_Init(int channel);
    int XMPLAY_Play();
    void XMPLAY_Pause();
    int XMPLAY_Stop();
    void XMPLAY_End();
    int XMPLAY_Load(char *filename);
    void XMPLAY_GetTimeString(char *dest);

#ifdef __cplusplus
}
#endif
#endif
