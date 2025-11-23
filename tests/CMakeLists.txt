# onigpp_test.exe
add_executable(onigpp_test onigpp_test.cpp)
target_compile_definitions(onigpp_test PRIVATE ONIG_STATIC ONIGPP_HEADER_ONLY)
target_include_directories(onigpp_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(onigpp_test PRIVATE onigpp)

# onigpp_testw.exe
add_executable(onigpp_testw onigpp_testw.cpp)
target_compile_definitions(onigpp_testw PRIVATE ONIG_STATIC ONIGPP_HEADER_ONLY)
target_include_directories(onigpp_testw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(onigpp_testw PRIVATE onigpp)
