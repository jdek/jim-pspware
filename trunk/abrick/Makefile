#
# Copyright 2004 Milan Babuskov
#
# This file is part of Abandoned Bricks game
#
# Abandoned Bricks is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Abandoned Bricks is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Abandoned Bricks in file COPYING; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# directory where the games are installed (/usr, /usr/local, ... or something)
ifndef PREFIX
PREFIX=/usr
endif

PROGRAM = abrick
OBJECTS = game.o player.o shape.o main.o allmenus.o njamfont.o sutils.o menu.o hiscore.o effects.o config.o music.o sound.o keys.o
CC = g++

# If you hate warnings under Cygwin use these flags instead
# MY_CFLAGS = -I/usr/local/include/SDL -Dmain=SDL_main -DWIN32 -Uunix -mno-cygwin
MY_CFLAGS = `sdl-config --cflags`

%.o:	%.cpp
	$(CC) -c $(MY_CFLAGS) $< -o $@

all:	$(PROGRAM)

$(PROGRAM):	$(OBJECTS)
	$(CC) `sdl-config --libs` -o$(PROGRAM) $(OBJECTS) -lSDL -lSDL_mixer

clean:
	$(RM) *.o
	$(RM) $(PROGRAM)
	$(RM) $(PROGRAM).exe


