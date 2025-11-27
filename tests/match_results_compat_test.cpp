// match_results_compat_test.cpp --- Test for match_results std::match_results compatibility
// Author: katahiromz
// License: BSD-2-Clause
#include "tests.h"
#include <iostream>
#include <string>
#include <exception>

// =================================================================
// Helper Functions
// =================================================================

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

// -----------------------------------------------------------------
// 1. Test match_results member swap function
// -----------------------------------------------------------------

void TestMatchResultsSwapMember() {
	TEST_CASE("TestMatchResultsSwapMember")

	rex::regex re1("(\\d+)");
	rex::regex re2("(\\w+)");
	
	std::string text1 = "abc123def";
	std::string text2 = "hello world";
	
	rex::smatch m1, m2;
	bool found1 = rex::regex_search(text1, m1, re1);
	bool found2 = rex::regex_search(text2, m2, re2);
	
	assert(found1);
	assert(found2);
	assert(m1[0].str() == "123");
	assert(m2[0].str() == "hello");
	
	// Swap using member function
	m1.swap(m2);
	
	// After swap, m1 should have m2's content and vice versa
	assert(m1[0].str() == "hello");
	assert(m2[0].str() == "123");
	
	TEST_CASE_END("TestMatchResultsSwapMember")
}

// -----------------------------------------------------------------
// 2. Test match_results non-member swap function
// -----------------------------------------------------------------

void TestMatchResultsSwapNonMember() {
	TEST_CASE("TestMatchResultsSwapNonMember")

	rex::regex re("([a-z]+)(\\d+)");
	
	std::string text1 = "abc123";
	std::string text2 = "xyz789";
	
	rex::smatch m1, m2;
	bool found1 = rex::regex_search(text1, m1, re);
	bool found2 = rex::regex_search(text2, m2, re);
	
	assert(found1);
	assert(found2);
	assert(m1[1].str() == "abc");
	assert(m1[2].str() == "123");
	assert(m2[1].str() == "xyz");
	assert(m2[2].str() == "789");
	
	// Swap using non-member function
	rex::swap(m1, m2);
	
	// After swap, m1 should have m2's content and vice versa
	assert(m1[1].str() == "xyz");
	assert(m1[2].str() == "789");
	assert(m2[1].str() == "abc");
	assert(m2[2].str() == "123");
	
	TEST_CASE_END("TestMatchResultsSwapNonMember")
}

// -----------------------------------------------------------------
// 3. Test match_results swap with ADL (using std::swap)
// -----------------------------------------------------------------

void TestMatchResultsSwapADL() {
	TEST_CASE("TestMatchResultsSwapADL")

	rex::regex re("(test)(\\d+)");
	
	std::string text1 = "test111";
	std::string text2 = "test222";
	
	rex::smatch m1, m2;
	rex::regex_search(text1, m1, re);
	rex::regex_search(text2, m2, re);
	
	assert(m1[0].str() == "test111");
	assert(m2[0].str() == "test222");
	
	// Use std::swap which should find our non-member swap via ADL
	using std::swap;
	swap(m1, m2);
	
	assert(m1[0].str() == "test222");
	assert(m2[0].str() == "test111");
	
	TEST_CASE_END("TestMatchResultsSwapADL")
}

// -----------------------------------------------------------------
// 4. Test match_results equality operator
// -----------------------------------------------------------------

void TestMatchResultsEquality() {
	TEST_CASE("TestMatchResultsEquality")

	rex::regex re("(\\d+)");
	std::string text = "abc123def";
	
	rex::smatch m1, m2;
	
	// Two default-constructed match_results should be equal
	rex::smatch empty1, empty2;
	assert(empty1 == empty2);
	assert(!(empty1 != empty2));
	
	// Search and compare
	rex::regex_search(text, m1, re);
	rex::regex_search(text, m2, re);
	
	// Same results should be equal
	assert(m1 == m2);
	assert(!(m1 != m2));
	
	// Different text should produce different results
	std::string text2 = "xyz456";
	rex::smatch m3;
	rex::regex_search(text2, m3, re);
	
	assert(m1 != m3);
	assert(!(m1 == m3));
	
	TEST_CASE_END("TestMatchResultsEquality")
}

// -----------------------------------------------------------------
// 5. Test match_results inequality operator
// -----------------------------------------------------------------

void TestMatchResultsInequality() {
	TEST_CASE("TestMatchResultsInequality")

	rex::regex re("([a-z]+)");
	
	std::string text1 = "hello";
	std::string text2 = "world";
	
	rex::smatch m1, m2;
	rex::regex_search(text1, m1, re);
	rex::regex_search(text2, m2, re);
	
	// Different match results should not be equal
	assert(m1 != m2);
	assert(!(m1 == m2));
	
	// Same match results should be equal
	rex::smatch m3;
	rex::regex_search(text1, m3, re);
	
	assert(m1 == m3);
	assert(!(m1 != m3));
	
	TEST_CASE_END("TestMatchResultsInequality")
}

// -----------------------------------------------------------------
// 6. Test match_results get_allocator function
// -----------------------------------------------------------------

void TestMatchResultsGetAllocator() {
	TEST_CASE("TestMatchResultsGetAllocator")

	rex::smatch m;
	
	// get_allocator should return the allocator
	auto alloc = m.get_allocator();
	
	// Just verify it compiles and returns an allocator type
	// The allocator itself should be valid
	(void)alloc;
	
	// Verify the allocator_type is accessible
	using alloc_type = rex::smatch::allocator_type;
	static_assert(std::is_same<alloc_type, decltype(alloc)>::value, "allocator_type mismatch");
	
	std::cout << "get_allocator() returned valid allocator\n";
	
	TEST_CASE_END("TestMatchResultsGetAllocator")
}

// -----------------------------------------------------------------
// 7. Test match_results type aliases
// -----------------------------------------------------------------

void TestMatchResultsTypeAliases() {
	TEST_CASE("TestMatchResultsTypeAliases")

	// Verify all type aliases are accessible
	using smatch = rex::smatch;
	
	// value_type
	using vt = typename smatch::value_type;
	(void)sizeof(vt);
	
	// const_reference
	using cr = typename smatch::const_reference;
	(void)sizeof(cr);
	
	// reference
	using ref = typename smatch::reference;
	(void)sizeof(ref);
	
	// const_iterator
	using cit = typename smatch::const_iterator;
	(void)sizeof(cit);
	
	// iterator
	using it = typename smatch::iterator;
	(void)sizeof(it);
	
	// difference_type
	using dt = typename smatch::difference_type;
	(void)sizeof(dt);
	
	// size_type
	using st = typename smatch::size_type;
	(void)sizeof(st);
	
	// allocator_type
	using at = typename smatch::allocator_type;
	(void)sizeof(at);
	
	// char_type
	using ct = typename smatch::char_type;
	(void)sizeof(ct);
	
	// string_type
	using str_t = typename smatch::string_type;
	(void)sizeof(str_t);
	
	std::cout << "All type aliases are accessible\n";
	
	TEST_CASE_END("TestMatchResultsTypeAliases")
}

// -----------------------------------------------------------------
// 8. Test match_results empty() noexcept
// -----------------------------------------------------------------

void TestMatchResultsEmptyNoexcept() {
	TEST_CASE("TestMatchResultsEmptyNoexcept")

	rex::smatch m;
	
#ifndef USE_STD_FOR_TESTS
	// empty() should be noexcept (only check for onigpp, std::match_results may vary)
	static_assert(noexcept(m.empty()), "empty() should be noexcept");
#endif
	
	// Default constructed should be empty
	assert(m.empty());
	
	// After a search that finds a match, should not be empty
	rex::regex re("\\d+");
	std::string text = "123";
	rex::regex_search(text, m, re);
	assert(!m.empty());
	
	std::cout << "empty() is noexcept and works correctly\n";
	
	TEST_CASE_END("TestMatchResultsEmptyNoexcept")
}

// -----------------------------------------------------------------
// 9. Test match_results ready() noexcept
// -----------------------------------------------------------------

void TestMatchResultsReadyNoexcept() {
	TEST_CASE("TestMatchResultsReadyNoexcept")

	rex::smatch m;
	
#ifndef USE_STD_FOR_TESTS
	// ready() should be noexcept (only check for onigpp, std::match_results may vary)
	static_assert(noexcept(m.ready()), "ready() should be noexcept");
#endif
	
	// Default constructed should not be ready
	assert(!m.ready());
	
	// After a search, should be ready
	rex::regex re("\\d+");
	std::string text = "abc";  // No match
	rex::regex_search(text, m, re);
	assert(m.ready());  // Even if no match, ready() returns true after search
	
	std::cout << "ready() is noexcept and works correctly\n";
	
	TEST_CASE_END("TestMatchResultsReadyNoexcept")
}

// -----------------------------------------------------------------
// 10. Test match_results swap noexcept
// -----------------------------------------------------------------

void TestMatchResultsSwapNoexcept() {
	TEST_CASE("TestMatchResultsSwapNoexcept")

	rex::smatch m1, m2;
	
#ifndef USE_STD_FOR_TESTS
	// Member swap should be noexcept (only check for onigpp, std::match_results may vary)
	static_assert(noexcept(m1.swap(m2)), "swap() member should be noexcept");
	
	// Non-member swap should be noexcept
	static_assert(noexcept(rex::swap(m1, m2)), "swap() non-member should be noexcept");
#endif
	
	std::cout << "swap() is noexcept\n";
	
	TEST_CASE_END("TestMatchResultsSwapNoexcept")
}

// -----------------------------------------------------------------
// 11. Test match_results with different allocators
// -----------------------------------------------------------------

void TestMatchResultsWithAllocator() {
	TEST_CASE("TestMatchResultsWithAllocator")

	// Test construction with allocator
	// Use the allocator_type from smatch to ensure type consistency
	using alloc_type = typename rex::smatch::allocator_type;
	alloc_type alloc;
	
	rex::smatch m(alloc);
	
	// The allocator should be stored
	auto retrieved_alloc = m.get_allocator();
	(void)retrieved_alloc;
	
	std::cout << "match_results constructed with allocator correctly\n";
	
	TEST_CASE_END("TestMatchResultsWithAllocator")
}

// -----------------------------------------------------------------
// 12. Test match_results copy and move
// -----------------------------------------------------------------

void TestMatchResultsCopyMove() {
	TEST_CASE("TestMatchResultsCopyMove")

	rex::regex re("(\\w+)");
	std::string text = "hello";
	
	rex::smatch m1;
	rex::regex_search(text, m1, re);
	
	// Copy constructor
	rex::smatch m2(m1);
	assert(m1 == m2);
	assert(m2[0].str() == "hello");
	
	// Move constructor
	rex::smatch m3(std::move(m2));
	assert(m3[0].str() == "hello");
	
	// Copy assignment
	rex::smatch m4;
	m4 = m1;
	assert(m4 == m1);
	assert(m4[0].str() == "hello");
	
	// Move assignment
	rex::smatch m5;
	m5 = std::move(m4);
	assert(m5[0].str() == "hello");
	
	std::cout << "Copy and move operations work correctly\n";
	
	TEST_CASE_END("TestMatchResultsCopyMove")
}

// -----------------------------------------------------------------
// 13. Test match_results equality with ready states
// -----------------------------------------------------------------

void TestMatchResultsEqualityReady() {
	TEST_CASE("TestMatchResultsEqualityReady")

	// Two default-constructed (not ready) match_results should be equal
	rex::smatch m1, m2;
	assert(!m1.ready() && !m2.ready());
	assert(m1 == m2);
	
	// One ready, one not ready - should not be equal
	rex::regex re("\\d+");
	std::string text = "123";
	rex::regex_search(text, m1, re);
	assert(m1.ready() && !m2.ready());
	assert(m1 != m2);
	
	// Both ready with same match - should be equal
	rex::regex_search(text, m2, re);
	assert(m1.ready() && m2.ready());
	assert(m1 == m2);
	
	TEST_CASE_END("TestMatchResultsEqualityReady")
}

// -----------------------------------------------------------------
// 14. Test match_results equality with empty results
// -----------------------------------------------------------------

void TestMatchResultsEqualityEmpty() {
	TEST_CASE("TestMatchResultsEqualityEmpty")

	rex::regex re("\\d+");
	std::string text_nomatch = "abc";
	
	rex::smatch m1, m2;
	rex::regex_search(text_nomatch, m1, re);
	rex::regex_search(text_nomatch, m2, re);
	
	// Both should be ready but empty (no match)
	assert(m1.ready() && m2.ready());
	assert(m1.empty() && m2.empty());
	assert(m1 == m2);
	
	TEST_CASE_END("TestMatchResultsEqualityEmpty")
}

// =================================================================
// Main function
// =================================================================

int main() {
	TESTS_OUTPUT_INIT();

	// Oniguruma initialization (no-op for std::regex)
	ONIGPP_TEST_INIT;

	std::cout << "========================================\n";
	std::cout << "match_results std::match_results Compatibility Tests\n";
	std::cout << "========================================\n";

	// Run tests
	TestMatchResultsSwapMember();
	TestMatchResultsSwapNonMember();
	TestMatchResultsSwapADL();
	TestMatchResultsEquality();
	TestMatchResultsInequality();
	TestMatchResultsGetAllocator();
	TestMatchResultsTypeAliases();
	TestMatchResultsEmptyNoexcept();
	TestMatchResultsReadyNoexcept();
	TestMatchResultsSwapNoexcept();
	TestMatchResultsWithAllocator();
	TestMatchResultsCopyMove();
	TestMatchResultsEqualityReady();
	TestMatchResultsEqualityEmpty();

	std::cout << "\n========================================\n";
	std::cout << "All tests PASSED!\n";
	std::cout << "========================================\n";

	return 0;
}
