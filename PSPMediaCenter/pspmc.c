/*********************************************************************
 * 
 *  Main file for modplayer sample for PSP
 *  adresd 2005
 */
#include <kernel.h>
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <fileio.h>
#include <ctrl.h>
#include <audio.h>

#include "codec.h"
#include "audiolib.h"
#include "mp3/mp3player.h"
#include "ogg/oggplayer.h"
#include "mod/modplayer.h"

/* Define the module info section */
MODULE_INFO("PSPMC", 0x01010000)

/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf
codecStubs stubs[MAX_CODECS];
codecStubs *decoder;
static unsigned char banner[] = "PSP Media Center v1.0 by John_K & adresd\0";

/* Exit callback */
void exit_callback(void)
{
    sceKernelExitGame();
}

/* Callback thread */
void CallbackThread(void *arg)
{
    int cbid;
    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
    int thid = 0;
    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0)
	sceKernelStartThread(thid, 0, 0);
    return thid;
}


unsigned char *load_file(const char *filename, int *size)
{
    unsigned char *ptr = 0;
    int fileid;
    if ((fileid = sceIoOpen((char *) filename, O_RDONLY, 777)) > 0) {	//  opened file, so get size now
	long filelen;
	filelen = sceIoLseek(fileid, 0, SEEK_END);
	sceIoLseek(fileid, 0, SEEK_SET);
	ptr = (unsigned char *) malloc(filelen);
	if (ptr != 0) {		// Read file in
	    sceIoRead(fileid, ptr, filelen);
	} else
	    printf("Error allocing\n");
	// Close file
	sceIoClose(fileid);
    } else
	printf("Error opening file\n");
    return ptr;
}

void strcat2(char *dest, char *src)
{
    int pos = 0;
    int pos2 = 0;
    while (*(dest + pos2) != 0)
	pos2++;
    while (*(src + pos) != 0) {
	*(dest + pos2) = *(src + pos);
	pos++;
	pos2++;
    }
    *(dest + pos2) = 0;
}

void test_modplayer(char *rootpath, char *modname)
{
    unsigned char *dataptr;
    char filename[200];
    int size;
    u32 buttonsold;
    ctrl_data_t pad;
    int codec;
    int finished = 0;

    filename[0] = 0;
    strcat2(filename, rootpath);
    strcat2(filename, modname);

    pspDebugScreenClear();
    printf("%s\n\n", banner);
    printf("loading media File : %s\n", filename);

    //determine codec of the file
    for (codec = 0; codec <= MAX_CODECS; codec++)
	if (strncasecmp(&modname[strlen(modname) - 3], validExtensions[codec], 3) == 0) {
	    decoder = &stubs[codec];
	    break;
	}

    decoder->init(0);
    if (decoder->load(filename)) {
//    OGGPlay_DebugPrint();

	decoder->play();

	printf("\nPlaying\n\nX = Play.  O = Stop.  START = Tune Select.  SELECT = Exit.\n");

	sceCtrlReadBufferPositive(&pad, 1);
	buttonsold = pad.buttons;
	while (finished == 0) {
	    sceCtrlReadBufferPositive(&pad, 1);

	    if (pad.buttons != buttonsold) {
		if (pad.buttons & CTRL_CIRCLE)
		    decoder->stop();
		if (pad.buttons & CTRL_CROSS)
		    decoder->pause();
		if (pad.buttons & CTRL_START)
		    finished = 1;
		if (pad.buttons & CTRL_SELECT)
		    finished = 2;
		buttonsold = pad.buttons;
	    }
	}
	decoder->stop();
	decoder->end();
	if (finished == 2) {
	    AudioEnd();
	    sceKernelExitGame();
	}
    }
}

static io_dirent_t dirent;
static io_dirent_t dirent2;

char *mods_infoname[100];
int mods_infonum;
void fillmedialist(char *path)
{
    int dirid;
    int retval;
    int count = 0;
    if ((dirid = sceIoDopen(path)) > 0) {	//  Opened ok
	retval = 1;
	while ((retval > 0) && (count < 99)) {
	    retval = sceIoDread(dirid, (io_dirent_t *) & dirent);
	    if (retval > 0) {
		if (dirent.name[0] != '.' &&
		    (strncmp(&dirent.name[strlen(dirent.name) - 3],
			     "mp3", 3) == 0
		     || strncmp(&dirent.name[strlen(dirent.name) - 3],
				"ogg", 3) == 0 || strncmp(&dirent.name[strlen(dirent.name) - 3], "mod", 3) == 0)) {
		    mods_infoname[count] = (char *) malloc(200);
		    memcpy(mods_infoname[count], dirent.name, 200);
		    count++;
		}
	    }
	}
	sceIoDclose(dirid);
    }
    mods_infonum = count;
}

char *selectmod()
{
    ctrl_data_t pad;
    char *retptr;
    int highlight = 0;
    int finished = 0;
    int count;
    int x, y;
    u32 buttonsold = 0;
    printf("Select media to play:\n");
    // Save screen position
    x = pspDebugScreenGetX();
    y = pspDebugScreenGetY();

    sceCtrlReadBufferPositive(&pad, 1);
    buttonsold = pad.buttons;

    while (finished == 0) {	// Draw the menu firstly
	pspDebugScreenSetXY(x, y);
	for (count = 0; count < mods_infonum; count++) {
	    if (highlight == count)
		printf("%02d - %s <-\n", count, mods_infoname[count]);
	    else
		printf("%02d - %s   \n", count, mods_infoname[count]);
	}
	printf("Up/Down = Move cursor.  X = Select.  START/SELECT = Exit.\n");
	// Now read the keys and act appropriately
	sceCtrlReadBufferPositive(&pad, 1);

	if (buttonsold != pad.buttons) {
	    if (pad.buttons & CTRL_UP)
		if (highlight >= 1)
		    highlight--;
	    if (pad.buttons & CTRL_DOWN)
		if (highlight < (mods_infonum - 1))
		    highlight++;
	    if (pad.buttons & CTRL_CROSS)
		return mods_infoname[highlight];
	    if (pad.buttons & CTRL_START)
		return -1;
	    if (pad.buttons & CTRL_SELECT)
		return -1;
	}
	buttonsold = pad.buttons;
    }
}

void getproperpath(char *dest, char *src)
{
    int pos;
    int found;
    pos = 0;
    while (*(src + pos) != 0) {
	if (*(src + pos) == '/')
	    found = pos;
	*(dest + pos) = *(src + pos);
	pos++;
    }
    *(dest + found + 1) = 0;
}

/* main routine */
int main(int argc, char *argv[])
{
    char rootpath[200];
    char *modfile;

    pspDebugScreenInit();
    pspDebugScreenClear();
    sprintf(rootpath, "ms0:/PSP/MUSIC/");

    SetupCallbacks();

    // Setup Pad
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(0);

    //get codecStubs
    MP3setStubs(&stubs[CODEC_MP3]);
    OGGsetStubs(&stubs[CODEC_OGG]);
    MODsetStubs(&stubs[CODEC_MOD]);

    fillmedialist(rootpath);

    AudioInit();

    //  Loop around, offering a mod, till they cancel
    modfile = 1;
    while (modfile != -1) {
	// Setup screen  so it doesnt get messy
	pspDebugScreenClear();
	printf("%s\n\n", banner);
	printf("Rootpath: %s\n\n", rootpath);

	modfile = selectmod();
	if (modfile != -1) {
	    test_modplayer(rootpath, modfile);
	}
    }
    AudioEnd();
    sceKernelExitGame();

    // wait forever
    sceKernelSleepThread();
    return 0;
}
