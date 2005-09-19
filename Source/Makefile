#----------------------------------------------------------------------------
#       MAKEFILE
#
#	Controlling makefile for File Assistant
#
#	Created:	1st August 2005
#
#	Copyright (C) 1995-2005 T Swann
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#	Target to make
#	--------------

TARGET :=				FileAssistant
BUILD_V15 :=			TRUE

#----------------------------------------------------------------------------
#	Project folders
#	---------------

SOURCE_DIR :=			.
INCLUDE_DIR :=			.

PSP_MEDIA_CENTER_DIR :=	$(SOURCE_DIR)/../../PSPMediaCenter
CODEC_DIR :=			$(PSP_MEDIA_CENTER_DIR)/Codec
CODEC_INCLUDE_DIR :=	$(PSP_MEDIA_CENTER_DIR)/Codec

TINY_XML_DIR :=			$(SOURCE_DIR)/TinyXML
TINY_XML_INCLUDE_DIR :=	$(TINY_XML_DIR)

PLUGINS_DIR :=			$(SOURCE_DIR)/Plugins

#----------------------------------------------------------------------------
#	Source to make
#	--------------

ASM_OBJS :=				

FA_OBJS :=				$(SOURCE_DIR)/Main.o \
						$(SOURCE_DIR)/CFrameWork.o \
						$(SOURCE_DIR)/CGfx.o \
						$(SOURCE_DIR)/CVector.o \
						$(SOURCE_DIR)/CUSBManager.o \
						$(SOURCE_DIR)/CInput.o \
						$(SOURCE_DIR)/CString.o \
						$(SOURCE_DIR)/CFileSystem.o \
						$(SOURCE_DIR)/CTextureManager.o \
						$(SOURCE_DIR)/PNGReader.o \
						$(SOURCE_DIR)/JPGReader.o \
						$(SOURCE_DIR)/TGAReader.o \
						$(SOURCE_DIR)/BMPReader.o \
						$(SOURCE_DIR)/CFont.o \
						$(SOURCE_DIR)/CProcess.o \
						$(SOURCE_DIR)/CSizedItem.o \
						$(SOURCE_DIR)/CWindow.o \
						$(SOURCE_DIR)/CWindowItem.o \
						$(SOURCE_DIR)/CWindowTable.o \
						$(SOURCE_DIR)/CWindowText.o \
						$(SOURCE_DIR)/CDirectoryList.o \
						$(SOURCE_DIR)/CFileOptions.o \
						$(SOURCE_DIR)/CSkinManager.o \
						$(SOURCE_DIR)/CFileAssistant.o \
						$(SOURCE_DIR)/CMessageBox.o \
						$(SOURCE_DIR)/CBackground.o \
						$(SOURCE_DIR)/CRenderable.o \
						$(SOURCE_DIR)/CFileHandler.o \
						$(SOURCE_DIR)/CExecutableFileHandler.o \
						$(SOURCE_DIR)/CImageFileHandler.o \
						$(SOURCE_DIR)/CMusicFileHandler.o \
						$(SOURCE_DIR)/CTextFileHandler.o \
						$(SOURCE_DIR)/CHUD.o \
						$(SOURCE_DIR)/CTextInput.o \
						$(SOURCE_DIR)/COptionsMenu.o \
						$(SOURCE_DIR)/CSkinSelectMenu.o \
						$(SOURCE_DIR)/CConfigFile.o \
						$(SOURCE_DIR)/CAssert.o \
						$(SOURCE_DIR)/CInformationDialog.o \
						$(SOURCE_DIR)/CPRXManager.o \
						$(SOURCE_DIR)/CKeyboard.o \
						$(SOURCE_DIR)/CLanguage.o

CODEC_OBJS :=			$(CODEC_DIR)/ahx/ahx.o \
						$(CODEC_DIR)/mod/modplayer.o \
						$(CODEC_DIR)/mp3/mp3player.o \
						$(CODEC_DIR)/ogg/oggplayer.o

TINY_XML_OBJS :=		$(TINY_XML_DIR)/tinystr.o \
						$(TINY_XML_DIR)/tinyxml.o \
						$(TINY_XML_DIR)/tinyxmlerror.o \
						$(TINY_XML_DIR)/tinyxmlparser.o

#----------------------------------------------------------------------------
#	All objects to make
#	-------------------

OBJS :=					$(FA_OBJS) $(ASM_OBJS) $(CODEC_OBJS) $(TINY_XML_OBJS)

#----------------------------------------------------------------------------
#	Additional includes
#	-------------------

INCDIR   :=				. $(INCDIR) $(INCLUDE_DIR) $(PSP_MEDIA_CENTER_DIR) $(CODEC_INCLUDE_DIR) $(TINY_XML_INCLUDE_DIR)

#----------------------------------------------------------------------------
#	Addditional libraries
#	---------------------

SDK_LIBS :=				-lpsprtc -lpspsdk -lpsputility -lpspaudiolib -lpspaudio -lpsphprm -lpspctrl -lpspusb -lpspusbstor -lpsppower -lpspumd -lpspgu

EXTERN_LIBS :=			-ljpeg -lpng -lmad -lvorbisidec -lz

LIBS :=					$(SDK_LIBS) $(EXTERN_LIBS) -lc -lm -lstdc++

#----------------------------------------------------------------------------
#	Preprocesser defines
#	--------------------

DEFINES :=				-D_DEBUG

#----------------------------------------------------------------------------
#	Compiler settings
#	-----------------

CFLAGS :=				$(DEFINES)
CXXFLAGS :=				$(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS :=				$(CFLAGS)

LIBDIR :=	
LDFLAGS :=	

#----------------------------------------------------------------------------
#	PBP parameters
#	--------------

EXTRA_TARGETS :=		EBOOT.PBP
#PSP_EBOOT_ICON :=		../ICON0.PNG
#PSP_EBOOT_PIC1 :=		../PIC1.PNG
PSP_EBOOT_TITLE :=		$(TARGET)

#----------------------------------------------------------------------------
#	Default build settings
#	----------------------

PSPSDK :=				$(shell psp-config --pspsdk-path)

include					$(PSPSDK)/lib/build.mak

#----------------------------------------------------------------------------
#	Copy to PSP
#	-----------

ifneq ($VS_PATH),)
CC       = vs-psp-gcc
CXX      = vs-psp-g++
endif

kx-install: kxploit
ifeq ($(PSP_MOUNT),)
		@echo '*** Error: $$(PSP_MOUNT) undefined. Please set it to for example /cygdrive/e'
		@echo if your PSP is mounted to E: in cygwin.
else
		cp -r $(KXDIR) $(PSP_MOUNT)/PSP/GAME/
		cp -r $(KXDUMMY) $(PSP_MOUNT)/PSP/GAME/
		cp -r -u "../Data" $(PSP_MOUNT)/PSP/GAME/$(KXDIR)
endif
