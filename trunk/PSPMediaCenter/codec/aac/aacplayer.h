// aacplayer.h: headers for psp aacplayer code
//
// All public functions for aacplayer
//
//////////////////////////////////////////////////////////////////////
#ifndef _AACPLAYER_H_
#define _AACPLAYER_H_

#include <aacdec.h>
#include "../../codec.h"

#ifdef __cplusplus
extern "C" {
#endif

//  Function prototypes for public functions
    void AACsetStubs(codecStubs * stubs);

//private functions
    void AAC_Init(int channel);
    int AAC_Play();
    void AAC_Pause();
    int AAC_Stop();
    void AAC_End();
    int AAC_Load(char *filename);
    int AAC_EndOfStream();

#ifdef __cplusplus
}
#endif
#endif
