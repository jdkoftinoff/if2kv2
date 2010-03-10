include $(TOP)/version.txt

CXXFLAGS += -DIF2K_MINI_VERSION="\"$(IF2K_MINI_VERSION)\""
CFLAGS += -DIF2K_MINI_VERSION="\"$(IF2K_MINI_VERSION)\""

CFLAGS += -I$(TOP)/libjdk/include -I$(TOP)/libif2k/include -I$(TOP)/libzip/include -DJDK_IS_LINUX=1 -DJDK_HAS_THREADS=1 $(DEFINES)

CXXFLAGS += -I$(TOP)/libjdk/include -I$(TOP)/libif2k/include -I$(TOP)/libzip/include -DJDK_IS_LINUX=1 -DJDK_HAS_THREADS=1 $(DEFINES)

VPATH += $(TOP)/libjdk/src:$(TOP)/libif2k/src:$(TOP)/libjdk/tool:$(TOP)/libif2k/tool

LIB_CPPFILES += $(notdir $(wildcard $(TOP)/libjdk/src/*.cpp)) \
$(notdir $(wildcard $(TOP)/libzip/src/*.cpp)) \
$(notdir $(wildcard $(TOP)/libif2k/src/*.cpp)) \


LIB_CFILES += $(notdir $(wildcard $(TOP)/libzip/src/*.c)) 

LIB_OFILES = $(LIB_CPPFILES:.cpp=.o) $(LIB_CPPFILES:.c=.o)

all : libjdks.a if2kd if2k_precompile_all if2k_text_licensor


libjdks.a : libjdks.a( $(LIB_OFILES) )

IF2KD_OFILES = if2kd.o
LDFLAGS+=-L. 
LDLIBS+=-ljdks -lpthread

if2kd : $(IF2KD_OFILES)	
	g++ -o $@ $^ $(LDFLAGS) $(LDLIBS)

if2k_precompile_all : if2k_precompile_all.o
	g++ -o $@ $^ $(LDFLAGS) $(LDLIBS)

if2k_text_licensor : if2k_text_licensor.o
	g++ -o $@ $^ $(LDFLAGS) $(LDLIBS)

if2k : if2k.build/src/if2k 
	(cp if2k.build/src/if2k . )

if2k.build/src/if2k : if2k.build/Makefile 
	(cd if2k.build && make)

if2k.build/Makefile : 
	(rm -r -f if2k.build; mkdir if2k.build && cd if2k.build && ../../if2k/autogen.sh )




