TARGET = rr
PSPSDK = $(shell psp-config --pspsdk-path)
PSPPREFIX = $(shell psp-config --psp-prefix)
PSPBIN = $(PSPPREFIX)/bin
SDL_CONFIG = $(PSPBIN)/sdl-config

O = o
OBJS =	$(TARGET).$(O) \
	foe.$(O) foecommand.$(O) barragemanager.$(O) boss.$(O) ship.$(O) laser.$(O) \
	frag.$(O) background.$(O) letterrender.$(O) shot.$(O) \
	screen.$(O) vector.$(O) degutil.$(O) rand.$(O) mt19937int.$(O) \
	soundmanager.$(O) attractmanager.$(O) movetosdk.$(O)

DEFAULT_CFLAGS = $(shell $(SDL_CONFIG) --cflags)

MORE_CFLAGS = -O2 -fsingle-precision-constant #-ffast-math

CFLAGS = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
CXXFLAGS = $(DEFAULT_CFLAGS) $(MORE_CFLAGS) -fno-exceptions

LDFLAGS = -L$(PSPPREFIX)/lib
LIBS = -lbulletml -lSDL_mixer -lvorbisidec -lGLU $(shell $(SDL_CONFIG) --libs) -lstdc++ -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = rRootage

include $(PSPSDK)/lib/build.mak
