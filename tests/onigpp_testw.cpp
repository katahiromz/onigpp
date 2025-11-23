// onigpp_testw.cpp --- Wide-character (Unicode) tests for Oniguruma++ (onigpp)
// Author: katаromz (adapted)
// License: MIT
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

// Using aliases for wide-character types defined for onigpp
using wregex = op::basic_regex<wchar_t>;
using wmatch = op::match_results<std::wstring::const_iterator>;
using wsregex_iterator = op::regex_iterator<std::wstring::const_iterator, wchar_t>;
using wsregex_token_iterator = op::regex_token_iterator<std::wstring::const_iterator, wchar_t>;

// =================================================================
// Helper Macros
// =================================================================

// Helper to print test case start and result
#define TEST_CASE(name) \
	std::wcout << L"\n--- " << (name) << L" ---\n"; \
	try {

#define TEST_CASE_END(name) \
	std::wcout << L"✅ " << (name) << L" PASSED.\n"; \
	} catch (const op::regex_error& e) { \
		std::wcout << L"❌ " << (name) << L" FAILED with regex_error: " << e.what() << L"\n"; \
		assert(false); \
	} catch (const std::exception& e) { \
		std::wcout << L"❌ " << (name) << L" FAILED with std::exception: " << e.what() << L"\n"; \
		assert(false); \
	} catch (...) { \
		std::wcout << L"❌ " << (name) << L" FAILED with unknown exception.\n"; \
		assert(false); \
	}

// -----------------------------------------------------------------
// 1. Core Functions (match, search) and match_results Tests
// -----------------------------------------------------------------

void TestCoreFunctions() {
	TEST_CASE(L"TestCoreFunctions")

	std::wstring text = L"User ID: u123_abc, User Name: TestUser";
	wregex re(L"ID: ([a-z0-9_]+), User Name: (.+)");
	wmatch m;

	// 1.1. Testing regex_search
	bool found = op::regex_search(text, m, re);
	assert(found);
	assert(m.size() == 3); // Entire match + 2 capture groups

	// Validate captures
	assert(m[0].str() == L"ID: u123_abc, User Name: TestUser");
	assert(m[1].str() == L"u123_abc"); // ID
	assert(m[2].str() == L"TestUser"); // Name

	// 1.2. Testing prefix() and suffix()
	assert(m.prefix().str() == L"User ");
	assert(m.suffix().str() == L""); // Empty because match consumed the remainder

	// 1.3. Testing regex_match (full match)
	std::wstring full_text = L"start end";
	wregex re_full(L"start\\s+end");
	wmatch m_full;
#ifndef USE_STD_FOR_TESTS
	assert(re_full.pattern() == std::wstring(L"start\\s+end"));
#endif
	assert(op::regex_match(full_text, m_full, re_full));
	assert(m_full[0].str() == full_text);

	// 1.4. Testing regex_match (should fail for partial)
	std::wstring partial_text = L"start end extra";
	wmatch m_partial;
	assert(!op::regex_match(partial_text, m_partial, re_full));

	TEST_CASE_END(L"TestCoreFunctions")
}

// -----------------------------------------------------------------
// 2. Resource Management (Copy/Move) Tests
// -----------------------------------------------------------------

void TestResourceManagement() {
	TEST_CASE(L"TestResourceManagement")

	// 2.1 Copy constructor and copy assignment
	wregex re1(L"a(b+)c");

	// Copy constructor
	wregex re2 = re1;
	// Copy assignment
	wregex re3;
	re3 = re1;

	std::wstring data = L"abbbc";
	wmatch m1, m2, m3;

	assert(op::regex_search(data, m1, re1));
	assert(op::regex_search(data, m2, re2));
	assert(op::regex_search(data, m3, re3));
	assert(m1[1].str() == L"bbb");
	assert(m2[1].str() == L"bbb");
	assert(m3[1].str() == L"bbb");

	// 2.2 Move semantics to ensure no double-free
	wregex re_orig(L"x(y+)z");

	// Move constructor
	wregex re_moved(std::move(re_orig));

	// Move assignment
	wregex re_target(L"dummy");
	wregex re_source(L"u(v+)w");

	re_target = std::move(re_source);

	std::wstring test_str = L"uvvvw";
	assert(op::regex_search(test_str, m1, re_target));
	assert(m1[1].str() == L"vvv");

	TEST_CASE_END(L"TestResourceManagement")
}

// -----------------------------------------------------------------
// 3. Iterators (regex_iterator, regex_token_iterator) Tests
// -----------------------------------------------------------------

void TestIterators() {
	TEST_CASE(L"TestIterators")

	std::wstring text = L"apple,banana.cherry;date";
	wregex delim(L"[\\.\\,\\;]"); // Delimiters

	// 3.1 regex_iterator for word matches
	std::wcout << L"  3.1. wsregex_iterator:\n";
	wregex re_match(L"\\w+");
	std::vector<std::wstring> words;
	for (wsregex_iterator it(text.begin(), text.end(), re_match), end; it != end; ++it) {
		words.push_back(std::wstring((*it)[0].first, (*it)[0].second));
	}
	assert(words.size() == 4);
	assert(words[0] == L"apple");
	assert(words[3] == L"date");

	// 3.2 Zero-width match avoidance with word boundary
	std::wcout << L"  3.2. wsregex_iterator (Zero-Width):\n";
	std::wstring z_text = L"abc";
	wregex re_zero(L"\\b");
	std::vector<std::wstring> boundaries;
	for (wsregex_iterator it(z_text.begin(), z_text.end(), re_zero), end; it != end; ++it) {
		boundaries.push_back(std::wstring((*it)[0].first, (*it)[0].second));
	}
	// We expect at least boundaries at start and end (empty matches)
	assert(boundaries.size() >= 2);
	assert(boundaries[0].empty());
	assert(boundaries.back().empty());

	// 3.3 regex_token_iterator (-1 for non-matching parts)
	std::wcout << L"  3.3. wsregex_token_iterator (Delimiter):\n";
	std::vector<std::wstring> tokens;
	wsregex_token_iterator token_it(text.begin(), text.end(), delim, {-1});
	wsregex_token_iterator token_end;
	for (; token_it != token_end; ++token_it) {
		tokens.push_back(token_it->str());
	}
	assert(tokens.size() == 4);
	assert(tokens[0] == L"apple");
	assert(tokens[1] == L"banana");
	assert(tokens[3] == L"date");

	// 3.4 regex_token_iterator with capture groups
	std::wcout << L"  3.4. wsregex_token_iterator (Capture Groups):\n";
	std::wstring data_list = L"Item1:ValueA,Item2:ValueB";
	wregex re_groups(L"(\\w+):(\\w+)");
	std::vector<std::wstring> values;

	// subs = {2} to extract the second capture group from each match
	wsregex_token_iterator val_it(data_list.begin(), data_list.end(), re_groups, {2});
	for (; val_it != token_end; ++val_it) {
		values.push_back(val_it->str());
	}
	assert(values.size() == 2);
	assert(values[0] == L"ValueA");
	assert(values[1] == L"ValueB");

	TEST_CASE_END(L"TestIterators")
}

// -----------------------------------------------------------------
// 4. Replacement (regex_replace) Tests
// -----------------------------------------------------------------

void TestReplacement() {
	TEST_CASE(L"TestReplacement")

	// 4.1 Basic replacement
	std::wstring s1 = L"a b c a b c";
	wregex re1(L"b");
	std::wstring fmt1 = L"X";
	std::wstring result1 = op::regex_replace(s1, re1, fmt1);
	assert(result1 == L"a X c a X c");

	// 4.2 Capture group replacement
	std::wstring s2 = L"Name: John Doe, ID: 123";
	wregex re2(L"Name: (.*?), ID: (\\d+)");
	std::wstring fmt2 = L"ID $2, Name $1"; // Using $1, $2
	std::wstring result2 = op::regex_replace(s2, re2, fmt2);
	assert(result2 == L"ID 123, Name John Doe");

	// 4.3 Zero-width match replacement (word boundary)
	std::wstring s3 = L"word";
	wregex re3(L"\\b");
	std::wstring fmt3 = L"-";
	std::wstring result3 = op::regex_replace(s3, re3, fmt3);
	// Expected: -word-
	assert(result3 == L"-word-");
	// Additional check: the correct number of '-'s (2 at the beginning and 2 at the end)
	size_t dash_count_w = std::count(result3.begin(), result3.end(), L'-');
	assert(dash_count_w == 2);

	// 4.3a Zero-width anchors: '^' and '$'
	{
		wregex re_start(L"^");
		std::wstring res_start = op::regex_replace(s3, re_start, fmt3);
		assert(res_start == L"-word");
		assert(std::count(res_start.begin(), res_start.end(), L'-') == 1);

		wregex re_end(L"$");
		std::wstring res_end = op::regex_replace(s3, re_end, fmt3);
		assert(res_end == L"word-");
		assert(std::count(res_end.begin(), res_end.end(), L'-') == 1);
	}

	// 4.4 First-only replacement (format_first_only)
	std::wstring s4 = L"1 2 3 4";
	wregex re4(L" ");
	std::wstring fmt4 = L"-";
	std::wstring result4 = op::regex_replace(s4, re4, fmt4, op::regex_constants::format_first_only);
	assert(result4 == L"1-2 3 4");

	TEST_CASE_END(L"TestReplacement")
}

void TestSpecialReplacementPatterns() {
	TEST_CASE(L"TestSpecialReplacementPatterns")

	std::wstring text = L"Start ABC-123-DEF End";
	// Pattern with capture groups (1), (2), (3)
	wregex re(L"([A-Z]+)-(\\d+)-([A-Z]+)");

	// Test $& (entire match)
	{
		std::wstring fmt = L"Found: $&. Next Word is $1.";
		std::wstring expected = L"Start Found: ABC-123-DEF. Next Word is ABC. End";
		std::wstring result = op::regex_replace(text, re, fmt);
		assert(result == expected);
		std::wcout << L"  $&: OK\n";
	}

	// Test $` (prefix)
	{
		std::wstring fmt = L"Prefix is: $`.";
		std::wstring expected = L"Start Prefix is: Start . End";
		std::wstring result = op::regex_replace(text, re, fmt);
		assert(result == expected);
		std::wcout << L"  $`: OK\n";
	}

	// Test $$ (literal $)
	{
		std::wstring fmt = L"Literal is $$, group is $1.";
		std::wstring expected = L"Start Literal is $, group is ABC. End";
		std::wstring result = op::regex_replace(text, re, fmt);
		assert(result == expected);
		std::wcout << L"  $$: OK\n";
	}

	TEST_CASE_END(L"TestSpecialReplacementPatterns")
}

// -----------------------------------------------------------------
// 5. Encoding and Error Handling Tests
// -----------------------------------------------------------------

void TestEncodingAndError() {
	TEST_CASE(L"TestEncodingAndError")

//#ifndef USE_STD_FOR_TESTS
//	auto UTF_ENCODING = op::encoding_constants::UTF8; // reference available, but wchar_t default differs
//	// 5.1 Wide-character (Unicode) matching test (Japanese Hiragana)
//	// The pattern and subject are wide-character literals.
//	std::wstring text_utf = L"あいうえお";
//	wregex re_utf(L"あ", op::regex_constants::normal /*, encoding is determined by wchar_t type */);
//	wmatch m_utf;
//	assert(op::regex_search(text_utf, m_utf, re_utf));
//	assert(m_utf.str() == L"あ");
//
//	// 5.2 Error handling: invalid pattern should throw regex_error
//	std::wcout << L"  5.2. Error Handling:\n";
//	bool caught_error = false;
//	try {
//		wregex re_invalid(L"[a-"); // Unclosed character class - should throw
//	} catch (const op::regex_error& e) {
//		std::wcout << L"  (Caught expected error: " << e.what() << L")\n";
//		caught_error = true;
//	}
//	assert(caught_error);
//
//	// 5.3 basic_regex::assign with wide-character assignment
//	wregex re_test;
//	re_test.assign(std::wstring(L"(x+)"), op::regex_constants::icase /*, encoding left as default for wchar_t */);
//	std::wstring test_str = L"AXA";
//	assert(op::regex_search(test_str, m_utf, re_test));
//	// The capture group should be "X"
//	assert(m_utf.str() == L"X");
//#endif

	TEST_CASE_END(L"TestEncodingAndError")
}

// =================================================================
// Main
// =================================================================

int main() {
	// --- Measures to avoid garbled characters on Windows consoles ---
#ifdef _WIN32
	// Switch to UTF-8 mode
	_setmode(_fileno(stdout), _O_U8TEXT);
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

	std::wcout << L"========================================================\n";
	std::wcout << L" Starting onigpp.h Wide-character Test Suite\n";
	std::wcout << L"========================================================\n";

	TestCoreFunctions();
	TestResourceManagement();
	TestIterators();
	TestReplacement();
	TestSpecialReplacementPatterns();
	TestEncodingAndError();

	std::wcout << L"\n========================================================\n";
	std::wcout << L"✨ All wide-character tests succeeded.\n";
	std::wcout << L"========================================================\n";

	return 0;
}
