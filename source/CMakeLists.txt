cmake_minimum_required(VERSION 3.12)
project(TINY)

set(Boost_INCLUDE_DIR /usr/local/Cellar/boost/1.68.0/include)
include_directories(${Boost_INCLUDE_DIR})

LINK_DIRECTORIES(/usr/include/boost)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++fs -lboost_system")

file(GLOB SOURCES "*.cpp")
set(SOURCES ${SOURCES})

message(${CMAKE_CXX_STANDARD_LIBRARIES})
add_executable(TINY ${SOURCES})




