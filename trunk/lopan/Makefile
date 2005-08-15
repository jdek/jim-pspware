CC	= gcc
CFLAGS	= -O2 -I/usr/local/include/SDL -D_REENTRANT
EXE	= lopan
OBJS	= $(EXE).o gfx.o font.o

$(EXE):	$(OBJS)
	gcc -o $(EXE) $(OBJS) -lSDL -ldl -lpthread

$(EXE).o: $(EXE).c gfx.h font.h

gfx.o:	gfx.c gfx.h

font.o:	font.c font.h

clean:
	rm *.o $(EXE)
