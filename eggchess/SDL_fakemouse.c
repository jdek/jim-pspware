/*  

    SDL_fakemouse.c

    public domain.

	rinco 2005

*/

#include <SDL.h>                                        
#include <SDL_events.h>                                        

#ifdef PSP
/* The PSP needs a bigger deadzone than the default. */
#define JOY_DEADZONE (256 * 8)
#else
#define JOY_DEADZONE (256)
#endif

extern int SDL_PrivateMouseMotion(Uint8 buttonstate, int relative, Sint16 x, Sint16 y);
extern int SDL_PrivateMouseButton(Uint8 state, Uint8 button, Sint16 x, Sint16 y);

/**
 * Kludge a mouse event by taking advantage of unprotected global funcs 
 *
 * \param ev An analog joystick event
 */
void fakemouse_event(SDL_Event ev) {
	int x, y;

	SDL_GetMouseState (&x, &y);

	switch (ev.type) {
		case SDL_JOYAXISMOTION:
			if ((ev.jaxis.value > -JOY_DEADZONE) && (ev.jaxis.value < JOY_DEADZONE))
				return;
			switch (ev.jaxis.axis) {
				case 0:         
					SDL_PrivateMouseMotion(0, 1, ev.jaxis.value / 1024, 0);
					break;
				case 1:         
					SDL_PrivateMouseMotion(0, 1, 0, ev.jaxis.value / 1024);
					break;
			}
			break;

		case SDL_JOYBUTTONUP:
			SDL_PrivateMouseButton(SDL_RELEASED, 1, x, y);
			break;

		case SDL_JOYBUTTONDOWN:
			SDL_PrivateMouseButton(SDL_PRESSED, 1, x, y);
			break;
	}
}
