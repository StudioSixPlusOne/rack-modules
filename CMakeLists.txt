cmake_minimum_required(VERSION 3.16)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
project(MyPlugin)

set(PLUGIN_NAME ${PROJECT_NAME})
set(ADDITIONAL_PLUGIN_DISTRIBUTABLES README.md)
include(RackSDK.cmake)

file(GLOB plugin_SRC CONFIGURE_DEPENDS "src/modules/*.cpp" "src/*.cpp" "src/dsp/WaveShapes/*.cpp")
target_sources(${RACK_PLUGIN_LIB} PRIVATE ${plugin_SRC})
target_include_directories(${RACK_PLUGIN_LIB}
        PRIVATE
        src/third-party/sqsrc/util/
        src/composites/
        src/
        src/dsp/
        src/modules/
        src/composites/framework/)

