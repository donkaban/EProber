# HOST - MACOS,LINUX,ARM, ANDROID
# API  - OPENGLES2, OPENGLES3, OPENGL

HOST 	= ARM
API  	= OPENGLES2
TARGET	= ./test

# -------------------------------------------------------------------------------------------

SOURCES = main.cpp 
HEADERS =
COMMON_CXX_FLAGS = -DAPI_$(API) -DHOST_$(HOST)

X_API=-L/usr/X11R6/lib -lX11
ifeq ($(HOST),LINUX)
	CXX=g++
	CXX_FLAGS=-c -Wall -Wextra -pedantic -std=c++11 -O3 -m32 $(COMMON_CXX_FLAGS)
	LNK_FLAGS=-m32 $(X_API)
endif
ifeq ($(HOST),MACOS)
	CXX=clang++
	CXX_FLAGS=-c -Wall -Wextra -pedantic -std=c++11 -O3 -m32 $(COMMON_CXX_FLAGS)
	LNK_FLAGS=-m32 $(X_API)
endif
ifeq ($(HOST),ARM)
	CXX=arm-linux-gnueabi-g++
	CXX_FLAGS=-c -Wall -Wextra -pedantic -std=c++0x  -marm -march=armv7-a $(COMMON_CXX_FLAGS)
	LNK_FLAGS=-marm -march=armv7-a -pthread $(X_API)
endif

ifeq ($(API),OPENGLES2)
	API_LIBS=-lGLESv2 -lEGL
endif

OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(HEADERS) $(TARGET) Makefile
	rm -f $(OBJECTS)

$(TARGET): $(OBJECTS) $(HEADERS)  Makefile
	$(CXX) $(OBJECTS) $(LNK_FLAGS) $(API_LIBS) -o $@
	
.cpp.o: $(SOURCES)  $(HEADERS) 
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
	rm -f *.bin
	rm -f *.result*
	
	
	