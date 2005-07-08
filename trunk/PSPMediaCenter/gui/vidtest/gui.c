/*********************************************************************
 * 
 *  Main file for modplayer sample for PSP
 *  adresd 2005
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

#include "../../codec.h"
//  These are the headers for the different codecs
//  auto-generated by the makefile
#include "../../codecincs.h"

/* Define printf, just to make typing easier */
#define printf  pspDebugScreenPrintf

// Common externs
extern unsigned char banner[];
extern codecStubs stubs[100];
extern codecStubs *decoder;
extern int errno, __errno;
extern int codecnum;



unsigned char *load_file(const char *filename, long *size)
{
    unsigned char *ptr = 0;
    int fileid;
    if ((fileid = sceIoOpen((char *) filename, PSP_O_RDONLY, 777)) > 0) {	//  opened file, so get size now
	long filelen;
	filelen = sceIoLseek(fileid, 0, PSP_SEEK_END);
	sceIoLseek(fileid, 0, PSP_SEEK_SET);
	ptr = (unsigned char *) malloc(filelen);
	if (ptr != 0) {		// Read file in
	    sceIoRead(fileid, ptr, filelen);
	    *size = filelen;
	} else
	    printf("Error allocing\n");
	// Close file
	sceIoClose(fileid);
    } else
	printf("Error opening file\n");
    return ptr;
}


int frame;
void *g_vram_base, *g_vram_base_2;

unsigned char *getVramAddr()
{
    if (frame == 0)
	return g_vram_base;
    return g_vram_base_2;
}

void swapBuffer()
{
    if (frame == 0) {
	sceDisplaySetFrameBuf(g_vram_base, 512, 3, 1);
	frame = 1;
    } else {
	sceDisplaySetFrameBuf(g_vram_base_2, 512, 3, 1);
	frame = 0;
    }
}
int linePos = 0;
void fillvram(unsigned long color)
{
    unsigned long *vptr0;	//pointer to vram
    unsigned long i, j;

    vptr0 = getVramAddr();
    for (i = 0; i < 272; i++) {
	if (frame == 0) {	//skip a line
	    for (j = 0; j < 480; j++) {
		*vptr0 = 0;
		vptr0 += 1;
	    }
	    vptr0 += 512 - 480;
	}

	for (j = 0; j < 480; j++) {
	    *vptr0 = color;
	    vptr0 += 1;
	}
	vptr0 += 512 - 480;	//skip offscreen buffer

	//skip a line
	if (frame == 1) {
	    for (j = 0; j < 480; j++) {
		*vptr0 = 0;
		vptr0 += 1;
	    }
	    vptr0 += 512 - 480;
	}
    }

    //draw line
    vptr0 = getVramAddr();
    vptr0 += 512 * linePos;
    for (i = 0; i < 4; i++) {
	for (j = 0; j < 480; j++) {
	    *vptr0 = 0x00FFFFFF;
	    vptr0 += 1;
	}
	vptr0 += 512 - 480;
    }
    linePos++;
    //linePos = linePos % 272;
    if (linePos > 268)
	linePos = 0;
}


/* main routine */
int gui_main(void)
{
    int i, fd, size, xerr;
    char buffer[0x500000];
    char filename[] = "ms0:/xvid.avi";
    xvid_gbl_init_t xvidInit;
    xvid_dec_create_t xvidDec;
    xvid_dec_frame_t xvidFrame;

    memset(&xvidInit, 0, sizeof(xvid_gbl_init_t));
    memset(&xvidDec, 0, sizeof(xvid_dec_create_t));
    memset(&xvidFrame, 0, sizeof(xvid_dec_frame_t));


    g_vram_base = (void *) (0x40000000 | sceGeEdramGetAddr());
    g_vram_base_2 = (void *) (g_vram_base + 512 * 272 * 4);

    frame = 0;

    sceDisplaySetMode(0, 480, 272);
    sceDisplaySetFrameBuf(g_vram_base, 512, 3, 1);
    printf("starting pspmc\n");

    if ((fd = sceIoOpen(filename, O_RDONLY, 0777)) > 0) {
	//  opened file, so get size now
	size = sceIoLseek(fd, 0, SEEK_END);
	sceIoLseek(fd, 0, SEEK_SET);
	printf("read %d bytes into buffer\n", sceIoRead(fd, buffer, size));
	sceIoClose(fd);
    } else {
	sceKernelExitGame();
    }

    xvidInit.version = XVID_VERSION;
    printf("xvid init returned %d\n", xvid_global(NULL, XVID_GBL_INIT, &xvidInit, NULL));
    xvidDec.width = 480;
    xvidDec.height = 272;
    xvidDec.version = XVID_VERSION;
    printf("creating decoder\n");
    printf("xvid decore create returned %d\n", xvid_decore(NULL, XVID_DEC_CREATE, &xvidDec, NULL));
    printf("created decoder\n");
    xvidFrame.bitstream = buffer;
    xvidFrame.length = size;
    xvidFrame.output.csp = XVID_CSP_ABGR;
    xvidFrame.output.stride[0] = 512;

    //xvidFrame.stride = 512;
    //xvidFrame.colorspace = XVID_CSP_ABGR;

    i = 0;
    for (;;) {
	/*
	   if (frame == 0)
	   fillvram(0x00FF0000);
	   else
	   fillvram(0x00FF0000);
	 */
	//for (i = 0; i < 272; i+=1)
	//      memset(frame?g_vram_base:g_vram_base_2, i%2==0?0x7F:0x00, 720);
	xvidFrame.output.plane[0] = getVramAddr();
	if (i < 5) {
	    printf("xvid decore returned %d\n", xvid_decore(xvidDec.handle, XVID_DEC_DECODE, &xvidFrame, NULL));
	    i++;
	} else
	    xerr = xvid_decore(xvidDec.handle, XVID_DEC_DECODE, &xvidFrame, NULL);
	sceDisplayWaitVblankStart();
	swapBuffer();
    }
    return 0;
}
