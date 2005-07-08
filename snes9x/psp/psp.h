#ifndef __PSP_H__
#define __PSP_H__

#include <pspkernel.h>
#include <pspctrl.h>
#include "port.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define VRAM_ADDR	(0x04000000)

#define SCREEN_WIDTH	480
#define SCREEN_HEIGHT	272

void sceDisplayWaitVblankStart();
void sceDisplaySetMode(long unknown, long width, long height);
void sceDisplaySetFrameBuf(char *topaddr,long linesize,long pixelsize,long);

enum { 
    TYPE_DIR=0x10, 
    TYPE_FILE=0x20 
}; 

struct dirent_tm {
	u16 unk[2]; //常にゼロ？
	u16 year;
	u16 mon;
	u16 mday;
	u16 hour;
	u16 min;
	u16 sec;
};

struct dirent {
    u32 unk0;
    u32 type;
    u32 size;
	struct dirent_tm ctime; //作成日時
	struct dirent_tm atime; //最終アクセス日時
	struct dirent_tm mtime; //最終更新日時
	u32 unk[7]; //常にゼロ？
    char name[0x108];
};

#define MAX_ENTRY 1024

#define POWER_CB_POWER		0x80000000 
#define POWER_CB_HOLDON		0x40000000 
#define POWER_CB_STANDBY	0x00080000 
#define POWER_CB_RESCOMP	0x00040000 
#define POWER_CB_RESUME		0x00020000 
#define POWER_CB_SUSPEND	0x00010000 
#define POWER_CB_EXT		0x00001000 
#define POWER_CB_BATLOW		0x00000100 
#define POWER_CB_BATTERY	0x00000080 
#define POWER_CB_BATTPOWER	0x0000007F 

long scePowerSetClockFrequency(long freq0, long freq1, long freq2);
void scePowerRegisterCallback( int unknown, int id );

int sceKernelWaitThreadEnd(int hthread, void *unk);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PSP_H__ */
