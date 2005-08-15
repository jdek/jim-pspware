/*  

    SDL_fakemouse.c

    public domain.

	rinco 2005

*/

#include <SDL.h>                                        
#include <SDL_events.h>                                        

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
			switch (ev.jaxis.axis) {
				case 0:         
					SDL_PrivateMouseMotion(NULL, 1, ev.jaxis.value / 512, 0);
					break;
				case 1:         
					SDL_PrivateMouseMotion(NULL, 1, 0, ev.jaxis.value / 512);
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
