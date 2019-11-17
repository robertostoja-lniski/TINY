cmake_minimum_required(VERSION 3.15)
project(TINY)

LINK_DIRECTORIES(/usr/include/boost)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++fs -lboost_system")

message(${CMAKE_CXX_STANDARD_LIBRARIES})
add_executable(TINY main.cpp ../include/P2PNode.h ../include/P2PRecord.h ../include/P2PFiles.h ../include/File.h ../include/LocalSystemHandler.h LocalSystemHandler.cpp P2PNode.cpp P2PRecord.cpp File.cpp)

