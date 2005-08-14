#include <string>
#include <sstream>
#include "sutils.h"
#include "keys.h"
//-----------------------------------------------------------------------------
bool Keyboard::isKeyDown(SDLKey k)
{
	return keysM[k] > 6;	// 6 = repeat delay
}
//-----------------------------------------------------------------------------
void Keyboard::setKeyDown(SDLKey k, bool state)
{
	if (keysM.find(k) == keysM.end())
		keysM.insert(std::pair<SDLKey,char>(k, state?6:0));
	else
		keysM[k] = state?6:0;
}
//-----------------------------------------------------------------------------
SDLKey Keyboard::update(bool updateRepeated)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_KEYDOWN)
		{
			SDLKey k = event.key.keysym.sym;
			if (keysM.find(k) == keysM.end())
				keysM.insert(std::pair<SDLKey,char>(k, 1));
			else
				keysM[k] = 1;
			return k;
		}
		if (event.type == SDL_KEYUP)
		{
			keysM[event.key.keysym.sym] = 0;
			continue;
		}
		if (event.type == SDL_QUIT)
			throw Exiter("Close button of window clicked");
	}

	// update already pressed keys
	if (updateRepeated)
		for (std::map<SDLKey,char>::iterator it = keysM.begin(); it != keysM.end(); ++it)
			if ((*it).second)
				(*it).second++;

    return SDLK_LAST;	// this is probably never used by SDL, so it denotes that key isn't pressed
}
//-----------------------------------------------------------------------------
// fill the names
KeyNames::KeyNames()
{
	keynames.insert(std::pair<int, std::string>(0,"UNKNOWN"));
	keynames.insert(std::pair<int, std::string>(8,"BACKSPACE"));
	keynames.insert(std::pair<int, std::string>(9,"TAB"));
	keynames.insert(std::pair<int, std::string>(12,"CLEAR"));
	keynames.insert(std::pair<int, std::string>(13,"RETURN"));
	keynames.insert(std::pair<int, std::string>(19,"PAUSE"));
	keynames.insert(std::pair<int, std::string>(27,"ESCAPE"));
	keynames.insert(std::pair<int, std::string>(32,"SPACE"));
	keynames.insert(std::pair<int, std::string>(33,"EXCLAIM"));
	keynames.insert(std::pair<int, std::string>(34,"QUOTEDBL"));
	keynames.insert(std::pair<int, std::string>(35,"HASH"));
	keynames.insert(std::pair<int, std::string>(36,"DOLLAR"));
	keynames.insert(std::pair<int, std::string>(38,"AMPERSAND"));
	keynames.insert(std::pair<int, std::string>(39,"QUOTE"));
	keynames.insert(std::pair<int, std::string>(40,"LEFTPAREN"));
	keynames.insert(std::pair<int, std::string>(41,"RIGHTPAREN"));
	keynames.insert(std::pair<int, std::string>(42,"ASTERISK"));
	keynames.insert(std::pair<int, std::string>(43,"PLUS"));
	keynames.insert(std::pair<int, std::string>(44,"COMMA"));
	keynames.insert(std::pair<int, std::string>(45,"MINUS"));
	keynames.insert(std::pair<int, std::string>(46,"PERIOD"));
	keynames.insert(std::pair<int, std::string>(47,"SLASH"));
	keynames.insert(std::pair<int, std::string>(48,"0"));
	keynames.insert(std::pair<int, std::string>(49,"1"));
	keynames.insert(std::pair<int, std::string>(50,"2"));
	keynames.insert(std::pair<int, std::string>(51,"3"));
	keynames.insert(std::pair<int, std::string>(52,"4"));
	keynames.insert(std::pair<int, std::string>(53,"5"));
	keynames.insert(std::pair<int, std::string>(54,"6"));
	keynames.insert(std::pair<int, std::string>(55,"7"));
	keynames.insert(std::pair<int, std::string>(56,"8"));
	keynames.insert(std::pair<int, std::string>(57,"9"));
	keynames.insert(std::pair<int, std::string>(58,"COLON"));
	keynames.insert(std::pair<int, std::string>(59,"SEMICOLON"));
	keynames.insert(std::pair<int, std::string>(60,"LESS"));
	keynames.insert(std::pair<int, std::string>(61,"EQUALS"));
	keynames.insert(std::pair<int, std::string>(62,"GREATER"));
	keynames.insert(std::pair<int, std::string>(63,"QUESTION"));
	keynames.insert(std::pair<int, std::string>(64,"AT"));
	keynames.insert(std::pair<int, std::string>(91,"LEFTBRACKET"));
	keynames.insert(std::pair<int, std::string>(92,"BACKSLASH"));
	keynames.insert(std::pair<int, std::string>(93,"RIGHTBRACKET"));
	keynames.insert(std::pair<int, std::string>(94,"CARET"));
	keynames.insert(std::pair<int, std::string>(95,"UNDERSCORE"));
	keynames.insert(std::pair<int, std::string>(96,"BACKQUOTE"));
	keynames.insert(std::pair<int, std::string>(97,"A"));
	keynames.insert(std::pair<int, std::string>(98,"B"));
	keynames.insert(std::pair<int, std::string>(99,"C"));
	keynames.insert(std::pair<int, std::string>(100,"D"));
	keynames.insert(std::pair<int, std::string>(101,"E"));
	keynames.insert(std::pair<int, std::string>(102,"F"));
	keynames.insert(std::pair<int, std::string>(103,"G"));
	keynames.insert(std::pair<int, std::string>(104,"H"));
	keynames.insert(std::pair<int, std::string>(105,"I"));
	keynames.insert(std::pair<int, std::string>(106,"J"));
	keynames.insert(std::pair<int, std::string>(107,"K"));
	keynames.insert(std::pair<int, std::string>(108,"L"));
	keynames.insert(std::pair<int, std::string>(109,"M"));
	keynames.insert(std::pair<int, std::string>(110,"N"));
	keynames.insert(std::pair<int, std::string>(111,"O"));
	keynames.insert(std::pair<int, std::string>(112,"P"));
	keynames.insert(std::pair<int, std::string>(113,"Q"));
	keynames.insert(std::pair<int, std::string>(114,"R"));
	keynames.insert(std::pair<int, std::string>(115,"S"));
	keynames.insert(std::pair<int, std::string>(116,"T"));
	keynames.insert(std::pair<int, std::string>(117,"U"));
	keynames.insert(std::pair<int, std::string>(118,"V"));
	keynames.insert(std::pair<int, std::string>(119,"W"));
	keynames.insert(std::pair<int, std::string>(120,"X"));
	keynames.insert(std::pair<int, std::string>(121,"Y"));
	keynames.insert(std::pair<int, std::string>(122,"Z"));
	keynames.insert(std::pair<int, std::string>(127,"DELETE"));
	keynames.insert(std::pair<int, std::string>(256,"KEYPAD0"));
	keynames.insert(std::pair<int, std::string>(257,"KEYPAD1"));
	keynames.insert(std::pair<int, std::string>(258,"KEYPAD2"));
	keynames.insert(std::pair<int, std::string>(259,"KEYPAD3"));
	keynames.insert(std::pair<int, std::string>(260,"KEYPAD4"));
	keynames.insert(std::pair<int, std::string>(261,"KEYPAD5"));
	keynames.insert(std::pair<int, std::string>(262,"KEYPAD6"));
	keynames.insert(std::pair<int, std::string>(263,"KEYPAD7"));
	keynames.insert(std::pair<int, std::string>(264,"KEYPAD8"));
	keynames.insert(std::pair<int, std::string>(265,"KEYPAD9"));
	keynames.insert(std::pair<int, std::string>(266,"KEYPAD_PERIOD"));
	keynames.insert(std::pair<int, std::string>(267,"KEYPAD_DIVIDE"));
	keynames.insert(std::pair<int, std::string>(268,"KEYPAD_MULTIPLY"));
	keynames.insert(std::pair<int, std::string>(269,"KEYPAD_MINUS"));
	keynames.insert(std::pair<int, std::string>(270,"KEYPAD_PLUS"));
	keynames.insert(std::pair<int, std::string>(271,"KEYPAD_ENTER"));
	keynames.insert(std::pair<int, std::string>(272,"KEYPAD_EQUALS"));
	keynames.insert(std::pair<int, std::string>(273,"UP"));
	keynames.insert(std::pair<int, std::string>(274,"DOWN"));
	keynames.insert(std::pair<int, std::string>(275,"RIGHT"));
	keynames.insert(std::pair<int, std::string>(276,"LEFT"));
	keynames.insert(std::pair<int, std::string>(277,"INSERT"));
	keynames.insert(std::pair<int, std::string>(278,"HOME"));
	keynames.insert(std::pair<int, std::string>(279,"END"));
	keynames.insert(std::pair<int, std::string>(280,"PAGEUP"));
	keynames.insert(std::pair<int, std::string>(281,"PAGEDOWN"));
	keynames.insert(std::pair<int, std::string>(282,"F1"));
	keynames.insert(std::pair<int, std::string>(283,"F2"));
	keynames.insert(std::pair<int, std::string>(284,"F3"));
	keynames.insert(std::pair<int, std::string>(285,"F4"));
	keynames.insert(std::pair<int, std::string>(286,"F5"));
	keynames.insert(std::pair<int, std::string>(287,"F6"));
	keynames.insert(std::pair<int, std::string>(288,"F7"));
	keynames.insert(std::pair<int, std::string>(289,"F8"));
	keynames.insert(std::pair<int, std::string>(290,"F9"));
	keynames.insert(std::pair<int, std::string>(291,"F10"));
	keynames.insert(std::pair<int, std::string>(292,"F11"));
	keynames.insert(std::pair<int, std::string>(293,"F12"));
	keynames.insert(std::pair<int, std::string>(294,"F13"));
	keynames.insert(std::pair<int, std::string>(295,"F14"));
	keynames.insert(std::pair<int, std::string>(296,"F15"));
	keynames.insert(std::pair<int, std::string>(300,"NUMLOCK"));
	keynames.insert(std::pair<int, std::string>(301,"CAPSLOCK"));
	keynames.insert(std::pair<int, std::string>(302,"SCROLLOCK"));
	keynames.insert(std::pair<int, std::string>(303,"RSHIFT"));
	keynames.insert(std::pair<int, std::string>(304,"LSHIFT"));
	keynames.insert(std::pair<int, std::string>(305,"RCTRL"));
	keynames.insert(std::pair<int, std::string>(306,"LCTRL"));
	keynames.insert(std::pair<int, std::string>(307,"RALT"));
	keynames.insert(std::pair<int, std::string>(308,"LALT"));
	keynames.insert(std::pair<int, std::string>(309,"RMETA"));
	keynames.insert(std::pair<int, std::string>(310,"LMETA"));
	keynames.insert(std::pair<int, std::string>(315,"HELP"));
	keynames.insert(std::pair<int, std::string>(316,"PRINT"));
	keynames.insert(std::pair<int, std::string>(317,"SYSREQ"));
	keynames.insert(std::pair<int, std::string>(318,"BREAK"));
	keynames.insert(std::pair<int, std::string>(319,"MENU"));
}
//-----------------------------------------------------------------------------
std::string KeyNames::getKeyName(const int& key)
{
	if (keynames.find(key) == keynames.end())
	{
		std::stringstream ss;
		ss << "KEY " << key;
		std::string retval = ss.str();
		return retval;
	}
	else
		return keynames[key];
}
//-----------------------------------------------------------------------------
KeyNames &KeyNames::keyNamesProvider()
{
	static KeyNames k;
	return k;
}
//-----------------------------------------------------------------------------
