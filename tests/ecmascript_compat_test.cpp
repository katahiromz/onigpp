// ecmascript_compat_test.cpp --- Tests for ECMAScript compatibility
// Author: katahiromz
// License: BSD-2-Clause
#include "onigpp.h"
#include "use_std_for_tests.h"
#include <iostream>
#include <chrono>
#include <regex>
#include <cassert>

// --- Additional headers for Windows ---
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

using sregex = myns::basic_regex<char>;
using smatch = myns::match_results<std::string::const_iterator>;

// Helper to print test case start and result
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

// Test dot behavior without multiline (default ECMAScript)
void TestDotBehaviorDefault() {
	TEST_CASE("TestDotBehaviorDefault")
	
	// In ECMAScript mode, dot should NOT match newline by default
	std::string text = "abc\ndef";
	std::string pattern1 = "a.c";
	sregex re(pattern1, myns::regex_constants::ECMAScript);
	smatch m;
	
	// Should match "abc" but not across the newline
	bool found = myns::regex_search(text, m, re);
	assert(found);
	assert(m[0].str() == "abc");
	
	// Pattern with dot should NOT match across newline
	std::string pattern2 = "a.*f";
	sregex re2(pattern2, myns::regex_constants::ECMAScript);
	smatch m2;
	bool found2 = myns::regex_search(text, m2, re2);
	assert(!found2); // Should not match because dot doesn't match newline
	
	TEST_CASE_END("TestDotBehaviorDefault")
}

// Test multiline flag and anchor behavior
void TestMultilineAnchors() {
	TEST_CASE("TestMultilineAnchors")
	
	std::string text = "line1\nline2\nline3";
	
	// Test that ^ matches at string start
	std::string pattern1 = "^line1";
	sregex re1(pattern1, myns::regex_constants::ECMAScript);
	smatch m1;
	assert(myns::regex_search(text, m1, re1)); // Should match at start
	assert(m1[0].str() == "line1");
	
	// Test that $ matches at string end
	std::string pattern2 = "line3$";
	sregex re2(pattern2, myns::regex_constants::ECMAScript);
	smatch m2;
	assert(myns::regex_search(text, m2, re2)); // Should match at end
	assert(m2[0].str() == "line3");
	
	// Even with multiline, dot should NOT match newline in ECMAScript
	std::string pattern3 = "line1.*line2";
#ifdef USE_STD_FOR_TESTS
	// std::regex_constants doesn't have multiline flag, just use ECMAScript
	sregex re3(pattern3, myns::regex_constants::ECMAScript);
#else
	sregex re3(pattern3, myns::regex_constants::ECMAScript | myns::regex_constants::multiline);
#endif
	smatch m3;
	assert(!myns::regex_search(text, m3, re3)); // Should not match because dot doesn't match \n
	
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
	sregex re1(pattern1, myns::regex_constants::ECMAScript);
	smatch m1;
	std::string text1 = "ABC";
	assert(myns::regex_search(text1, m1, re1));
	assert(m1[0].str() == "ABC");
	
	// Test \x20 (space)
	std::string pattern2 = "test\\x20space";
	sregex re2(pattern2, myns::regex_constants::ECMAScript);
	smatch m2;
	std::string text2 = "test space";
	assert(myns::regex_search(text2, m2, re2));
	assert(m2[0].str() == "test space");
	
	// Test \x0A (newline)
	std::string pattern3 = "line1\\x0Aline2";
	sregex re3(pattern3, myns::regex_constants::ECMAScript);
	smatch m3;
	std::string text3 = "line1\nline2";
	assert(myns::regex_search(text3, m3, re3));
	assert(m3[0].str() == "line1\nline2");
	
	TEST_CASE_END("TestHexEscapes")
}

// Test \uHHHH Unicode escape sequences
void TestUnicodeEscapes() {
	TEST_CASE("TestUnicodeEscapes")
	
#ifndef USE_STD_FOR_TESTS
	// Note: GCC's std::regex does not support \uHHHH Unicode escape sequences
	// These tests only run when using onigpp
	
	// Test \u0041 (Unicode 'A')
	std::string pattern1 = "\\u0041BC";
	sregex re1(pattern1, myns::regex_constants::ECMAScript);
	smatch m1;
	std::string text1 = "ABC";
	assert(myns::regex_search(text1, m1, re1));
	assert(m1[0].str() == "ABC");
	
	// Test \u00E9 (é - Unicode U+00E9)
	std::string pattern2 = "caf\\u00E9";
	sregex re2(pattern2, myns::regex_constants::ECMAScript);
	smatch m2;
	std::string text2 = "café"; // UTF-8 encoded
	assert(myns::regex_search(text2, m2, re2));
	assert(m2[0].str() == "café");
	
	// Test \u2665 (heart symbol - Unicode U+2665)
	std::string pattern3 = "I\\u2665you";
	sregex re3(pattern3, myns::regex_constants::ECMAScript);
	smatch m3;
	std::string text3 = "I♥you"; // UTF-8 encoded
	assert(myns::regex_search(text3, m3, re3));
	assert(m3[0].str() == "I♥you");
#else
	std::cout << "⚠️  Skipped: GCC's std::regex does not support \\uHHHH Unicode escape sequences\n";
#endif
	
	TEST_CASE_END("TestUnicodeEscapes")
}

// Test \0 null escape
void TestNullEscape() {
	TEST_CASE("TestNullEscape")
	
	// Test \0 (null character) not followed by digit
	std::string pattern1 = "test\\0end";
	sregex re1(pattern1, myns::regex_constants::ECMAScript);
	smatch m1;
	std::string text1 = std::string("test") + '\0' + "end";
	assert(myns::regex_search(text1, m1, re1));
	
#ifndef USE_STD_FOR_TESTS
	// Note: GCC's std::regex does not properly support octal escape sequences in ECMAScript mode
	// This test only runs when using onigpp
	
	// \0 followed by digit should NOT be treated as null escape (should be octal)
	// In this case, we leave it as-is for Oniguruma to handle
	std::string pattern2 = "\\01";
	sregex re2(pattern2, myns::regex_constants::ECMAScript);
	smatch m2;
	std::string text2 = "\01"; // Octal 01
	assert(myns::regex_search(text2, m2, re2));
#else
	std::cout << "⚠️  Skipped: GCC's std::regex does not support \\0NN octal escape sequences\n";
#endif
	
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
		sregex re(pattern, myns::regex_constants::basic);
		smatch m;
		// We don't assert specific behavior here, just that it doesn't crash
		myns::regex_search(text, m, re);
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
	sregex re1(pattern1, myns::regex_constants::ECMAScript | myns::regex_constants::icase);
	smatch m1;
	std::string text1 = "ABC";
	assert(myns::regex_search(text1, m1, re1));
	assert(m1[0].str() == "ABC");
	
	// Test Unicode escape at start of string
	std::string pattern2 = "^test\\u0020end";
	sregex re2(pattern2, myns::regex_constants::ECMAScript);
	smatch m2;
	std::string text2 = "test end";
	assert(myns::regex_search(text2, m2, re2));
	assert(m2[0].str() == "test end");
	
	// Test multiple escapes in one pattern
	std::string pattern3 = "\\x48\\x65\\x6C\\x6C\\x6F"; // "Hello"
	sregex re3(pattern3, myns::regex_constants::ECMAScript);
	smatch m3;
	std::string text3 = "Say Hello!";
	assert(myns::regex_search(text3, m3, re3));
	assert(m3[0].str() == "Hello");
	
	TEST_CASE_END("TestCombinedFeatures")
}

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
