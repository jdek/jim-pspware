#ifndef AB_GAME_H
#define AB_GAME_H

#include "keys.h"
#include "sutils.h"
#include "njamfont.h"
#include "effects.h"
#include "music.h"
#include "sound.h"
#include "player.h"
//-----------------------------------------------------------------------------
class Game
{
public:
	enum tSound { tLine, tDrop };
	void playSound(tSound which);

private:
	Keyboard kbdM;
	NjamFont fontM;
	SDL_Surface *wallpaperM;

	Sound sfxDropM;
	Sound sfxLineM;
	//Music sfxRotateM;

	int playerCountM;
	int currentLevelM;
	int rowsToGoM;
	Player playersM[2];

	void RenderScene(bool flip = true);
	void SetupGame();
	void SetupLevel();
	void Play();
	bool ProcessKeyboard();

	void CheckHiscore();
	void getName(char *name);
	void ShowVictory();
	void Message(const std::string& text, bool wait = true);
public:
	Effects specialEffects;

	Game(int players);
	~Game();

	int getRandomShape();
	bool linesDropped(Player *, int lines_dropped);		// update other player
	NjamFont *getFont() { return &fontM; };
};
//-----------------------------------------------------------------------------
#endif
