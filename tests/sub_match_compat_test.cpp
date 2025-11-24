// sub_match_compat_test.cpp --- Test std::sub_match compatibility features
// Author: katahiromz
// License: BSD-2-Clause
#include "onigpp.h"
#include <iostream>
#include <string>
#include <cassert>

// =================================================================
// Helper Functions
// =================================================================

#define TEST_CASE(name) \
	std::cout << "\n--- " << (name) << " ---\n"; \
	try {

#define TEST_CASE_END(name) \
	std::cout << "✅ " << (name) << " PASSED.\n"; \
	} catch (const onigpp::regex_error& e) { \
		std::cout << "❌ " << (name) << " FAILED with regex_error: " << e.what() << "\n"; \
		assert(false); \
	} catch (const std::exception& e) { \
		std::cout << "❌ " << (name) << " FAILED with std::exception: " << e.what() << "\n"; \
		assert(false); \
	} catch (...) { \
		std::cout << "❌ " << (name) << " FAILED with unknown exception.\n"; \
		assert(false); \
	}

// -----------------------------------------------------------------
// 1. Test templated converting constructor
// -----------------------------------------------------------------

void TestConvertingConstructor() {
	TEST_CASE("TestConvertingConstructor")

	// Create a sub_match with const char* iterators
	const char* str = "hello world";
	const char* start = str;
	const char* end = str + 5; // "hello"
	
	onigpp::csub_match csm(start, end, true);
	assert(csm.str() == "hello");
	assert(csm.matched == true);
	
	// Convert from csub_match to ssub_match (const char* to string::const_iterator)
	std::string s(str);
	onigpp::ssub_match ssm(csm);  // Should compile with templated converting constructor
	
	// Note: The converted sub_match will have the same matched state
	assert(ssm.matched == true);
	assert(ssm.str() == "hello");
	
	// Test converting unmatched sub_match
	onigpp::csub_match csm_unmatched(start, start, false);
	assert(csm_unmatched.matched == false);
	assert(csm_unmatched.str() == "");
	
	onigpp::ssub_match ssm_unmatched(csm_unmatched);
	assert(ssm_unmatched.matched == false);
	assert(ssm_unmatched.str() == "");

	TEST_CASE_END("TestConvertingConstructor")
}

// -----------------------------------------------------------------
// 2. Test default value for is_matched parameter
// -----------------------------------------------------------------

void TestDefaultIsMatched() {
	TEST_CASE("TestDefaultIsMatched")

	const char* str = "test";
	const char* start = str;
	const char* end = str + 4;
	
	// Without specifying is_matched, it should default to true
	onigpp::csub_match csm(start, end);
	assert(csm.matched == true);  // Default is true
	assert(csm.str() == "test");
	
	// Explicitly set to false
	onigpp::csub_match csm_false(start, end, false);
	assert(csm_false.matched == false);
	assert(csm_false.str() == "test");
	
	// Explicitly set to true
	onigpp::csub_match csm_true(start, end, true);
	assert(csm_true.matched == true);
	assert(csm_true.str() == "test");

	TEST_CASE_END("TestDefaultIsMatched")
}

// -----------------------------------------------------------------
// 3. Test implicit conversion to string_type
// -----------------------------------------------------------------

void TestImplicitStringConversion() {
	TEST_CASE("TestImplicitStringConversion")

	const char* str = "example";
	const char* start = str;
	const char* end = str + 7;
	
	onigpp::csub_match csm(start, end, true);
	
	// Implicit conversion to std::string
	std::string result = csm;  // Should compile due to operator string_type()
	assert(result == "example");
	
	// Test with string iterator
	std::string s = "testing";
	onigpp::ssub_match ssm(s.begin(), s.begin() + 4, true);
	std::string result2 = ssm;  // Should also work
	assert(result2 == "test");
	
	// Test assignment to string
	std::string assigned;
	assigned = csm;
	assert(assigned == "example");

	TEST_CASE_END("TestImplicitStringConversion")
}

// -----------------------------------------------------------------
// 4. Test length() helper method
// -----------------------------------------------------------------

void TestLengthHelper() {
	TEST_CASE("TestLengthHelper")

	const char* str = "hello world";
	const char* start = str;
	const char* end = str + 5; // "hello"
	
	onigpp::csub_match csm(start, end, true);
	assert(csm.length() == 5);
	assert(csm.str() == "hello");
	
	// Test with empty match
	onigpp::csub_match csm_empty(start, start, true);
	assert(csm_empty.length() == 0);
	assert(csm_empty.str() == "");
	
	// Test with string iterator
	std::string s = "testing";
	onigpp::ssub_match ssm(s.begin(), s.begin() + 7, true);
	assert(ssm.length() == 7);
	assert(ssm.str() == "testing");
	
	// Test length matches str().length()
	assert(csm.length() == csm.str().length());
	assert(ssm.length() == ssm.str().length());

	TEST_CASE_END("TestLengthHelper")
}

// -----------------------------------------------------------------
// 5. Test integration with regex_search
// -----------------------------------------------------------------

void TestIntegrationWithRegex() {
	TEST_CASE("TestIntegrationWithRegex")

	std::string text = "User ID: u123";
	onigpp::regex re("ID: ([a-z0-9]+)");
	onigpp::smatch m;
	
	bool found = onigpp::regex_search(text, m, re);
	assert(found);
	assert(m.size() == 2);
	
	// Test length() on matched sub_match
	assert(m[0].length() == 8);  // "ID: u123" is 8 characters
	assert(m[1].length() == 4);  // "u123"
	
	// Test implicit conversion to string
	std::string full_match = m[0];
	std::string captured = m[1];
	assert(full_match == "ID: u123");
	assert(captured == "u123");
	
	// Test that default constructor still sets matched to false
	onigpp::ssub_match default_sm;
	assert(default_sm.matched == false);

	TEST_CASE_END("TestIntegrationWithRegex")
}

// -----------------------------------------------------------------
// Main function
// -----------------------------------------------------------------

int main() {
	std::cout << "=================================\n";
	std::cout << "sub_match std::sub_match Compatibility Tests\n";
	std::cout << "=================================\n";

	TestConvertingConstructor();
	TestDefaultIsMatched();
	TestImplicitStringConversion();
	TestLengthHelper();
	TestIntegrationWithRegex();

	std::cout << "\n=================================\n";
	std::cout << "All tests passed! ✅\n";
	std::cout << "=================================\n";

	return 0;
}
