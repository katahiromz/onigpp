# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.10)

# enable testing
enable_testing()

# project info
project(onigpp VERSION 6.9.15
	DESCRIPTION "Oniguruma++ (onigpp) regular expression library"
	LANGUAGES C CXX)

# Set C++11 standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Fallback for older CMake versions that don't support CMAKE_CXX_STANDARD
if(CMAKE_VERSION VERSION_LESS "3.1")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if (WIN32)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		# using Clang
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		# using GCC
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
	endif()
endif()

if (MSVC)
	# replace "/MD" with "/MT" (building without runtime DLLs)
	set(CompilerFlags
		CMAKE_C_FLAGS
		CMAKE_C_FLAGS_DEBUG
		CMAKE_C_FLAGS_RELEASE
		CMAKE_C_FLAGS_RELWITHDEBINFO
		CMAKE_CXX_FLAGS
		CMAKE_CXX_FLAGS_DEBUG
		CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_RELWITHDEBINFO)
	foreach(CompilerFlags ${CompilerFlags})
		string(REPLACE "/MD" "/MT" ${CompilerFlags} "${${CompilerFlags}}")
	endforeach()
endif()

# We want flat layout
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# UTF-8 source support for MSVC
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# option USE_STD_FOR_TESTS
option(USE_STD_FOR_TESTS "Use std:: for tests" OFF)
message(STATUS "USE_STD_FOR_TESTS: ${USE_STD_FOR_TESTS}")
if(USE_STD_FOR_TESTS)
	add_definitions(-DUSE_STD_FOR_TESTS)
endif()

# option NO_TESTS
option(NO_TESTS "Build without tests" OFF)
message(STATUS "NO_TESTS: ${NO_TESTS}")
if(NO_TESTS)
	add_definitions(-DNO_TESTS)
endif()

##############################################################################

# oniguruma
add_subdirectory(oniguruma)

# libonigpp.a
add_library(onigpp STATIC src/onigpp.cpp)
target_include_directories(onigpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/oniguruma/src)
target_link_libraries(onigpp PUBLIC onig)

# tests
if(NOT NO_TESTS)
	add_subdirectory(tests)
endif()

# dialog
if(WIN32 AND NOT NO_TESTS)
	add_subdirectory(dialog)
endif()

##############################################################################
