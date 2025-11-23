# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.10)

# project name and languages
project(onigpp C CXX)

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

##############################################################################

# oniguruma
add_subdirectory(oniguruma)

# onigpp library
add_library(onigpp STATIC onigpp.cpp)
target_compile_definitions(onigpp PUBLIC ONIG_STATIC)
target_include_directories(onigpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(onigpp PUBLIC onig)

# onigpp_test.exe
add_executable(onigpp_test onigpp_test.cpp)
target_compile_definitions(onigpp_test PRIVATE ONIG_STATIC)
target_include_directories(onigpp_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(onigpp_test onigpp)

# onigpp_testw.exe
add_executable(onigpp_testw onigpp_testw.cpp)
target_compile_definitions(onigpp_testw PRIVATE ONIG_STATIC)
target_include_directories(onigpp_testw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(onigpp_testw onigpp)

##############################################################################
