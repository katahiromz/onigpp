// error_type_constants_test.cpp --- Test for error_type constants
// Author: katahiromz
// License: BSD-2-Clause

#include "tests.h"

// Test that error_type constants are defined and have expected values
int main() {
	TESTS_OUTPUT_INIT();

	using namespace rex::regex_constants;
	
	// Both onigpp and std::regex now use the same error constant values (starting from 0)
	assert(error_collate == 0);
	assert(error_ctype == 1);
	assert(error_escape == 2);
	assert(error_backref == 3);
	assert(error_brack == 4);
	assert(error_paren == 5);
	assert(error_brace == 6);
	assert(error_badbrace == 7);
	assert(error_range == 8);
	assert(error_space == 9);
	assert(error_badrepeat == 10);
	assert(error_complexity == 11);
	assert(error_stack == 12);
	
	// error_type is now an enum in both onigpp and std::regex (for compatibility)
	static_assert(std::is_enum<error_type>::value, "error_type should be an enum");
	
	// Test that constants are constexpr (can be used in constant expressions)
	constexpr error_type test_constexpr = error_collate;
	(void)test_constexpr;
	
	std::cout << "error_type constants test passed!" << std::endl;
	return 0;
}
