cmake_minimum_required(VERSION 3.6)
project(pceemu)

set(CMAKE_CXX_STANDARD 14)

#set(CMAKE_EXE_LINKER_FLAGS -mwindows)

set(SOURCE_FILES main.cpp huc6280.cpp)

add_executable(pceemu ${SOURCE_FILES})
