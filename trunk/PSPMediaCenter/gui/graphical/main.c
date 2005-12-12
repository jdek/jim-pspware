/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * Copyright (c) 2005 Jesper Svennevid
 */

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include <pspge.h>
#include <pspgu.h>

#include "texfont_psp.h"

PSP_MODULE_INFO("Blit Sample", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define printf	pspDebugScreenPrintf

#define SLICE_SIZE 64 // change this to experiment with different page-cache sizes

static unsigned int __attribute__((aligned(16))) list[262144];

int done = 0;
/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	done = 1;
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

static unsigned short __attribute__((aligned(16))) pixels[512*272];
extern unsigned char font_start[];

struct Vertex
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
};

// converts the image and uploads it to vram in one go
#define VRAM_OFFSET ((512*280*4)*3)	// 0x198000
static unsigned int vramaddr = 0;
unsigned char *convertimage(unsigned char *inptr,int size)
{
  // convert our raw image
  // saved as raw. no header .. interleaved and order RGB
  int x;
  unsigned char *input = inptr;
  unsigned char *output,*outptr;
  int tsize = size*size;
  if (vramaddr == 0)
    vramaddr = (0x40000000 | sceGeEdramGetAddr()) + VRAM_OFFSET;
  outptr = output = (unsigned char *)vramaddr;
  for (x=0;x<tsize;x++) {
    *(outptr++) = *(input++)/2;
    *(outptr++) = *(input++)/2;
    *(outptr++) = *(input++)/2;
    *(outptr++) = 0xff;
  }
  vramaddr += tsize * 4;
  if ((vramaddr & 0xff) != 0)
    vramaddr = (vramaddr & 0xffffff00) + 0x100;
  return output;
}


unsigned char *vram_pointer_font;
int main(int argc, char* argv[])
{
	unsigned int x,y;

	pspDebugScreenInit();
	SetupCallbacks();

	sceGuInit();

	// setup
	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_4444,(void*)0,512);
	sceGuDispBuffer(480,272,(void*)0x88000,512);
	sceGuDepthBuffer((void*)0x110000,512);
	sceGuOffset(2048 - (480/2),2048 - (272/2));
	sceGuViewport(2048,2048,480,272);
	sceGuDepthRange(0xc350,0x2710);
	sceGuScissor(0,0,480,272);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);

	int val = 0;

	// generate dummy image to blit

	for (y = 0; y < 272; ++y)
	{
		unsigned short* row = &pixels[y * 512];
		for (x = 0; x < 480; ++x)
		{
			row[x] = x * y;
		}
	}

	float curr_ms = 1.0f;
	struct timeval time_slices[16];

	vram_pointer_font = convertimage(font_start,256);			//	Upload the font texture to vram

	texfont_init();									//	Init texfont
	texfont_setimages(0,256,256,GU_PSM_8888,vram_pointer_font,0);	//	setup the font texture
  texfont_setmetric(20,32,13);
  texfont_setorigin(10,10);
	texfont_setimages(1,256,256,GU_PSM_8888,vram_pointer_font,0);	//	setup the background texture

	while (!done)
	{
		unsigned int j;
		struct Vertex* vertices;
    static unsigned int count=0;
    static char buffer[200];

		sceGuStart(GU_DIRECT,list);

  	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

		texfont_selectbgtex();
		//texfont_drawrect(100,0,200,100);

		texfont_selectfonttex();
		texfont_drawstring(0,0,"Hello");
		texfont_drawchar(0,1,"A");

    sprintf(buffer,"Count : %d",count);
    texfont_drawstring(1,2,buffer);
    count++;

		sceGuFinish();
		sceGuSync(0,0);

		sceDisplayWaitVblankStart();
		pspDebugScreenSetXY(0,0);
    pspDebugScreenPrintf("count : %d    ",count);
		sceGuSwapBuffers();
	}

	sceGuTerm();

	sceKernelExitGame();
	return 0;
}
