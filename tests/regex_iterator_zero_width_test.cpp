// regex_iterator_zero_width_test.cpp --- Tests for zero-width match handling in regex_iterator
// This test verifies std::regex_iterator compatibility for zero-width matches.
// Author: katahiromz
// License: BSD-2-Clause

#include "tests.h"
#include <vector>
#include <utility>
#include <regex>

// Helper to collect all matches from regex_iterator
std::vector<std::pair<int, int>> collect_matches(const std::string& input, const rex::regex& re) {
	std::vector<std::pair<int, int>> results;
	auto it = rex::sregex_iterator(input.begin(), input.end(), re);
	auto end = rex::sregex_iterator();
	for (; it != end; ++it) {
		results.push_back(std::make_pair(static_cast<int>(it->position()), static_cast<int>(it->length())));
	}
	return results;
}

#ifndef USE_STD_FOR_TESTS
// Helper for std::regex matches (only when not using std for tests)
std::vector<std::pair<int, int>> collect_std_matches(const std::string& input, const std::regex& re) {
	std::vector<std::pair<int, int>> results;
	std::sregex_iterator it(input.begin(), input.end(), re);
	std::sregex_iterator end;
	for (; it != end; ++it) {
		results.push_back(std::make_pair(static_cast<int>(it->position()), static_cast<int>(it->length())));
	}
	return results;
}
#endif

void compare_results(const std::string& pattern, const std::string& input,
                     const std::vector<std::pair<int, int>>& expected,
                     const std::vector<std::pair<int, int>>& actual) {
	if (expected.size() != actual.size()) {
		std::cout << "Pattern: \"" << pattern << "\", Input: \"" << input << "\"" << std::endl;
		std::cout << "Expected " << expected.size() << " matches, got " << actual.size() << std::endl;
		assert(false && "Match count mismatch");
	}
	for (size_t i = 0; i < expected.size(); ++i) {
		if (expected[i] != actual[i]) {
			std::cout << "Pattern: \"" << pattern << "\", Input: \"" << input << "\"" << std::endl;
			std::cout << "Match " << i << ": expected pos=" << expected[i].first << " len=" << expected[i].second
			          << ", got pos=" << actual[i].first << " len=" << actual[i].second << std::endl;
			assert(false && "Match position/length mismatch");
		}
	}
}

void test_empty_pattern() {
	std::cout << "Test: Empty pattern zero-width matches" << std::endl;

	// Empty pattern matches at every position (n+1 matches for string of length n)
	{
		rex::regex re("");
		auto results = collect_matches("ab", re);
		// Expected: matches at positions 0, 1, 2 (all zero-width)
		std::vector<std::pair<int, int>> expected = {{0, 0}, {1, 0}, {2, 0}};
		compare_results("", "ab", expected, results);
	}

	// Empty input with empty pattern
	{
		rex::regex re("");
		auto results = collect_matches("", re);
		// Expected: single match at position 0
		std::vector<std::pair<int, int>> expected = {{0, 0}};
		compare_results("", "", expected, results);
	}

	std::cout << "  PASSED" << std::endl;
}

void test_lookahead_patterns() {
	std::cout << "Test: Lookahead patterns (zero-width)" << std::endl;

	// Positive lookahead at each position
	{
		rex::regex re("(?=.)");
		auto results = collect_matches("abc", re);
		// Matches before each character (not at end since (?=.) requires a char)
		std::vector<std::pair<int, int>> expected = {{0, 0}, {1, 0}, {2, 0}};
		compare_results("(?=.)", "abc", expected, results);
	}

	// End-of-string lookahead
	{
		rex::regex re("(?=$)");
		auto results = collect_matches("abc", re);
		// Only matches at end of string
		std::vector<std::pair<int, int>> expected = {{3, 0}};
		compare_results("(?=$)", "abc", expected, results);
	}

	std::cout << "  PASSED" << std::endl;
}

void test_word_boundary() {
	std::cout << "Test: Word boundary patterns (zero-width)" << std::endl;

	// Word boundary at start and end
	{
		rex::regex re("\\b");
		auto results = collect_matches("word", re);
		// Matches at start (pos 0) and end (pos 4)
		std::vector<std::pair<int, int>> expected = {{0, 0}, {4, 0}};
		compare_results("\\b", "word", expected, results);
	}

	// Word boundary with multiple words
	{
		rex::regex re("\\b");
		auto results = collect_matches("a b", re);
		// Before 'a', after 'a', before 'b', after 'b'
		std::vector<std::pair<int, int>> expected = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
		compare_results("\\b", "a b", expected, results);
	}

	std::cout << "  PASSED" << std::endl;
}

void test_anchor_patterns() {
	std::cout << "Test: Anchor patterns (zero-width)" << std::endl;

	// Start anchor
	{
		rex::regex re("^");
		auto results = collect_matches("abc", re);
		std::vector<std::pair<int, int>> expected = {{0, 0}};
		compare_results("^", "abc", expected, results);
	}

	// End anchor
	{
		rex::regex re("$");
		auto results = collect_matches("abc", re);
		std::vector<std::pair<int, int>> expected = {{3, 0}};
		compare_results("$", "abc", expected, results);
	}

	// Both anchors on empty string
	{
		rex::regex re("^$");
		auto results = collect_matches("", re);
		std::vector<std::pair<int, int>> expected = {{0, 0}};
		compare_results("^$", "", expected, results);
	}

	std::cout << "  PASSED" << std::endl;
}

void test_optional_patterns() {
	std::cout << "Test: Optional patterns that can match zero-width" << std::endl;

	// a* can match zero characters
	{
		rex::regex re("a*");
		auto results = collect_matches("aab", re);
		// First: "aa" at pos 0, then "" at pos 2, then "" at pos 3
		std::vector<std::pair<int, int>> expected = {{0, 2}, {2, 0}, {3, 0}};
		compare_results("a*", "aab", expected, results);
	}

	// a? can match zero or one
	{
		rex::regex re("a?");
		auto results = collect_matches("ba", re);
		// At pos 0: "" (no 'a' at start), at pos 1: "a", at pos 2: "" (end of string)
		std::vector<std::pair<int, int>> expected = {{0, 0}, {1, 1}, {2, 0}};
		compare_results("a?", "ba", expected, results);
	}

	std::cout << "  PASSED" << std::endl;
}

void test_consecutive_zero_width() {
	std::cout << "Test: Consecutive zero-width matches" << std::endl;

	// Multiple consecutive zero-width matches
	{
		rex::regex re("");
		auto results = collect_matches("x", re);
		// Positions 0 and 1
		std::vector<std::pair<int, int>> expected = {{0, 0}, {1, 0}};
		compare_results("", "x", expected, results);
	}

	// Zero-width at string boundaries
	{
		rex::regex re("(?=b|$)");
		auto results = collect_matches("ab", re);
		// Before 'b' (pos 1) and at end (pos 2)
		std::vector<std::pair<int, int>> expected = {{1, 0}, {2, 0}};
		compare_results("(?=b|$)", "ab", expected, results);
	}

	std::cout << "  PASSED" << std::endl;
}

void test_end_iterator_equality() {
	std::cout << "Test: End iterator equality" << std::endl;

	// Default-constructed iterators should be equal
	{
		rex::sregex_iterator end1, end2;
		assert(end1 == end2 && "Two default-constructed iterators should be equal");
	}

	// Exhausted iterators should equal end
	{
		rex::regex re("x");
		std::string input = "";
		auto it = rex::sregex_iterator(input.begin(), input.end(), re);
		rex::sregex_iterator end;
		assert(it == end && "Iterator with no matches should equal end");
	}

	// Iterator incremented past last match should equal end
	{
		rex::regex re("a");
		std::string input = "a";
		auto it = rex::sregex_iterator(input.begin(), input.end(), re);
		rex::sregex_iterator end;
		assert(it != end && "Should have one match");
		++it;
		assert(it == end && "After incrementing past last match, should equal end");
	}

	std::cout << "  PASSED" << std::endl;
}

#ifndef USE_STD_FOR_TESTS
// This test is only for onigpp - incrementing past end is undefined behavior in std::regex_iterator
void test_increment_past_end() {
	std::cout << "Test: Incrementing past end is safe" << std::endl;

	rex::regex re("a");
	std::string input = "a";
	auto it = rex::sregex_iterator(input.begin(), input.end(), re);
	rex::sregex_iterator end;

	// Advance to end
	++it;
	assert(it == end);

	// Incrementing end iterator should remain at end (no crash)
	++it;
	assert(it == end && "Iterator should remain at end after extra increment");

	std::cout << "  PASSED" << std::endl;
}
#endif

#ifndef USE_STD_FOR_TESTS
void test_comparison_with_std() {
	std::cout << "Test: Comparison with std::regex_iterator" << std::endl;

	auto compare_with_std = [](const std::string& pattern, const std::string& input) {
		std::regex std_re(pattern);
		auto std_results = collect_std_matches(input, std_re);

		rex::regex onigpp_re(pattern);
		auto onigpp_results = collect_matches(input, onigpp_re);

		if (std_results.size() != onigpp_results.size()) {
			std::cout << "  Pattern: \"" << pattern << "\", Input: \"" << input << "\"" << std::endl;
			std::cout << "  std: " << std_results.size() << " matches, onigpp: " << onigpp_results.size() << std::endl;
			assert(false && "Match count differs from std::regex");
		}
		for (size_t i = 0; i < std_results.size(); ++i) {
			if (std_results[i] != onigpp_results[i]) {
				std::cout << "  Pattern: \"" << pattern << "\", Input: \"" << input << "\"" << std::endl;
				std::cout << "  Match " << i << " differs: std pos=" << std_results[i].first
				          << " len=" << std_results[i].second
				          << ", onigpp pos=" << onigpp_results[i].first
				          << " len=" << onigpp_results[i].second << std::endl;
				assert(false && "Match differs from std::regex");
			}
		}
	};

	// Test various patterns
	compare_with_std("", "ab");
	compare_with_std("", "");
	compare_with_std("(?=.)", "abc");
	compare_with_std("\\b", "word");
	compare_with_std("^", "abc");
	compare_with_std("$", "abc");
	compare_with_std("a*", "aab");
	compare_with_std("a?", "ba");
	compare_with_std("(?=$)", "abc");

	std::cout << "  PASSED" << std::endl;
}
#endif

int main() {
	TESTS_OUTPUT_INIT();
	ONIGPP_TEST_INIT;

	std::cout << "========================================" << std::endl;
	std::cout << "regex_iterator Zero-Width Match Tests" << std::endl;
	std::cout << "========================================" << std::endl;

	test_empty_pattern();
	test_lookahead_patterns();
	test_word_boundary();
	test_anchor_patterns();
	test_optional_patterns();
	test_consecutive_zero_width();
	test_end_iterator_equality();
#ifndef USE_STD_FOR_TESTS
	test_increment_past_end();
	test_comparison_with_std();
#endif

	std::cout << "\n========================================" << std::endl;
	std::cout << "All zero-width match tests PASSED" << std::endl;
	std::cout << "========================================" << std::endl;

	return 0;
}
