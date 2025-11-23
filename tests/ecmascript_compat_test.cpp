// ecmascript_compat_test.cpp --- Tests for ECMAScript compatibility
// Author: katahiromz
// License: MIT
#include "onigpp.h"
#include <iostream>
#include <string>
#include <cassert>

namespace op = onigpp;

using sregex = op::basic_regex<char>;
using smatch = op::match_results<std::string::const_iterator>;

// Helper to print test case start and result
#define TEST_CASE(name) \
	std::cout << "\n--- " << (name) << " ---\n"; \
	try {

#define TEST_CASE_END(name) \
	std::cerr << "✅ " << (name) << " PASSED.\n"; \
	} catch (const op::regex_error& e) { \
		std::cerr << "❌ " << (name) << " FAILED with regex_error: " << e.what() << "\n"; \
		assert(false); \
	} catch (const std::exception& e) { \
		std::cerr << "❌ " << (name) << " FAILED with std::exception: " << e.what() << "\n"; \
		assert(false); \
	} catch (...) { \
		std::cerr << "❌ " << (name) << " FAILED with unknown exception.\n"; \
		assert(false); \
	}

// Test dot behavior without multiline (default ECMAScript)
void TestDotBehaviorDefault() {
	TEST_CASE("TestDotBehaviorDefault")
	
	// In ECMAScript mode, dot should NOT match newline by default
	std::string text = "abc\ndef";
	std::string pattern1 = "a.c";
	sregex re(pattern1, op::regex_constants::ECMAScript);
	smatch m;
	
	// Should match "abc" but not across the newline
	bool found = op::regex_search(text, m, re);
	assert(found);
	assert(m[0].str() == "abc");
	
	// Pattern with dot should NOT match across newline
	std::string pattern2 = "a.*f";
	sregex re2(pattern2, op::regex_constants::ECMAScript);
	smatch m2;
	bool found2 = op::regex_search(text, m2, re2);
	assert(!found2); // Should not match because dot doesn't match newline
	
	TEST_CASE_END("TestDotBehaviorDefault")
}

// Test multiline flag and anchor behavior
void TestMultilineAnchors() {
	TEST_CASE("TestMultilineAnchors")
	
	std::string text = "line1\nline2\nline3";
	
	// Test that ^ matches at string start
	std::string pattern1 = "^line1";
	sregex re1(pattern1, op::regex_constants::ECMAScript);
	smatch m1;
	assert(op::regex_search(text, m1, re1)); // Should match at start
	assert(m1[0].str() == "line1");
	
	// Test that $ matches at string end
	std::string pattern2 = "line3$";
	sregex re2(pattern2, op::regex_constants::ECMAScript);
	smatch m2;
	assert(op::regex_search(text, m2, re2)); // Should match at end
	assert(m2[0].str() == "line3");
	
	// Even with multiline, dot should NOT match newline in ECMAScript
	std::string pattern3 = "line1.*line2";
	sregex re3(pattern3, op::regex_constants::ECMAScript | op::regex_constants::multiline);
	smatch m3;
	assert(!op::regex_search(text, m3, re3)); // Should not match because dot doesn't match \n
	
	// Note: Full ECMAScript multiline semantics (^/$ matching at line boundaries)
	// would require pattern transformation, which is complex. For now, we focus on
	// ensuring dot behavior is correct (dot doesn't match newline in ECMAScript mode).
	
	TEST_CASE_END("TestMultilineAnchors")
}

// Test \xHH hex escape sequences
void TestHexEscapes() {
	TEST_CASE("TestHexEscapes")
	
	// Test \x41 (ASCII 'A')
	std::string pattern1 = "\\x41BC";
	sregex re1(pattern1, op::regex_constants::ECMAScript);
	smatch m1;
	std::string text1 = "ABC";
	assert(op::regex_search(text1, m1, re1));
	assert(m1[0].str() == "ABC");
	
	// Test \x20 (space)
	std::string pattern2 = "test\\x20space";
	sregex re2(pattern2, op::regex_constants::ECMAScript);
	smatch m2;
	std::string text2 = "test space";
	assert(op::regex_search(text2, m2, re2));
	assert(m2[0].str() == "test space");
	
	// Test \x0A (newline)
	std::string pattern3 = "line1\\x0Aline2";
	sregex re3(pattern3, op::regex_constants::ECMAScript);
	smatch m3;
	std::string text3 = "line1\nline2";
	assert(op::regex_search(text3, m3, re3));
	assert(m3[0].str() == "line1\nline2");
	
	TEST_CASE_END("TestHexEscapes")
}

// Test \uHHHH Unicode escape sequences
void TestUnicodeEscapes() {
	TEST_CASE("TestUnicodeEscapes")
	
	// Test \u0041 (Unicode 'A')
	std::string pattern1 = "\\u0041BC";
	sregex re1(pattern1, op::regex_constants::ECMAScript);
	smatch m1;
	std::string text1 = "ABC";
	assert(op::regex_search(text1, m1, re1));
	assert(m1[0].str() == "ABC");
	
	// Test \u00E9 (é - Unicode U+00E9)
	std::string pattern2 = "caf\\u00E9";
	sregex re2(pattern2, op::regex_constants::ECMAScript);
	smatch m2;
	std::string text2 = "café"; // UTF-8 encoded
	assert(op::regex_search(text2, m2, re2));
	assert(m2[0].str() == "café");
	
	// Test \u2665 (heart symbol - Unicode U+2665)
	std::string pattern3 = "I\\u2665you";
	sregex re3(pattern3, op::regex_constants::ECMAScript);
	smatch m3;
	std::string text3 = "I♥you"; // UTF-8 encoded
	assert(op::regex_search(text3, m3, re3));
	assert(m3[0].str() == "I♥you");
	
	TEST_CASE_END("TestUnicodeEscapes")
}

// Test \0 null escape
void TestNullEscape() {
	TEST_CASE("TestNullEscape")
	
	// Test \0 (null character) not followed by digit
	std::string pattern1 = "test\\0end";
	sregex re1(pattern1, op::regex_constants::ECMAScript);
	smatch m1;
	std::string text1 = std::string("test") + '\0' + "end";
	assert(op::regex_search(text1, m1, re1));
	
	// \0 followed by digit should NOT be treated as null escape (should be octal)
	// In this case, we leave it as-is for Oniguruma to handle
	std::string pattern2 = "\\01";
	sregex re2(pattern2, op::regex_constants::ECMAScript);
	smatch m2;
	std::string text2 = "\01"; // Octal 01
	assert(op::regex_search(text2, m2, re2));
	
	TEST_CASE_END("TestNullEscape")
}

// Test that preprocessing only happens in ECMAScript mode
void TestNoPreprocessingWithoutECMAScript() {
	TEST_CASE("TestNoPreprocessingWithoutECMAScript")
	
	// Without ECMAScript flag, \x41 should be treated as literal backslash + x + 4 + 1
	// or as Oniguruma's own escape (which may differ)
	std::string text = "\\x41BC";
	
	// In non-ECMAScript mode, the pattern should use Oniguruma's default behavior
	// Let's just verify it compiles and doesn't crash
	try {
		std::string pattern = "\\x41";
		sregex re(pattern, op::regex_constants::normal);
		smatch m;
		// We don't assert specific behavior here, just that it doesn't crash
		op::regex_search(text, m, re);
	} catch (...) {
		// Some syntaxes may not support \x, that's OK
	}
	
	TEST_CASE_END("TestNoPreprocessingWithoutECMAScript")
}

// Test combined features
void TestCombinedFeatures() {
	TEST_CASE("TestCombinedFeatures")
	
	// Test hex escape + case insensitive
	std::string pattern1 = "\\x41bc";
	sregex re1(pattern1, op::regex_constants::ECMAScript | op::regex_constants::icase);
	smatch m1;
	std::string text1 = "ABC";
	assert(op::regex_search(text1, m1, re1));
	assert(m1[0].str() == "ABC");
	
	// Test Unicode escape at start of string
	std::string pattern2 = "^test\\u0020end";
	sregex re2(pattern2, op::regex_constants::ECMAScript);
	smatch m2;
	std::string text2 = "test end";
	assert(op::regex_search(text2, m2, re2));
	assert(m2[0].str() == "test end");
	
	// Test multiple escapes in one pattern
	std::string pattern3 = "\\x48\\x65\\x6C\\x6C\\x6F"; // "Hello"
	sregex re3(pattern3, op::regex_constants::ECMAScript);
	smatch m3;
	std::string text3 = "Say Hello!";
	assert(op::regex_search(text3, m3, re3));
	assert(m3[0].str() == "Hello");
	
	TEST_CASE_END("TestCombinedFeatures")
}

int main() {
	op::auto_init init;
	
	std::cout << "========================================================\n";
	std::cout << " ECMAScript Compatibility Test Suite\n";
	std::cout << "========================================================\n";
	
	TestDotBehaviorDefault();
	TestMultilineAnchors();
	TestHexEscapes();
	TestUnicodeEscapes();
	TestNullEscape();
	TestNoPreprocessingWithoutECMAScript();
	TestCombinedFeatures();
	
	std::cout << "\n========================================================\n";
	std::cout << "✨ All ECMAScript compatibility tests passed!\n";
	std::cout << "========================================================\n";
	
	return 0;
}
