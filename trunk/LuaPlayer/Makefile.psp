TARGET = luaplayer
OBJS = src/graphics.o src/sound.o src/luaplayer.o src/luacontrols.o src/luagraphics.o src/luasound.o src/luasystem.o src/utility.o src/main.o src/framebuffer.o
PSPOBJS = ${LPOBJS} main.o framebuffer.o
WINOBJS = ${LPOBJS} windows.o framebuffer.o


INCDIR =
CFLAGS = -G0 -Wall -O2 -fno-strict-aliasing
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -llua -llualib -lpng -lz -lpspgu -lm -lmikmod -lmmio -lpspaudiolib -lpspaudio -lpspusb -lpspusbstor


EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Lua Player 0.10
PSP_EBOOT_ICON = title-icon.png

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
