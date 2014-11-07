# HOST - MACOS,LINUX,ARM, ANDROID
# API  - OPENGLES2, OPENGLES3, OPENGL

HOST 	= ARM
API  	= OPENGLES2
TARGET	= ./xtest

# -------------------------------------------------------------------------------------------

SOURCES = main.cpp 
HEADERS =

# -------------------------------------------------------------------------------------------

LINUX_CXX = g++
MACOS_CXX = clang
ARM_CXX   = arm-g++

COMMON_CXX_FLAGS = -DAPI_$(API) -DHOST_$(HOST) 

LINUX_CXX_FLAGS  = -c -Wall -Wextra -pedantic -std=c++11 -O3 -m64 $(COMMON_CXX_FLAGS)
MACOS_CXX_FLAGS  = -c -Wall -Wextra -pedantic -std=c++11 -O3 -m32 $(COMMON_CXX_FLAGS) -I/opt/X11/include
ARM_CXX_FLAGS    = -c -Wall -Wextra -pedantic -std=c++0x  $(COMMON_CXX_FLAGS) 

LINUX_LINK_FLAGS = -m64 -lX11
MACOS_LINK_FLAGS = -m32 -L/opt/X11R6/lib -lX11 
ARM_LINK_FLAGS   = -pthread -lX11

CXX=$($(HOST)_CXX)

ifeq ($(API),OPENGLES2)
	API_LIBS=-lGLESv2 -lEGL
endif

OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(HEADERS) $(TARGET) Makefile
	rm -f $(OBJECTS)

$(TARGET): $(OBJECTS) $(HEADERS)  Makefile
	$(CXX) $(OBJECTS) $($(HOST)_LINK_FLAGS) $(API_LIBS) -o $@
	
.cpp.o: $(SOURCES)  $(HEADERS) 
	$(CXX) $($(HOST)_CXX_FLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
	rm -f *.bin
	rm -f *.result*
	
	
	