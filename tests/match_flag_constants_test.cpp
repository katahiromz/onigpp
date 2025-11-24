// Test to verify that new match_flag_type constants are properly defined
// This test ensures the constants compile and have non-conflicting values

#include "tests.h"

int main() {
	using namespace myns::regex_constants;

	TESTS_OUTPUT_INIT();

	// Oniguruma initialization (no-op for std::regex)
	ONIGPP_TEST_INIT;

	// Verify constants are defined and have expected bit positions
	std::cout << "Testing match_flag_type constants..." << std::endl;
	
	// Check that constants are defined (these exist in both std::regex and onigpp)
	match_flag_type bow = match_not_bow;
	match_flag_type eow = match_not_eow;
	match_flag_type cont = match_continuous;
	
	// Verify they have non-zero values
	assert(bow != 0);
	assert(eow != 0);
	assert(cont != 0);
	
	// Verify they are distinct from each other
	assert(bow != eow);
	assert(bow != cont);
	assert(eow != cont);
	
	// Verify they don't conflict with existing flags
	assert(bow != match_not_bol);
	assert(bow != match_not_eol);
	assert(bow != match_any);
	assert(bow != match_not_null);
	assert(bow != match_prev_avail);
	assert(bow != format_first_only);
	assert(bow != format_no_copy);
	
#ifndef USE_STD_FOR_TESTS
	// format_literal is only defined in onigpp, not in std::regex
	assert(bow != format_literal);
	
	// onigpp uses different bit positions than std::regex
	assert(match_not_bow == (1 << 11));
	assert(match_not_eow == (1 << 12));
	assert(match_continuous == (1 << 13));
	
	std::cout << "match_not_bow = " << match_not_bow << " (bit 11)" << std::endl;
	std::cout << "match_not_eow = " << match_not_eow << " (bit 12)" << std::endl;
	std::cout << "match_continuous = " << match_continuous << " (bit 13)" << std::endl;
#else
	// std::regex uses different bit positions
	assert(match_not_bow == (1 << 2));
	assert(match_not_eow == (1 << 3));
	assert(match_continuous == (1 << 6));
	
	std::cout << "match_not_bow = " << match_not_bow << " (bit 2)" << std::endl;
	std::cout << "match_not_eow = " << match_not_eow << " (bit 3)" << std::endl;
	std::cout << "match_continuous = " << match_continuous << " (bit 6)" << std::endl;
#endif
	
	// Test that flags can be combined
	match_flag_type combined = match_not_bow | match_not_eow | match_continuous;
	assert(combined != 0);
	assert((combined & match_not_bow) == match_not_bow);
	assert((combined & match_not_eow) == match_not_eow);
	assert((combined & match_continuous) == match_continuous);
	
	std::cout << "All tests passed!" << std::endl;
	
	return 0;
}
