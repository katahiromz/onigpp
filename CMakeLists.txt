# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.10)

# enable testing
enable_testing()

# project info
project(onigpp VERSION 6.9.11
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

##############################################################################

# oniguruma
add_subdirectory(oniguruma)

# libonigpp.a
add_library(onigpp STATIC src/onigpp.cpp)
target_include_directories(onigpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/oniguruma/src)
target_link_libraries(onigpp PUBLIC onig)

# tests
add_subdirectory(tests)

# CTest tests
add_test(
	NAME test1
	COMMAND $<TARGET_FILE:onigpp_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test2
	COMMAND $<TARGET_FILE:onigpp_testw>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test3
	COMMAND $<TARGET_FILE:cyclic_replace>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test4
	COMMAND $<TARGET_FILE:onigpp_bidir_iterator_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test5
	COMMAND $<TARGET_FILE:ecmascript_compat_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test6
	COMMAND $<TARGET_FILE:iterator_range_ctor_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test7
	COMMAND $<TARGET_FILE:match_results_aliases_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test8
	COMMAND $<TARGET_FILE:swap_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test9
	COMMAND $<TARGET_FILE:initializer_list_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test10
	COMMAND $<TARGET_FILE:match_results_position_length_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test11
	COMMAND $<TARGET_FILE:regex_traits_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test12
	COMMAND $<TARGET_FILE:sub_match_compat_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test13
	COMMAND $<TARGET_FILE:nosubs_collate_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test14
	COMMAND $<TARGET_FILE:match_flag_constants_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test15
	COMMAND $<TARGET_FILE:error_type_constants_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test16
	COMMAND $<TARGET_FILE:ecmascript_multiline_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test17
	COMMAND $<TARGET_FILE:compat_regression_partial_match>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test18
	COMMAND $<TARGET_FILE:nosubs_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test19
	COMMAND $<TARGET_FILE:sub_match_behavior_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME compat_test
	COMMAND $<TARGET_FILE:compat_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

##############################################################################
