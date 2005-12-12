/*********************************************************************
* 
*  Graphical GUI for PSP Media Center
*  John_K 2005
*/
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspaudio.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>
#include <pspmoduleinfo.h>
#include <pspaudiolib.h>
#include <psphprm.h>
#include <pspgu.h>
#include <pspgum.h>
#include "texfont_psp.h"

/*
typedef struct {
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
} font_vertex;
*/

#include "../../codec.h"

//  These are the headers for the different codecs
//  auto-generated by the makefile
#include "../../codecincs.h"

/* Define printf, just to make typing easier */
#define printf  pspDebugScreenPrintf

// Common externs
extern unsigned char banner[];
extern codecStubs stubs[100];
extern int codecnum;

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4) /* change this if you change to another screenmode */
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2) /* zbuffer seems to be 16-bit? */

static unsigned int __attribute__((aligned(16))) list[262144];
u8 *fontTex;// __attribute__((aligned(16)));
u32 clut4[256] __attribute__((aligned(16)));
	
void setupGU() {
	// setup GU

	sceGuInit();

	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_8888,(void*)0,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)0x88000,BUF_WIDTH);
	sceGuDepthBuffer((void*)0x110000,BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(0xc350,0x2710);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuAlphaFunc(GU_GREATER,0,0xff);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuFinish();
	sceGuSync(0,0);
	
	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
}
void drawQuad(short x, short y, short width, short height, short colorUL, short colorUR, short colorBR, short colorBL) {
	font_vertex* vertices = (font_vertex *)sceGuGetMemory(4 * sizeof(font_vertex));
	
	vertices[0].u = vertices[0].v = 0;
	vertices[0].color = colorUL;
	vertices[0].x = x; vertices[0].y = y; vertices[0].z = 0;
	
	vertices[1].u = vertices[1].v = 0;
	vertices[1].color = colorUR;
	vertices[1].x = x+width; vertices[1].y = y; vertices[1].z = 0;
	
	vertices[2].u = vertices[2].v = 0;
	vertices[2].color = colorBL;
	vertices[2].x = x; vertices[2].y = y+height; vertices[2].z = 0;
	
	vertices[3].u = vertices[3].v = 0;
	vertices[3].color = colorBR;
	vertices[3].x = x+width; vertices[3].y = y+height; vertices[3].z = 0;
	sceGuDrawArray(GU_TRIANGLE_STRIP,GU_TEXTURE_16BIT|GU_COLOR_4444|GU_VERTEX_16BIT|GU_TRANSFORM_2D,4,0, vertices);
}

void drawFontTex(int x, int y, int width, int height) {
	font_vertex* vertices = (font_vertex *)sceGuGetMemory(4 * sizeof(font_vertex));
	/*
	sceGuClutMode(GU_PSM_8888,0,0xff,0); // 32-bit palette
	sceGuClutLoad(16,clut4); // upload 16*8 entries (256)
	sceGuTexMode(GU_PSM_T8,0,0,1); // 4-bit image
	sceGuTexImage(0,256,256,256,fontTex);
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGB);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuTexScale(1.0f,1.0f);
	sceGuTexOffset(0.0f,0.0f);
	sceGuAmbientColor(0);
	*/
	sceGuTexMode(GU_PSM_8888,0,0,0);
	sceGuTexImage(0,256,256,256,fontTex);
	sceGuTexFunc(GU_TFX_ADD,GU_TCC_RGB);
	sceGuTexEnvColor(0xffff00);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuTexScale(1.0f/256.0f,1.0f/256.0f);
	sceGuTexOffset(0.0f,0.0f);
	sceGuAmbientColor(0xffffffff);
		
	vertices[0].u = vertices[0].v = 0;
	vertices[0].color = 0x9999;
	vertices[0].x = x; vertices[0].y = y; vertices[0].z = 0;
	
	vertices[1].u = 1; vertices[1].v = 0;
	vertices[1].color = 0x9999;
	vertices[1].x = x+width; vertices[1].y = y; vertices[1].z = 0;
	
	vertices[2].u = 0; vertices[2].v = 1;
	vertices[2].color = 0x9999;
	vertices[2].x = x; vertices[2].y = y+height; vertices[2].z = 0;
	
	vertices[3].u = vertices[3].v = 1;
	vertices[3].color = 0x9999;
	vertices[3].x = x+width; vertices[3].y = y+height; vertices[3].z = 0;
	
	sceGuDrawArray(GU_TRIANGLE_STRIP,GU_TEXTURE_16BIT|GU_COLOR_4444|GU_VERTEX_16BIT|GU_TRANSFORM_2D,4,0, vertices);
}

void drawBaseBackground() {
   drawQuad(0,   0, 480,  20, 0xF333, 0xF333, 0xF333, 0xF333);
   drawQuad(0,  20, 480, 232, 0xF888, 0xF888, 0xFDDD, 0xFDDD);
   drawQuad(0, 252, 480,   7, 0xF841, 0xF841, 0xFC61, 0xFC61);
   drawQuad(0, 259, 480,  13, 0xFC61, 0xFC61, 0xF310, 0xF310);
}

void drawUI() {
   sceGuStart(GU_DIRECT,list);
   // clear screen
   sceGuClearColor(0xFF00FF00);
   sceGuClearDepth(0);
   sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
   sceGuDisable(GU_TEXTURE_2D);

   drawBaseBackground();
   
   drawFontTex(20,10,256,256);
   
   //finish
   sceGuFinish();
   sceGuSync(0,0);

   sceDisplayWaitVblankStart();
   sceGuSwapBuffers();
}

/* main routine */
int gui_main(void)
{
	char path[256];
	short fontWidths[256];
	int fd, size;
	
	pspDebugScreenInit();
	setupGU();
	getcwd(path, 256);
	
	//load font texture
	strcat(path, "/font32.raw");
	fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	size = sceIoLseek32(fd, 0, SEEK_END);
	sceIoLseek32(fd, 0, SEEK_SET);
	if (fd > 0) {
		fontTex = (u8 *)memalign(16, size);
		if (sceIoRead(fd, fontTex, size) != size)
			fprintf(stdout, "Did not read expected size for texture.\r\n");
		//if (sceIoRead(fd, clut4, 16*16) != 16*16)
		//	fprintf(stdout, "Did not read expected size for CLUT.\r\n");
		sceIoClose(fd);
	} else {
		fprintf(stdout, "Could not open file '%s' for reading\r\n", path);
	}
	//fprintf(stdout, "0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\r\n", clut4[0], clut4[1], clut4[2], clut4[3], clut4[4], clut4[5], clut4[6], clut4[7]);
	//load font size
	memcpy(&path[strlen(path)-3], "dat", 3);
	fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	size = sceIoLseek32(fd, 0, SEEK_END);
	sceIoLseek32(fd, 0, SEEK_SET);
	if (fd > 0) {
		if(sceIoRead(fd, fontWidths, size) != size)
			fprintf(stdout, "Did not read expected size.\r\n");
		sceIoClose(fd);
	} else {
		fprintf(stdout, "Could not open file '%s' for reading\r\n", path);
	}
	//fprintf(stdout, "%d %d %d\r\n", fontWidths[0], fontWidths[1], fontWidths[2]);
	while(1) {
		drawUI();
	}
}
