project (GrinGo)

include(pch.cmake)

cmake_minimum_required(VERSION 2.4)

set(EXECUTABLE_OUTPUT_PATH ${GrinGo_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables." FORCE)
set(LIBRARY_OUTPUT_PATH ${GrinGo_BINARY_DIR}/lib CACHE PATH "Single output directory for building all libraries." FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wno-deprecated" CACHE STRING "Flags used by the compiler during debug builds." FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wno-deprecated" CACHE STRING "Flags used by the compiler during release builds" FORCE)

if(NOT CMAKE_VERBOSE_MAKEFILE)
	set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "If this value is on, makefiles will be generated without the .SILENT directive, and all commands will be echoed to the console during the make.  This is useful for debugging only. With Visual Studio IDE projects all commands are done without /nologo" FORCE)
endif(NOT CMAKE_VERBOSE_MAKEFILE)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(WITH_CLASP OFF CACHE BOOL "Enable this flag to build GrinGo with an internal clasp version.")
if(WITH_CLASP)
	add_definitions(-DWITH_CLASP)
endif(WITH_CLASP)

add_subdirectory (app)
add_subdirectory (lib)

