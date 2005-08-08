TARGET = luaplayer
OBJS = main.o graphics.o sound.o luaplayer.o luacontrols.o luagraphics.o luasound.o luasystem.o utility.o

INCDIR =
CFLAGS = -G0 -Wall -O2 -fno-strict-aliasing
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -llua -llualib -lpng -lz -lpspgu -lm -lmikmod -lmmio -lpspaudiolib -lpspaudio


EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Lua Player 0.7b5
PSP_EBOOT_ICON = title-icon.png

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
