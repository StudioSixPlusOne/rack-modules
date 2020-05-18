#MIT License

#Copyright (c) 2018 squinkylabs

#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:

#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.


# makefile fragment to make test.exe, the unit test program.
#include "../../arch.mk"
include $(RACK_DIR)/arch.mk

CXXFLAGS += -I$(RACK_DIR)/include/dsp/ -I./test/fft 
CXXFLAGS += -I./test/kiss_fft130 
CXXFLAGS += -I./test/kiss_fft130/tools

TEST_SOURCES = $(wildcard test/*.cpp)
TEST_SOURCES += $(wildcard test/fft/*.cpp)
TEST_SOURCES += test/kiss_fft130/kiss_fft.c
TEST_SOURCES += test/kiss_fft130/tools/kiss_fftr.c

# DONT USE src/.cpp files

## This is a list of full paths to the .o files we want to build
TEST_OBJECTS = $(patsubst %, build_test/%.o, $(TEST_SOURCES))

build_test/%.cpp.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build_test/%.c.o: %.c
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Always define _PERF for the perf tests.
perf.exe : PERFFLAG = -D _PERF

# Turn off asserts for perf, unless user overrides on command line
perf.exe : FLAGS += $(ASSERTOFF)

FLAGS += $(PERFFLAG)

test.exe : FLAGS += -D _TESTEX

ifeq ($(ARCH), win)
	# don't need these yet
	#  -lcomdlg32 -lole32 -ldsound -lwinmm
test.exe perf.exe : LDFLAGS = -static \
		-mwindows \
		-lpthread -lopengl32 -lgdi32 -lws2_32
endif

ifeq ($(ARCH), lin)
test.exe perf.exe : LDFLAGS = -rdynamic \
		-lpthread -lGL -ldl \
		$(shell pkg-config --libs gtk+-2.0)
endif

ifeq ($(ARCH), mac)
test.exe perf.exe : LDFLAGS = -stdlib=libc++ -lpthread -ldl \
		-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
endif

test : test.exe

## Note that perf and test targets both used build_test for object files,
## So you need to be careful to delete/clean when switching between the two.
## Consider fixing this in the future.
perf : perf.exe

## cleantest will clean out all the test and perf build products
cleantest :
	rm -rfv build_test
	rm -fv test.exe
	rm -fv perf.exe

test.exe : $(TEST_OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

perf.exe : $(TEST_OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)