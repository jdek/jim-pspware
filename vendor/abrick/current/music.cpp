#include "music.h"
#include "config.h"
//-----------------------------------------------------------------------------
Music::Music(char *filename, Type type)
{
	typeM = type;
	songM = 0;
 	if (config().getValue("audio_available"))
	{
	   	printf("Loading song: %s ... ", filename);
	    songM = Mix_LoadMUS(filename);
	    if (songM)
			printf("Ok.\n");
		else
	    	printf("ERROR: Mix_LoadMUS(): %s\n", Mix_GetError());
	}
}
//-----------------------------------------------------------------------------
Music::~Music()
{
	if (songM)
		Mix_FreeMusic(songM);
}
//-----------------------------------------------------------------------------
void Music::Play(bool doPlay, int loops)
{
	if (!songM)
		return;

	bool canPlay = (typeM == tMusic && config().getValue("play_music") == true
		|| typeM == tSfx && config().getValue("play_sfx") == true);

	if (doPlay && canPlay)
	{
		if (Mix_PlayingMusic())
			printf("Warning, music is already playing!\n");
		if (Mix_PlayMusic(songM, loops)==-1)
			printf("Error, can't play music. %s", Mix_GetError());
	}
	else
	{
		if (!Mix_PlayingMusic())
			printf("Warning, music is not playing!\n");
		Mix_HaltMusic();
	}
}
//-----------------------------------------------------------------------------
void Music::Stop()
{
	Play(false);
}
//-----------------------------------------------------------------------------
