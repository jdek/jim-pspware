TARGET = REminiscence
OBJS = collision.o cutscene.o file.o game.o graphics.o locale.o \
	main.o menu.o mixer.o mod_player.o piege.o resource.o scaler.o staticres.o systemstub_sdl.o unpack.o util.o video.o \
	zlib/inflate.o zlib/inftrees.o zlib/crc32.o zlib/adler32.o zlib/zutil.o \
	zlib/inffast.o zlib/gzio.o zlib/deflate.o zlib/trees.o zlib/compress.o

INCDIR = ../include zlib
CFLAGS = -O2 -G0 -Wall -DSYS_LITTLE_ENDIAN -DSYS_NEED_ALIGNMENT
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS = -lstdc++

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = REminiscence

PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
CFLAGS += `$(PSPBIN)/sdl-config --cflags`
LIBS += `$(PSPBIN)/sdl-config --libs`
include $(PSPSDK)/lib/build.mak
