
EXE=

PROGRAM = if2klicensor

VPATH=$(TOP)/libif2k/gui/license

OBJECTS = ${PROGRAM}.o 
#${PROGRAM}_resources.o

CC = g++

INCLUDES = 

CCSW1 = -Wall -fno-pcc-struct-return -O2 -fno-rtti -fno-exceptions  \
				-DJDK_IS_LINUX=1 -DJDK_HAS_THREADS=0 $(shell wx-config --cxxflags)

CCSW2 = $(shell wx-config --ldflags)

LIBS  = $(shell wx-config --libs) 


.SUFFIXES: .o .cpp

all:    ${OBJECTS}
	$(CC) -o $(PROGRAM)$(EXE) ${OBJECTS} ${CCSW2} ${LIBS} 

.cpp.o: 
	$(CC) ${CCSW1} ${INCLUDES} -c -o $@ $<

%_resources.o : %.rc
	$(RC) ${RESSW} $< $@

.PHONY : clean

clean:
	echo cleaning up
	rm $(OBJECTS)
	rm ${PROGRAM}.exe
