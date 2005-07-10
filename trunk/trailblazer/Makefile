TARGET = TRAILBLAZER
OBJS = game.o main.o road.o map.o gfx.o

SDL_CONFIG = \`psp-config --pspsdk-path\`/../bin/sdl-config

INCDIR = 
CFLAGS = -O2 -G0 -Wall -I`psp-config --pspsdk-path`/../include `$(SDL_CONFIG) --cflags`
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

USE_NEWLIB_LIBC = 1
LIBDIR =
LDFLAGS = `$(SDL_CONFIG) --libs`
LIBS = -lm -lg -lSDLmain -lSDL 

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Trailblazer Remake

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
