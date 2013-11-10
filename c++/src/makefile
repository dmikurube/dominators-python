# 
# source files 
#

SOURCES = dom.cpp rfw_timer.cpp dgraph.cpp dgraph_iter.cpp dgraph_snca.cpp \
          dgraph_slt.cpp dgraph_lt.cpp dgraph_sdom.cpp

#
# parameters for various compilers
#

GCC_NAME    = g++
GCC_FLAGS   = -Wall -O4
GCC_LIBS    = -lm -L/usr/lib32/
GCC_DEFINES = -DBOSSA_RUSAGE
GCC_OBJECTS = $(SOURCES:.cpp=.o)
GCC_COUNTOBJ= $(SOURCES:.cpp=.oc)
GCC_REMOVE  = 'rm' -f *.o *.oc

VCC_NAME    = cl 
VCC_FLAGS   = /W3 /O2 /nologo
VCC_DEFINES = -DWIN32 -DNDEBUG -D_CONSOLE 
VCC_LIBS    = 
VCC_OBJECTS = $(SOURCES:.cpp=.obj)
VCC_COUNTOBJ= $(SOURCES:.cpp=.obc)
VCC_REMOVE  = del *.obj *.obc

#
# CHANGE THESE LINES TO USE YOUR FAVORITE COMPILER
#
CCC      = $(GCC_NAME)
FLAGS    = $(GCC_FLAGS)
LIBS     = $(GCC_LIBS)
DEFINES  = $(GCC_DEFINES)
OBJECTS  = $(GCC_OBJECTS)
OBJECTSC = $(GCC_COUNTOBJ)
REMOVE   = $(GCC_REMOVE)

DEFINESC = -DCOUNTOPS $(DEFINES)
INCLUDES = -I.

.SUFFIXES: .cpp

dom: $(OBJECTS)
	$(CCC) $(FLAGS) $(DEFINES) $(INCLUDES) $(OBJECTS) $(LIBS) -o dom

domcount: $(OBJECTSC)
	$(CCC) $(FLAGS) $(DEFINES) -DCOUNTOPS $(INCLUDES) $(OBJECTSC) $(LIBS) -o domcount

all: clean dom domcount

clean: 
	$(REMOVE)	
	
%.oc: %.cpp
	$(CCC) $(DEFINESC) $(FLAGS) -o $*.oc -c $<

.cpp.obc:
	$(CCC) $(DEFINESC) $(FLAGS) /Fo$*.obc -c $<

.cpp.o:
	$(CCC) $(DEFINES) $(FLAGS) -c $<

.cpp.obj:
	$(CCC) $(DEFINES) $(FLAGS) -c $<
