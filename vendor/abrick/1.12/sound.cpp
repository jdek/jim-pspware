#include "config.h"
#include "sound.h"
//-----------------------------------------------------------------------------
Sound::Sound(char *filename)
{
	channelM = -1;
	chunkM = 0;
 	if (config().getValue("audio_available"))
	{
	   	printf("Loading sfx: %s ... ", filename);
	    chunkM = Mix_LoadWAV(filename);
	    if (chunkM)
			printf("Ok.\n");
		else
	    	printf("ERROR: Mix_LoadWAV(): %s\n", Mix_GetError());
	}
}
//-----------------------------------------------------------------------------
Sound::~Sound()
{
	if (chunkM)
		Mix_FreeChunk(chunkM);
}
//-----------------------------------------------------------------------------
void Sound::Play(bool doPlay, int loops)
{
	if (!chunkM)
		return;

	if (doPlay && config().getValue("play_sfx") == true)
	{
		channelM = Mix_PlayChannel(-1, chunkM, loops);
		if (channelM == -1)
			printf("Error, can't play Sound. %s", Mix_GetError());
	}
	else
	{
		if (channelM == -1)
			return;
		if (Mix_Playing(channelM))
			Mix_HaltChannel(channelM);	// stop playing
	}
}
//-----------------------------------------------------------------------------
void Sound::Stop()
{
	Play(false);
}
//-----------------------------------------------------------------------------
