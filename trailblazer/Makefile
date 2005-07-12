TARGET = TRAILBLAZER
PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
SDL_CONFIG = $(PSPBIN)/sdl-config

OBJS = game.o main.o road.o map.o gfx.o

CFLAGS = -O2 -G0 -Wall $(shell $(SDL_CONFIG) --cflags)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = $(shell $(SDL_CONFIG) --libs)

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Trailblazer Remake

include $(PSPSDK)/lib/build.mak
