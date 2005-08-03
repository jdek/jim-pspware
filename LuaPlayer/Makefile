TARGET = luaplayer
OBJS = main.o graphics.o luaplayer.o

INCDIR =
CFLAGS = -G0 -Wall -O2 -fno-strict-aliasing
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -llua -llualib -lpng -lz -lpspgu -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Lua Player 0.6 by Shine
PSP_EBOOT_ICON = title-icon.png

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
