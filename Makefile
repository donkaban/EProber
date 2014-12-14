
HOST 	= ARM
API  	= OPENGLES2
TARGET	= ./xtest

# -------------------------------------------------------------------------------------------

SOURCES = main.cpp 
HEADERS =

# -------------------------------------------------------------------------------------------

LINUX_CXX = g++
ARM_CXX   = arm-angstrom-linux-gnueabi-g++

COMMON_CXX_FLAGS = -DAPI_$(API) -DHOST_$(HOST) 

LINUX_CXX_FLAGS  = -c -Wall -Wextra -pedantic -O3 -m32 $(COMMON_CXX_FLAGS)
ARM_CXX_FLAGS    = -c -Wall -Wextra -pedantic  $(COMMON_CXX_FLAGS) 

LINUX_LINK_FLAGS = -m32 -lX11
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
	
	
	