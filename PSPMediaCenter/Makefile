# Makefile for PSP MediaCenter
PSPSDK=$(shell psp-config --pspsdk-path)
PSP_EBOOT = EBOOT.PBP
TARGET = pspmc

# EBOOT.PBP Settings, to save hand editing
PSP_EBOOT_SFO = media/PARAM.SFO
PSP_EBOOT_ICON = media/PSPMC_icon.png
PSP_EBOOT_TITLE = PSP Media Center v0.91
PSP_MSDIR = /cygdrive/I/PSP/GAME/PSPMC

# Work thru the codecs, and sort out settings
PSPMC_BASE = ./
include codec.conf

# Now postprocess the values
OBJS := pspmc.o $(PSP_OBJS)
LIBS = $(PLAYER_LIBS) -lpspaudiolib  -lpspaudio -lpsphprm -lpspgu -lpspgum
# Take cflags and ldflags from codec script and add them in
CFLAGS  := -G0 $(PSP_CFLAGS) $(CFLAGS)
#CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti
CXXFLAGS := $(CFLAGS)
#ASFLAGS := $(CFLAGS)
LDFLAGS := $(LDFLAGS) $(PSP_LDFLAGS)
EXTRA_TARGETS := header 
#USE_PSPSDK_LIBC = 1


# This must be included here
include $(PSPSDK)/lib/build.mak
LIBS += -lpsphprm_driver
# Now our own build targets
#deps: 


cleanall:
	rm -f *.elf *.o *.a $(PSP_EBOOT) $(FINAL_TARGET) release/* release/v1.0/* release/PSPMC/* release/PSPMC%/* codecincs.h $(PSP_OBJS) 2> /dev/null

cleanup:
	@find -name "*.bak" -exec rm -f {} \;
	@find -name "*.?~" -exec rm -f {} \;

indent:
	@find -name "*.[h|c]" -exec indent -kr -l120 '{}' \;

# Generate the headerfile for included codecs
header:
	@echo Generating Header
	@rm -rf codecincs.h
	@echo /*Generated by makefile, do not hand edit!!!*/ > codecincs.h
	@echo -e $(PSP_CODEC_INCLUDE) >> codecincs.h

# Build a PBP file	
pbp:
	psp-strip --strip-all $(FINAL_TARGET)
	mksfo $(PSP_EBOOT_TITLE) $(PSP_EBOOT_SFO)
	mkdir -p release/PSPMC release/kxploit/PSPMC_~1% release/kxploit/PSPMC\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ _
	pack-pbp release/PSPMC/$(PSP_EBOOT) $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON) NULL NULL NULL NULL $(FINAL_TARGET) NULL
	pack-pbp release/kxploit/PSPMC_~1%/$(PSP_EBOOT) $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON) NULL NULL NULL NULL NULL NULL
	cp -rf $(FINAL_TARGET) release/kxploit/PSPMC\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ _/EBOOT.PBP
	cp -rf Readme release/Readme.txt
	unix2dos -D release/Readme.txt
	
# Upload PBP to MemoryStick	
upload10:
	cp release/PSPMC/$(PSP_EBOOT) $(PSP_MSDIR)/EBOOT.PBP
upload15:
	cp release/PSPMC/EBOOT.PBP $(PSP_MSDIR)/EBOOT.PBP
	
