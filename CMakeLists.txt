cmake_minimum_required(VERSION 3.6)
project(pceemu)

INCLUDE_DIRECTORIES(d:/projects/SDL2-2.0.5/i686-w64-mingw32/include)
LINK_DIRECTORIES(d:/projects/SDL2-2.0.5/i686-w64-mingw32/lib)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_EXE_LINKER_FLAGS -mwindows)

set(SOURCE_FILES main.cpp source/huc6280/huc6280.cpp source/huc6260/huc6260.cpp source/huc6260/huc6260.h source/pce.cpp source/pce.h source/huc6280/huc6280.h source/huc6270/huc6270.cpp source/huc6270/huc6270.h)

add_executable(pceemu ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(pceemu mingw32 SDL2 SDL2main)
