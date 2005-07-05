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
    void XMsetStubs(codecStubs * stubs);

//private functions
    void XM_Init(int channel);
    int XM_Play();
    void XM_Pause();
    int XM_Stop();
    void XM_End();
    int XM_Load(char *filename);
    void XM_GetTimeString(char *dest);

#ifdef __cplusplus
}
#endif
#endif
