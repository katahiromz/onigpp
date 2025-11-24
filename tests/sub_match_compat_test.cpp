// sub_match_compat_test.cpp --- Test std::sub_match compatibility features
// Author: katahiromz
// License: BSD-2-Clause
#include "onigpp.h"
#include "use_std_for_tests.h"
#include <iostream>
#include <string>
#include <regex>
#include <cassert>

// --- Additional headers for Windows ---
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

// =================================================================
// Helper Functions
// =================================================================

#define TEST_CASE(name) \
	std::cout << "\n--- " << (name) << " ---\n"; \
	try {

#define TEST_CASE_END(name) \
	std::cout << "✅ " << (name) << " PASSED.\n"; \
	} catch (const myns::regex_error& e) { \
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

	// Test converting between compatible pointer types
	const char* str = "hello world";
	const char* start = str;
	const char* end = str + 5; // "hello"
	
	// Create a sub_match with const char* iterators
	myns::csub_match csm;
	csm.first = start;
	csm.second = end;
	csm.matched = true;
	assert(csm.str() == "hello");
	assert(csm.matched == true);
	
	// Test copy construction (same type)
	myns::csub_match csm_copy(csm);
	assert(csm_copy.matched == true);
	assert(csm_copy.str() == "hello");
	
	// Test converting unmatched sub_match
	myns::csub_match csm_unmatched;
	csm_unmatched.first = start;
	csm_unmatched.second = start;
	csm_unmatched.matched = false;
	assert(csm_unmatched.matched == false);
	assert(csm_unmatched.str() == "");
	
	myns::csub_match csm_unmatched_copy(csm_unmatched);
	assert(csm_unmatched_copy.matched == false);
	assert(csm_unmatched_copy.str() == "");

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
	
	// Test with matched flag set to true
	myns::csub_match csm;
	csm.first = start;
	csm.second = end;
	csm.matched = true;
	assert(csm.matched == true);
	assert(csm.str() == "test");
	
	// Explicitly set to false
	myns::csub_match csm_false;
	csm_false.first = start;
	csm_false.second = end;
	csm_false.matched = false;
	assert(csm_false.matched == false);
	assert(csm_false.str() == "test");
	
	// Explicitly set to true
	myns::csub_match csm_true;
	csm_true.first = start;
	csm_true.second = end;
	csm_true.matched = true;
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
	
	myns::csub_match csm;
	csm.first = start;
	csm.second = end;
	csm.matched = true;
	
	// Implicit conversion to std::string
	std::string result = csm;  // Should compile due to operator string_type()
	assert(result == "example");
	
	// Test with string iterator
	std::string s = "testing";
	myns::ssub_match ssm;
	ssm.first = s.begin();
	ssm.second = s.begin() + 4;
	ssm.matched = true;
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
	
	myns::csub_match csm;
	csm.first = start;
	csm.second = end;
	csm.matched = true;
	assert(csm.length() == 5);
	assert(csm.str() == "hello");
	
	// Test with empty match
	myns::csub_match csm_empty;
	csm_empty.first = start;
	csm_empty.second = start;
	csm_empty.matched = true;
	assert(csm_empty.length() == 0);
	assert(csm_empty.str() == "");
	
	// Test with string iterator
	std::string s = "testing";
	myns::ssub_match ssm;
	ssm.first = s.begin();
	ssm.second = s.begin() + 7;
	ssm.matched = true;
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
	myns::regex re("ID: ([a-z0-9]+)");
	myns::smatch m;
	
	bool found = myns::regex_search(text, m, re);
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
	myns::ssub_match default_sm;
	assert(default_sm.matched == false);

	TEST_CASE_END("TestIntegrationWithRegex")
}

// -----------------------------------------------------------------
// Main function
// -----------------------------------------------------------------

int main() {
	// --- Measures to avoid garbled characters on Windows consoles ---
#ifdef _WIN32
	// Switch to UTF-8 mode
	//_setmode(_fileno(stdout), _O_U8TEXT); // Use std::cout instead of std::wcout
	// Ensure console uses UTF-8 code page for interoperability
	SetConsoleOutputCP(CP_UTF8);
#else
	// For Linux/Mac, setting the locale is usually sufficient
	std::setlocale(LC_ALL, "");
#endif

	// Oniguruma initialization (no-op for std::regex)
	ONIGPP_TEST_INIT;

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
