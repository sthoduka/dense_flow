cmake_minimum_required(VERSION 2.8)
project( denseFlow )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


find_package( OpenCV REQUIRED )

# BOOST
FIND_PACKAGE(Boost REQUIRED python)
FIND_PACKAGE(PythonLibs REQUIRED)


include_directories( ${OpenCV_INCLUDE_DIRS}  include/ include/easylogging++/src include/CLUE/include)
include_directories(SYSTEM ${Boost_INCLUDE_DIR} ${PYTHON_INCLUDE_DIR})

add_library(denseflow src/common.cpp src/dense_flow.cpp src/dense_warp_flow_cpu.cpp)
target_link_libraries( denseflow ${OpenCV_LIBS} )

add_executable( extract_cpu tools/extract_flow.cpp)
target_link_libraries( extract_cpu ${OpenCV_LIBS}  denseflow)

add_executable( extract_warp_cpu tools/extract_warp_flow_cpu.cpp)
target_link_libraries( extract_warp_cpu ${OpenCV_LIBS}  denseflow)
