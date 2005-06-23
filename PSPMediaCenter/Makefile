PSP_EBOOT = EBOOT.PBP
PSP_BIN = pspmc.elf
PSP_OBJS = pspmc.o audiolib.o mp3/mp3player.o ogg/oggplayer.o
PSP_LIBS = -ldebug -lmad -lvorbisidec -lc
PSP_CFLAGS := $(PSP_CFLAGS) -I../libmad/include -I../libTremor/include
PSP_LDFLAGS :=  -v $(PSP_LDFLAGS) -L../libmad/lib -L../libTremor/lib


# EBOOT.PBP Settings, to save hand editing
PSP_EBOOT_SFO = media/PARAM.SFO
PSP_EBOOT_ICON = media/pspdev.png
PSP_EBOOT_TITLE = "PSP Media Center v1.0"
PSP_MSDIR = /cygdrive/I/PSP/GAME/PSPMC

all: release

release : $(PSP_BIN)

clean:
	rm -f *.elf *.o *.a $(PSP_EBOOT) $(PSP_BIN).fix $(PSP_EBOOT_SFO)

buildpbp:
	#psp-strip --strip-all $(PSP_BIN).fix
	#mksfo $(PSP_EBOOT_TITLE) $(PSP_EBOOT_SFO)
	#pack-pbp $(PSP_EBOOT) $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON) NULL NULL NULL NULL $(PSP_BIN).fix NULL
	#cp -rf $(PSP_EBOOT) $(PSP_MSDIR)

include $(PSPSDK)/build/Makefile.pref
include $(PSPSDK)/build/Makefile.global

