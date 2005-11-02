/*  

    SDL_fakemouse.c

    public domain.

	rinco 2005

*/

#include "SDL.h"
#include "SDL_events.h"
#include "SDL_thread.h"
#include "SDL_timer.h"
#include "SDL_mutex.h"

extern int SDL_PrivateMouseMotion(Uint8 buttonstate, int relative, Sint16 x, Sint16 y);
extern int SDL_PrivateMouseButton(Uint8 state, Uint8 button, Sint16 x, Sint16 y);
static short dx = 0;
static short dy = 0;
static SDL_sem *sem = NULL;
static SDL_Thread *thread = NULL;
static int running = 0;

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
			SDL_SemWait(sem);
			switch (ev.jaxis.axis) {
				case 0:         
					dx = ev.jaxis.value / 2048;
					break;
				case 1:         
					dy = ev.jaxis.value / 2048;
					break;
			}
			SDL_SemPost(sem);
			break;

		case SDL_JOYBUTTONUP:
			SDL_PrivateMouseButton(SDL_RELEASED, 1, x, y);
			break;

		case SDL_JOYBUTTONDOWN:
			SDL_PrivateMouseButton(SDL_PRESSED, 1, x, y);
			break;
	}
}

static int fakemouse_update(void *data)
{
	while (running) {
		SDL_SemWait(sem);
		/* Delay 1/60th of a second */
		SDL_Delay(1000 / 60);  
		if (dx ||  dy)
			SDL_PrivateMouseMotion(0, 1, dx, dy);
		SDL_SemPost(sem);
	}
	return 0;
}

void fakemouse_init(void) {
	if((sem =  SDL_CreateSemaphore(1)) == NULL) {
		SDL_SetError("Can't create fakemouse semaphore\n");
		return;
	}
	running = 1;
	if((thread = SDL_CreateThread(fakemouse_update, NULL)) == NULL) {
		SDL_SetError("Can't create fakemouse thread\n");
		return;
	}
}

void fakemouse_finish(void) {
	/* Cleanup Threads and Semaphore. */
	running = 0;
	SDL_WaitThread(thread, NULL);
	SDL_DestroySemaphore(sem);
}

