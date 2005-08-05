#include <stdlib.h>
#include <malloc.h>
#include <psputils.h>
#include "mikmod.h"

#include "sound.h"

#define MAX_MUSIC_CHAN 128
#define MAX_SFX_CHAN 32
static UNIMOD *musichandle = NULL;

//Internals. Implement this somewhere else to get error reporting lua-style.
void my_mikmod_error_handler(void)
{
	printf("LuaPlayer's Mikmod has a critical error:\n");
	printf("_mm_critical %d\n", _mm_critical);
	printf("_mm_errno %d\n", _mm_errno);
	printf("%s\n", _mm_errmsg[_mm_errno]);
	return;
}
extern void my_error_handler();

// For internal Lua use

void initMikmod() {
	_mm_RegisterErrorHandler(my_mikmod_error_handler);
	MikMod_RegisterAllLoaders();
	MikMod_RegisterAllDrivers();
	md_mode = DMODE_16BITS|DMODE_STEREO|DMODE_SOFT_SNDFX|DMODE_SOFT_MUSIC; 
	md_reverb = 0;
	md_pansep = 128;
	md_volume = 128;
	md_musicvolume = 96;
	md_sndfxvolume = 128;
	musichandle = NULL;
	MikMod_Init();
	MikMod_SetNumVoices(MAX_MUSIC_CHAN, MAX_SFX_CHAN);
	Player_Start(0);
}

void unloadMikmod() {
	if(musichandle) MikMod_FreeSong(musichandle);
	Player_Stop();
	MikMod_Exit();
}

// For external Lua use

void loadAndPlayMusicFile(char* filename, BOOL loop) {
	if(musichandle) stopAndUnloadMusic();
	
	musichandle = MikMod_LoadSong(filename, MAX_MUSIC_CHAN);
	musichandle->loop = loop;
	Player_Start(musichandle);
}

void stopAndUnloadMusic() {
	Player_Stop();
	MikMod_FreeSong(musichandle);
	musichandle = NULL;
}

Sound* loadSound(char* filename) {
	return WAV_LoadFN(filename);
}

void unloadSound(Sound* handle) {
	WAV_Free(handle);
}


Voice playSound(Sound* handle) {
	if(handle) { // Just for good measure, even if it's a precondition...
		Voice voice = MikMod_PlaySample(handle,0,0);
		setVoicePanning(voice, 127);
		setVoiceVolume(voice, 255);
		return voice;
	}
	return 0;
}


void stopSound(Voice handle) {
	Voice_Stop(handle);
}



void setVoiceVolume(Voice handle, UWORD vol) {
	if(vol>255) vol = 255;
	Voice_SetVolume(handle, vol);
}



void setVoicePanning(Voice handle, ULONG pan) {
	if(pan>255) pan = 255;
	Voice_SetPanning(handle, pan);
}


void setVoiceFrequency(Voice handle, ULONG freq) {
	if(freq>44000) freq = 44000;
	Voice_SetFrequency(handle, freq);
}

extern BOOL musicIsPlaying() {
	return Player_Active();
}

extern BOOL voiceIsPlaying(Voice handle) {
	return Voice_Stopped(handle);
}


extern void setMusicVolume(unsigned arg) {
	if(arg>128) arg = 128;
	md_musicvolume = arg;
}
extern void setSFXVolume(unsigned arg) {
	if(arg>128) arg = 128;
	md_sndfxvolume = arg;
}
extern void setReverb(unsigned arg) {
	if(arg>15) arg = 15;
	md_reverb = arg;
}
extern void setPanSep(unsigned arg) {
	if(arg>128) arg = 128;
	md_pansep = arg;
}
