#include <string>
#include <sstream>
#include "keys.h"
#include "sutils.h"
#include "hiscore.h"
#include "config.h"
#include "game.h"
extern SDL_Surface *Screen;		// global object
//-----------------------------------------------------------------------------
Game::Game(int players):
	fontM("data/font-white.bmp", 8, 14),
	sfxDropM("data/drop.wav"),
	sfxLineM("data/line.wav")
{
	playerCountM = players;
	wallpaperM = 0;
	if (!LoadImage(&wallpaperM, "data/back.bmp"))
		throw Exiter("Cannot load wallpaper image.");

	SetupGame();
	SetupLevel();
	Play();

	if (playerCountM == 1)
		CheckHiscore();
	else
		ShowVictory();
}
//-----------------------------------------------------------------------------
Game::~Game()
{
	if (wallpaperM)
		SDL_FreeSurface(wallpaperM);
}
//-----------------------------------------------------------------------------
void Game::playSound(tSound which)
{
	Sound *sfx;
	switch (which)
	{
		case tDrop:		sfx = &sfxDropM;		break;
		case tLine:		sfx = &sfxLineM;		break;
		default:
			sfx = 0;
	}

	if (sfx)
		sfx->Play();
}
//-----------------------------------------------------------------------------
void Game::SetupGame()
{
	playersM[0].SetupGame(this,  50, 50, 220,  50);
	playersM[1].SetupGame(this, 400, 50, 260, 285);
	currentLevelM = 0;
}
//-----------------------------------------------------------------------------
void Game::SetupLevel()
{
	currentLevelM++;
	playersM[0].SetupLevel();		// clean the fields
	playersM[1].SetupLevel();		// clean the fields
	if (playerCountM == 1)
	{
		rowsToGoM = currentLevelM * 5;

		char buffer[20];
		sprintf(buffer, "LEVEL %d", currentLevelM);
		TextEffect *t = new TextEffect(&fontM, 100, 180, std::string(buffer), 0);
		specialEffects.addEffect(t);

		int gameType = 0;
		config().getValue("game_type", gameType);
		if (gameType == 1)	// add trash
			playersM[0].AddBottomLines(currentLevelM%15);
	}
}
//-----------------------------------------------------------------------------
void Game::Play()
{
	while (true)			// check for single player
	{
		Uint32 startTime = SDL_GetTicks();
		bool ok = ProcessKeyboard() && playersM[0].Gravity() && (playerCountM == 1 || playersM[1].Gravity());
		RenderScene();
		if (!ok)
			return;

        while (startTime + 34 > SDL_GetTicks());	// target about 30 fps
	}
}
//-----------------------------------------------------------------------------
void Game::RenderScene(bool flip)
{
	PatternFill(wallpaperM, Screen);	// render background

	playersM[0].Render();				// render field & current brick
	if (playerCountM == 2)
		playersM[1].Render();			// render field & current brick
	else
	{
		const int xp = 220;
		const int yp = 380;
		SDL_Rect r;
		Uint32 yellow = SDL_MapRGB(Screen->format, 255, 255, 0);
		NjamSetRect(r, xp-1, yp-1, 120, 40);
		SDL_FillRect(Screen, &r, yellow);
		NjamSetRect(r, xp, yp, 118, 38);
		SDL_FillRect(Screen, &r, 0);
		char buffer[30];
		sprintf(buffer, "LEVEL:  %6d", currentLevelM);
		getFont()->WriteTextXY(Screen, xp+3, yp+5, buffer);
		sprintf(buffer, "LINES LEFT:%3d", rowsToGoM);
		getFont()->WriteTextXY(Screen, xp+3, yp+20, buffer);
	}

	specialEffects.Render();
	if (flip)
		SDL_Flip(Screen);
}
//-----------------------------------------------------------------------------
enum taAction { taLeft, taRight, taDown, taRotate, taDrop };
SDLKey getKey(int player, taAction action)
{
	std::string keys[5] = { "LEFT  ", "RIGHT ", "DOWN  ", "ROTATE", "DROP  " };
	std::stringstream ss;
	ss << "PLAYER" << (player+1) << " " << keys[(int)action];
	std::string kn = ss.str();
	int key;
	if (!config().getValue(kn, key))
		return SDLK_LAST;
	else
		return (SDLKey)key;
}
//-----------------------------------------------------------------------------
bool Game::ProcessKeyboard()
{
	SDLKey key;
	bool repeatKeys = true;

	// process single keys
    while ((key = kbdM.update(repeatKeys)) != SDLK_LAST)
    {
		repeatKeys = false;		// so we don't influence the repeat-delay

        if (key == SDLK_ESCAPE)
            return false;

        // pause
        if (key == SDLK_p)		// only server can pause the game
        {
            while (NjamGetch(false) != SDLK_LAST);	// wait to release the key

			TextEffect *t = new TextEffect(&fontM, 100, 220, std::string("PAUSED"), -1);
			specialEffects.addEffect(t);

            while (SDLK_p != NjamGetch(false))    	// wait to press the key again
				RenderScene();
			specialEffects.removeEffect(t);
        }

		for (int i=0; i<playerCountM; ++i)
		{
			if (key == getKey(i, taRotate))
				playersM[i].rotate(1);
			else if (key == getKey(i, taRight))
				playersM[i].move( 1, 0);
			else if (key == getKey(i, taLeft))
				playersM[i].move(-1, 0);
			else if (key == getKey(i, taDown))
				playersM[i].move( 0, 1);
			else if (key == getKey(i, taDrop))
			{
				kbdM.setKeyDown(getKey(i, taRight), false);
				kbdM.setKeyDown(getKey(i, taLeft), false);
				if (!playersM[i].drop())	// no more space
					return false;
			}
		}
    }

	// process sticky keys (those that can be held down)
	for (int i=0; i<playerCountM; ++i)
	{
		if (kbdM.isKeyDown(getKey(i, taDown)))
			playersM[i].move( 0, 1);
		if (kbdM.isKeyDown(getKey(i, taRight)))
			playersM[i].move( 1, 0);
		if (kbdM.isKeyDown(getKey(i, taLeft)))
			playersM[i].move(-1, 0);
	}

    return true;
}
//-----------------------------------------------------------------------------
void Game::CheckHiscore()
{
	int score = playersM[0].getScore();
	HiScores &hs = hiScores();
	if (!hs.canEnter(score))
	{
		SDL_Delay(600);
		Message("GAME OVER");
		return;
	}

	// ask for name
	char name[30];
	getName(name);
	hs.addHiscore(name, currentLevelM, score);
}
//-----------------------------------------------------------------------------
void Game::ShowVictory()
{
	Message("GAME OVER", false);
	SDL_Delay(600);
	while (NjamGetch(false) != SDLK_LAST);		// wait to release the key

	std::string p1, p2;
	p1 = (playersM[0].canPlay() ? "WINNER" : "LOSER");
	p2 = (playersM[1].canPlay() ? "WINNER" : "LOSER");

	TextEffect *t = new TextEffect(&fontM, 100, 220, p1, -1);
	specialEffects.addEffect(t);
	t = new TextEffect(&fontM, 450, 220, p2, -1);
	specialEffects.addEffect(t);

	while (SDLK_LAST == NjamGetch(false))    	// wait to press the key again
	{
		RenderScene(false);
		Message("GAME OVER", false);
	}
	specialEffects.removeEffect(t);
}
//-----------------------------------------------------------------------------
// all shapes are retrieved from here
int Game::getRandomShape()
{
	int gameType = 0;
	config().getValue("game_type", gameType);

	if (gameType == 1)			// challenge game type
	{
		int s1 = currentLevelM % 7;			// make sure the bricks aren't the same
		int s2 = (currentLevelM*2+3) % 7;
		if (s1 == s2)
			s2 = (s2+1)%7;
		if (s1 + s2 == 9)	// Z and S together... very hard. So, make it not happen.
			s2 = (s2+2)%7;
		if ((s1 == 4 || s2 == 4 || s1 == 5 || s2 == 5) && (s1 == 6 || s2 == 6))
			s1 = 1;
		return (NjamRandom(2) ? s1:s2);
	}
	else if (gameType == 0)		// classic game
		return NjamRandom(7);
	else 						// "bastet" game type
		return playersM[0].getWorstBrick();
}
//-----------------------------------------------------------------------------
//! returns false if level is complete
bool Game::linesDropped(Player *pl, int lines_dropped)		// update other player
{
	sfxLineM.Play(true);

	if (playerCountM == 1)
	{
		rowsToGoM -= lines_dropped;
		if (rowsToGoM < 1)			// level finished
		{
			pl->LevelComplete();
			SetupLevel();
			return false;
		}
	}
	else
	{
		if (lines_dropped > 1)
		{
			if (pl == &playersM[0])
				pl = &playersM[1];
			else
				pl = &playersM[0];
			pl->AddBottomLines(lines_dropped-1);
		}
	}
	return true;
}
//-----------------------------------------------------------------------------
void Game::getName(char *name)
{
	NjamFont *font = &fontM;
	Uint32 white = SDL_MapRGB(Screen->format, 255, 255, 255);
	Uint32 back = SDL_MapRGB(Screen->format, 100, 50, 0);
	Uint32 t = SDL_GetTicks();
	bool CursorOn = true;

	const int namelen = 19;
	const int yp = 220;

	int fieldw = font->GetCharWidth() * (namelen+2);
	int xp = (640 -  fieldw)/2;
	int textw = font->GetCharWidth() * namelen;
	int fieldh = (int)((double)font->GetCharHeight() * 3.5);
	int textx = xp+font->GetCharWidth();
	int texty = yp+font->GetCharHeight()*2;

	name[0] = '\0';
	while (true)
	{
		SDL_Rect dest;
		NjamSetRect(dest, xp, yp, fieldw, fieldh);
		SDL_FillRect(Screen, &dest, 0);
		NjamSetRect(dest, xp+1, yp+1, fieldw-2, fieldh-2);
		SDL_FillRect(Screen, &dest, back);
		font->WriteTextXY(Screen, textx, yp+font->GetCharHeight()/2, "*** NEW HISCORE ***");
		NjamSetRect(dest, textx, texty, textw, font->GetCharHeight()+1);
		SDL_FillRect(Screen, &dest, 0);
		font->WriteTextXY(Screen, textx+1, texty+1, name);

		int l = 0;	// get len
		while (name[l])
			l++;

		if (SDL_GetTicks() > t + 400)	// make the cursor blink
		{
			t = SDL_GetTicks();
			CursorOn = !CursorOn;
		}

		if (CursorOn)
		{
			NjamSetRect(dest, textx + font->GetCharWidth() * l + 2, texty+2, font->GetCharWidth()-2,
				font->GetCharHeight()-2);	// draw cursor
			SDL_FillRect(Screen, &dest, white);
		}

		SDL_Flip(Screen);

		SDLKey key = NjamGetch(false);
		char allowed[] = "0123456789abcdefghijklmnopqrstuvwxyz. :)(!*";
		bool ok = false;
		for (int k=0; allowed[k]; k++)
			if (allowed[k] == key)
				ok = true;

		if (ok && l < 20)
		{
			char c = key;
			if (c >= 'a' && c <= 'z')
				c -= ('a' - 'A');
			name[l] = c;
			name[l+1] = '\0';
		}

		if (key == SDLK_BACKSPACE && l > 0)			// backspace
			name[l-1] = '\0';

		if ((key == SDLK_KP_ENTER || key == SDLK_RETURN) && name[0])	// entered
			break;
	}
}
//-----------------------------------------------------------------------------
void Game::Message(const std::string& text, bool wait)
{
	if (wait)
		while (NjamGetch(false) != SDLK_LAST);	// clear the buffer

	int l = text.length();
	int w = fontM.GetCharWidth();

	// render background
	SDL_Rect dest;
	NjamSetRect(dest, (640-l*w-50)/2, 195, l*w+50, 55);
	SDL_FillRect(Screen, &dest, 0);
	Uint32 FillColor = SDL_MapRGB(Screen->format, 255, 200, 0);
	NjamSetRect(dest, (640-l*w-50)/2 + 1, 196, l*w+48, 53);
	SDL_FillRect(Screen, &dest, FillColor);

	NjamSetRect(dest, (640-l*w-30)/2, 205, l*w+30, 35);
	SDL_FillRect(Screen, &dest, 0);

	fontM.WriteTextXY(Screen, (640-l*w)/2, 215, text.c_str());
	SDL_Flip(Screen);

	if (wait)
		NjamGetch(true);
}
//-----------------------------------------------------------------------------
