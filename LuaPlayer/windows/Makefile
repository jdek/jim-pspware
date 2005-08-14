LFLAGS = -L/usr/local/lib -lglut32 -lglu32 -llua -llualib -lopengl32 -lpng
CFLAGS = -I$(PSPSDK)/sdk/include -I$(PSPSDK)/include -I..

.c.o:
	gcc -c $< $(CFLAGS)

luaplayer.exe: $(OBJS)
	gcc -o luaplayer.exe \
		../graphics.c ../sound.c ../luaplayer.c ../luacontrols.c ../luagraphics.c ../luasound.c ../luasystem.c ../utility.c \
		../emulator/emulator.c ../emulator/framebuffer.c windows.c \
		$(CFLAGS) $(LFLAGS) -lkernel32
