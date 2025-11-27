// format_literal_test.cpp --- Tests for onigpp::format_literal function
// Author: katahiromz
// License: BSD-2-Clause

#include "onigpp.h"
#include <iostream>
#include <cassert>
#include <cstring>

// Test helper macro
#define TEST_ASSERT(cond, msg) \
	do { \
		if (!(cond)) { \
			std::cerr << "FAIL: " << msg << std::endl; \
			return 1; \
		} \
	} while(0)

#define TEST_PASS(name) std::cout << "PASS: " << name << std::endl

// Test basic escape sequences
int test_basic_escapes() {
	using namespace onigpp;

	// Test backslash
	TEST_ASSERT(format_literal("\\\\") == "\\", "backslash escape");

	// Test newline
	TEST_ASSERT(format_literal("\\n") == "\n", "newline escape");

	// Test carriage return
	TEST_ASSERT(format_literal("\\r") == "\r", "carriage return escape");

	// Test tab
	TEST_ASSERT(format_literal("\\t") == "\t", "tab escape");

	// Test vertical tab
	TEST_ASSERT(format_literal("\\v") == "\v", "vertical tab escape");

	// Test form feed
	TEST_ASSERT(format_literal("\\f") == "\f", "form feed escape");

	// Test alert
	TEST_ASSERT(format_literal("\\a") == "\a", "alert escape");

	// Test backspace
	TEST_ASSERT(format_literal("\\b") == "\b", "backspace escape");

	// Test null
	TEST_ASSERT(format_literal("\\0").size() == 1 && format_literal("\\0")[0] == '\0', "null escape");

	TEST_PASS("test_basic_escapes");
	return 0;
}

// Test hexadecimal escapes
int test_hex_escapes() {
	using namespace onigpp;

	// Test \x00
	std::string s = format_literal("\\x00");
	TEST_ASSERT(s.size() == 1 && s[0] == '\0', "\\x00 escape");

	// Test \x41 (ASCII 'A')
	TEST_ASSERT(format_literal("\\x41") == "A", "\\x41 escape");

	// Test \xff
	s = format_literal("\\xff");
	TEST_ASSERT(s.size() == 1 && static_cast<unsigned char>(s[0]) == 0xff, "\\xff escape");

	// Test mixed
	TEST_ASSERT(format_literal("\\x48\\x65\\x6c\\x6c\\x6f") == "Hello", "hex 'Hello'");

	// Test incomplete hex (should pass through)
	TEST_ASSERT(format_literal("\\x4") == "\\x4", "incomplete hex");
	TEST_ASSERT(format_literal("\\x") == "\\x", "incomplete hex 2");

	TEST_PASS("test_hex_escapes");
	return 0;
}

// Test Unicode escapes
int test_unicode_escapes() {
	using namespace onigpp;

	// Test \u0041 (ASCII 'A')
	TEST_ASSERT(format_literal("\\u0041") == "A", "\\u0041 escape");

	// Test \u3042 (Japanese hiragana 'a')
	std::string expected_a;
	expected_a += static_cast<char>(0xE3);
	expected_a += static_cast<char>(0x81);
	expected_a += static_cast<char>(0x82);
	TEST_ASSERT(format_literal("\\u3042") == expected_a, "\\u3042 escape");

	// Test \u20AC (Euro sign)
	std::string expected_euro;
	expected_euro += static_cast<char>(0xE2);
	expected_euro += static_cast<char>(0x82);
	expected_euro += static_cast<char>(0xAC);
	TEST_ASSERT(format_literal("\\u20AC") == expected_euro, "\\u20AC escape");

	// Test \U00010000 (Linear B Syllable B008 A)
	std::string expected_linear_b;
	expected_linear_b += static_cast<char>(0xF0);
	expected_linear_b += static_cast<char>(0x90);
	expected_linear_b += static_cast<char>(0x80);
	expected_linear_b += static_cast<char>(0x80);
	TEST_ASSERT(format_literal("\\U00010000") == expected_linear_b, "\\U00010000 escape");

	// Test incomplete Unicode (should pass through)
	TEST_ASSERT(format_literal("\\u004") == "\\u004", "incomplete unicode");
	TEST_ASSERT(format_literal("\\u") == "\\u", "incomplete unicode 2");

	TEST_PASS("test_unicode_escapes");
	return 0;
}

// Test octal escapes
int test_octal_escapes() {
	using namespace onigpp;

	// Test \0 (null) - already tested
	// Test \7
	TEST_ASSERT(format_literal("\\7") == "\7", "\\7 escape");

	// Test \101 (ASCII 'A')
	TEST_ASSERT(format_literal("\\101") == "A", "\\101 escape");

	// Test \377 (0xFF)
	std::string s = format_literal("\\377");
	TEST_ASSERT(s.size() == 1 && static_cast<unsigned char>(s[0]) == 0xff, "\\377 escape");

	// Test \777 - only first 3 digits count, value wraps
	s = format_literal("\\777");
	TEST_ASSERT(s.size() == 1, "\\777 escape size");

	// Test octal + regular
	TEST_ASSERT(format_literal("\\101B") == "AB", "octal + regular");

	TEST_PASS("test_octal_escapes");
	return 0;
}

// Test combined patterns
int test_combined() {
	using namespace onigpp;

	// Test pattern with mixed escapes
	TEST_ASSERT(format_literal("Hello\\nWorld") == "Hello\nWorld", "hello newline world");

	// Test tabs and spaces
	TEST_ASSERT(format_literal("col1\\tcol2\\tcol3") == "col1\tcol2\tcol3", "tabs");

	// Test path with backslashes
	TEST_ASSERT(format_literal("C:\\\\path\\\\to\\\\file") == "C:\\path\\to\\file", "path");

	// Test regex pattern
	TEST_ASSERT(format_literal("\\\\d+\\.\\\\d+") == "\\d+\\.\\d+", "regex pattern");

	TEST_PASS("test_combined");
	return 0;
}

// Test unknown escapes (should pass through)
int test_unknown_escapes() {
	using namespace onigpp;

	// Unknown escapes should pass through unchanged
	TEST_ASSERT(format_literal("\\q") == "\\q", "unknown escape \\q");
	TEST_ASSERT(format_literal("\\z") == "\\z", "unknown escape \\z");
	TEST_ASSERT(format_literal("\\w") == "\\w", "unknown escape \\w");
	TEST_ASSERT(format_literal("\\d") == "\\d", "unknown escape \\d");

	TEST_PASS("test_unknown_escapes");
	return 0;
}

// Test empty and simple strings
int test_edge_cases() {
	using namespace onigpp;

	// Empty string
	TEST_ASSERT(format_literal("") == "", "empty string");

	// No escapes
	TEST_ASSERT(format_literal("hello") == "hello", "no escapes");

	// Only backslash at end (should remain)
	TEST_ASSERT(format_literal("test\\") == "test\\", "trailing backslash");

	// Multiple backslashes
	TEST_ASSERT(format_literal("\\\\\\\\") == "\\\\", "multiple backslashes");

	TEST_PASS("test_edge_cases");
	return 0;
}

// Test with std::string input
int test_string_input() {
	using namespace onigpp;

	std::string input = "Hello\\nWorld";
	TEST_ASSERT(format_literal(input) == "Hello\nWorld", "std::string input");

	TEST_PASS("test_string_input");
	return 0;
}

// Test with wchar_t
int test_wstring() {
	using namespace onigpp;

	// Test basic escapes
	TEST_ASSERT(format_literal(L"\\n") == L"\n", "wstring newline");
	TEST_ASSERT(format_literal(L"\\t") == L"\t", "wstring tab");
	TEST_ASSERT(format_literal(L"\\\\") == L"\\", "wstring backslash");

	// Test hex
	TEST_ASSERT(format_literal(L"\\x41") == L"A", "wstring hex");

	// Test unicode
	TEST_ASSERT(format_literal(L"\\u0041") == L"A", "wstring unicode");

	TEST_PASS("test_wstring");
	return 0;
}

int main() {
	onigpp::auto_init init;

	std::cout << "=== format_literal tests ===" << std::endl;

	int result = 0;
	result += test_basic_escapes();
	result += test_hex_escapes();
	result += test_unicode_escapes();
	result += test_octal_escapes();
	result += test_combined();
	result += test_unknown_escapes();
	result += test_edge_cases();
	result += test_string_input();
	result += test_wstring();

	if (result == 0) {
		std::cout << "\n=== All tests passed ===" << std::endl;
	} else {
		std::cout << "\n=== " << result << " test(s) failed ===" << std::endl;
	}

	return result;
}
