# Makefile for the SGE library

include Makefile.conf

CFLAGS =$(SGE_CFLAGS) $(FT_CFLAGS)
LIBS =$(SGE_LIBS)

SGE_VER = 010629

ifeq ($(C_ONLY),y)
  CFLAGS +=-D_SGE_C
endif


OBJECTS=sge_surface.o sge_primitives.o sge_tt_text.o sge_bm_text.o sge_misc.o sge_textpp.o sge_blib.o sge_rotation.o sge_collision.o sge_shape.o

all:	config $(OBJECTS) 
	@ar rsc libSGE.a $(OBJECTS)

$(OBJECTS):	%.o:%.cpp %.h   #Each object depends on thier .cpp and .h file
	$(CXX) $(CFLAGS) -c $<

shared: all
	$(CXX) $(CFLAGS) -fpic -fPIC -shared -o libSGE.so $(OBJECTS) $(LIBS)

shared-strip:	shared
	@strip libSGE.so

# Building a dll... I have no idea how to do this, but it should be something like below.
dll:	config $(OBJECTS)
	dlltool --output-def SGE.def $(OBJECTS)
	dllwrap --driver-name $(CXX) -o SGE.dll --def SGE.def --output-lib libSGE.a --dllname SGE.dll $(OBJECTS) $(LIBS)

dll-strip:	dll
	@strip SGE.dll

clean:
	@rm -f *.o *.so *.a *.dll *.def

config:
	@echo "/* SGE Config header (generated automatically) */" >sge_config.h
	@echo "#define SGE_VER $(SGE_VER)" >>sge_config.h
ifeq ($(C_ONLY),y)
	@echo "--| Warning: Using C references!"
endif
ifeq ($(NOTTF),y)
	@echo "--| Building without TTF support."
	@echo "#define _SGE_NOTTF" >>sge_config.h
else
	@echo "--| Building with TTF support."
endif
ifeq ($(USE_IMG),y)
	@echo "--| Building with SDL_Img (SFont) support."
	@echo "#define _SGE_HAVE_IMG" >>sge_config.h
else
	@echo "--| Building without SDL_Img (SFont) support."
endif
ifeq ($(NO_CLASSES),y)
	@echo "--| Warning: No C++ classes will be build!"
	@echo "#define _SGE_NO_CLASSES" >>sge_config.h
endif

install:	shared
	@mkdir -p $(PREFIX)/include/SDL
	install -c -m 644 sge*.h $(PREFIX)/include/SDL
	@mkdir -p $(PREFIX)/lib
	install -c -m 644 libSGE.a $(PREFIX)/lib
	install -c libSGE.so $(PREFIX)/lib/libSGE.so.$(SGE_VER)
	@cd $(PREFIX)/lib;\
	rm -f libSGE.so;\
	ln -s libSGE.so.$(SGE_VER) libSGE.so
	@echo "** Headerfiles installed in $(PREFIX)/include/SDL"
	@echo "** Library files installed in $(PREFIX)/lib"
