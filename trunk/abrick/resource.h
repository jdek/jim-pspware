#ifndef MB_RESOURCE_H
#define MB_RESOURCE_H

#include "sdl.h"
//-----------------------------------------------------------------------------
class Resource
{
public:
	virtual ~Resource() = 0;	// pure virtual dtor, so noone can create instances of this class
};
//-----------------------------------------------------------------------------
class Graphics: public Resource
{
private:
	SDL_Surface *surfaceM;
public:
	Graphics(std::string filename);		// load
	~Graphics();	// free surfaceM
	SDL_Surface *getSurface();
};
//-----------------------------------------------------------------------------
class Music: public Resource
{
private:
	Mix_Music *songM;
public:
	Music(std::string filename);
	~Music();

	bool Play(int flag = -1);	// infinite
}
//-----------------------------------------------------------------------------
class Sfx: public Resource
{
private:
	Mix_Chunk *soundM;
public:
	Sfx(std::string filename);
	~Sfx();

	bool Play(int loops = -1);	// infinite
}
//-----------------------------------------------------------------------------
class Resources
{
private:
	std::map<std::string, Resource *> resourcesM;
public:
	~Resources();	// iterate through resourcesM and "delete" each *

	bool loadResource(std::string filename, std::string resname);	// autodetect type (bmp/png/jpg, wav, mod/s3m)
	// return false if it already exists, or cannot be loaded

	Resource *getResource(std::string name) const;
	SDL_Surface *getGfx(std::string name)   const;
	Sfx         *getSfx(std::string name)   const;
	Music       *getMusic(std::string name) const;
}
//-----------------------------------------------------------------------------
#endif
