# Makefile for Egg Chess

include sge/Makefile.conf

# Change the following line to point to the directory
# with the sge*.h files (or other needed directories)
CFLAGS =$(SGE_CFLAGS) -Isge/

LIBS =-Lsge/ -lSGE $(SGE_LIBS)


TARGETS = eggchess
OBJECTS = $(addsuffix .o, $(TARGETS))


all:	sgelib $(TARGETS)

$(TARGETS):	%:%.o
	$(CXX) -o $@ $< $(LIBS)		

$(OBJECTS):	%.o:%.cpp
	$(CXX) $(CFLAGS) -c $< 	

strip:	all
	@strip $(TARGETS)

sgelib:
	@(cd sge; $(MAKE))

clean:
	@rm -f *.o $(TARGETS)
	@(cd sge; $(MAKE) clean)
