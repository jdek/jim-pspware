#ifndef AB_MUSIC_H
#define AB_MUSIC_H

#include "SDL_mixer.h"
//-----------------------------------------------------------------------------
class Music
{
public:
	enum Type { tSfx, tMusic };
private:
	Type typeM;
	Mix_Music *songM;

public:
	Music(char *filename, Type type = tMusic);
	~Music();

	void Play(bool doPlay = true, int loops = -1);		// -1 = forever
	void Stop();
};
//-----------------------------------------------------------------------------
#endif
