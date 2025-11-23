// onigpp_bidir_iterator_test.cpp --- Tests for non-contiguous BidirectionalIterators
// Author: katahiromz
// License: MIT
#include "onigpp.h"
#include <iostream>
#include <string>
#include <list>
#include <deque>
#include <vector>
#include <regex>
#include <cassert>
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
// 1. Test regex_search with std::list
// -----------------------------------------------------------------

void TestListRegexSearch() {
	TEST_CASE("TestListRegexSearch")

	// Create a list containing the subject string
	std::string subject_str = "Hello World 123";
	std::list<char> subject_list(subject_str.begin(), subject_str.end());

	// Create a regex pattern
	op::regex re("World\\s+(\\d+)");

	// Create match_results for list iterators
	op::match_results<std::list<char>::iterator> m;

	// Perform regex_search
	bool found = op::regex_search(subject_list.begin(), subject_list.end(), m, re);

	assert(found);
	assert(m.size() == 2); // Entire match + 1 capture group

	// Verify the match by converting sub_match iterators back to string
	std::string match_str(m[0].first, m[0].second);
	assert(match_str == "World 123");

	std::string group1_str(m[1].first, m[1].second);
	assert(group1_str == "123");

	TEST_CASE_END("TestListRegexSearch")
}

// -----------------------------------------------------------------
// 2. Test regex_match with std::deque
// -----------------------------------------------------------------

void TestDequeRegexMatch() {
	TEST_CASE("TestDequeRegexMatch")

	// Create a deque containing the subject string
	std::string subject_str = "test123";
	std::deque<char> subject_deque(subject_str.begin(), subject_str.end());

	// Create a regex pattern that matches the entire string
	op::regex re("test\\d+");

	// Create match_results for deque iterators
	op::match_results<std::deque<char>::iterator> m;

	// Perform regex_match (should match the entire string)
	bool matched = op::regex_match(subject_deque.begin(), subject_deque.end(), m, re);

	assert(matched);
	assert(m.size() == 1); // Only the entire match, no capture groups

	// Verify the match
	std::string match_str(m[0].first, m[0].second);
	assert(match_str == "test123");

	// Test that partial match fails
	std::string partial_str = "test123extra";
	std::deque<char> partial_deque(partial_str.begin(), partial_str.end());
	op::match_results<std::deque<char>::iterator> m2;

	bool matched2 = op::regex_match(partial_deque.begin(), partial_deque.end(), m2, re);
	assert(!matched2);

	TEST_CASE_END("TestDequeRegexMatch")
}

// -----------------------------------------------------------------
// 3. Test regex_iterator over std::list
// -----------------------------------------------------------------

void TestListRegexIterator() {
	TEST_CASE("TestListRegexIterator")

	// Create a list containing the subject string with multiple matches
	std::string subject_str = "abc123 def456 ghi789";
	std::list<char> subject_list(subject_str.begin(), subject_str.end());

	// Create a regex pattern to find word-digit sequences
	op::regex re("([a-z]+)(\\d+)");

	// Create regex_iterator for list iterators
	using list_iter = std::list<char>::iterator;
	op::regex_iterator<list_iter, char> it(subject_list.begin(), subject_list.end(), re);
	op::regex_iterator<list_iter, char> end;

	// Collect all matches
	std::vector<std::string> matches;
	std::vector<std::string> group1_matches;
	std::vector<std::string> group2_matches;

	for (; it != end; ++it) {
		const auto& m = *it;
		matches.push_back(std::string(m[0].first, m[0].second));
		group1_matches.push_back(std::string(m[1].first, m[1].second));
		group2_matches.push_back(std::string(m[2].first, m[2].second));
	}

	// Verify we found all matches
	assert(matches.size() == 3);
	assert(matches[0] == "abc123");
	assert(matches[1] == "def456");
	assert(matches[2] == "ghi789");

	assert(group1_matches[0] == "abc");
	assert(group1_matches[1] == "def");
	assert(group1_matches[2] == "ghi");

	assert(group2_matches[0] == "123");
	assert(group2_matches[1] == "456");
	assert(group2_matches[2] == "789");

	TEST_CASE_END("TestListRegexIterator")
}

// -----------------------------------------------------------------
// 4. Test empty range with std::list
// -----------------------------------------------------------------

void TestEmptyListRegexSearch() {
	TEST_CASE("TestEmptyListRegexSearch")

	// Create an empty list
	std::list<char> empty_list;

	// Create a regex pattern
	op::regex re(".*");

	// Create match_results for list iterators
	op::match_results<std::list<char>::iterator> m;

	// Perform regex_search on empty range
	bool found = op::regex_search(empty_list.begin(), empty_list.end(), m, re);

	// For .* pattern, it should match empty string
	assert(found);
	assert(m.size() >= 1);

	// Verify the match is empty
	std::string match_str(m[0].first, m[0].second);
	assert(match_str == "");

	TEST_CASE_END("TestEmptyListRegexSearch")
}

// -----------------------------------------------------------------
// 5. Test std::deque with capture groups
// -----------------------------------------------------------------

void TestDequeCaptureGroups() {
	TEST_CASE("TestDequeCaptureGroups")

	// Create a deque containing a string with multiple parts
	std::string subject_str = "Name: John, Age: 30";
	std::deque<char> subject_deque(subject_str.begin(), subject_str.end());

	// Create a regex pattern with capture groups
	op::regex re("Name: (\\w+), Age: (\\d+)");

	// Create match_results for deque iterators
	op::match_results<std::deque<char>::iterator> m;

	// Perform regex_search
	bool found = op::regex_search(subject_deque.begin(), subject_deque.end(), m, re);

	assert(found);
	assert(m.size() == 3); // Entire match + 2 capture groups

	// Verify the matches
	std::string match_str(m[0].first, m[0].second);
	assert(match_str == "Name: John, Age: 30");

	std::string name_str(m[1].first, m[1].second);
	assert(name_str == "John");

	std::string age_str(m[2].first, m[2].second);
	assert(age_str == "30");

	TEST_CASE_END("TestDequeCaptureGroups")
}

// -----------------------------------------------------------------
// 6. Test compatibility with std::vector (contiguous iterators still work)
// -----------------------------------------------------------------

void TestVectorStillWorks() {
	TEST_CASE("TestVectorStillWorks")

	// Verify that std::vector still works (contiguous iterators)
	std::string subject_str = "test vector 42";
	std::vector<char> subject_vec(subject_str.begin(), subject_str.end());

	op::regex re("vector (\\d+)");
	op::match_results<std::vector<char>::iterator> m;

	bool found = op::regex_search(subject_vec.begin(), subject_vec.end(), m, re);

	assert(found);
	assert(m.size() == 2);

	std::string match_str(m[0].first, m[0].second);
	assert(match_str == "vector 42");

	std::string group_str(m[1].first, m[1].second);
	assert(group_str == "42");

	TEST_CASE_END("TestVectorStillWorks")
}

// =================================================================
// Main function
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

	std::cout << "========================================\n";
	std::cout << "Onigpp BidirectionalIterator Tests\n";
	std::cout << "========================================\n";

	// Run tests
	TestListRegexSearch();
	TestDequeRegexMatch();
	TestListRegexIterator();
	TestEmptyListRegexSearch();
	TestDequeCaptureGroups();
	TestVectorStillWorks();

	std::cout << "\n========================================\n";
	std::cout << "All tests PASSED!\n";
	std::cout << "========================================\n";

	return 0;
}
