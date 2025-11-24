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

# iterator_range_ctor_test.exe
add_executable(iterator_range_ctor_test iterator_range_ctor_test.cpp)
target_compile_definitions(iterator_range_ctor_test PRIVATE "")
target_include_directories(iterator_range_ctor_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(iterator_range_ctor_test PRIVATE onigpp)

# match_results_aliases_test.exe
add_executable(match_results_aliases_test match_results_aliases_test.cpp)
target_compile_definitions(match_results_aliases_test PRIVATE "")
target_include_directories(match_results_aliases_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(match_results_aliases_test PRIVATE onigpp)

# swap_test.exe
add_executable(swap_test swap_test.cpp)
target_compile_definitions(swap_test PRIVATE "")
target_include_directories(swap_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(swap_test PRIVATE onigpp)

# initializer_list_test.exe
add_executable(initializer_list_test initializer_list_test.cpp)
target_compile_definitions(initializer_list_test PRIVATE "")
target_include_directories(initializer_list_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(initializer_list_test PRIVATE onigpp)

# match_results_position_length_test.exe
add_executable(match_results_position_length_test match_results_position_length_test.cpp)
target_compile_definitions(match_results_position_length_test PRIVATE "")
target_include_directories(match_results_position_length_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(match_results_position_length_test PRIVATE onigpp)

# regex_traits_test.exe
add_executable(regex_traits_test regex_traits_test.cpp)
target_compile_definitions(regex_traits_test PRIVATE "")
target_include_directories(regex_traits_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(regex_traits_test PRIVATE onigpp)
