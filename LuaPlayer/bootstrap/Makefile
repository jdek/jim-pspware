TARGET = luaboot
OBJS = bootstrap.o sio.o


# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=exports.exp

PSP_EBOOT_TITLE = LuaPlayer 0.17
PSP_EBOOT_ICON = title-icon.png

INCDIR = 
CFLAGS = -g -O0 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBS = -lpspdebug -lpsphprm_driver 
LIBDIR =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

