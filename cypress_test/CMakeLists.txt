cmake_minimum_required(VERSION 3.5)
project(cypress_test)

#link_directories(./dependencies/lib)
#include_directories(./dependencies/inc)

# link_directories(/home/mscvadmin/libcyusbserial/lib)
#link_directories(/Users/mscvadmin/mimic/osx_port/cypress_test/dependencies/lib)

include_directories(/home/mscvadmin/libcyusbserial/include)
#include_directories(/Users/mscvadmin/mimic/osx_port/cypress_test/dependencies/inc)

link_libraries(libcyusbserial.so.1)
#link_libraries(/Users/jsaragih/code/mimic/osx_port/cypress_dynamic_libs/libcyusbserial.0.1.dylib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES Test_Utility.c )
add_executable(cypress_test ${SOURCE_FILES})
