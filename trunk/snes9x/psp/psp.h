#ifndef __PSP_H__
#define __PSP_H__

#include <pspkernel.h>
#include <pspctrl.h>
#include "port.h"

//#include "gfx.h" // For the RGB macro to work properly

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define VRAM_ADDR	(0x04000000)

#define SCREEN_WIDTH	480
#define SCREEN_HEIGHT	272

void sceDisplayWaitVblankStart();
void sceDisplaySetMode(long unknown, long width, long height);
void sceDisplaySetFrameBuf(char *topaddr,long linesize,long pixelsize,long);

void S9xShutdownPSP (void);

#define MAX_ENTRY 1024

typedef struct
{
	char	vercnf[48];
	int		iSaveSlot;
	int		iSkipFrames;
	bool8	bShowFPS;
	bool8	bVSync;
	bool8	bSoundOff;
	int		iSoundRate;
	bool8	bTrans;
	int		iHBlankCycleDiv;
	int		iAPUTimerCycleDiv;
	bool8	bSwapAnalog;
	bool8	bSaveThumb;
	int		iPSP_ClockUp;
	int		iScreenSize;
	bool8	bAutoSkip;
	int		iBackgroundColor;
	int		iCompression;
	int		iSoundSync;
	bool8	bRunInDebugMode;
	bool8	bBilinearFilter;
	bool8	bUseGUBlit;
	bool8	bSupportHiRes;
	int		iAltSampleDecode;
} PSPSETTINGS;

extern PSPSETTINGS PSP_Settings;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PSP_H__ */
