#include "psp.h"

int ExitCallback (void)
{
	S9xSetSoundMute (TRUE);
	
	g_bLoop  = false;
	g_bSleep = true;
	
	// Cleanup the games resources etc (if required)
	Settings.Paused = TRUE;
	if (g_bROMLoaded) {
		save_config     ();
		Memory.SaveSRAM (S9xGetFilename ("srm"));
	}

	if (g_thread !=-1) {
		Settings.ThreadSound = FALSE;
		sceKernelWaitThreadEnd (g_thread, NULL);
		sceKernelDeleteThread  (g_thread);
	}

	S9xCloseSoundDevice ();

	scePowerSetClockFrequency (222, 222, 111);

	// Exit game
	sceKernelExitGame ();

	return 0;
}

void PowerCallback (int unknown, int pwrflags)
{
	if (pwrflags & PSP_POWER_CB_HOLD_SWITCH)
	{
		extern bool8 g_bShowProfilerIno;
		g_bShowProfilerInfo = (! g_bShowProfilerInfo);
	}
	
	if (pwrflags & PSP_POWER_CB_POWER_SWITCH){
		if (g_bROMLoaded) {
			scePowerSetClockFrequency (222, 222, 111);
			S9xSetSoundMute           (TRUE);
			g_bSleep = true;
			save_config     ();
			Memory.SaveSRAM (S9xGetFilename ("srm"));
		}
	} else if (pwrflags & PSP_POWER_CB_RESUME_COMPLETE) {
		g_bSleep = false;
	}

	if (pwrflags & PSP_POWER_CB_BATTERY_LOW){
		scePowerSetClockFrequency (222,222,111);
		S9xSetInfoString          ("WARNING: PSP Battery is Low! (Automatically Throttling CPU)");
	}

	int cbid;
	cbid = sceKernelCreateCallback ("Power Callback", (void *)PowerCallback, NULL);
	scePowerRegisterCallback       (0, cbid);
}


// Thread to create the callbacks and then begin polling
int CallbackThread (int args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback ("Exit Callback", (void *)ExitCallback, NULL);
	sceKernelRegisterExitCallback  (cbid);
	cbid = sceKernelCreateCallback ("Power Callback", (void *)PowerCallback, NULL);
	scePowerRegisterCallback       (0, cbid);

	sceKernelSleepThreadCB ();
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks (void)
{
	int thid = 0;

	thid = sceKernelCreateThread ("update_thread", (SceKernelThreadEntry)CallbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if (thid >= 0){
		sceKernelStartThread (thid, 0, 0);
	}

	return thid;
}

