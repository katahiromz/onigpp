# onigpp_test.exe
add_executable(onigpp_test onigpp_test.cpp)
target_compile_definitions(onigpp_test PRIVATE "")
target_include_directories(onigpp_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(onigpp_test PRIVATE onigpp)

# onigpp_testw.exe
add_executable(onigpp_testw onigpp_testw.cpp)
target_compile_definitions(onigpp_testw PRIVATE "")
target_include_directories(onigpp_testw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(onigpp_testw PRIVATE onigpp)

# cyclic_replace.exe
add_executable(cyclic_replace cyclic_replace.cpp)
target_compile_definitions(cyclic_replace PRIVATE "")
target_include_directories(cyclic_replace PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cyclic_replace PRIVATE onigpp)

# onigpp_bidir_iterator_test.exe
add_executable(onigpp_bidir_iterator_test onigpp_bidir_iterator_test.cpp)
target_compile_definitions(onigpp_bidir_iterator_test PRIVATE "")
target_include_directories(onigpp_bidir_iterator_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(onigpp_bidir_iterator_test PRIVATE onigpp)

# benchmark_optimization.exe
add_executable(benchmark_optimization benchmark_optimization.cpp)
target_compile_definitions(benchmark_optimization PRIVATE "")
target_include_directories(benchmark_optimization PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(benchmark_optimization PRIVATE onigpp)

# ecmascript_compat_test.exe
add_executable(ecmascript_compat_test ecmascript_compat_test.cpp)
target_compile_definitions(ecmascript_compat_test PRIVATE "")
target_include_directories(ecmascript_compat_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ecmascript_compat_test PRIVATE onigpp)
