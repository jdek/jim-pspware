PSPSDK=$(shell psp-config --pspsdk-path)

PSP_LIB = libnewlib-fix.a
PSP_OBJS = newlib-fix.o

PSP_LDFLAGS :=  -v $(PSP_LDFLAGS)

all: release

release : $(PSP_LIB)

clean:
	rm -f *.a *.o

include ../make.base

