#ifndef AB_SOUND_H
#define AB_SOUND_H

#include "SDL_mixer.h"
//-----------------------------------------------------------------------------
class Sound
{
private:
	int channelM;
	Mix_Chunk *chunkM;

public:
	Sound(char *filename);
	~Sound();

	void Play(bool doPlay = true, int loops = 0);		// 0 = play once
	void Stop();
};
//-----------------------------------------------------------------------------
#endif
