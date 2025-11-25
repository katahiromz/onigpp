// backref_test.cpp --- Tests for backreference support
// Author: katahiromz
// License: BSD-2-Clause
// 
// These tests verify Oniguruma-style backreference support in onigpp:
// - Numeric backreferences (\1, \2, ...)
// - Named backreferences (\k<name>, \k'name')
// - Replacement-side capture expansion ($1, $2, ...)
// - Ambiguous-digit resolution (\10 vs \1 + '0')

#include "tests.h"

using sregex = myns::basic_regex<char>;
using smatch = myns::match_results<std::string::const_iterator>;

// Helper macros for test cases
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

// Test basic numeric backreference matching
// Pattern (.+)\1 should match "abab" (where "ab" is captured and repeated)
void TestBasicNumericBackref() {
	TEST_CASE("TestBasicNumericBackref")
	
	// (.+)\1 should match "abab" where "ab" is captured by group 1 and then repeated
	sregex re("(.+)\\1");
	smatch m;
	std::string input = "abab";
	bool found = myns::regex_search(input, m, re);
	assert(found);
	assert(m.size() >= 2);
	assert(m[0].str() == "abab");
	assert(m[1].str() == "ab");
	
	// Should not match "abc" since there's no repetition
	std::string input2 = "abc";
	smatch m2;
	bool found2 = myns::regex_search(input2, m2, re);
	assert(!found2);
	
	TEST_CASE_END("TestBasicNumericBackref")
}

// Test that backreference fails when the referenced group doesn't match the substring
void TestBackrefMismatch() {
	TEST_CASE("TestBackrefMismatch")
	
	// Pattern (\w+)\s+\1 requires the same word twice
	sregex re("(\\w+)\\s+\\1");
	smatch m;
	
	// "hi hi" should match (same word repeated)
	std::string input1 = "hi hi";
	bool found1 = myns::regex_search(input1, m, re);
	assert(found1);
	assert(m[1].str() == "hi");
	
	// "hi bye" should not match (different words)
	std::string input2 = "hi bye";
	smatch m2;
	bool found2 = myns::regex_search(input2, m2, re);
	assert(!found2);
	
	TEST_CASE_END("TestBackrefMismatch")
}

// Test named backreference with \k<name> syntax
void TestNamedBackrefAngleBrackets() {
	TEST_CASE("TestNamedBackrefAngleBrackets")
	
	// (?<word>\w+)\s+\k<word> should match repeated words
	sregex re("(?<word>\\w+)\\s+\\k<word>");
	smatch m;
	
	std::string input = "hello hello";
	bool found = myns::regex_search(input, m, re);
	assert(found);
	assert(m.size() >= 2);
	assert(m[0].str() == "hello hello");
	assert(m[1].str() == "hello");
	
	TEST_CASE_END("TestNamedBackrefAngleBrackets")
}

// Test named backreference with \k'name' syntax (alternative Oniguruma syntax)
void TestNamedBackrefSingleQuotes() {
	TEST_CASE("TestNamedBackrefSingleQuotes")
	
	// (?<word>\w+)\s+\k'word' should also match repeated words
	sregex re("(?<word>\\w+)\\s+\\k'word'");
	smatch m;
	
	std::string input = "world world";
	bool found = myns::regex_search(input, m, re);
	assert(found);
	assert(m.size() >= 2);
	assert(m[0].str() == "world world");
	assert(m[1].str() == "world");
	
	TEST_CASE_END("TestNamedBackrefSingleQuotes")
}

// Test multiple groups with backreferences
void TestMultipleBackrefs() {
	TEST_CASE("TestMultipleBackrefs")
	
	// Pattern (.)(.)(.)\3\2\1 should match palindromes like "abccba"
	sregex re("(.)(.)(.)(.)\\4\\3\\2\\1");
	smatch m;
	
	std::string input = "abcddcba";
	bool found = myns::regex_match(input, m, re);
	assert(found);
	assert(m[0].str() == "abcddcba");
	assert(m[1].str() == "a");
	assert(m[2].str() == "b");
	assert(m[3].str() == "c");
	assert(m[4].str() == "d");
	
	TEST_CASE_END("TestMultipleBackrefs")
}

// Test ambiguous-digit resolution: \10 should resolve to group 10 when defined
void TestAmbiguousDigitResolutionGroup10() {
	TEST_CASE("TestAmbiguousDigitResolutionGroup10")
	
	// Define 10 groups and use \10 to reference group 10
	// Pattern: (a)(b)(c)(d)(e)(f)(g)(h)(i)(j)\10
	// This should match "abcdefghijj" where \10 refers to group 10 (j)
	sregex re("(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)\\10");
	smatch m;
	
	std::string input = "abcdefghijj";
	bool found = myns::regex_match(input, m, re);
	assert(found);
	assert(m.size() == 11);  // 10 groups + full match
	assert(m[10].str() == "j");
	
	TEST_CASE_END("TestAmbiguousDigitResolutionGroup10")
}

// Test that when fewer than 10 groups exist, \12 is interpreted as octal escape
// (Oniguruma interprets \12 as octal 012 = decimal 10 = newline, not as \1 + '2')
void TestAmbiguousDigitResolutionOctal() {
	TEST_CASE("TestAmbiguousDigitResolutionOctal")
	
	// With only 3 groups, \12 is interpreted as octal escape (char 10 = newline)
	sregex re("(.)(.)(.)\\12");
	smatch m;
	
	// "abc\n" should match: groups capture a, b, c; \12 = newline (octal 012)
	std::string input = "abc\n";
	bool found = myns::regex_match(input, m, re);
	assert(found);
	assert(m[1].str() == "a");
	assert(m[2].str() == "b");
	assert(m[3].str() == "c");
	
	TEST_CASE_END("TestAmbiguousDigitResolutionOctal")
}

// Test replacement-side capture expansion ($1, $2, ...)
void TestReplacementCaptures() {
	TEST_CASE("TestReplacementCaptures")
	
	// Pattern (\w+):(\w+) captures two groups
	// Replace with "$2=$1" to swap them
	sregex re("(\\w+):(\\w+)");
	std::string input = "key:value";
	std::string result = myns::regex_replace(input, re, std::string("$2=$1"));
	assert(result == "value=key");
	
	TEST_CASE_END("TestReplacementCaptures")
}

// Test replacement with unmatched optional groups (should expand to empty string)
void TestReplacementUnmatchedGroup() {
	TEST_CASE("TestReplacementUnmatchedGroup")
	
	// Pattern (a)?(b) - group 1 is optional
	sregex re("(a)?(b)");
	
	// Input "b" - group 1 is unmatched, group 2 matches "b"
	std::string input = "b";
	std::string result = myns::regex_replace(input, re, std::string("[$1][$2]"));
	// Unmatched group should expand to empty string
	assert(result == "[][b]");
	
	// Input "ab" - both groups match
	std::string input2 = "ab";
	std::string result2 = myns::regex_replace(input2, re, std::string("[$1][$2]"));
	assert(result2 == "[a][b]");
	
	TEST_CASE_END("TestReplacementUnmatchedGroup")
}

// Test replacement with $& (entire match)
void TestReplacementEntireMatch() {
	TEST_CASE("TestReplacementEntireMatch")
	
	sregex re("\\w+");
	std::string input = "hello world";
	std::string result = myns::regex_replace(input, re, std::string("[$&]"));
	assert(result == "[hello] [world]");
	
	TEST_CASE_END("TestReplacementEntireMatch")
}

// Test replacement with $` (prefix) and $' (suffix)
void TestReplacementPrefixSuffix() {
	TEST_CASE("TestReplacementPrefixSuffix")
	
	sregex re("world");
	std::string input = "hello world!";
	
	// Replace with prefix
	std::string result1 = myns::regex_replace(input, re, std::string("[prefix:$`]"));
	assert(result1 == "hello [prefix:hello ]!");
	
	// Replace with suffix
	std::string result2 = myns::regex_replace(input, re, std::string("[suffix:$']"));
	assert(result2 == "hello [suffix:!]!");
	
	TEST_CASE_END("TestReplacementPrefixSuffix")
}

// Test literal backslash+digit in replacement (escape sequence)
void TestReplacementLiteralBackslash() {
	TEST_CASE("TestReplacementLiteralBackslash")
	
	sregex re("(\\w+)");
	std::string input = "hello";
	
	// $$ should produce literal $
	std::string result = myns::regex_replace(input, re, std::string("$$1"));
	assert(result == "$1");
	
	TEST_CASE_END("TestReplacementLiteralBackslash")
}

// Test case-insensitive backreference
void TestCaseInsensitiveBackref() {
	TEST_CASE("TestCaseInsensitiveBackref")
	
	// With icase flag, backreference should match case-insensitively
	std::string pattern = "(\\w+)\\s+\\1";
	sregex re(pattern, myns::regex_constants::ECMAScript | myns::regex_constants::icase);
	smatch m;
	
	std::string input = "Hello HELLO";
	bool found = myns::regex_search(input, m, re);
	assert(found);
	assert(m[0].str() == "Hello HELLO");
	assert(m[1].str() == "Hello");
	
	TEST_CASE_END("TestCaseInsensitiveBackref")
}

// Test forward reference behavior (reference to group that hasn't matched yet)
// In regex, forward references typically fail or have special semantics
void TestForwardReference() {
	TEST_CASE("TestForwardReference")
	
	// This is a forward reference pattern that uses \1 before group 1 is defined
	// Most regex engines will fail to match this
	sregex re("(\\1a|b)+");
	smatch m;
	
	// This pattern is tricky - testing that it doesn't crash
	std::string input = "bba";
	bool found = myns::regex_search(input, m, re);
	// Just verify it doesn't crash; exact behavior may vary
	std::cout << "  Forward reference test: found=" << found << std::endl;
	
	TEST_CASE_END("TestForwardReference")
}

// Test the oniguruma flag (should work the same since Oniguruma already supports it)
void TestBackrefFlagExplicit() {
	TEST_CASE("TestBackrefFlagExplicit")
	
#ifndef USE_STD_FOR_TESTS
	// Test with explicit oniguruma flag
	std::string pattern = "(\\w+)\\s+\\1";
	sregex re(pattern, myns::regex_constants::ECMAScript | myns::regex_constants::oniguruma);
	smatch m;
	
	std::string input = "test test";
	bool found = myns::regex_search(input, m, re);
	assert(found);
	assert(m[0].str() == "test test");
	std::cout << "  oniguruma flag works correctly" << std::endl;
#else
	std::cout << "  Skipping (USE_STD_FOR_TESTS mode)" << std::endl;
#endif
	
	TEST_CASE_END("TestBackrefFlagExplicit")
}

// Test replacement-side Oniguruma-style backreferences (\1, \2, ...)
// These are only enabled when the regex has the oniguruma flag
void TestOnigurumaReplacementBackrefs() {
	TEST_CASE("TestOnigurumaReplacementBackrefs")
	
#ifndef USE_STD_FOR_TESTS
	// Test basic \1, \2 replacement with oniguruma flag
	std::string pattern = "(\\w+):(\\w+)";
	sregex re(pattern, myns::regex_constants::ECMAScript | myns::regex_constants::oniguruma);
	std::string input = "key:value";
	std::string result = myns::regex_replace(input, re, std::string("\\2=\\1"));
	assert(result == "value=key");
	std::cout << "  \\1, \\2 replacement: PASSED" << std::endl;
	
	// Test that \\ produces a literal backslash
	std::string result2 = myns::regex_replace(input, re, std::string("\\\\1"));
	assert(result2 == "\\1");
	std::cout << "  \\\\ (literal backslash): PASSED" << std::endl;
	
	// Test mixing $-style and \-style backreferences
	std::string result3 = myns::regex_replace(input, re, std::string("$1-\\2"));
	assert(result3 == "key-value");
	std::cout << "  Mixed $1 and \\2: PASSED" << std::endl;
	
	// Test multi-digit backreference
	std::string pattern10 = "(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)";
	sregex re10(pattern10, myns::regex_constants::ECMAScript | myns::regex_constants::oniguruma);
	std::string input10 = "abcdefghij";
	std::string result10 = myns::regex_replace(input10, re10, std::string("[\\10]"));
	assert(result10 == "[j]");  // \10 refers to group 10
	std::cout << "  \\10 multi-digit: PASSED" << std::endl;
#else
	std::cout << "  Skipping (USE_STD_FOR_TESTS mode)" << std::endl;
#endif
	
	TEST_CASE_END("TestOnigurumaReplacementBackrefs")
}

// Test that without oniguruma flag, \1 is NOT expanded (literal backslash)
void TestNoOnigurumaFlagBackslashLiteral() {
	TEST_CASE("TestNoOnigurumaFlagBackslashLiteral")
	
#ifndef USE_STD_FOR_TESTS
	// Without oniguruma flag, \1 should be literal backslash followed by '1'
	std::string pattern = "(\\w+):(\\w+)";
	sregex re(pattern, myns::regex_constants::ECMAScript);  // No oniguruma flag
	std::string input = "key:value";
	std::string result = myns::regex_replace(input, re, std::string("\\1-\\2"));
	// \1 and \2 are NOT expanded, they remain as literal \1 and \2
	assert(result == "\\1-\\2");
	std::cout << "  Without oniguruma flag, \\1 is literal: PASSED" << std::endl;
#else
	std::cout << "  Skipping (USE_STD_FOR_TESTS mode)" << std::endl;
#endif
	
	TEST_CASE_END("TestNoOnigurumaFlagBackslashLiteral")
}

int main() {
	TESTS_OUTPUT_INIT();
	ONIGPP_TEST_INIT;
	
	std::cout << "========================================" << std::endl;
	std::cout << "Backreference Support Tests" << std::endl;
	std::cout << "========================================" << std::endl;
	
	TestBasicNumericBackref();
	TestBackrefMismatch();
	TestNamedBackrefAngleBrackets();
	TestNamedBackrefSingleQuotes();
	TestMultipleBackrefs();
	TestAmbiguousDigitResolutionGroup10();
	TestAmbiguousDigitResolutionOctal();
	TestReplacementCaptures();
	TestReplacementUnmatchedGroup();
	TestReplacementEntireMatch();
	TestReplacementPrefixSuffix();
	TestReplacementLiteralBackslash();
	TestCaseInsensitiveBackref();
	TestForwardReference();
	TestBackrefFlagExplicit();
	TestOnigurumaReplacementBackrefs();
	TestNoOnigurumaFlagBackslashLiteral();
	
	std::cout << "\n========================================" << std::endl;
	std::cout << "All backreference tests PASSED!" << std::endl;
	std::cout << "========================================" << std::endl;
	
	return 0;
}
