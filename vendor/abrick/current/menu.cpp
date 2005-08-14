#include <string>
#include <vector>
#include "SDL.h"
#include "njamfont.h"
#include "sutils.h"
#include "menu.h"

extern SDL_Surface *Screen;
//-----------------------------------------------------------------------------
Menu::Menu(NjamFont *font_ptr, int x, int y)
{
	x_pos = x;
	y_pos = y;
	font = font_ptr;
	indicator = 0;
	firstIsTitleM = false;
}
//-----------------------------------------------------------------------------
Menu::~Menu()
{
	if (background)
		SDL_FreeSurface(background);	// release background surface
}
//-----------------------------------------------------------------------------
bool Menu::process_keys()					// up/down/enter/esc: return false on menu exit
{
	SDLKey key = NjamGetch(false);
	if (key == SDLK_UP) indicator--;
	if (key == SDLK_DOWN) indicator++;
	if (firstIsTitleM && indicator < 1 || !firstIsTitleM && indicator < 0)
		indicator = options.size()-1;
	if ((unsigned)indicator >= options.size())
		indicator = (firstIsTitleM ? 1 : 0);
	if (key == SDLK_ESCAPE)
		return false;
	if ((key == SDLK_RETURN || key == SDLK_KP_ENTER) && !onEnter())
		return false;
	return true;
}
//-----------------------------------------------------------------------------
void Menu::draw(bool flipit)
{
	if (background)
		SDL_BlitSurface(background, NULL, Screen, NULL);

	render();			// draw some animations if needed, scroll, etc.

	int i=0;			// blit menu items
	for (std::vector<std::string>::iterator it = options.begin(); it != options.end(); ++it)
	{
		font->WriteTextXY(Screen, x_pos, y_pos + i*font->GetCharHeight(), (*it).c_str());
		i++;
	}

	SDL_Rect r;			// blit rectangle around selected item
	int border = 2;
	int length = 0;
	if (options.size() > (unsigned)indicator)
		length = options[indicator].length();
	r.x = x_pos-border;
	r.y = y_pos+indicator*font->GetCharHeight()-2;
	r.w = font->GetCharWidth() * length + 2*border;
	r.h = font->GetCharHeight() + 2*border;
	Box(Screen, r, 1, 255, 190, 50);

	if (firstIsTitleM)	// blit line below title (=first option)
	{
		border = 2;
		length = options[0].length();
		r.x = x_pos;
		r.y = font->GetCharHeight() + y_pos - 1;
		r.w = font->GetCharWidth() * length + 2*border;
		r.h = 1;
		Box(Screen, r, 1, 255, 190, 50);
	}

	if (flipit)
	{
		SDL_Flip(Screen);
		SDL_Delay(30);
	}
}
//-----------------------------------------------------------------------------
void Menu::getDimensions(SDL_Rect& rect, int border, int minWidth)
{
	unsigned int char_width = minWidth;
	for (std::vector<std::string>::iterator it = options.begin(); it != options.end(); ++it)
		if ((*it).length() > char_width)
			char_width = (*it).length();

	unsigned int char_height = options.size();
	rect.w = (char_width+1) * font->GetCharWidth() + 2*border;
	rect.h = char_height * font->GetCharHeight() + 2*border;
	rect.x = x_pos-border;
	rect.y = y_pos-border;
}
//-----------------------------------------------------------------------------
void Menu::start(int minWidth)
{
	background = SDL_DisplayFormat(Screen);		// save screen to buffer, so we can redraw
	if (!background)
		throw Exiter("Cannot make a copy of background");

	SDL_Rect r;
	getDimensions(r, 6, minWidth);
	SurfaceEffect(background, r, fxDarkenAlot);
	Box(background, r, 1, 255, 255, 0);

	do
	{
		draw();				// also waits for vsync
	}
	while (process_keys());	// until user exits
};
//-----------------------------------------------------------------------------
SDLKey Menu::Message(std::string text)
{
	while (NjamGetch(false) != SDLK_LAST);	// clear the buffer

	int l = text.length();
	int w = font->GetCharWidth();

	draw(false);	// render background

	SDL_Rect dest;
	NjamSetRect(dest, (640-l*w-50)/2, 195, l*w+50, 55);
	SDL_FillRect(Screen, &dest, 0);
	Uint32 FillColor = SDL_MapRGB(Screen->format, 220, 50, 0);
	NjamSetRect(dest, (640-l*w-50)/2 + 1, 196, l*w+48, 53);
	SDL_FillRect(Screen, &dest, FillColor);

	NjamSetRect(dest, (640-l*w-30)/2, 205, l*w+30, 35);
	SDL_FillRect(Screen, &dest, 0);

	font->WriteTextXY(Screen, (640-l*w)/2, 215, text.c_str());
	SDL_Flip(Screen);

	return NjamGetch(true);
}
//-----------------------------------------------------------------------------
