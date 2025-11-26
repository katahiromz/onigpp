# onigpp_test.exe
add_executable(onigpp_test onigpp_test.cpp)
target_include_directories(onigpp_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(onigpp_test PRIVATE onigpp)

# onigpp_testw.exe
add_executable(onigpp_testw onigpp_testw.cpp)
target_include_directories(onigpp_testw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(onigpp_testw PRIVATE onigpp)

# cyclic_replace.exe
add_executable(cyclic_replace cyclic_replace.cpp)
target_include_directories(cyclic_replace PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cyclic_replace PRIVATE onigpp)

# onigpp_bidir_iterator_test.exe
add_executable(onigpp_bidir_iterator_test onigpp_bidir_iterator_test.cpp)
target_include_directories(onigpp_bidir_iterator_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(onigpp_bidir_iterator_test PRIVATE onigpp)

# benchmark_optimization.exe
add_executable(benchmark_optimization benchmark_optimization.cpp)
target_include_directories(benchmark_optimization PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(benchmark_optimization PRIVATE onigpp)

# ecmascript_compat_test.exe
add_executable(ecmascript_compat_test ecmascript_compat_test.cpp)
target_include_directories(ecmascript_compat_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ecmascript_compat_test PRIVATE onigpp)

# iterator_range_ctor_test.exe
add_executable(iterator_range_ctor_test iterator_range_ctor_test.cpp)
target_include_directories(iterator_range_ctor_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(iterator_range_ctor_test PRIVATE onigpp)

# match_results_aliases_test.exe
add_executable(match_results_aliases_test match_results_aliases_test.cpp)
target_include_directories(match_results_aliases_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(match_results_aliases_test PRIVATE onigpp)

# swap_test.exe
add_executable(swap_test swap_test.cpp)
target_include_directories(swap_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(swap_test PRIVATE onigpp)

# initializer_list_test.exe
add_executable(initializer_list_test initializer_list_test.cpp)
target_include_directories(initializer_list_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(initializer_list_test PRIVATE onigpp)

# match_results_position_length_test.exe
add_executable(match_results_position_length_test match_results_position_length_test.cpp)
target_include_directories(match_results_position_length_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(match_results_position_length_test PRIVATE onigpp)

# regex_traits_test.exe
add_executable(regex_traits_test regex_traits_test.cpp)
target_include_directories(regex_traits_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(regex_traits_test PRIVATE onigpp)

# sub_match_compat_test.exe
add_executable(sub_match_compat_test sub_match_compat_test.cpp)
target_include_directories(sub_match_compat_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sub_match_compat_test PRIVATE onigpp)

# nosubs_collate_test.exe
add_executable(nosubs_collate_test nosubs_collate_test.cpp)
target_include_directories(nosubs_collate_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nosubs_collate_test PRIVATE onigpp)

# match_flag_constants_test.exe
add_executable(match_flag_constants_test match_flag_constants_test.cpp)
target_include_directories(match_flag_constants_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(match_flag_constants_test PRIVATE onigpp)

# error_type_constants_test.exe
add_executable(error_type_constants_test error_type_constants_test.cpp)
target_include_directories(error_type_constants_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(error_type_constants_test PRIVATE onigpp)

# ecmascript_multiline_test.exe
add_executable(ecmascript_multiline_test ecmascript_multiline_test.cpp)
target_include_directories(ecmascript_multiline_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ecmascript_multiline_test PRIVATE onigpp)

# compat_regression_partial_match.exe
add_executable(compat_regression_partial_match compat_regression_partial_match.cpp)
target_include_directories(compat_regression_partial_match PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# Note: This test invokes the compat_test binary, so it doesn't need to link to onigpp
# target_link_libraries(compat_regression_partial_match PRIVATE onigpp)

# nosubs_test.exe
add_executable(nosubs_test nosubs_test.cpp)
target_include_directories(nosubs_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nosubs_test PRIVATE onigpp)

# sub_match_behavior_test.exe
add_executable(sub_match_behavior_test sub_match_behavior_test.cpp)
target_include_directories(sub_match_behavior_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sub_match_behavior_test PRIVATE onigpp)

# backref_test.exe
add_executable(backref_test backref_test.cpp)
target_include_directories(backref_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(backref_test PRIVATE onigpp)

# empty_pattern_test.exe
add_executable(empty_pattern_test empty_pattern_test.cpp)
target_include_directories(empty_pattern_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(empty_pattern_test PRIVATE onigpp)

# oniguruma_flag_test.exe
add_executable(oniguruma_flag_test oniguruma_flag_test.cpp)
target_include_directories(oniguruma_flag_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(oniguruma_flag_test PRIVATE onigpp)

# match_results_format_test.exe
add_executable(match_results_format_test match_results_format_test.cpp)
target_include_directories(match_results_format_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(match_results_format_test PRIVATE onigpp)

# match_results_ready_test.exe
add_executable(match_results_ready_test match_results_ready_test.cpp)
target_include_directories(match_results_ready_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(match_results_ready_test PRIVATE onigpp)

# compat tests subdirectory
add_subdirectory(compat)

##############################################################################

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
	NAME test20
	COMMAND $<TARGET_FILE:compat_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test21
	COMMAND $<TARGET_FILE:backref_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test22
	COMMAND $<TARGET_FILE:empty_pattern_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test23
	COMMAND $<TARGET_FILE:oniguruma_flag_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test24
	COMMAND $<TARGET_FILE:match_results_format_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
add_test(
	NAME test25
	COMMAND $<TARGET_FILE:match_results_ready_test>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
