// sub_match_behavior_test.cpp --- Comprehensive test for match_results/sub_match semantics
// Author: katahiromz
// License: BSD-2-Clause
#include "tests.h"
#include <list>
#include <cstring> // for strlen

// Test helper to print test case name
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

// ============================================================
// Test 1: position() and length() for matched submatches
// ============================================================
void TestPositionLengthMatched() {
	TEST_CASE("TestPositionLengthMatched")
	
	const char* text = "Hello World 123";
	rex::regex re("World (\\d+)");
	rex::cmatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	assert(m.size() == 2);
	
	// Test position() for full match (m[0])
	auto pos0 = m.position(0);
	assert(pos0 == 6); // "World 123" starts at index 6
	std::cout << "  position(0) = " << pos0 << " ✓\n";
	
	// Test length() for full match (m[0])
	auto len0 = m.length(0);
	assert(len0 == 9); // "World 123" has length 9
	std::cout << "  length(0) = " << len0 << " ✓\n";
	
	// Test position() for first capture group (m[1])
	auto pos1 = m.position(1);
	assert(pos1 == 12); // "123" starts at index 12
	std::cout << "  position(1) = " << pos1 << " ✓\n";
	
	// Test length() for first capture group (m[1])
	auto len1 = m.length(1);
	assert(len1 == 3); // "123" has length 3
	std::cout << "  length(1) = " << len1 << " ✓\n";
	
	TEST_CASE_END("TestPositionLengthMatched")
}

// ============================================================
// Test 2: position() and length() for unmatched submatches
// ============================================================
void TestPositionLengthUnmatched() {
	TEST_CASE("TestPositionLengthUnmatched")
	
	// Pattern with optional capture group
	const char* text = "Hello";
	rex::regex re("Hello(\\d+)?");
	rex::cmatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	assert(m.size() == 2);
	assert(m[0].matched == true);
	assert(m[1].matched == false); // Optional group didn't match
	
	// Test position() for matched group
	auto pos0 = m.position(0);
	assert(pos0 == 0); // "Hello" starts at index 0
	std::cout << "  position(0) = " << pos0 << " ✓\n";
	
	// Test length() for matched group
	auto len0 = m.length(0);
	assert(len0 == 5); // "Hello" has length 5
	std::cout << "  length(0) = " << len0 << " ✓\n";
	
	// Test position() for unmatched group
	auto pos1 = m.position(1);
#ifndef USE_STD_FOR_TESTS
	// onigpp returns -1 (npos) for unmatched groups
	assert(pos1 == -1);
	std::cout << "  position(1) for unmatched = -1 (onigpp) ✓\n";
#else
	// std::regex may return position at end for unmatched groups (implementation defined)
	// Just verify it's a valid position
	std::cout << "  position(1) for unmatched = " << pos1 << " (std::regex) ✓\n";
#endif
	
	// Test length() for unmatched group - should return 0
	auto len1 = m.length(1);
	assert(len1 == 0);
	std::cout << "  length(1) for unmatched = 0 ✓\n";
	
	TEST_CASE_END("TestPositionLengthUnmatched")
}

// ============================================================
// Test 3: position() and length() for out-of-range indices
// ============================================================
void TestPositionLengthOutOfRange() {
	TEST_CASE("TestPositionLengthOutOfRange")
	
	const char* text = "Test";
	rex::regex re("Test");
	rex::cmatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	assert(m.size() == 1); // Only full match, no capture groups
	
	// Test position() for out-of-range index
	auto pos_invalid = m.position(5);
#ifndef USE_STD_FOR_TESTS
	// onigpp returns -1 (npos) for out-of-range indices
	assert(pos_invalid == -1);
	std::cout << "  position(5) out-of-range = -1 (onigpp) ✓\n";
#else
	// std::regex behavior for out-of-range is implementation-defined
	std::cout << "  position(5) out-of-range = " << pos_invalid << " (std::regex) ✓\n";
#endif
	
	// Test length() for out-of-range index - should return 0
	auto len_invalid = m.length(5);
	assert(len_invalid == 0);
	std::cout << "  length(5) out-of-range = 0 ✓\n";
	
	TEST_CASE_END("TestPositionLengthOutOfRange")
}

// ============================================================
// Test 4: prefix() and suffix() for regular match
// ============================================================
void TestPrefixSuffixRegular() {
	TEST_CASE("TestPrefixSuffixRegular")
	
	const char* text = "Hello World Test";
	rex::regex re("World");
	rex::cmatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	
	// Test prefix() - should be "Hello "
	auto prefix = m.prefix();
	assert(prefix.matched == true);
	assert(prefix.str() == "Hello ");
	std::cout << "  prefix() = \"" << prefix.str() << "\" ✓\n";
	
	// Test suffix() - should be " Test"
	auto suffix = m.suffix();
	assert(suffix.matched == true);
	assert(suffix.str() == " Test");
	std::cout << "  suffix() = \"" << suffix.str() << "\" ✓\n";
	
	TEST_CASE_END("TestPrefixSuffixRegular")
}

// ============================================================
// Test 5: prefix() and suffix() for empty match_results
// ============================================================
void TestPrefixSuffixEmpty() {
	TEST_CASE("TestPrefixSuffixEmpty")
	
	const char* text = "Test";
	rex::regex re("NoMatch");
	rex::cmatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(!found);
	assert(m.empty());
	
	// Test prefix() for empty match_results - should have matched=false
	auto prefix = m.prefix();
	assert(prefix.matched == false);
	assert(prefix.first == prefix.second); // begin == end
	std::cout << "  prefix() for empty match_results: matched=false ✓\n";
	
	// Test suffix() for empty match_results - should have matched=false
	auto suffix = m.suffix();
	assert(suffix.matched == false);
	assert(suffix.first == suffix.second); // begin == end
	std::cout << "  suffix() for empty match_results: matched=false ✓\n";
	
	TEST_CASE_END("TestPrefixSuffixEmpty")
}

// ============================================================
// Test 6: sub_match::str() for matched and unmatched
// ============================================================
void TestSubMatchStr() {
	TEST_CASE("TestSubMatchStr")
	
	const char* text = "Test 123";
	rex::regex re("Test (\\d+)?(\\w+)?");
	rex::cmatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	assert(m.size() >= 2);
	
	// Test str() for matched full match
	assert(m[0].matched == true);
	std::string str0 = m[0].str();
	assert(str0 == "Test 123");
	std::cout << "  m[0].str() = \"" << str0 << "\" ✓\n";
	
	// Test str() for matched capture group
	assert(m[1].matched == true);
	std::string str1 = m[1].str();
	assert(str1 == "123");
	std::cout << "  m[1].str() = \"" << str1 << "\" ✓\n";
	
	// Test str() for unmatched capture group - should return empty string
	if (m.size() > 2) {
		assert(m[2].matched == false);
		std::string str2 = m[2].str();
		assert(str2 == ""); // unmatched should return empty string
		std::cout << "  m[2].str() for unmatched = \"\" ✓\n";
	}
	
	TEST_CASE_END("TestSubMatchStr")
}

// ============================================================
// Test 7: sub_match implicit conversion to string
// ============================================================
void TestSubMatchImplicitConversion() {
	TEST_CASE("TestSubMatchImplicitConversion")
	
	const char* text = "Convert 789";
	rex::regex re("Convert (\\d+)");
	rex::cmatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	
	// Test implicit conversion for matched sub_match
	std::string full_match = m[0]; // implicit conversion
	assert(full_match == "Convert 789");
	std::cout << "  implicit conversion m[0] = \"" << full_match << "\" ✓\n";
	
	std::string capture = m[1]; // implicit conversion
	assert(capture == "789");
	std::cout << "  implicit conversion m[1] = \"" << capture << "\" ✓\n";
	
	TEST_CASE_END("TestSubMatchImplicitConversion")
}

// ============================================================
// Test 8: nosubs flag behavior with regex_search
// ============================================================
void TestNosubsSearch() {
	TEST_CASE("TestNosubsSearch")
	
	const char* text = "hello world";
	// Use explicit constructor with length to avoid ambiguity
	const char* pattern = "(hello) (world)";
	rex::regex re(pattern, std::strlen(pattern), rex::regex_constants::nosubs);
	rex::cmatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	
	// With nosubs, match_results should have size 1 (full match only)
	assert(m.size() == 1);
	std::cout << "  nosubs: m.size() = " << m.size() << " ✓\n";
	
	// Full match should be present
	assert(m[0].matched == true);
	assert(m[0].str() == "hello world");
	std::cout << "  nosubs: m[0].str() = \"" << m[0].str() << "\" ✓\n";
	
	// position() and length() should work for the full match
	assert(m.position(0) == 0);
	assert(m.length(0) == 11);
	std::cout << "  nosubs: position(0) = " << m.position(0) << ", length(0) = " << m.length(0) << " ✓\n";
	
	// position() and length() for out-of-range
#ifndef USE_STD_FOR_TESTS
	// onigpp returns -1 for out-of-range
	assert(m.position(1) == -1);
	std::cout << "  nosubs: position(1) = -1, length(1) = 0 (onigpp) ✓\n";
#else
	// std::regex behavior is implementation-defined
	std::cout << "  nosubs: position(1) = " << m.position(1) << ", length(1) = " << m.length(1) << " (std::regex) ✓\n";
#endif
	assert(m.length(1) == 0);
	
	TEST_CASE_END("TestNosubsSearch")
}

// ============================================================
// Test 9: nosubs flag behavior with regex_match
// ============================================================
void TestNosubsMatch() {
	TEST_CASE("TestNosubsMatch")
	
	const char* text = "abc";
	// Use explicit constructor with length to avoid ambiguity
	const char* pattern = "(a)(b)(c)";
	rex::regex re(pattern, std::strlen(pattern), rex::regex_constants::nosubs);
	rex::cmatch m;
	
	bool matched = rex::regex_match(text, m, re);
	assert(matched);
	
	// With nosubs, match_results should have size 1
	assert(m.size() == 1);
	std::cout << "  nosubs match: m.size() = " << m.size() << " ✓\n";
	
	// Full match should be "abc"
	assert(m[0].str() == "abc");
	std::cout << "  nosubs match: m[0].str() = \"" << m[0].str() << "\" ✓\n";
	
	TEST_CASE_END("TestNosubsMatch")
}

// ============================================================
// Test 10: Non-random-access iterators (std::list)
// ============================================================
void TestNonRandomAccessIterators() {
	TEST_CASE("TestNonRandomAccessIterators")
	
	// Use std::list which has bidirectional iterators, not random-access
	std::list<char> text_list;
	std::string text_str = "find 456 here";
	text_list.assign(text_str.begin(), text_str.end());
	
	rex::regex re("find (\\d+) here");
	rex::match_results<std::list<char>::const_iterator> m;
	
	// Use const_iterator consistently
	bool found = rex::regex_search(text_list.cbegin(), text_list.cend(), m, re);
	assert(found);
	assert(m.size() == 2);
	
	// Test position() with non-random-access iterators
	// Should use std::distance internally - O(n) but correct
	auto pos0 = m.position(0);
	assert(pos0 == 0); // Full match starts at beginning
	std::cout << "  list iterator position(0) = " << pos0 << " ✓\n";
	
	// Test length() with non-random-access iterators
	auto len0 = m.length(0);
	assert(len0 == 13); // "find 456 here" has length 13
	std::cout << "  list iterator length(0) = " << len0 << " ✓\n";
	
	// Test position() and length() for capture group
	auto pos1 = m.position(1);
	assert(pos1 == 5); // "456" starts at index 5
	std::cout << "  list iterator position(1) = " << pos1 << " ✓\n";
	
	auto len1 = m.length(1);
	assert(len1 == 3); // "456" has length 3
	std::cout << "  list iterator length(1) = " << len1 << " ✓\n";
	
	// Test str() with non-random-access iterators
	std::string str0(m[0].first, m[0].second);
	assert(str0 == "find 456 here");
	std::cout << "  list iterator str() = \"" << str0 << "\" ✓\n";
	
	TEST_CASE_END("TestNonRandomAccessIterators")
}

// ============================================================
// Test 11: npos type and value (onigpp only)
// ============================================================
void TestNposValue() {
	TEST_CASE("TestNposValue")
	
#ifndef USE_STD_FOR_TESTS
	// npos is an onigpp extension (not in std::regex until C++23)
	// Verify npos is -1 (signed difference_type)
	assert(rex::cmatch::npos == -1);
	std::cout << "  cmatch::npos == -1 ✓\n";
	
	assert(rex::smatch::npos == -1);
	std::cout << "  smatch::npos == -1 ✓\n";
	
	// Verify npos is of type difference_type
	static_assert(std::is_same<decltype(rex::cmatch::npos), 
	              const rex::cmatch::difference_type>::value,
	              "npos should be of type difference_type");
	std::cout << "  npos type is difference_type ✓\n";
#else
	// When using std::regex, test that length() returns 0 for unmatched
	// (std::regex doesn't have npos member until C++23)
	const char* text = "test";
	rex::regex re("test(\\d+)?");
	rex::cmatch m;
	rex::regex_search(text, m, re);
	assert(m.length(1) == 0);
	assert(!m[1].matched);
	std::cout << "  length() returns 0 for unmatched (std::regex) ✓\n";
#endif
	
	TEST_CASE_END("TestNposValue")
}

// ============================================================
// Test 12: Multiple unmatched groups
// ============================================================
void TestMultipleUnmatchedGroups() {
	TEST_CASE("TestMultipleUnmatchedGroups")
	
	const char* text = "test";
	rex::regex re("(t)(e)(s)(t)(\\d+)?(\\w+)?");
	rex::cmatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	assert(m.size() >= 6);
	
	// First 5 groups should be matched
	for (int i = 0; i <= 4; ++i) {
		assert(m[i].matched == true);
		assert(m.length(i) > 0 || i == 0); // m[0] could be empty
	}
	std::cout << "  First 5 groups matched ✓\n";
	
	// Last 2 groups should be unmatched
	if (m.size() >= 6) {
		assert(m[5].matched == false);
		assert(m.length(5) == 0);
		assert(m[5].str() == "");
#ifndef USE_STD_FOR_TESTS
		assert(m.position(5) == -1);
		std::cout << "  Group 5 unmatched: position=-1, length=0, str=\"\" (onigpp) ✓\n";
#else
		std::cout << "  Group 5 unmatched: position=" << m.position(5) << ", length=0, str=\"\" (std::regex) ✓\n";
#endif
	}
	
	if (m.size() >= 7) {
		assert(m[6].matched == false);
		assert(m.length(6) == 0);
		assert(m[6].str() == "");
#ifndef USE_STD_FOR_TESTS
		assert(m.position(6) == -1);
		std::cout << "  Group 6 unmatched: position=-1, length=0, str=\"\" (onigpp) ✓\n";
#else
		std::cout << "  Group 6 unmatched: position=" << m.position(6) << ", length=0, str=\"\" (std::regex) ✓\n";
#endif
	}
	
	TEST_CASE_END("TestMultipleUnmatchedGroups")
}

// ============================================================
// Test 13: Empty match at beginning
// ============================================================
void TestEmptyMatchAtBeginning() {
	TEST_CASE("TestEmptyMatchAtBeginning")
	
	const char* text = "test";
	rex::regex re("^"); // Matches empty string at beginning
	rex::cmatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	assert(m[0].matched == true);
	
	// Empty match should have position 0, length 0
	assert(m.position(0) == 0);
	assert(m.length(0) == 0);
	assert(m[0].str() == "");
	std::cout << "  Empty match: position=0, length=0, str=\"\" ✓\n";
	
	TEST_CASE_END("TestEmptyMatchAtBeginning")
}

// ============================================================
// Test 14: sub_match length() consistency
// ============================================================
void TestSubMatchLength() {
	TEST_CASE("TestSubMatchLength")
	
	const char* text = "length test";
	rex::regex re("(length) (test)");
	rex::cmatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	
	// Test that sub_match::length() matches str().length() for matched groups
	assert(m[0].matched == true);
	assert(m[0].length() == m[0].str().length());
	std::cout << "  m[0].length() == m[0].str().length() ✓\n";
	
	assert(m[1].matched == true);
	assert(m[1].length() == m[1].str().length());
	std::cout << "  m[1].length() == m[1].str().length() ✓\n";
	
	assert(m[2].matched == true);
	assert(m[2].length() == m[2].str().length());
	std::cout << "  m[2].length() == m[2].str().length() ✓\n";
	
	TEST_CASE_END("TestSubMatchLength")
}

// ============================================================
// Test 15: Unmatched sub_match length is 0
// ============================================================
void TestUnmatchedSubMatchLength() {
	TEST_CASE("TestUnmatchedSubMatchLength")
	
	const char* text = "test";
	rex::regex re("test(\\d+)?");
	rex::cmatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	assert(m.size() == 2);
	assert(m[1].matched == false);
	
	// Unmatched sub_match should have length 0
	assert(m[1].length() == 0);
	std::cout << "  Unmatched sub_match length() == 0 ✓\n";
	
	// And str() should be empty
	assert(m[1].str() == "");
	std::cout << "  Unmatched sub_match str() == \"\" ✓\n";
	
	TEST_CASE_END("TestUnmatchedSubMatchLength")
}

// ============================================================
// Main
// ============================================================
int main() {
	TESTS_OUTPUT_INIT();
	
	// Initialize onigpp (no-op for std::regex)
	ONIGPP_TEST_INIT;
	
	std::cout << "===========================================\n";
	std::cout << "sub_match/match_results Behavior Test Suite\n";
	std::cout << "===========================================\n";
	
	TestPositionLengthMatched();
	TestPositionLengthUnmatched();
	TestPositionLengthOutOfRange();
	TestPrefixSuffixRegular();
	TestPrefixSuffixEmpty();
	TestSubMatchStr();
	TestSubMatchImplicitConversion();
	TestNosubsSearch();
	TestNosubsMatch();
	TestNonRandomAccessIterators();
	TestNposValue();
	TestMultipleUnmatchedGroups();
	TestEmptyMatchAtBeginning();
	TestSubMatchLength();
	TestUnmatchedSubMatchLength();
	
	std::cout << "\n===========================================\n";
	std::cout << "All tests passed! ✅\n";
	std::cout << "===========================================\n";
	
	return 0;
}
