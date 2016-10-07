cmake_minimum_required(VERSION 3.5)
project(kao)

#link_directories(./dependencies/lib)
#include_directories(./dependencies/inc)
#link_libraries(libcyusbserial.0.1.dylib)

#setup opencv
#set(OpenCV_DIR /users/jsaragih/lib/opencv/opencv-3.1.0-config)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})
add_definitions(${OpenCV_DEFINITIONS})

find_package( Threads )
link_libraries(${CMAKE_THREAD_LIBS_INIT})

set(CMAKE_BUILD_TYPE Debug)

find_package(Boost 1.54 COMPONENTS filesystem REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

#link_directories(/home/mscvadmin/libcyusbserial/lib)
#include_directories(/home/mscvadmin/libcyusbserial/include)

include_directories(/home/mscvadmin/libcyusbserial/include)
#include_directories(/Users/mscvadmin/mimic/osx_port/cypress_test/dependencies/inc)
link_libraries(libcyusbserial.so.1)

link_libraries(${OpenCV_LIBRARIES})

link_libraries(${Boost_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#set(SOURCE_FILES GloopMeasureSpiDemo.cpp gloop_measure_spi_cypressusb.c)
#add_executable(kao ${SOURCE_FILES})

#set(SOURCE_FILES test.cpp gloop_measure_spi_cypressusb.c)
set(SOURCE_FILES record_data.cpp gloop_measure_spi_cypressusb.c stfile_io.cpp)

add_executable(record_data ${SOURCE_FILES})
add_executable(read_data read_data.cpp stfile_io.cpp)
add_executable(convert_data convert_data.cpp stfile_io.cpp)

#set(DEVICE_SOURCE_FILES get_device_num.c)
#add_executable(get_num ${DEVICE_SOURCE_FILES})
