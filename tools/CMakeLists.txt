
# re_replace.exe
add_executable(re_replace re_replace.cpp)
target_include_directories(re_replace PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(re_replace PRIVATE onigpp)
