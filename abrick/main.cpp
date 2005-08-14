#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_image.h"
#include "njamfont.h"
#include "sutils.h"

#include "config.h"
#include "menu.h"
#include "allmenus.h"
SDL_Surface *Screen;		// global object
//-----------------------------------------------------------------------------
//! initialize SDL, and start main menu
int main(int argc, char **argv)
{
	Screen = 0;
 	bool Fullscreen = false;	// defaults
 	bool SWSurface = true;
 	if (argc > 1)
 	{
 		for (int i=1; i<argc; i++)
	 	{
	 		bool ok = true;
	 		if (argv[i][0] == '-')
			{
				if (argv[i][1] == 'f')
					Fullscreen = true;	// use -f fullscreen mode
				else if (argv[i][1] == 'h')
					SWSurface = false;	// use -h for hardware surfaces
				else
					ok = false;
			}
			else
				ok = false;

			if (!ok)
			{
				printf("Usage: ab [-w] [-h]\n\n -w  start in Windowed mode (as opposed to fullscreen).\n -h  use Hardware surfaces (faster, doesn't work well with all graphic cards).\n");
				return 1;
			}
		}
	}

 	printf("Initializing SDL: VIDEO & AUDIO...");
	if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER ) < 0 )
	{
		fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Surface *icon = 0;

	// just in case, since values are "true" by default
	config().setValue("audio_available", false);

	try
	{
		printf("OK.\nSetting window title...");
		SDL_WM_SetCaption("Abandoned bricks " AB_VERSION "     http://abrick.sourceforge.net", NULL);

		printf("done.\nLoading icon...");
		icon = SDL_LoadBMP("data/abicon.bmp");
		if (!icon)
			throw Exiter(SDL_GetError());
		printf("OK.\n");
		SDL_WM_SetIcon(icon, NULL);

		printf("Setting video mode: 640x480x16...");
		Uint32 flags = SDL_ANYFORMAT;
		if (Fullscreen)
			flags |= SDL_FULLSCREEN;
		if (SWSurface)
			flags |= SDL_SWSURFACE;
		else
			flags |= SDL_HWSURFACE;

		Screen = SDL_SetVideoMode(640, 480, 16, flags);
		if ( Screen == NULL )
			throw Exiter(SDL_GetError());
		else
			printf("OK\n");

		printf("Opening audio...");
		if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 1, 2048) == -1)
		{
			printf("Failed.\n");
			config().setValue("audio_available", false);
			config().setValue("play_music", false);
			config().setValue("play_sfx", false);
		}
		else
		{
			config().setValue("audio_available", true);
			printf("OK.\nReserving 4 channels for sfx...\n");
			printf("Reserved %d channels from default mixing.\n", Mix_ReserveChannels(4));
		}

		if (Fullscreen)						// hide mouse cursor in fullscreen
			SDL_ShowCursor(SDL_DISABLE);

		// render some background picture for menu
		SDL_Surface *pattern;
		if (!LoadImage(&pattern, "data/back.bmp"))
			throw Exiter("Cannot load wallpaper image.");
		PatternFill(pattern, Screen);
		SDL_Flip(Screen);
		SDL_FreeSurface(pattern);

		NjamFont font("data/font-yellow.bmp", 11, 22);
		MainMenu m(&font, 40, 150);
		m.start();
	}
	catch(Exiter &e)
	{
		printf("\nExiting: %s\n", e.message.c_str());
	}

 	if (config().getValue("audio_available"))
		Mix_CloseAudio();
	if (icon)
		SDL_FreeSurface(icon);
	SDL_Quit();
	return 0;
}
//-----------------------------------------------------------------------------
