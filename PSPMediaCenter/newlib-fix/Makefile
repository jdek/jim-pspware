PSP_LIB = libnewlib-fix.a
PSP_OBJS = newlib-fix.o

PSP_LDFLAGS :=  -v $(PSP_LDFLAGS)

all: release

release : $(PSP_LIB)

clean:
	rm -f *.a *.o

include $(PSPSDK)/build/Makefile.pref
include $(PSPSDK)/build/Makefile.global

