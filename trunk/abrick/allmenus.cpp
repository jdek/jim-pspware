#include <sstream>
#include "game.h"
#include "menu.h"
#include "config.h"
#include "hiscore.h"
#include "allmenus.h"

extern SDL_Surface *Screen;
//-----------------------------------------------------------------------------
// MAIN MENU
//-----------------------------------------------------------------------------
MainMenu::MainMenu(NjamFont *font_ptr, int x, int y):
	Menu(font_ptr, x, y),
	whiteFontM("data/font-white.bmp", 8, 14),
	musicM("data/menu.xm"),
	gameMusicM("data/main.xm")
{
	options.push_back("SINGLE PLAYER GAME");
	options.push_back("DUEL GAME");
	options.push_back("OPTIONS");
	options.push_back("EXIT");

 	if (config().getValue("play_music"))
		musicM.Play();
}
//-----------------------------------------------------------------------------
bool MainMenu::onEnter()
{
	if (indicator < 2)
	{
		musicM.Stop();
		gameMusicM.Play();

		if (indicator == 0)		// single player game, select game type
		{
			SelectGameTypeMenu sgt(font, 200, 170);
			sgt.start();
		}
		else					// duel game (2 players)
		{
			int gameType = 0;
			config().getValue("game_type", gameType);
			config().setValue("game_type", 0);			// CLASSIC rules for duel game
			Game g(2);
			config().setValue("game_type", gameType);	// revert to old value
		}

		gameMusicM.Stop();
		musicM.Play();
	}
	else if (indicator == 2)		// options
	{
		OptionsMenu o(font, 120, 200, &musicM);
		o.start();
	}
	else									// exit
		return false;
	return true;
}
//-----------------------------------------------------------------------------
void MainMenu::render()
{
	SDL_Rect r;
	Uint32 color = SDL_MapRGB(Screen->format, 128, 0, 255);

	// render hiscore list
	const int hsx = 320;
	const int hsy = 100;

	NjamSetRect(r, hsx, hsy, 38*whiteFontM.GetCharWidth(), 18*whiteFontM.GetCharHeight()+font->GetCharWidth());
	SDL_FillRect(Screen, &r, 0);
	Box(Screen, r, 1, 150, 150, 100);
	font->WriteTextXY(Screen, hsx+80, hsy+4, "TOP 15 PLAYERS");
	whiteFontM.WriteTextXY(Screen, hsx+5, hsy+2*whiteFontM.GetCharHeight(), "   NAME               LEVEL   POINTS");
	NjamSetRect(r, hsx+5, hsy+3*whiteFontM.GetCharHeight(), 36*whiteFontM.GetCharWidth(), 1);
	Box(Screen, r, 1, 150, 150, 100);

	HiScores &hs = hiScores();
	int i=1;
	for (HiScores::iterator it = hs.begin(); it != hs.end() && i<=15; ++it, ++i)
	{
		char buffer[80];
		sprintf(buffer, "%2d %-20s %3d %8d", i, (*it).name.c_str(), (*it).level, (*it).points);
		whiteFontM.WriteTextXY(Screen, hsx+5, hsy+(i+2)*whiteFontM.GetCharHeight(), buffer);
	}

	NjamSetRect(r, 0, 30, 639, 20);
	SDL_FillRect(Screen, &r, color);
	NjamSetRect(r, 0, 31, 639, 18);
	SDL_FillRect(Screen, &r, 0);

	effectsM.Render();

	static int counter = -1;
	counter++;

	std::string text[] = {
		"MILAN BABUSKOV PRESENTS",
		"ABANDONED BRICKS",
		"VERSION " AB_VERSION,
		"MUSIC BY LIFESOUND",
#ifdef PSP
		"PSP PORT BY RINCO",
#endif
		"VISIT   HTTP://ABRICK.SOURCEFORGE.NET   ONLINE",
#ifdef PSP
		"USE DPAD TO MOVE AND CIRCLE TO ROTATE THE BRICK",
		"USE DOWN ARROW TO MOVE IT DOWN FASTER",
		"USE CROSS TO DROP IT ON THE FLOOR",
#else
		"USE ARROW KEYS TO MOVE AND ROTATE THE BRICK",
		"USE DOWN ARROW TO MOVE IT DOWN FASTER",
		"USE SPACE TO DROP IT ON THE FLOOR",
#endif
		"GAME SPEEDS UP WHEN LINES ARE DROPPED",
		"TRY DROPPING AS MANY LINES YOU CAN AT ONCE",
		"HAVE FUN PLAYING",
		"ABANDONED BRICKS IS A FREE GAME",
		"HOWEVER, IF YOU WISH TO SUPPORT THE DEVELOPMENT YOU CAN SEND SOME MONEY",
		"10 USD OR EURO IS QUITE ENOUGH",
		"CONTACT ME AT EMAIL:  MBABUSKOV@YAHOO.COM  FOR DETAILS",
		" "
	};
	int items = sizeof(text)/sizeof(std::string);
	const int speed = 210;

	if (counter > speed*items-1)
		counter = 0;
	if (counter % speed != 0)
		return;

	std::string txt = text[counter/speed];
	int p = (640 - txt.length() * whiteFontM.GetCharWidth()) / 2;

	SpecialEffect *t = new ScrollEffect(&whiteFontM, p, 32, txt);
	effectsM.addEffect(t);
}
//-----------------------------------------------------------------------------
// OPTIONS MENU
//-----------------------------------------------------------------------------
OptionsMenu::OptionsMenu(NjamFont *font_ptr, int x, int y, Music *mmm)
	: Menu(font_ptr, x, y)
{
	mmmM = mmm;
	bool sfx = config().getValue("play_sfx");
	bool music = config().getValue("play_music");
	std::string dropMarker("LINE");
	config().getValue("drop_marker", dropMarker);

	options.push_back(sfx 			? "SOUND ON" 		: "SOUND OFF");
	options.push_back(music 		? "MUSIC ON" 		: "MUSIC OFF");
	options.push_back("DROP MARKER: " + dropMarker);
	options.push_back("CONFIGURE KEYBOARD");
	options.push_back("BACK");
}
//-----------------------------------------------------------------------------
bool OptionsMenu::onEnter()
{
	if (indicator == 3)
	{
		KeyboardMenu k(font, 270, 220);
		k.start(27);
		return true;	// stay in options menu
	}

	bool sfx, music;
	std::string dropMarker("LINE");
	switch (indicator)
	{
		case 0:
			sfx = !config().getValue("play_sfx");
			options[0] = (sfx ? "SOUND ON" : "SOUND OFF");
			config().setValue("play_sfx", sfx);
			break;
		case 1:
			music = !config().getValue("play_music");
			options[1] = (music ? "MUSIC ON" : "MUSIC OFF");
			config().setValue("play_music", music);
			mmmM->Play(music);
			break;
		case 2:
			config().getValue("drop_marker", dropMarker);
			if (dropMarker == "OFF")
				dropMarker = "LINE";
			else if (dropMarker == "SHADOW")
				dropMarker = "OFF";
			else
				dropMarker = "SHADOW";
			options[2] = "DROP MARKER: " + dropMarker;
			config().setValue("drop_marker", dropMarker);
			break;
		default:
			return false;
	}
	return true;		// stay in menu
};
//-----------------------------------------------------------------------------
KeyboardMenu::KeyboardMenu(NjamFont *font_ptr, int x, int y):
	Menu(font_ptr, x, y)
{
	std::string keys[5] = { "LEFT  ", "RIGHT ", "DOWN  ", "ROTATE", "DROP  " };
	for (int ply = 0; ply < 2; ++ply)
	{
		for (int i=0; i<5; ++i)
		{
			std::stringstream ss;
			ss << "PLAYER" << (ply+1) << " " << keys[i];

			int key;
			if (!config().getValue(ss.str(), key))
				key = 0;
			std::string keyname = KeyNames::keyNamesProvider().getKeyName(key);
			ss << ":" << keyname;
			options.push_back(ss.str());
		}
	}

	options.push_back("DONE");
}
//-----------------------------------------------------------------------------
bool KeyboardMenu::onEnter()
{
	if (indicator == 10)
		return false;

	int ply = indicator/5;
	int key = indicator%5;
	std::string keys[5] = { "LEFT  ", "RIGHT ", "DOWN  ", "ROTATE", "DROP  " };
	std::stringstream ss;
	ss << "PLAYER" << (ply+1) << " " << keys[key];
	std::string kn = ss.str();

	// ask user to press new key
	SDLKey k = Message("PRESS KEY FOR " + kn);
	if (k == SDLK_ESCAPE)
		return true;

	// assing key to action via config().setValue()
	config().setValue(kn, k);

	// change menu item's text
	std::string keyname = KeyNames::keyNamesProvider().getKeyName(k);
	kn += ":" + keyname;
	options[indicator] = kn;
	return true;
}
//-----------------------------------------------------------------------------
// GAME TYPE MENU
//-----------------------------------------------------------------------------
SelectGameTypeMenu::SelectGameTypeMenu(NjamFont *font_ptr, int x, int y)
	: Menu(font_ptr, x, y)
{
	options.push_back("SELECT GAME TYPE");
	options.push_back("CLASSIC");
	options.push_back("CHALLENGE");		// only 2 brick types per level + trash at bottom
	options.push_back("BASTET");
	//options.push_back("BACK");

	config().getValue("game_type", indicator);	// previous selection
	firstIsTitleM = true;
	indicator++;
}
//-----------------------------------------------------------------------------
bool SelectGameTypeMenu::onEnter()
{
	if (indicator < 4)
	{
		config().setValue("game_type", indicator-1);
		Game g(1);		// 1 player
	}
	return false;
}
//-----------------------------------------------------------------------------
