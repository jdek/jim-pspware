#----------------------------------------------------------------------------
#       MAKEFILE
#
#	Plugin makefile for File Assistant
#
#	Created:	1st August 2005
#
#	Copyright (C) 1995-2005 71M
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#	Plugin folders
#	---------------

PLUGINS_DIR :=		../../Data/Plugins/

IMAGE_DIR :=		Image
JPG_DIR :=			$(IMAGE_DIR)/JPG

#----------------------------------------------------------------------------
#	Build Commands
#	--------------

kxploit:
	$(MAKE) -C $(JPG_DIR)
	cp $(JPG_DIR)/*.prx $(PLUGINS_DIR)

clean:
	$(MAKE) -C $(JPG_DIR) clean
