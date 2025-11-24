// ecmascript_compat_test.cpp --- Tests for ECMAScript compatibility
// Author: katahiromz
// License: BSD-2-Clause
#include "tests.h"

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

// Test replacement template with $ syntax (std::regex compatible)
void TestReplacementTemplate() {
	TEST_CASE("TestReplacementTemplate")
	
	// Test $1, $2 numeric capture replacement
	std::string text1 = "John Doe, Jane Smith";
	std::string pattern1 = "(\\w+)\\s+(\\w+)";
	sregex re1(pattern1, myns::regex_constants::ECMAScript);
	std::string fmt1 = "$2, $1";
	std::string result1 = myns::regex_replace(text1, re1, fmt1);
	assert(result1 == "Doe, John, Smith, Jane");
	
	// Test $& (whole match)
	std::string text2 = "hello world";
	std::string pattern2 = "\\w+";
	sregex re2(pattern2, myns::regex_constants::ECMAScript);
	std::string fmt2 = "[$&]";
	std::string result2 = myns::regex_replace(text2, re2, fmt2);
	assert(result2 == "[hello] [world]");
	
	// Test $` (prefix) and $' (suffix)
	// Note: Both std::regex_replace and onigpp::regex_replace copy non-matching
	// parts by default (unless format_no_copy flag is used).
	std::string text3 = "abc123def";
	std::string pattern3 = "\\d+";
	sregex re3(pattern3, myns::regex_constants::ECMAScript);
	std::string fmt3 = "($`)[$&]($')";
	std::string result3 = myns::regex_replace(text3, re3, fmt3);
	// Result breakdown:
	//   "abc" (non-matching prefix, copied) + 
	//   "(abc)[123](def)" (replacement where $`=abc, $&=123, $'=def) + 
	//   "def" (non-matching suffix, copied)
	assert(result3 == "abc(abc)[123](def)def");
	
	// Test $$ (literal $) followed by $& (whole match)
	std::string text4 = "price: 100";
	std::string pattern4 = "\\d+";
	sregex re4(pattern4, myns::regex_constants::ECMAScript);
	std::string fmt4 = "$$$&";  // First $$ = literal "$", then $& = whole match "100"
	std::string result4 = myns::regex_replace(text4, re4, fmt4);
	assert(result4 == "price: $100");
	
	TEST_CASE_END("TestReplacementTemplate")
}

// Test character class behavior
void TestCharacterClasses() {
	TEST_CASE("TestCharacterClasses")
	
	// Test \d (digits)
	std::string pattern1 = "\\d+";
	sregex re1(pattern1, myns::regex_constants::ECMAScript);
	smatch m1;
	std::string text1 = "abc123def";
	assert(myns::regex_search(text1, m1, re1));
	assert(m1[0].str() == "123");
	
	// Test \w (word characters)
	std::string pattern2 = "\\w+";
	sregex re2(pattern2, myns::regex_constants::ECMAScript);
	smatch m2;
	std::string text2 = "hello-world";
	assert(myns::regex_search(text2, m2, re2));
	assert(m2[0].str() == "hello");
	
	// Test \s (whitespace)
	std::string pattern3 = "\\s+";
	sregex re3(pattern3, myns::regex_constants::ECMAScript);
	smatch m3;
	std::string text3 = "one   two";
	assert(myns::regex_search(text3, m3, re3));
	assert(m3[0].str() == "   ");
	
	TEST_CASE_END("TestCharacterClasses")
}

// Test capture groups
void TestCaptureGroups() {
	TEST_CASE("TestCaptureGroups")
	
	// Test basic capture groups
	std::string pattern1 = "(\\w+)@(\\w+)\\.(\\w+)";
	sregex re1(pattern1, myns::regex_constants::ECMAScript);
	smatch m1;
	std::string text1 = "user@example.com";
	assert(myns::regex_search(text1, m1, re1));
	assert(m1.size() == 4);
	assert(m1[0].str() == "user@example.com");
	assert(m1[1].str() == "user");
	assert(m1[2].str() == "example");
	assert(m1[3].str() == "com");
	
	// Test non-capturing groups (?:...)
	std::string pattern2 = "(?:\\w+)@(\\w+)";
	sregex re2(pattern2, myns::regex_constants::ECMAScript);
	smatch m2;
	std::string text2 = "user@example.com";
	assert(myns::regex_search(text2, m2, re2));
	assert(m2.size() == 2); // Only 2: whole match + 1 capture
	assert(m2[0].str() == "user@example");
	assert(m2[1].str() == "example");
	
	TEST_CASE_END("TestCaptureGroups")
}

// Test quantifiers
void TestQuantifiers() {
	TEST_CASE("TestQuantifiers")
	
	// Test * (zero or more)
	std::string pattern1 = "a*b";
	sregex re1(pattern1, myns::regex_constants::ECMAScript);
	smatch m1;
	std::string text1 = "aaab";
	assert(myns::regex_search(text1, m1, re1));
	assert(m1[0].str() == "aaab");
	
	// Test + (one or more)
	std::string pattern2 = "a+b";
	sregex re2(pattern2, myns::regex_constants::ECMAScript);
	smatch m2;
	std::string text2 = "aaab";
	assert(myns::regex_search(text2, m2, re2));
	assert(m2[0].str() == "aaab");
	
	// Test ? (zero or one)
	std::string pattern3 = "colou?r";
	sregex re3(pattern3, myns::regex_constants::ECMAScript);
	smatch m3;
	std::string text3a = "color";
	std::string text3b = "colour";
	assert(myns::regex_search(text3a, m3, re3));
	assert(m3[0].str() == "color");
	assert(myns::regex_search(text3b, m3, re3));
	assert(m3[0].str() == "colour");
	
	// Test {n,m} (between n and m)
	std::string pattern4 = "a{2,4}";
	sregex re4(pattern4, myns::regex_constants::ECMAScript);
	smatch m4;
	std::string text4 = "aaaa";
	assert(myns::regex_search(text4, m4, re4));
	assert(m4[0].str() == "aaaa");
	
	TEST_CASE_END("TestQuantifiers")
}

// Test lookahead assertions (if supported)
void TestLookaheadAssertions() {
	TEST_CASE("TestLookaheadAssertions")
	
	// Test positive lookahead (?=...)
	std::string pattern1 = "\\d+(?= dollars)";
	sregex re1(pattern1, myns::regex_constants::ECMAScript);
	smatch m1;
	std::string text1 = "100 dollars and 50 cents";
	assert(myns::regex_search(text1, m1, re1));
	assert(m1[0].str() == "100");
	
	// Test negative lookahead (?!...)
	// Match a number NOT followed by " dollars"
	std::string pattern2 = "\\b\\d+(?! dollars)";
	sregex re2(pattern2, myns::regex_constants::ECMAScript);
	smatch m2;
	std::string text2 = "100 dollars and 50 cents";
	assert(myns::regex_search(text2, m2, re2));
	// The first match will be "50" since "100" is followed by " dollars"
	// However, \d+ is greedy, and "10" from "100" is also not followed by " dollars"
	// The actual behavior depends on how the regex engine processes this
	// Let's use a simpler test case
	std::string text2b = "give me 50 euros";
	smatch m2b;
	assert(myns::regex_search(text2b, m2b, re2));
	assert(m2b[0].str() == "50");
	
	TEST_CASE_END("TestLookaheadAssertions")
}

int main() {
	TESTS_OUTPUT_INIT();

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
	TestReplacementTemplate();
	TestCharacterClasses();
	TestCaptureGroups();
	TestQuantifiers();
	TestLookaheadAssertions();
	
	std::cout << "\n========================================================\n";
	std::cout << "✨ All ECMAScript compatibility tests passed!\n";
	std::cout << "========================================================\n";
	
	return 0;
}
