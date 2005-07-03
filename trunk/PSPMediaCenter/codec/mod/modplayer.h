// modplayer.h: headers for psp modplayer code
//
// All public functions for modplayer
//
//////////////////////////////////////////////////////////////////////
#ifndef _MODPLAYER_GENERAL_H
#define _MODPLAYER_GENERAL_H

#include "../../codec.h"

#ifdef __cplusplus
extern "C" {
#endif

//  Function prototypes for public functions
    void MODsetStubs(codecStubs * stubs);

//  Function prototypes for private functions
    void Mod_Init(int channel);
    int Mod_Play();
    void Mod_Pause();
    int Mod_Stop();
    void Mod_End();
    int Mod_Load(char *filename);
    void Mod_Tick();
    void Mod_Close();

#ifdef __cplusplus
}
#endif
#endif
