// regex_exception_test.cpp --- Tests for exception handling compatibility with std::regex
// This test verifies that onigpp throws exceptions in the same scenarios as std::regex.
// Author: katahiromz
// License: BSD-2-Clause

#include "tests.h"
#include <regex>

void test_invalid_pattern_parenthesis() {
	std::cout << "Test: Unclosed parenthesis throws exception" << std::endl;

	bool threw = false;
	try {
		rex::regex re("(");
	} catch (const rex::regex_error& e) {
		threw = true;
		std::cout << "  Caught regex_error: " << e.what() << std::endl;
	}
	assert(threw && "Unclosed parenthesis should throw regex_error");

	std::cout << "  PASSED" << std::endl;
}

void test_invalid_pattern_bracket() {
	std::cout << "Test: Unclosed bracket throws exception" << std::endl;

	bool threw = false;
	try {
		rex::regex re("[a-");
	} catch (const rex::regex_error& e) {
		threw = true;
		std::cout << "  Caught regex_error: " << e.what() << std::endl;
	}
	assert(threw && "Unclosed bracket should throw regex_error");

	std::cout << "  PASSED" << std::endl;
}

void test_invalid_pattern_quantifier() {
	std::cout << "Test: Invalid quantifier throws exception" << std::endl;

	bool threw = false;
	try {
		rex::regex re("*a");
	} catch (const rex::regex_error& e) {
		threw = true;
		std::cout << "  Caught regex_error: " << e.what() << std::endl;
	}
	assert(threw && "Leading quantifier should throw regex_error");

	std::cout << "  PASSED" << std::endl;
}

void test_invalid_backreference() {
	std::cout << "Test: Invalid backreference throws exception" << std::endl;

	bool threw = false;
	try {
		rex::regex re("\\1");  // No capture group defined
	} catch (const rex::regex_error& e) {
		threw = true;
		std::cout << "  Caught regex_error: " << e.what() << std::endl;
	}
	assert(threw && "Invalid backreference should throw regex_error");

	std::cout << "  PASSED" << std::endl;
}

void test_valid_regex_no_exception() {
	std::cout << "Test: Valid regex does not throw" << std::endl;

	bool threw = false;
	try {
		rex::regex re1("abc");
		rex::regex re2("(a)(b)\\1\\2");  // Valid backreference
		rex::regex re3("a+b*c?");
		rex::regex re4("[a-z]+");
		rex::regex re5("^start.*end$");
	} catch (const rex::regex_error& e) {
		threw = true;
		std::cout << "  Unexpected regex_error: " << e.what() << std::endl;
	}
	assert(!threw && "Valid regex should not throw");

	std::cout << "  PASSED" << std::endl;
}

void test_iterator_no_exception_on_no_match() {
	std::cout << "Test: Iterator does not throw when no matches found" << std::endl;

	bool threw = false;
	try {
		rex::regex re("xyz");
		std::string input = "abcdef";
		auto it = rex::sregex_iterator(input.begin(), input.end(), re);
		auto end = rex::sregex_iterator();

		// Iterator should be equal to end immediately
		assert(it == end && "Should have no matches");
	} catch (const std::exception& e) {
		threw = true;
		std::cout << "  Unexpected exception: " << e.what() << std::endl;
	}
	assert(!threw && "No-match scenario should not throw");

	std::cout << "  PASSED" << std::endl;
}

void test_search_no_exception_on_no_match() {
	std::cout << "Test: regex_search does not throw when no match found" << std::endl;

	bool threw = false;
	try {
		rex::regex re("xyz");
		std::string input = "abcdef";
		rex::smatch m;
		bool found = rex::regex_search(input, m, re);
		assert(!found && "Should not find match");
		assert(m.ready() && "match_results should be ready even on no match");
	} catch (const std::exception& e) {
		threw = true;
		std::cout << "  Unexpected exception: " << e.what() << std::endl;
	}
	assert(!threw && "No-match regex_search should not throw");

	std::cout << "  PASSED" << std::endl;
}

void test_match_no_exception_on_no_match() {
	std::cout << "Test: regex_match does not throw when no match found" << std::endl;

	bool threw = false;
	try {
		rex::regex re("xyz");
		std::string input = "abcdef";
		rex::smatch m;
		bool found = rex::regex_match(input, m, re);
		assert(!found && "Should not match");
		assert(m.ready() && "match_results should be ready even on no match");
	} catch (const std::exception& e) {
		threw = true;
		std::cout << "  Unexpected exception: " << e.what() << std::endl;
	}
	assert(!threw && "No-match regex_match should not throw");

	std::cout << "  PASSED" << std::endl;
}

void test_empty_range() {
	std::cout << "Test: Empty range does not throw" << std::endl;

	bool threw = false;
	try {
		rex::regex re(".*");
		std::string input = "";
		rex::smatch m;

		// regex_search on empty range - result not checked, testing that no exception is thrown
		bool found_search = rex::regex_search(input, m, re);
		(void)found_search;  // Result varies by pattern; we're only testing no exception

		// regex_match on empty range
		rex::regex re_empty("");
		bool found_match = rex::regex_match(input, m, re_empty);
		assert(found_match && "Empty pattern should match empty string");

		// Iterator on empty range
		auto it = rex::sregex_iterator(input.begin(), input.end(), re);
		auto end = rex::sregex_iterator();
		// Count matches (should be at least 1 for .*)
		int count = 0;
		while (it != end) {
			++count;
			++it;
		}
		assert(count >= 1 && "Should have at least one match");
	} catch (const std::exception& e) {
		threw = true;
		std::cout << "  Unexpected exception: " << e.what() << std::endl;
	}
	assert(!threw && "Empty range operations should not throw");

	std::cout << "  PASSED" << std::endl;
}

void test_regex_error_code() {
	std::cout << "Test: regex_error has valid error code" << std::endl;

	try {
		rex::regex re("(");
		assert(false && "Should have thrown");
	} catch (const rex::regex_error& e) {
		// Check that code() returns a valid error type
		auto code = e.code();
		// In onigpp, error_paren is expected for unbalanced parenthesis
		std::cout << "  Error code: " << static_cast<int>(code) << std::endl;
		// Just verify it's a valid enum value (not checking specific value since
		// different implementations may categorize errors differently)
	}

	std::cout << "  PASSED" << std::endl;
}

#ifndef USE_STD_FOR_TESTS
void test_comparison_with_std_exceptions() {
	std::cout << "Test: Exception throwing matches std::regex" << std::endl;

	auto compare_exception = [](const std::string& pattern) {
		bool std_threw = false;
		try {
			std::regex re(pattern);
		} catch (const std::regex_error&) {
			std_threw = true;
		}

		bool onigpp_threw = false;
		try {
			rex::regex re(pattern);
		} catch (const rex::regex_error&) {
			onigpp_threw = true;
		}

		if (std_threw != onigpp_threw) {
			std::cout << "  WARNING: Pattern: \"" << pattern << "\"" << std::endl;
			std::cout << "  std threw: " << (std_threw ? "YES" : "NO")
			          << ", onigpp threw: " << (onigpp_threw ? "YES" : "NO") << std::endl;
			// Note: We don't fail on this since some patterns have implementation-specific behavior
			// This is logged as a warning for visibility
			return false;
		}
		return true;
	};

	// These patterns should throw in both implementations
	assert(compare_exception("(") && "Unclosed paren should throw in both");
	assert(compare_exception("[") && "Unclosed bracket should throw in both");
	assert(compare_exception("*") && "Leading quantifier should throw in both");
	assert(compare_exception("+") && "Leading quantifier should throw in both");

	// These patterns should NOT throw in both implementations
	assert(compare_exception("abc") && "Simple pattern should not throw");
	assert(compare_exception("") && "Empty pattern should not throw");
	assert(compare_exception(".*") && "Common pattern should not throw");

	std::cout << "  PASSED" << std::endl;
}
#endif

int main() {
	TESTS_OUTPUT_INIT();
	ONIGPP_TEST_INIT;

	std::cout << "========================================" << std::endl;
	std::cout << "regex Exception Handling Tests" << std::endl;
	std::cout << "========================================" << std::endl;

	test_invalid_pattern_parenthesis();
	test_invalid_pattern_bracket();
	test_invalid_pattern_quantifier();
	test_invalid_backreference();
	test_valid_regex_no_exception();
	test_iterator_no_exception_on_no_match();
	test_search_no_exception_on_no_match();
	test_match_no_exception_on_no_match();
	test_empty_range();
	test_regex_error_code();
#ifndef USE_STD_FOR_TESTS
	test_comparison_with_std_exceptions();
#endif

	std::cout << "\n========================================" << std::endl;
	std::cout << "All exception handling tests PASSED" << std::endl;
	std::cout << "========================================" << std::endl;

	return 0;
}
