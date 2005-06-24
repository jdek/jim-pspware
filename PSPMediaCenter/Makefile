PSP_EBOOT = EBOOT.PBP
PSP_BIN = pspmc.elf
PSP_OBJS = pspmc.o audiolib.o mp3/mp3player.o ogg/oggplayer.o mod/modplayer.o
PSP_LIBS = -ldebug -lmad -lvorbisidec -lpspc -lgcc -lm 
#-lxvidcore -lnewlib-fix 
PSP_CFLAGS := $(PSP_CFLAGS) -I../libmad/include -I../libTremor/include -I../xvidcore-1.1.0-beta2/src
PSP_LDFLAGS :=  -v $(PSP_LDFLAGS) -L../libmad/lib -L../libTremor/lib -L../xvidcore-1.1.0-beta2/ -Lnewlib-fix


# EBOOT.PBP Settings, to save hand editing
PSP_EBOOT_SFO = media/PARAM.SFO
PSP_EBOOT_ICON = media/pspdev.png
PSP_EBOOT_TITLE = "PSP Media Center v1.0"
PSP_MSDIR = /cygdrive/I/PSP/GAME/PSPMC

all: deps release

deps: 
	make -C newlib-fix
release : $(PSP_BIN)

clean:
	make -C newlib-fix clean
	rm -f *.elf *.o mp3/*.o ogg/*.o mod/*.o *.a $(PSP_EBOOT) $(PSP_BIN).fix release/* release/PSPMC/* release/PSPMC%/* 2> /dev/null

cleanup:
	rm -f *.bak *.?~ mp3/*.bak mp3/*.?~ ogg/*.bak ogg/*.?~ mod/*.bak mod/*.?~ newlib-fix/*.?~ newlib-fix/*.bak

indent:
	indent -kr -l120  *.c *.h mp3/*.c mp3/*.h ogg/*.c ogg/*.h mod/*.c mod/*.h ogg/*.c ogg/*.h newlib-fix/*.c
	
pbp:
	psp-strip --strip-all $(PSP_BIN)
	mksfo $(PSP_EBOOT_TITLE) $(PSP_EBOOT_SFO)
	pack-pbp release/v1.0/$(PSP_EBOOT) $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON) NULL NULL NULL NULL $(PSP_BIN) NULL
	pack-pbp release/PSPMC%/$(PSP_EBOOT) $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON) NULL NULL NULL NULL NULL NULL
	cp -rf $(PSP_BIN) release/PSPMC/EBOOT.PBP
	cp -rf Readme release/Readme.txt
	#cp -rf $(PSP_EBOOT) $(PSP_MSDIR)

upload15:
	cp $(PSP_BIN) $(PSP_MSDIR)/EBOOT.PBP
	
include $(PSPSDK)/build/Makefile.pref
include $(PSPSDK)/build/Makefile.global

