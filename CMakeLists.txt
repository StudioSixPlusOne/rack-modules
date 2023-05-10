cmake_minimum_required(VERSION 3.16)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
project(StudioSixPlusOne)


#set(RACK_SDK_DIR /Users/curly/Documents/Development/Rack/)
set(RACK_SDK_DIR /home/curly/Rack/)

set(PLUGIN_NAME ${PROJECT_NAME})
set(ADDITIONAL_PLUGIN_DISTRIBUTABLES README.md)
include(RackSDK.cmake)

file(GLOB plugin_SRC CONFIGURE_DEPENDS "src/modules/*.cpp"
        "src/*.cpp"
        "src/dsp/WaveShapes/*.cpp"
        "test/*.cpp"
        "test/third-party/*.cpp"
        "test/third-party/fft/*.cpp"
        "test/third-party/kiss_fft130/*.c"
        )
target_sources(${RACK_PLUGIN_LIB} PRIVATE ${plugin_SRC})
target_include_directories(${RACK_PLUGIN_LIB}
        PRIVATE
        src/third-party/sqsrc/util/
        src/composites/
        src/
        src/dsp/
        src/modules/
        src/composites/framework/
        test/
        test/signal/
        test/third-party/
        test/third-party/fft/
        test/third-party/kiss_fft130/
        test/third-party/kiss_fft130/tools
        test/utils)

