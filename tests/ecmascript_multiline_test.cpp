// ecmascript_multiline_test.cpp --- Tests for ECMAScript multiline emulation
// Author: katahiromz
// License: BSD-2-Clause
#include "tests.h"

using sregex = rex::basic_regex<char>;
using smatch = rex::match_results<std::string::const_iterator>;

// Helper to print test case start and result
#define TEST_CASE(name) \
	std::cout << "\n--- " << (name) << " ---\n"; \
	try {

#define TEST_CASE_END(name) \
	std::cout << "✅ " << (name) << " PASSED.\n"; \
	} catch (const rex::regex_error& e) { \
		std::cout << "❌ " << (name) << " FAILED with regex_error: " << e.what() << "\n"; \
		assert(false); \
	} catch (const std::exception& e) { \
		std::cout << "❌ " << (name) << " FAILED with std::exception: " << e.what() << "\n"; \
		assert(false); \
	} catch (...) { \
		std::cout << "❌ " << (name) << " FAILED with unknown exception.\n"; \
		assert(false); \
	}

// Test ^ matching at beginning of lines with LF
void TestCaretMatchesLineStartLF() {
	TEST_CASE("TestCaretMatchesLineStartLF")
	
#ifndef USE_STD_FOR_TESTS
	// Test pattern ^line matches at start of string and after \n
	std::string text = "line1\nline2\nline3";
	std::string pattern = "^line\\d";
	sregex re(pattern, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	
	// Find all matches
	auto begin = rex::sregex_iterator(text.begin(), text.end(), re);
	auto end = rex::sregex_iterator();
	
	int count = 0;
	std::vector<std::string> matches;
	for (auto it = begin; it != end; ++it) {
		matches.push_back(it->str());
		count++;
	}
	
	// Should match "line1", "line2", "line3"
	assert(count == 3);
	assert(matches[0] == "line1");
	assert(matches[1] == "line2");
	assert(matches[2] == "line3");
#endif
	
	TEST_CASE_END("TestCaretMatchesLineStartLF")
}

// Test $ matching at end of lines with LF
void TestDollarMatchesLineEndLF() {
	TEST_CASE("TestDollarMatchesLineEndLF")
	
#ifndef USE_STD_FOR_TESTS
	// Test pattern line$ matches at end of each line
	std::string text = "line1\nline2\nline3";
	std::string pattern = "line\\d$";
	sregex re(pattern, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	
	// Find all matches
	auto begin = rex::sregex_iterator(text.begin(), text.end(), re);
	auto end = rex::sregex_iterator();
	
	int count = 0;
	std::vector<std::string> matches;
	for (auto it = begin; it != end; ++it) {
		matches.push_back(it->str());
		count++;
	}
	
	// Should match "line1", "line2", "line3"
	assert(count == 3);
	assert(matches[0] == "line1");
	assert(matches[1] == "line2");
	assert(matches[2] == "line3");
#endif
	
	TEST_CASE_END("TestDollarMatchesLineEndLF")
}

// Test ^ and $ with CRLF line endings
void TestAnchorsWithCRLF() {
	TEST_CASE("TestAnchorsWithCRLF")
	
#ifndef USE_STD_FOR_TESTS
	// Test with Windows-style line endings
	std::string text = "line1\r\nline2\r\nline3";
	
	// Test ^
	std::string pattern1 = "^line\\d";
	sregex re1(pattern1, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	auto begin1 = rex::sregex_iterator(text.begin(), text.end(), re1);
	auto end1 = rex::sregex_iterator();
	int count1 = std::distance(begin1, end1);
	assert(count1 == 3);
	
	// Test $
	std::string pattern2 = "line\\d$";
	sregex re2(pattern2, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	auto begin2 = rex::sregex_iterator(text.begin(), text.end(), re2);
	auto end2 = rex::sregex_iterator();
	int count2 = std::distance(begin2, end2);
	assert(count2 == 3);
#endif
	
	TEST_CASE_END("TestAnchorsWithCRLF")
}

// Test ^ and $ with bare CR line endings
void TestAnchorsWithCR() {
	TEST_CASE("TestAnchorsWithCR")
	
#ifndef USE_STD_FOR_TESTS
	// Test with old Mac-style line endings
	std::string text = "line1\rline2\rline3";
	
	// Test ^
	std::string pattern1 = "^line\\d";
	sregex re1(pattern1, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	auto begin1 = rex::sregex_iterator(text.begin(), text.end(), re1);
	auto end1 = rex::sregex_iterator();
	int count1 = std::distance(begin1, end1);
	assert(count1 == 3);
	
	// Test $
	std::string pattern2 = "line\\d$";
	sregex re2(pattern2, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	auto begin2 = rex::sregex_iterator(text.begin(), text.end(), re2);
	auto end2 = rex::sregex_iterator();
	int count2 = std::distance(begin2, end2);
	assert(count2 == 3);
#endif
	
	TEST_CASE_END("TestAnchorsWithCR")
}

// Test ^ and $ with Unicode line separators
void TestAnchorsWithUnicodeSeparators() {
	TEST_CASE("TestAnchorsWithUnicodeSeparators")
	
#ifndef USE_STD_FOR_TESTS
	// Test with U+2028 (line separator) and U+2029 (paragraph separator)
	// U+2028 in UTF-8: 0xE2 0x80 0xA8
	// U+2029 in UTF-8: 0xE2 0x80 0xA9
	std::string text = "line1\xe2\x80\xa8line2\xe2\x80\xa9line3";
	
	// Test ^
	std::string pattern1 = "^line\\d";
	sregex re1(pattern1, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	auto begin1 = rex::sregex_iterator(text.begin(), text.end(), re1);
	auto end1 = rex::sregex_iterator();
	int count1 = std::distance(begin1, end1);
	assert(count1 == 3);
	
	// Test $
	std::string pattern2 = "line\\d$";
	sregex re2(pattern2, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	auto begin2 = rex::sregex_iterator(text.begin(), text.end(), re2);
	auto end2 = rex::sregex_iterator();
	int count2 = std::distance(begin2, end2);
	assert(count2 == 3);
#endif
	
	TEST_CASE_END("TestAnchorsWithUnicodeSeparators")
}

// Test that dot doesn't match newline even with multiline flag
void TestDotDoesNotMatchNewlineWithMultiline() {
	TEST_CASE("TestDotDoesNotMatchNewlineWithMultiline")
	
#ifndef USE_STD_FOR_TESTS
	// Ensure dot still doesn't match newline when multiline is enabled
	std::string text = "abc\ndef";
	std::string pattern = "a.*f";
	sregex re(pattern, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	smatch m;
	
	// Should NOT match across newline
	bool found = rex::regex_search(text, m, re);
	assert(!found);
#endif
	
	TEST_CASE_END("TestDotDoesNotMatchNewlineWithMultiline")
}

// Test anchors inside character classes (should NOT be transformed)
void TestAnchorsInCharacterClasses() {
	TEST_CASE("TestAnchorsInCharacterClasses")
	
#ifndef USE_STD_FOR_TESTS
	// Test that ^ inside character class is treated as negation, not an anchor
	// Pattern [^ab]c means: "any char except 'a' or 'b', followed by 'c'"
	std::string text1 = "xc";
	std::string pattern1 = "[^ab]c";
	sregex re1(pattern1, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	smatch m1;
	bool found1 = rex::regex_search(text1, m1, re1);
	assert(found1);
	assert(m1[0].str() == "xc");
	
	// Test that the same pattern doesn't match when first char is 'a' or 'b'
	std::string text1b = "ac";
	smatch m1b;
	bool found1b = rex::regex_search(text1b, m1b, re1);
	assert(!found1b);
	
	// $ inside character class should be a literal character, not end anchor
	std::string text2 = "ab$";
	std::string pattern2 = "ab[$]";
	sregex re2(pattern2, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	smatch m2;
	bool found2 = rex::regex_search(text2, m2, re2);
	assert(found2);
	assert(m2[0].str() == "ab$");
#endif
	
	TEST_CASE_END("TestAnchorsInCharacterClasses")
}

// Test escaped anchors (should NOT be transformed)
void TestEscapedAnchors() {
	TEST_CASE("TestEscapedAnchors")
	
#ifndef USE_STD_FOR_TESTS
	// \^ should match literal ^
	std::string text1 = "^test";
	std::string pattern1 = "\\^test";
	sregex re1(pattern1, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	smatch m1;
	bool found1 = rex::regex_search(text1, m1, re1);
	assert(found1);
	assert(m1[0].str() == "^test");
	
	// \$ should match literal $
	std::string text2 = "test$";
	std::string pattern2 = "test\\$";
	sregex re2(pattern2, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	smatch m2;
	bool found2 = rex::regex_search(text2, m2, re2);
	assert(found2);
	assert(m2[0].str() == "test$");
#endif
	
	TEST_CASE_END("TestEscapedAnchors")
}

// Test multiline with complex patterns
void TestComplexMultilinePattern() {
	TEST_CASE("TestComplexMultilinePattern")
	
#ifndef USE_STD_FOR_TESTS
	// Complex pattern with multiple anchors
	std::string text = "Name: John\nAge: 30\nCity: NYC";
	std::string pattern = "^[A-Z][a-z]+:";
	sregex re(pattern, rex::regex_constants::ECMAScript | rex::regex_constants::multiline);
	
	auto begin = rex::sregex_iterator(text.begin(), text.end(), re);
	auto end = rex::sregex_iterator();
	int count = std::distance(begin, end);
	
	// Should match "Name:", "Age:", "City:" at the start of each line
	assert(count == 3);
#endif
	
	TEST_CASE_END("TestComplexMultilinePattern")
}

// Test that without multiline flag, anchors work as string boundaries
void TestWithoutMultilineFlag() {
	TEST_CASE("TestWithoutMultilineFlag")
	
#ifndef USE_STD_FOR_TESTS
	// Without multiline, ^ should only match at string start
	std::string text = "line1\nline2\nline3";
	std::string pattern = "^line\\d";
	sregex re(pattern, rex::regex_constants::ECMAScript); // No multiline flag
	
	auto begin = rex::sregex_iterator(text.begin(), text.end(), re);
	auto end = rex::sregex_iterator();
	int count = std::distance(begin, end);
	
	// Should only match "line1" at string start
	assert(count == 1);
	assert(begin->str() == "line1");
#endif
	
	TEST_CASE_END("TestWithoutMultilineFlag")
}

int main() {
	TESTS_OUTPUT_INIT(false);
	ONIGPP_TEST_INIT;
	
	std::cout << "=== ECMAScript Multiline Emulation Tests ===\n";
	
	TestCaretMatchesLineStartLF();
	TestDollarMatchesLineEndLF();
	TestAnchorsWithCRLF();
	TestAnchorsWithCR();
	TestAnchorsWithUnicodeSeparators();
	TestDotDoesNotMatchNewlineWithMultiline();
	TestAnchorsInCharacterClasses();
	TestEscapedAnchors();
	TestComplexMultilinePattern();
	TestWithoutMultilineFlag();
	
	std::cout << "\n=== All ECMAScript Multiline Tests PASSED ===\n";
	return 0;
}
