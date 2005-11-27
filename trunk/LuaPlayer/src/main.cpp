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

#ifndef LUAPLAYER_USERMODE
#include "sio.h"
#endif

/* the boot.lua */
#include "boot.cpp"

/* Define the module info section */
#ifdef LUAPLAYER_USERMODE
PSP_MODULE_INFO("LUAPLAYER", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
#else
PSP_MODULE_INFO("LUAPLAYER", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
PSP_MAIN_THREAD_STACK_SIZE_KB(32); /* smaller stack for kernel thread */
#endif

// startup path
char path[256];

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
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

#ifdef LUAPLAYER_USERMODE
int main(int argc, char** argv)
#else
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

int user_main(SceSize argc, void* argv)
#endif
{
	SetupCallbacks();
	tzset();

	// init modules
	initGraphics();
	initMikmod();

	// install new output handlers	
#ifndef LUAPLAYER_USERMODE
	pspDebugInstallStdoutHandler(debugOutput); 
	pspDebugInstallStderrHandler(debugOutput); 
#endif

	// execute Lua script (according to boot sequence)
#ifndef LUAPLAYER_USERMODE
	getcwd(path, 256);
#endif
	char* bootStringWith0 = (char*) malloc(size_bootString + 1);
	memcpy(bootStringWith0, bootString, size_bootString);
	bootString[size_bootString] = 0;
	while(1) { // reload on error
		chdir(path); // set base path luaplater/
		clearScreen(0);
		flipScreen();
		clearScreen(0);

		if (runScript(bootStringWith0, true))
#ifndef LUAPLAYER_USERMODE
		{
			debugOutput("Error: No script file found.\n", 29);
		}
		debugOutput("\nPress start to restart\n", 26);
#else
;
#endif
		
		SceCtrlData pad; int i;
		sceCtrlReadBufferPositive(&pad, 1); 
		for(i = 0; i < 40; i++) sceDisplayWaitVblankStart();
		while(!(pad.Buttons&PSP_CTRL_START)) sceCtrlReadBufferPositive(&pad, 1); 
		
#ifndef LUAPLAYER_USERMODE
		debugOutput(0,0);
#endif
		initGraphics();
	}
	free(bootStringWith0);
	
	// wait until user ends the program
	sceKernelSleepThread();

	return 0;
}

#ifndef LUAPLAYER_USERMODE
int main(void)
{
	getcwd(path, 256);
	int err = pspSdkLoadInetModules();
	if (err != 0) {
		pspDebugScreenInit();
		pspDebugScreenPrintf("pspSdkLoadInetModules failed with %x\n", err);
	        sceKernelDelayThread(5*1000000); // 5 sec to read error
	}

	// create user thread, tweek stack size here if necessary
	SceUID thid = sceKernelCreateThread("User Mode Thread", user_main,
	    0x11, // default priority
	    256 * 1024, // stack size (256KB is regular default)
	    PSP_THREAD_ATTR_USER, NULL);
	
	// start user thread, then wait for it to do everything else
	sceKernelStartThread(thid, 0, NULL);
	sceKernelWaitThreadEnd(thid, NULL);
	
	sceKernelExitGame();
	return 0;
}

__attribute__((constructor)) void stdoutInit() 
{ 
	pspKernelSetKernelPC();
	pspSdkInstallNoDeviceCheckPatch();
	pspSdkInstallNoPlainModuleCheckPatch();
	pspKernelSetKernelPC();
	pspKernelSetKernelPC();
	pspSdkInstallNoDeviceCheckPatch();
	pspDebugInstallKprintfHandler(NULL);
	registerSIODriver();

	// ignore startup messages from kernel, but install the tty driver in kernel mode
	pspDebugInstallStdoutHandler(nullOutput); 
} 
#endif

