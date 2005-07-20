TARGET = gngeo
OBJS = conf.o driver.o emu.o fileio.o frame_skip.o \
	generator68k_interf.o gui_interf.o list.o main.o mamez80_interf.o memory.o messages.o neocrypt.o \
	pbar.o pd4990a.o profiler.o screen.o sound.o state.o streams.o timer.o transpack.o unzip.o video.o\
	blitter/soft.o effect/hq2x.o effect/hq3x.o effect/hqx_common.o effect/interp.o effect/lq2x.o effect/lq3x.o\
	effect/scale2x.o effect/scanline.o mamez80/z80.o ym2610/2610intf.o ym2610/ay8910.o ym2610/fm.o ym2610/ymdeltat.o \
	gui/button.o gui/combol.o gui/container.o gui/draw.o gui/frame.o gui/gui.o gui/label.o gui/pixmap.o gui/progress.o \
	gui/scrollbar.o gui/slist.o gui/style.o gui/textbox.o gui/thin.o gui/widget.o generator68k/cpu68k-0.o \
	generator68k/cpu68k-1.o generator68k/cpu68k-2.o generator68k/cpu68k-3.o generator68k/cpu68k-4.o generator68k/cpu68k-5.o \
	generator68k/cpu68k-6.o generator68k/cpu68k-7.o generator68k/cpu68k-8.o generator68k/cpu68k-9.o generator68k/cpu68k-a.o \
	generator68k/cpu68k-b.o generator68k/cpu68k-c.o generator68k/cpu68k-d.o generator68k/cpu68k-e.o generator68k/cpu68k-f.o \
	generator68k/cpu68k.o generator68k/reg68k.o generator68k/diss68k.o generator68k/tab68k.o debug.o getopt.o getoptint.o

INCDIR = ../include zlib
CFLAGS = -O2 -G0 -Wall -DHAVE_CONFIG_H -DUSE_GENERATOR68K -DUSE_MAMEZ80 -Wno-unused -funroll-loops -ffast-math -g
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS =  -lz

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = gngeo

PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
CFLAGS += `$(PSPBIN)/sdl-config --cflags`
LIBS += `$(PSPBIN)/sdl-config --libs`
include $(PSPSDK)/lib/build.mak
