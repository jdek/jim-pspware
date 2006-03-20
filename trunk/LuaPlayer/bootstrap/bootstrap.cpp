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
 * $Id: main.cpp 290 2005-12-03 08:49:10Z shine $
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
#include <stdio.h>

#include "sio.h"

/* Define the module info section */
PSP_MODULE_INFO("LUABOOT", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
PSP_MAIN_THREAD_STACK_SIZE_KB(32); 
PSP_HEAP_SIZE_KB(32);

// startup path
char path[256];

int debugOutput(const char *buff, int size)
{
	static int debugInitialized = 0;
	if(!buff) {
		debugInitialized = 0;
		return 0;
	}
	if (!debugInitialized) {
		//disableGraphics();
		pspDebugScreenInit();
		debugInitialized = 1;
	}
	return pspDebugScreenPrintData(buff, size);
}

int loadModule( char * moduleLocation )
{
	char path[256];	
	getcwd(path, 256);
	strcat(path, moduleLocation );

	int retVal = sceKernelLoadModule( path, 0, NULL );
	if (retVal < 0)
	{
		debugOutput("Error: loadModule luaplayer.prx\n", 33);		
		debugOutput( path, strlen(path) );
		return retVal;
	}

	int fd;
	retVal = sceKernelStartModule( retVal, strlen(path)+1, path, &fd, NULL );
	if ( retVal < 0 )
	{
		debugOutput("Error: strtModule luaplayer.prx\n", 33);		
		return retVal;
	}

	return 0;
}

int main( int argc, void **argp )
{
	pspSdkInstallNoDeviceCheckPatch();
	pspSdkInstallNoPlainModuleCheckPatch();

	getcwd(path, 256);
	int err = pspSdkLoadInetModules();
	if (err != 0) {
		pspDebugScreenInit();
		pspDebugScreenPrintf("pspSdkLoadInetModules failed with %x\n", err);
	        sceKernelDelayThread(5*1000000); // 5 sec to read error
	}

	int retVal = loadModule( "/luaplayer.prx" );
	if (retVal < 0)
	{
		debugOutput("Error: failed loadModule luaplayer.prx\n", 31);
		sceKernelSleepThread();
	}
	
	sceKernelSleepThread();
	return 0;
}
/*
__attribute__((constructor)) void stdoutInit() 
{ 
	//pspKernelSetKernelPC();
	pspSdkInstallNoDeviceCheckPatch();
	pspSdkInstallNoPlainModuleCheckPatch();
	//pspKernelSetKernelPC();
	//pspKernelSetKernelPC();
	//pspSdkInstallNoDeviceCheckPatch();
	//pspDebugInstallKprintfHandler(NULL);
	registerSIODriver();

	// ignore startup messages from kernel, but install the tty driver in kernel mode
	//pspDebugInstallStdoutHandler(nullOutput); 
} 
*/
