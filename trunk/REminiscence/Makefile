
ifdef PSP
TARGET = REminiscence
PSPSDK = $(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
endif

ifdef PSP
SDL_CFLAGS := $(shell $(PSPBIN)/sdl-config --cflags)
SDL_LIBS := $(shell $(PSPBIN)/sdl-config --libs)
else
SDL_CFLAGS = `sdl-config --cflags`
SDL_LIBS = `sdl-config --libs`
endif

DEFINES = -DSYS_LITTLE_ENDIAN
ifdef PSP
DEFINES+= -DSYS_NEED_ALIGNMENT
endif

ifndef PSP
CXX = g++
CXXFLAGS:= -g -O -Wall -Wuninitialized -Wno-unknown-pragmas -Wshadow -Wimplicit
CXXFLAGS+= -Wundef -Wreorder -Wwrite-strings -Wnon-virtual-dtor -Wno-multichar
endif
CXXFLAGS+= $(SDL_CFLAGS) $(DEFINES)
ifdef PSP
CXXFLAGS+= -O2 -G0 -fno-exceptions -fno-rtti
endif

SRCS = collision.cpp cutscene.cpp file.cpp game.cpp graphics.cpp locale.cpp \
	main.cpp menu.cpp mixer.cpp mod_player.cpp piege.cpp resource.cpp scaler.cpp \
	staticres.cpp systemstub_sdl.cpp unpack.cpp util.cpp video.cpp

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

ifdef PSP
LIBS = $(SDL_LIBS) -lz -lstdc++

EXTRA_TARGETS = EBOOT.PBP
EXTRA_CLEAN = cleandeps
PSP_EBOOT_TITLE = REminiscence for PSP
include $(PSPSDK)/lib/build.mak
endif

rs: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SDL_LIBS) -lz

.cpp.o:
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $*.o

ifndef PSP
clean:
	rm -f *.o *.d
else
cleandeps:
	rm -f *.d
endif

-include $(DEPS)
