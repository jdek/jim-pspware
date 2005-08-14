#ifndef AB_ALLMENUS_H
#define AB_ALLMENUS_H

#define AB_VERSION "1.12"
//-----------------------------------------------------------------------------
#include <list>
#include "effects.h"
#include "menu.h"
#include "njamfont.h"
#include "music.h"
//-----------------------------------------------------------------------------
class MainMenu: public Menu
{
private:
	Effects effectsM;
	NjamFont whiteFontM;
	Music musicM, gameMusicM;

	void start_game(int players);
	virtual bool onEnter();
	virtual void render();
public:
	MainMenu(NjamFont *font_ptr, int x, int y);
};
//-----------------------------------------------------------------------------
class OptionsMenu: public Menu
{
private:
	Music *mmmM;		// so it can start/stop it when option changes,  mmm = Main Menu Music :)
public:
	OptionsMenu(NjamFont *font_ptr, int x, int y, Music *mmm);
	virtual bool onEnter();
};
//-----------------------------------------------------------------------------
class KeyboardMenu: public Menu
{
public:
	KeyboardMenu(NjamFont *font_ptr, int x, int y);
	virtual bool onEnter();
};
//-----------------------------------------------------------------------------
class SelectGameTypeMenu: public Menu
{
public:
	SelectGameTypeMenu(NjamFont *font_ptr, int x, int y);
	virtual bool onEnter();
};
//-----------------------------------------------------------------------------
#endif
