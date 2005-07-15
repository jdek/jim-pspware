#include "psp.h"

int ExitCallback()
{
	// Cleanup the games resources etc (if required)
	Settings.Paused = TRUE;
	if (g_bROMLoaded) {
		save_config();
		Memory.SaveSRAM( S9xGetFilename("srm") );
	}

	if ( g_thread !=-1 ){
		Settings.ThreadSound = FALSE;
		sceKernelWaitThreadEnd( g_thread, NULL );
		sceKernelDeleteThread( g_thread );
	}

	S9xCloseSoundDevice();

	g_bLoop = false;

	scePowerSetClockFrequency(222,222,111);

	// Exit game
	sceKernelExitGame();

	return 0;
}

void PowerCallback(int unknown, int pwrflags)
{
	if(pwrflags & PSP_POWER_CB_POWER_SWITCH){
// mod by y
		if (g_bROMLoaded) {
			g_bSleep = true;
			scePowerSetClockFrequency(222,222,111);
			save_config();
			Memory.SaveSRAM( S9xGetFilename("srm") );
			S9xSetSoundMute( TRUE );
			pgWaitVn(180);
		}
//		if ( g_thread !=-1 ){
//			Settings.ThreadSound = FALSE;
//			sceKernelWaitThreadEnd( g_thread, NULL );
//			sceKernelDeleteThread( g_thread );
//		}
//		S9xCloseSoundDevice();
	}

	if(pwrflags & PSP_POWER_CB_BATTERY_LOW){
		scePowerSetClockFrequency(222,222,111);
		S9xSetInfoString( "PSP Battery is Low!" );
	}

	int cbid;
	cbid = sceKernelCreateCallback("Power Callback", (void *)PowerCallback, NULL);
	scePowerRegisterCallback(0, cbid);

}

// Thread to create the callbacks and then begin polling
int CallbackThread(int args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback( "Exit Callback", (void*)ExitCallback, NULL);
	sceKernelRegisterExitCallback( cbid );
	cbid = sceKernelCreateCallback( "Power Callback", (void*)PowerCallback, NULL);
	scePowerRegisterCallback( 0, cbid );

	sceKernelSleepThreadCB();
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks()
{
	int thid = 0;

	thid = sceKernelCreateThread( "update_thread", (SceKernelThreadEntry)CallbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0 );
	if( thid >= 0 ){
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

