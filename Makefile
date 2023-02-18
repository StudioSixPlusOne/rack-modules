# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..
include $(RACK_DIR)/arch.mk

EXTRA_CMAKE :=
RACK_PLUGIN_NAME := plugin
RACK_PLUGIN_ARCH :=
RACK_PLUGIN_EXT := so

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS += -I./src/third-party/sqsrc/util -I./src/composites -I./src -I./src/dsp
CXXFLAGS += -I./src/modules -I./src/composites/framework

ifdef ARCH_WIN
  RACK_PLUGIN_EXT := dll
endif

ifdef ARCH_MAC
  EXTRA_CMAKE := -DCMAKE_OSX_ARCHITECTURES="x86_64"
  RACK_PLUGIN_EXT := dylib
  ifdef ARCH_ARM64
    EXTRA_CMAKE := -DCMAKE_OSX_ARCHITECTURES="arm64"
    RACK_PLUGIN_ARCH := -arm64
  endif
endif

RACK_PLUGIN := $(RACK_PLUGIN_NAME)$(RACK_PLUGIN_ARCH).$(RACK_PLUGIN_EXT)

CMAKE_BUILD ?= dep/cmake-build
cmake_rack_plugin := $(CMAKE_BUILD)/$(RACK_PLUGIN)

# create empty plugin lib to skip the make target execution
$(shell touch $(RACK_PLUGIN))
$(info cmake_rack_plugin target is '$(cmake_rack_plugin)')

# trigger CMake build when running `make dep`
DEPS += $(cmake_rack_plugin)

$(cmake_rack_plugin): CMakeLists.txt
	$(CMAKE) -B $(CMAKE_BUILD) -DRACK_SDK_DIR=$(RACK_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(CMAKE_BUILD)/dist $(EXTRA_CMAKE)
	cmake --build $(CMAKE_BUILD) -- -j $(shell getconf _NPROCESSORS_ONLN)
	cmake --install $(CMAKE_BUILD)

rack_plugin: $(cmake_rack_plugin)
	cp -vf $(cmake_rack_plugin) .

dist: rack_plugin res

# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
