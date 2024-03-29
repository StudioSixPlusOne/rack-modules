

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS += -I./src/third-party/sqsrc/util -I./src/composites -I./src -I./src/dsp
CXXFLAGS += -I./src/modules -I./src/composites/framework

# compile for V1 vs 0.6
FLAGS += -D __V1x
FLAGS += -D _SEQ

# Macro to use on any target where we don't normally want asserts
ASSERTOFF = -D NDEBUG

# Make _ASSERT=true will nullify our ASSERTOFF flag, thus allowing them
ifdef _ASSERT
	ASSERTOFF =
endif

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS += -lpthread

# Add .cpp files to the build
SOURCES += $(wildcard src/modules/*.cpp)
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard src/dsp/WaveShapes/*.cpp)


# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res presets selections
DISTRIBUTABLES += $(wildcard LICENSE*)

# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

# This turns asserts off for make (plugin), not for test or perf
$(TARGET) :  FLAGS += $(ASSERTOFF)

$(TARGET) : FLAGS += -D __PLUGIN

# mac does not like this argument
ifdef ARCH_WIN
	FLAGS += -fmax-errors=5
endif

include test.mk
