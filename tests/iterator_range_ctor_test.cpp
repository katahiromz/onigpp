// iterator_range_ctor_test.cpp --- Test for iterator-range constructor and assign method
// Author: katahiromz
// License: MIT
#include "onigpp.h"
#include <iostream>
#include <string>
#include <list>
#include <deque>
#include <vector>
#include <cassert>

// Alias namespace for ease of use
#ifdef USE_STD_FOR_TESTS
	namespace op = std;
#else
	namespace op = onigpp;
#endif

// Type aliases for match_results
using smatch = op::match_results<std::string::const_iterator>;

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
// 1. Test iterator-range constructor with std::list
// -----------------------------------------------------------------

void TestIteratorRangeCtorWithList() {
	TEST_CASE("TestIteratorRangeCtorWithList")

	// Create a list containing a regex pattern
	std::string pattern_str = "World\\s+(\\d+)";
	std::list<char> pattern_list(pattern_str.begin(), pattern_str.end());

	// Create regex using iterator-range constructor
	op::regex re(pattern_list.begin(), pattern_list.end());

	// Test the regex on a subject string
	std::string subject = "Hello World 123";
	smatch m;
	bool found = op::regex_search(subject, m, re);

	assert(found);
	assert(m.size() == 2); // Entire match + 1 capture group
	assert(m[0].str() == "World 123");
	assert(m[1].str() == "123");

	TEST_CASE_END("TestIteratorRangeCtorWithList")
}

// -----------------------------------------------------------------
// 2. Test iterator-range constructor with std::deque
// -----------------------------------------------------------------

void TestIteratorRangeCtorWithDeque() {
	TEST_CASE("TestIteratorRangeCtorWithDeque")

	// Create a deque containing a regex pattern
	std::string pattern_str = "test\\d+";
	std::deque<char> pattern_deque(pattern_str.begin(), pattern_str.end());

	// Create regex using iterator-range constructor
	op::regex re(pattern_deque.begin(), pattern_deque.end());

	// Test the regex on a subject string
	std::string subject = "test123";
	smatch m;
	bool matched = op::regex_match(subject, m, re);

	assert(matched);
	assert(m.size() == 1);
	assert(m[0].str() == "test123");

	TEST_CASE_END("TestIteratorRangeCtorWithDeque")
}

// -----------------------------------------------------------------
// 3. Test iterator-range constructor with std::vector
// -----------------------------------------------------------------

void TestIteratorRangeCtorWithVector() {
	TEST_CASE("TestIteratorRangeCtorWithVector")

	// Create a vector containing a regex pattern
	std::string pattern_str = "vector (\\d+)";
	std::vector<char> pattern_vec(pattern_str.begin(), pattern_str.end());

	// Create regex using iterator-range constructor
	op::regex re(pattern_vec.begin(), pattern_vec.end());

	// Test the regex on a subject string
	std::string subject = "test vector 42";
	smatch m;
	bool found = op::regex_search(subject, m, re);

	assert(found);
	assert(m.size() == 2);
	assert(m[0].str() == "vector 42");
	assert(m[1].str() == "42");

	TEST_CASE_END("TestIteratorRangeCtorWithVector")
}

// -----------------------------------------------------------------
// 4. Test iterator-range constructor with string iterators
// -----------------------------------------------------------------

void TestIteratorRangeCtorWithStringIterators() {
	TEST_CASE("TestIteratorRangeCtorWithStringIterators")

	// Create regex using string iterators directly
	std::string pattern_str = "Name: (\\w+)";
	op::regex re(pattern_str.begin(), pattern_str.end());

	// Test the regex on a subject string
	std::string subject = "Name: John";
	smatch m;
	bool found = op::regex_search(subject, m, re);

	assert(found);
	assert(m.size() == 2);
	assert(m[0].str() == "Name: John");
	assert(m[1].str() == "John");

	TEST_CASE_END("TestIteratorRangeCtorWithStringIterators")
}

// -----------------------------------------------------------------
// 5. Test iterator-range assign with std::list
// -----------------------------------------------------------------

void TestIteratorRangeAssignWithList() {
	TEST_CASE("TestIteratorRangeAssignWithList")

	// Create an initial regex
	op::regex re("initial");

	// Create a list containing a new regex pattern
	std::string pattern_str = "Age: (\\d+)";
	std::list<char> pattern_list(pattern_str.begin(), pattern_str.end());

	// Assign using iterator-range
	re.assign(pattern_list.begin(), pattern_list.end());

	// Test the regex on a subject string
	std::string subject = "Age: 30";
	smatch m;
	bool found = op::regex_search(subject, m, re);

	assert(found);
	assert(m.size() == 2);
	assert(m[0].str() == "Age: 30");
	assert(m[1].str() == "30");

	TEST_CASE_END("TestIteratorRangeAssignWithList")
}

// -----------------------------------------------------------------
// 6. Test iterator-range assign with std::deque
// -----------------------------------------------------------------

void TestIteratorRangeAssignWithDeque() {
	TEST_CASE("TestIteratorRangeAssignWithDeque")

	// Create an initial regex
	op::regex re("initial");

	// Create a deque containing a new regex pattern
	std::string pattern_str = "abc\\d+";
	std::deque<char> pattern_deque(pattern_str.begin(), pattern_str.end());

	// Assign using iterator-range
	re.assign(pattern_deque.begin(), pattern_deque.end());

	// Test the regex on a subject string
	std::string subject = "abc123";
	smatch m;
	bool matched = op::regex_match(subject, m, re);

	assert(matched);
	assert(m.size() == 1);
	assert(m[0].str() == "abc123");

	TEST_CASE_END("TestIteratorRangeAssignWithDeque")
}

// -----------------------------------------------------------------
// 7. Test iterator-range assign with string iterators
// -----------------------------------------------------------------

void TestIteratorRangeAssignWithStringIterators() {
	TEST_CASE("TestIteratorRangeAssignWithStringIterators")

	// Create an initial regex
	op::regex re("initial");

	// Assign using string iterators directly
	std::string pattern_str = "City: (\\w+)";
	re.assign(pattern_str.begin(), pattern_str.end());

	// Test the regex on a subject string
	std::string subject = "City: Paris";
	smatch m;
	bool found = op::regex_search(subject, m, re);

	assert(found);
	assert(m.size() == 2);
	assert(m[0].str() == "City: Paris");
	assert(m[1].str() == "Paris");

	TEST_CASE_END("TestIteratorRangeAssignWithStringIterators")
}

// -----------------------------------------------------------------
// 8. Test iterator-range constructor with flags
// -----------------------------------------------------------------

void TestIteratorRangeCtorWithFlags() {
	TEST_CASE("TestIteratorRangeCtorWithFlags")

	// Create a list containing a regex pattern
	std::string pattern_str = "test";
	std::list<char> pattern_list(pattern_str.begin(), pattern_str.end());

	// Create regex using iterator-range constructor with icase flag
	op::regex re(pattern_list.begin(), pattern_list.end(), op::regex_constants::icase);

	// Test the regex on a subject string with different case
	std::string subject = "TEST";
	smatch m;
	bool matched = op::regex_match(subject, m, re);

	assert(matched);
	assert(m.size() == 1);
	assert(m[0].str() == "TEST");

	TEST_CASE_END("TestIteratorRangeCtorWithFlags")
}

// -----------------------------------------------------------------
// 9. Test iterator-range assign with flags
// -----------------------------------------------------------------

void TestIteratorRangeAssignWithFlags() {
	TEST_CASE("TestIteratorRangeAssignWithFlags")

	// Create an initial regex
	op::regex re("initial");

	// Create a deque containing a new regex pattern
	std::string pattern_str = "hello";
	std::deque<char> pattern_deque(pattern_str.begin(), pattern_str.end());

	// Assign using iterator-range with icase flag
	re.assign(pattern_deque.begin(), pattern_deque.end(), op::regex_constants::icase);

	// Test the regex on a subject string with different case
	std::string subject = "HELLO";
	smatch m;
	bool matched = op::regex_match(subject, m, re);

	assert(matched);
	assert(m.size() == 1);
	assert(m[0].str() == "HELLO");

	TEST_CASE_END("TestIteratorRangeAssignWithFlags")
}

// =================================================================
// Main function
// =================================================================

int main() {
#ifndef USE_STD_FOR_TESTS
	// Oniguruma initialization
	op::auto_init init;
#endif

	std::cout << "========================================\n";
	std::cout << "Iterator Range Constructor/Assign Tests\n";
	std::cout << "========================================\n";

	// Run tests
	TestIteratorRangeCtorWithList();
	TestIteratorRangeCtorWithDeque();
	TestIteratorRangeCtorWithVector();
	TestIteratorRangeCtorWithStringIterators();
	TestIteratorRangeAssignWithList();
	TestIteratorRangeAssignWithDeque();
	TestIteratorRangeAssignWithStringIterators();
	TestIteratorRangeCtorWithFlags();
	TestIteratorRangeAssignWithFlags();

	std::cout << "\n========================================\n";
	std::cout << "All tests PASSED!\n";
	std::cout << "========================================\n";

	return 0;
}
