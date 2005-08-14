#ifndef AB_MENU_H
#define AB_MENU_H
#include <string>
#include <vector>
#include "sutils.h"
#include "njamfont.h"
//-----------------------------------------------------------------------------
class Menu
{
protected:
	bool firstIsTitleM;
	NjamFont *font;
	SDL_Surface *background;
	int indicator;						// index of indicator
	int x_pos, y_pos;					// position on screen, set by caller
	bool process_keys();				// up/down/enter/esc: return false on menu exit
	virtual void draw(bool flipit = true);
	virtual void render() {};
	virtual bool onEnter() = 0;
	void getDimensions(SDL_Rect& rect, int border, int minWidth);
	SDLKey Message(std::string text);	// returns the key pressed

public:
	Menu(NjamFont *font_ptr, int x, int y);
	~Menu();
	std::vector<std::string> options;
	void start(int minWidth = 0);

};
//-----------------------------------------------------------------------------
#endif
