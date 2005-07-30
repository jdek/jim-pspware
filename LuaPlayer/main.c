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
#include <stdlib.h>
#include <string.h>

#include "graphics.h"
#include "luaplayer.h"

/* Define the module info section */
PSP_MODULE_INFO("LUAPLAYER", 0x1000, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

/* Exit callback */
int exit_callback(void)
{
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

	// ignore startup messages from kernel, but install the tty driver in kernel mode
	pspDebugInstallStdoutHandler(nullOutput); 
} 

int main(int argc, char** argv)
{
	SetupCallbacks();

	// init graphics
	initGraphics();

	// install new output handlers	
	pspDebugInstallStdoutHandler(debugOutput); 
	pspDebugInstallStderrHandler(debugOutput); 

	// execute Lua script (current directory is where EBOOT.PBP is)
	runScript("script.lua");
	
	// wait until user ends the program
	sceKernelSleepThread();

	return 0;
}
