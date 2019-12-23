cmake_minimum_required(VERSION 3.12)

project(TINY)

include_directories(include)

find_package(Boost COMPONENTS system filesystem REQUIRED)

LINK_DIRECTORIES(/usr/include/boost)

set(CMAKE_CXX_STANDARD 17)

if (APPLE)
    set(CMAKE_CXX_STANDARD_LIBRARIES "-mmacosx-version-min=10.15")
endif ()

set(CMAKE_CXX_FLAGS "-pthread")

file(GLOB SOURCES "source/*.cpp")
set(SOURCES ${SOURCES})

message("${CMAKE_CXX_STANDARD_LIBRARIES}")
add_executable(TINY ${SOURCES})

target_link_libraries(TINY
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        )

