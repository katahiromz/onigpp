# CMakeLists.txt for compat tests

# compat_test executable
add_executable(compat_test compat_test.cpp)
target_include_directories(compat_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)
target_link_libraries(compat_test PRIVATE onigpp)

# Copy patterns.json to build directory
add_custom_command(TARGET compat_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/patterns.json
    ${CMAKE_BINARY_DIR}/tests/compat/patterns.json)
