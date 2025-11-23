// onigpp_test.cpp --- Tests for Oniguruma++ (onigpp)
// Author: katahiromz
// License: BSD-2-Clause
#include "onigpp.h"
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cassert>
#include <locale>
#include <algorithm>

// --- Additional headers for Windows ---
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

// Alias namespace for ease of use
#ifdef USE_STD_FOR_TESTS
	namespace op = std;
#else
	namespace op = onigpp;
#endif

// Using aliases defined in onigpp.h
using sregex = op::basic_regex<char>;
using smatch = op::match_results<std::string::const_iterator>;
using sregex_iterator = op::regex_iterator<std::string::const_iterator, char>;
using sregex_token_iterator = op::regex_token_iterator<std::string::const_iterator, char>;

// =================================================================
// Helper Functions
// =================================================================

// Helper to print test case start and result
#define TEST_CASE(name) \
	std::cout << "\n--- " << (name) << " ---\n"; \
	try {

#define TEST_CASE_END(name) \
	std::cout << "✅ " << (name) << " PASSED.\n"; \
	} catch (const op::regex_error& e) { \
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
// 1. Core Functions (match, search) and match_results Tests
// -----------------------------------------------------------------

void TestCoreFunctions() {
	TEST_CASE("TestCoreFunctions")

	std::string text = "User ID: u123_abc, User Name: TestUser";
	sregex re("ID: ([a-z0-9_]+), User Name: (.+)");
	smatch m;

	// 1.1. Testing regex_search
	bool found = op::regex_search(text, m, re);
	assert(found);
	assert(m.size() == 3); // Entire match + 2 capture groups

	// Validating capture groups
	assert(m[0].str() == "ID: u123_abc, User Name: TestUser");
	assert(m[1].str() == "u123_abc"); // ID
	assert(m[2].str() == "TestUser"); // Name

	// 1.2. Testing prefix() and suffix()
	assert(m.prefix().str() == "User ");
	assert(m.suffix().str() == ""); // Empty because it matches the whole string

	// 1.3. Testing regex_match (Full match)
	std::string full_text = "start end";
	sregex re_full("start\\s+end");
	smatch m_full;
#ifndef USE_STD_FOR_TESTS
	assert(re_full.pattern() == std::string("start\\s+end"));
#endif
	assert(op::regex_match(full_text, m_full, re_full));
	assert(m_full[0].str() == full_text);

	// 1.4. Testing regex_match (Partial match -> Failure)
	std::string partial_text = "start end extra";
	smatch m_partial;
	assert(!op::regex_match(partial_text, m_partial, re_full));

	TEST_CASE_END("TestCoreFunctions")
}

// -----------------------------------------------------------------
// 2. Resource Management (Copy/Move) Tests
// -----------------------------------------------------------------

void TestResourceManagement() {
	TEST_CASE("TestResourceManagement")

	// 2.1. Testing Copy Constructor and Copy Assignment
	sregex re1("a(b+)c");

	// Copy Constructor
	sregex re2 = re1;
	// Copy Assignment
	sregex re3;
	re3 = re1;

	std::string data = "abbbc";
	smatch m1, m2, m3;

	// Verify they function as independent objects
	assert(op::regex_search(data, m1, re1));
	assert(op::regex_search(data, m2, re2));
	assert(op::regex_search(data, m3, re3));
	assert(m1[1].str() == "bbb");
	assert(m2[1].str() == "bbb");
	assert(m3[1].str() == "bbb");

	// 2.2. Testing Move Semantics (Verify resource is moved and no double-free)
	sregex re_orig("x(y+)z");

	// Move Constructor
	sregex re_moved(std::move(re_orig));

	// Move Assignment Operator
	sregex re_target("dummy");
	sregex re_source("u(v+)w");

	re_target = std::move(re_source);

	// Verify the moved-to object works
	std::string test_str = "uvvvw";
	assert(op::regex_search(test_str, m1, re_target));
	assert(m1[1].str() == "vvv");

	TEST_CASE_END("TestResourceManagement")
}

// -----------------------------------------------------------------
// 3. Iterators (regex_iterator, regex_token_iterator) Tests
// -----------------------------------------------------------------

void TestIterators() {
	TEST_CASE("TestIterators")

	std::string text = "apple,banana.cherry;date";
	sregex re("[\\.\\,\\;]"); // Delimiter

	// 3.1. Testing regex_iterator (Normal match)
	std::cout << "  3.1. regex_iterator:\n";
	sregex re_match("\\w+"); // Word match
	std::vector<std::string> words;
	for (sregex_iterator it(text.begin(), text.end(), re_match), end; it != end; ++it) {
		words.push_back(std::string((*it)[0].first, (*it)[0].second));
	}
	assert(words.size() == 4);
	assert(words[0] == "apple");
	assert(words[3] == "date");

	// 3.2. Testing regex_iterator (Zero-Width match avoidance)
	std::cout << "  3.2. regex_iterator (Zero-Width):\n";
	std::string z_text = "abc";
	sregex re_zero("\\b"); // Word boundary (zero-width)
	std::vector<std::string> boundaries;
	for (sregex_iterator it(z_text.begin(), z_text.end(), re_zero), end; it != end; ++it) {
		boundaries.push_back(std::string((*it)[0].first, (*it)[0].second));
	}
	// "abc" has 3 boundaries: ^, c's next, $ (or before 'a', after 'c')
	// In Oniguruma: start, end, and between \w and \W.
	// before 'a', after 'c'
	assert(boundaries.size() >= 2);
	assert(boundaries[0].empty());
	assert(boundaries.back().empty());


	// 3.3. Testing regex_token_iterator (-1: Non-matching part)
	std::cout << "  3.3. regex_token_iterator (Delimiter):\n";
	std::vector<std::string> tokens;
	// subs = {-1}: Get parts that are NOT the delimiter as tokens
	sregex_token_iterator token_it(text.begin(), text.end(), re, {-1});
	sregex_token_iterator token_end;

	for (; token_it != token_end; ++token_it) {
		tokens.push_back(token_it->str());
	}
	assert(tokens.size() == 4);
	assert(tokens[0] == "apple");
	assert(tokens[1] == "banana");
	assert(tokens[3] == "date");

	// 3.4. Testing regex_token_iterator (Capture Groups)
	std::cout << "  3.4. regex_token_iterator (Capture Groups):\n";
	std::string data_list = "Item1:ValueA,Item2:ValueB";
	sregex re_groups("(\\w+):(\\w+)");
	std::vector<std::string> values;

	// subs = {2}: Get only the 2nd capture group (ValueA, ValueB)
	sregex_token_iterator val_it(data_list.begin(), data_list.end(), re_groups, {2});
	for (; val_it != token_end; ++val_it) {
		values.push_back(val_it->str());
	}
	assert(values.size() == 2);
	assert(values[0] == "ValueA");
	assert(values[1] == "ValueB");

	TEST_CASE_END("TestIterators")
}

// -----------------------------------------------------------------
// 4. Replacement (regex_replace) Tests
// -----------------------------------------------------------------

void TestReplacement() {
	TEST_CASE("TestReplacement")

	// 4.1. Basic Replacement
	std::string s1 = "a b c a b c";
	sregex re1("b");
	std::string fmt1 = "X";
	std::string result1 = op::regex_replace(s1, re1, fmt1);
	assert(result1 == "a X c a X c");

	// 4.2. Capture Group Replacement
	std::string s2 = "Name: John Doe, ID: 123";
	sregex re2("Name: (.*?), ID: (\\d+)");
	std::string fmt2 = "ID $2, Name $1"; // Using $1, $2
	std::string result2 = op::regex_replace(s2, re2, fmt2);
	assert(result2 == "ID 123, Name John Doe");

	// 4.3. Zero-Width Match Replacement (Verification of previous fix)
	std::string s3 = "word";
	sregex re3("\\b"); // Word boundary (zero-width)
	std::string fmt3 = "-";
	std::string result3 = op::regex_replace(s3, re3, fmt3);
	std::cout << "result3: " << result3 << std::endl;
	// Expected result: -word-
	// If it forced the next character output after a zero-width match, it would be "-w-o-r-d-"
	assert(result3 == "-word-");
	// Additional check: the correct number of '-'s (2 at the beginning and 2 at the end)
	size_t dash_count = std::count(result3.begin(), result3.end(), '-');
	assert(dash_count == 2);

	// 4.3a Zero-width anchors: '^' (start) and '$' (end)
	{
		sregex re_start("^");
		std::string res_start = op::regex_replace(s3, re_start, fmt3);
		assert(res_start == "-word");
		assert(std::count(res_start.begin(), res_start.end(), '-') == 1);

		sregex re_end("$");
		std::string res_end = op::regex_replace(s3, re_end, fmt3);
		assert(res_end == "word-");
		assert(std::count(res_end.begin(), res_end.end(), '-') == 1);
	}

	// 4.4. First-only replacement (format_first_only)
	std::string s4 = "1 2 3 4";
	sregex re4(" ");
	std::string fmt4 = "-";
	std::string result4 = op::regex_replace(s4, re4, fmt4, op::regex_constants::format_first_only);
	assert(result4 == "1-2 3 4");

	TEST_CASE_END("TestReplacement")
}

void TestSpecialReplacementPatterns() {
	TEST_CASE("TestSpecialReplacementPatterns")
	
	std::string text = "Start ABC-123-DEF End";
	// Pattern with capture groups (1), (2), (3)
	sregex re("([A-Z]+)-(\\d+)-([A-Z]+)");
	
	// Values in this test case:
	// Entire match ($&): "ABC-123-DEF"
	// Prefix ($`): "Start "
	// Suffix ($'): " End"
	// Group $1: "ABC"

	// --------------------------------------------------
	// 1. Testing $& (Entire Match)
	// --------------------------------------------------
	{
		std::string fmt = "Found: $&. Next Word is $1.";
		std::string expected = "Start Found: ABC-123-DEF. Next Word is ABC. End";
		std::string result = op::regex_replace(text, re, fmt);
		assert(result == expected);
		std::cout << "  $&: OK\n";
	}

	// --------------------------------------------------
	// 2. Testing $` (Prefix)
	// --------------------------------------------------
	{
		// Expecting the entire replacement string to be the prefix
		std::string fmt = "Prefix is: $`."; 
		// After replacement: "Start Prefix is: Start . End"
		std::string expected = "Start Prefix is: Start . End"; 
		std::string result = op::regex_replace(text, re, fmt);
		assert(result == expected);
		std::cout << "  $`: OK\n";
	}

	// --------------------------------------------------
	// 4. Testing $$ (Literal $)
	// --------------------------------------------------
	{
		// Enclose $1 with $
		std::string fmt = "Literal is $$, group is $1.";
		// After replacement: "Start Literal is $, group is ABC. End"
		std::string expected = "Start Literal is $, group is ABC. End";
		std::string result = op::regex_replace(text, re, fmt);
		assert(result == expected);
		std::cout << "  $$: OK\n";
	}

	TEST_CASE_END("TestSpecialReplacementPatterns")
}

// -----------------------------------------------------------------
// 5. Encoding and Error Handling Tests
// -----------------------------------------------------------------

void TestEncodingAndError() {
	TEST_CASE("TestEncodingAndError")

#ifndef USE_STD_FOR_TESTS
	auto UTF8 = op::encoding_constants::UTF8;
	auto SJIS = op::encoding_constants::SHIFT_JIS;

	// 5.1. Encoding Test (UTF-8, Japanese)
	// Assuming Oniguruma's default encoding is UTF-8
	std::string text_utf8 = u8"あいうえお";
	sregex re_utf8(u8"あ", op::regex_constants::normal, UTF8);
	smatch m_utf8;
	assert(op::regex_search(text_utf8, m_utf8, re_utf8));
	assert(m_utf8.str() == u8"あ");

	// 5.2. Encoding Test (Shift_JIS)
	std::string text_sjis = "\x82\xa0\x82\xa2\x82\xa4"; // "あいう" in SJIS

	// Compile with SJIS encoding specified
	sregex re_sjis("\x82\xA0", op::regex_constants::normal, SJIS);

	// The search string is also "あ" in SJIS	
	std::string pattern_sjis = "\x82\xa0";

	smatch m_sjis;
	// Pass begin/end iterators for the std::string text_sjis
	assert(op::regex_search(text_sjis, m_sjis, re_sjis));

	// The match result is a byte-segment of the original string
	assert(m_sjis.str() == pattern_sjis);

	// 5.3. Error Handling Test (Invalid regular expression pattern)
	std::cout << "  5.3. Error Handling:\n";
	bool caught_error = false;
	try {
		sregex re_invalid("[a-"); // Unclosed character class
	} catch (const op::regex_error& e) {
		std::cout << "  (Caught expected error: " << e.what() << ")\n";
		caught_error = true;
	}
	assert(caught_error);

	// 5.4. Testing `basic_regex::assign`
	sregex re_test;
	re_test.assign(std::string("(x+)"), op::regex_constants::icase, SJIS); // Assign with ignore-case option
	std::string test_str = "AXA";
	assert(op::regex_search(test_str, m_utf8, re_test));
	assert(m_utf8.str() == "X"); // icase option is effective
#endif

	TEST_CASE_END("TestEncodingAndError")
}

// -----------------------------------------------------------------
// 6. Syntax Selection Tests
// -----------------------------------------------------------------

void TestSyntaxSelection() {
	TEST_CASE("TestSyntaxSelection")

	// 6.1. POSIX Basic: '+' is literal unless escaped
	std::cout << "  6.1. POSIX Basic Syntax:\n";
	sregex re_basic(std::string("a\\+b"), op::regex_constants::basic);
	smatch m_basic;
	std::string str1 = "a+b";
	assert(op::regex_search(str1, m_basic, re_basic));
	assert(m_basic[0].str() == "a+b");
	std::cout << "  POSIX Basic matched 'a+b' as literal\n";

	// 6.2. POSIX Extended: '+' is a quantifier
	std::cout << "  6.2. POSIX Extended Syntax:\n";
	std::string str2 = "ab+";
	sregex re_ext(str2, op::regex_constants::extended);
	smatch m_ext;
	std::string str3 = "abb";
	assert(op::regex_search(str3, m_ext, re_ext));
	assert(m_ext[0].str() == "abb");
	std::cout << "  POSIX Extended matched 'abb' with '+' as quantifier\n";

	TEST_CASE_END("TestSyntaxSelection")
}

// -----------------------------------------------------------------
// 7. POSIX Character Class Tests
// -----------------------------------------------------------------

void TestPOSIXClasses() {
	TEST_CASE("TestPOSIXClasses")

	// 7.1. Test [:digit:] POSIX class
	std::cout << "  7.1. POSIX [:digit:] class:\n";
	sregex re_digit(std::string("[[:digit:]]+"), op::regex_constants::extended);
	smatch m_digit;
	std::string str1 = "abc12345def";
	assert(op::regex_search(str1, m_digit, re_digit));
	assert(m_digit[0].str() == "12345");
	std::cout << "  [:digit:] matched '12345'\n";
	
	// 7.2. Test [:alpha:] POSIX class
	std::cout << "  7.2. POSIX [:alpha:] class:\n";
	std::string str2 = "[[:alpha:]]+";
	sregex re_alpha(str2, op::regex_constants::extended);
	smatch m_alpha;
	std::string str3 = "123abc456";
	assert(op::regex_search(str3, m_alpha, re_alpha));
	assert(m_alpha[0].str() == "abc");
	std::cout << "  [:alpha:] matched 'abc'\n";
	
	// 7.3. Test [:alnum:] POSIX class
	std::cout << "  7.3. POSIX [:alnum:] class:\n";
	std::string str4 = "[[:alnum:]]+";
	sregex re_alnum(str4, op::regex_constants::extended);
	smatch m_alnum;
	std::string str5 ="!@#abc123$%^";
	assert(op::regex_search(str5, m_alnum, re_alnum));
	assert(m_alnum[0].str() == "abc123");
	std::cout << "  [:alnum:] matched 'abc123'\n";
	
	// 7.4. Test [:space:] POSIX class
	std::cout << "  7.4. POSIX [:space:] class:\n";
	sregex re_space(std::string("[[:space:]]+"), op::regex_constants::extended);
	smatch m_space;
	std::string str6 = "hello   world";
	assert(op::regex_search(str6, m_space, re_space));
	assert(m_space[0].str() == "   ");
	std::cout << "  [:space:] matched three spaces\n";
	
	// 7.5. Test [:upper:] POSIX class
	std::cout << "  7.5. POSIX [:upper:] class:\n";
	std::string str7 = "[[:upper:]]+";
	sregex re_upper(str7, op::regex_constants::extended);
	smatch m_upper;
	std::string str8 = "abcDEFghi";
	assert(op::regex_search(str8, m_upper, re_upper));
	assert(m_upper[0].str() == "DEF");
	std::cout << "  [:upper:] matched 'DEF'\n";
	
	// 7.6. Test [:lower:] POSIX class
	std::cout << "  7.6. POSIX [:lower:] class:\n";
	std::string str9 = "[[:lower:]]+";
	sregex re_lower(str9, op::regex_constants::extended);
	smatch m_lower;
	std::string str10 = "ABCdefGHI";
	assert(op::regex_search(str10, m_lower, re_lower));
	assert(m_lower[0].str() == "def");
	std::cout << "  [:lower:] matched 'def'\n";
	
	// 7.7. Test [:punct:] POSIX class
	std::cout << "  7.7. POSIX [:punct:] class:\n";
	std::string str11 = "[[:punct:]]+";
	sregex re_punct(str11, op::regex_constants::extended);
	smatch m_punct;
	std::string str12 = "hello!@#world";
	assert(op::regex_search(str12, m_punct, re_punct));
	// Match should contain punctuation characters
	assert(m_punct[0].matched);
	std::cout << "  [:punct:] matched punctuation\n";
	
	// 7.8. Test [:xdigit:] POSIX class
	std::cout << "  7.8. POSIX [:xdigit:] class:\n";
	std::string str13 = "[[:xdigit:]]+";
	sregex re_xdigit(str13, op::regex_constants::extended);
	smatch m_xdigit;
	std::string str14 = "xyz1A2FGzz";
	assert(op::regex_search(str14, m_xdigit, re_xdigit));
	assert(m_xdigit[0].str() == "1A2F");
	std::cout << "  [:xdigit:] matched '1A2F'\n";

	TEST_CASE_END("TestPOSIXClasses")
}

// =================================================================
// Main Function
// =================================================================

int main() {
	// --- Measures to avoid garbled characters on Windows consoles ---
#ifdef _WIN32
	// Switch to UTF-8 mode
	//_setmode(_fileno(stdout), _O_U8TEXT);
	// Ensure console uses UTF-8 code page for interoperability
	SetConsoleOutputCP(CP_UTF8);
#else
	// For Linux/Mac, setting the locale is usually sufficient
	std::setlocale(LC_ALL, "");
#endif

#ifndef USE_STD_FOR_TESTS
	// Oniguruma initialization
	op::auto_init init;
#endif

	std::cout << "========================================================\n";
	std::cout << " Starting onigpp.h Comprehensive Test Suite\n";
	std::cout << "========================================================\n";

	TestCoreFunctions();
	TestResourceManagement();
	TestIterators();
	TestReplacement();
	TestSpecialReplacementPatterns();
	TestEncodingAndError();
	TestSyntaxSelection();
	TestPOSIXClasses();

	std::cout << "\n========================================================\n";
	std::cout << "✨ All tests succeeded.\n";
	std::cout << "========================================================\n";

	return 0;
}
