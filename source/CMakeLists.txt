cmake_minimum_required(VERSION 3.15)
project(TINY)

set(CMAKE_CXX_STANDARD 17)

add_executable(TINY main.cpp ../include/P2PNode.h ../include/P2Precord.h ../include/P2PFiles.h ../include/File.h)