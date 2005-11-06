# rRootage makefile(MinGW 2.0.0)
# $Id: Makefile,v 1.6 2003/08/10 03:21:28 kenta Exp $

NAME   = rr
O      = o
RM     = rm -f
CC     = gcc
CXX    = g++

PROG   = $(NAME).exe

DEFAULT_CFLAGS = `sdl-config --cflags`
LDFLAGS        = `sdl-config --libs` -L. -lglu32 -lopengl32 -lmingw32 -lmingwex -lSDL_mixer -lbulletml -mwindows -lstdc++

MORE_CFLAGS = -O3 -Wall

CFLAGS   = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
CPPFLAGS = $(DEFAULT_CFLAGS) $(MORE_CFLAGS) -I./bulletml/

OBJS =	$(NAME).$(O) \
	foe.$(O) foecommand.$(O) barragemanager.$(O) boss.$(O) ship.$(O) laser.$(O) \
	frag.$(O) background.$(O) letterrender.$(O) shot.$(O) \
	screen.$(O) vector.$(O) degutil.$(O) rand.$(O) mt19937int.$(O) \
	soundmanager.$(O) attractmanager.$(O) \
	$(NAME)_res.$(O)

$(PROG): $(OBJS) 
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)

$(NAME)_res.o: $(NAME).rc
	windres -i $(NAME).rc -o $(NAME)_res.o
clean:
	$(RM) $(PROG) *.$(O)
