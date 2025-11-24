// error_type_constants_test.cpp --- Test for error_type constants
// Author: katahiromz
// License: BSD-2-Clause

#include "../onigpp.h"
#include <cassert>
#include <iostream>

// Test that error_type constants are defined and have expected values
int main() {
	using namespace onigpp::regex_constants;
	
	// Test that constants are defined with correct values
	assert(error_collate == 1);
	assert(error_ctype == 2);
	assert(error_escape == 3);
	assert(error_backref == 4);
	assert(error_brack == 5);
	assert(error_paren == 6);
	assert(error_range == 7);
	assert(error_space == 8);
	assert(error_badrepeat == 9);
	assert(error_badbrace == 10);
	assert(error_badpattern == 11);
	assert(error_complexity == 12);
	assert(error_stack == 13);
	
	// Test that error_type is the correct type (int)
	static_assert(std::is_same<error_type, int>::value, "error_type should be int");
	
	// Test that constants are constexpr (can be used in constant expressions)
	constexpr error_type test_constexpr = error_collate;
	(void)test_constexpr;
	
	std::cout << "error_type constants test passed!" << std::endl;
	return 0;
}
