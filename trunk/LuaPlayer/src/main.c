/*
 * Lua Player for PSP
 * ------------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE for details.
 *
 * Copyright (c) 2005 Frank Buss <fb@frank-buss.de> (aka Shine)
 *
 * Credits:
 *   many thanks to the authors of the PSPSDK from http://forums.ps2dev.org
 *   and to the hints and discussions from #pspdev on freenode.net
 *
 * $Id$
 */

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <psputility.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "graphics.h"
#include "sound.h"
#include "luaplayer.h"
#include "sio.h"

/* the boot.lua */
#include "boot.c"

/* Define the module info section */
PSP_MODULE_INFO("LUAPLAYER", 0x1000, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

/* Exit callback */
int exit_callback(void)
{

// Unload modules
	unloadMikmod();
		
	sceKernelExitGame();
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", (void *) exit_callback, NULL);
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

int nullOutput(const char *buff, int size)
{
	return size;
}

int debugOutput(const char *buff, int size)
{
	static int debugInitialized = 0;
	if(!buff) {
		debugInitialized = 0;
		return 0;
	}
	if (!debugInitialized) {
		disableGraphics();
		pspDebugScreenInit();
		debugInitialized = 1;
	}
	return pspDebugScreenPrintData(buff, size);
}

__attribute__((constructor)) void stdoutInit() 
{ 
	pspKernelSetKernelPC();
	pspSdkInstallNoDeviceCheckPatch();
	pspDebugInstallKprintfHandler(NULL);
	registerSIODriver();

	// ignore startup messages from kernel, but install the tty driver in kernel mode
	pspDebugInstallStdoutHandler(nullOutput); 
} 

int main(int argc, char** argv)
{
	SetupCallbacks();
	tzset();

	// init modules
	initGraphics();
	initMikmod();

	// install new output handlers	
	pspDebugInstallStdoutHandler(debugOutput); 
	pspDebugInstallStderrHandler(debugOutput); 

	// execute Lua script (according to boot sequence)
	char path[256];
	getcwd(path, 256);
	char* bootStringWith0 = (char*) malloc(size_bootString + 1);
	memcpy(bootStringWith0, bootString, size_bootString);
	bootString[size_bootString] = 0;
	while(1) { // reload on error
		clearScreen(0);
		flipScreen();
		clearScreen(0);

		if (runScript(bootStringWith0, true))
			debugOutput("Error: No script file found.\n", 29);
		
		debugOutput("\nPress start to restart\n", 26);
		SceCtrlData pad; int i;
		sceCtrlReadBufferPositive(&pad, 1); 
		for(i = 0; i < 40; i++) sceDisplayWaitVblankStart();
		while(!(pad.Buttons&PSP_CTRL_START)) sceCtrlReadBufferPositive(&pad, 1); 
		
		chdir(path); // set base path luaplater/
		debugOutput(0,0);
		initGraphics();
	}
	free(bootStringWith0);
	
	// wait until user ends the program
	sceKernelSleepThread();

	return 0;
}
