#ifndef AB_KEYS_H
#define AB_KEYS_H

#include <string>
#include <map>
#include "SDL.h"
//-----------------------------------------------------------------------------
class Keyboard
{
private:
	std::map<SDLKey, char> keysM;
public:
	void setKeyDown(SDLKey k, bool state);
	bool isKeyDown(SDLKey k);
	SDLKey update(bool updateRepeated);
};
//-----------------------------------------------------------------------------
class KeyNames
{
private:
	KeyNames();
	std::map<int, std::string> keynames;
public:
	std::string getKeyName(const int& key);
	static KeyNames &keyNamesProvider();
};
//-----------------------------------------------------------------------------
#endif
