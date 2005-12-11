TARGET = rRootage
PSPSDK = $(shell psp-config --pspsdk-path)
PSPPREFIX = $(shell psp-config --psp-prefix)
PSPBIN = $(PSPPREFIX)/bin
SDL_CONFIG = $(PSPBIN)/sdl-config

O = o
OBJS =	rr.$(O) \
	foe.$(O) foecommand.$(O) barragemanager.$(O) boss.$(O) ship.$(O) laser.$(O) \
	frag.$(O) background.$(O) letterrender.$(O) shot.$(O) \
	screen.$(O) vector.$(O) degutil.$(O) rand.$(O) mt19937int.$(O) \
	soundmanager.$(O) attractmanager.$(O) movetosdk.$(O) screenshot.$(O)

DEFAULT_CFLAGS = $(shell $(SDL_CONFIG) --cflags)

MORE_CFLAGS = -O2 -fsingle-precision-constant -mno-check-zero-division #-ffast-math

CFLAGS = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
CXXFLAGS = $(DEFAULT_CFLAGS) $(MORE_CFLAGS) -fno-exceptions

LDFLAGS = -L$(PSPPREFIX)/lib
LIBS = -lbulletml -lSDL_mixer -lvorbisidec -lpng -lz -lGLU $(shell $(SDL_CONFIG) --libs) -lstdc++ -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = rRootage for PSP 1.3
PSP_EBOOT_ICON = rr-icon0.png
PSP_EBOOT_PIC1 = rr-pic1.png

include $(PSPSDK)/lib/build.mak
