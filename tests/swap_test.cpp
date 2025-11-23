// swap_test.cpp --- Test for non-member swap function
// Author: katahiromz
// License: BSD-2-Clause
#include "onigpp.h"
#include <iostream>
#include <string>
#include <cassert>

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
// 1. Test non-member swap function
// -----------------------------------------------------------------

void TestNonMemberSwap() {
	TEST_CASE("TestNonMemberSwap")

	// Create two regex objects with different patterns
	op::regex re1("pattern1 (\\d+)");
	op::regex re2("pattern2 (\\w+)");

	// Test re1 before swap
	std::string subject1 = "pattern1 123";
	smatch m1;
	bool found1 = op::regex_search(subject1, m1, re1);
	assert(found1);
	assert(m1.size() == 2);
	assert(m1[0].str() == "pattern1 123");
	assert(m1[1].str() == "123");

	// Test re2 before swap
	std::string subject2 = "pattern2 abc";
	smatch m2;
	bool found2 = op::regex_search(subject2, m2, re2);
	assert(found2);
	assert(m2.size() == 2);
	assert(m2[0].str() == "pattern2 abc");
	assert(m2[1].str() == "abc");

	// Perform swap using non-member function
	op::swap(re1, re2);

	// Test re1 after swap (should now have pattern2)
	smatch m3;
	bool found3 = op::regex_search(subject2, m3, re1);
	assert(found3);
	assert(m3.size() == 2);
	assert(m3[0].str() == "pattern2 abc");
	assert(m3[1].str() == "abc");

	// Test re2 after swap (should now have pattern1)
	smatch m4;
	bool found4 = op::regex_search(subject1, m4, re2);
	assert(found4);
	assert(m4.size() == 2);
	assert(m4[0].str() == "pattern1 123");
	assert(m4[1].str() == "123");

	TEST_CASE_END("TestNonMemberSwap")
}

// -----------------------------------------------------------------
// 2. Test swap with std::swap via ADL
// -----------------------------------------------------------------

void TestSwapWithStdSwap() {
	TEST_CASE("TestSwapWithStdSwap")

	// Create two regex objects
	op::regex re1("alpha");
	op::regex re2("beta");

	// Test re1 before swap
	const std::string subject1 = "alpha";
	assert(op::regex_match(subject1.begin(), subject1.end(), re1));

	// Test re2 before swap
	const std::string subject2 = "beta";
	assert(op::regex_match(subject2.begin(), subject2.end(), re2));

	// Use std::swap, which should find our non-member swap via ADL
	using std::swap;
	swap(re1, re2);

	// Test re1 after swap (should now match "beta")
	assert(!op::regex_match(subject1.begin(), subject1.end(), re1));
	assert(op::regex_match(subject2.begin(), subject2.end(), re1));

	// Test re2 after swap (should now match "alpha")
	assert(op::regex_match(subject1.begin(), subject1.end(), re2));
	assert(!op::regex_match(subject2.begin(), subject2.end(), re2));

	TEST_CASE_END("TestSwapWithStdSwap")
}

// -----------------------------------------------------------------
// 3. Test swap with different character types
// -----------------------------------------------------------------

void TestSwapWithWideCharacters() {
	TEST_CASE("TestSwapWithWideCharacters")

	// Create two wregex objects
	op::wregex wre1(L"test1");
	op::wregex wre2(L"test2");

	// Test wre1 before swap
	const std::wstring wsubject1 = L"test1";
	assert(op::regex_match(wsubject1.begin(), wsubject1.end(), wre1));

	// Test wre2 before swap
	const std::wstring wsubject2 = L"test2";
	assert(op::regex_match(wsubject2.begin(), wsubject2.end(), wre2));

	// Perform swap
#ifndef USE_STD_FOR_TESTS
	onigpp::swap(wre1, wre2);
#else
	std::swap(wre1, wre2);
#endif

	// Test wre1 after swap (should now match "test2")
	assert(!op::regex_match(wsubject1.begin(), wsubject1.end(), wre1));
	assert(op::regex_match(wsubject2.begin(), wsubject2.end(), wre1));

	// Test wre2 after swap (should now match "test1")
	assert(op::regex_match(wsubject1.begin(), wsubject1.end(), wre2));
	assert(!op::regex_match(wsubject2.begin(), wsubject2.end(), wre2));

	TEST_CASE_END("TestSwapWithWideCharacters")
}

// -----------------------------------------------------------------
// 4. Test swap with flags
// -----------------------------------------------------------------

void TestSwapWithFlags() {
	TEST_CASE("TestSwapWithFlags")

	// Create two regex objects with different flags
	std::string pattern1 = "test";
	std::string pattern2 = "pattern";
	op::regex re1(pattern1, op::regex_constants::icase);
	op::regex re2(pattern2, op::regex_constants::multiline);

	// Verify flags before swap
#ifndef USE_STD_FOR_TESTS
	assert(re1.flags() == op::regex_constants::icase);
	assert(re2.flags() == op::regex_constants::multiline);
#endif

	// Test matching behavior before swap
	const std::string subject1 = "TEST";
	assert(op::regex_match(subject1.begin(), subject1.end(), re1)); // icase allows matching

	// Perform swap
	op::swap(re1, re2);

	// Verify flags after swap
#ifndef USE_STD_FOR_TESTS
	assert(re1.flags() == op::regex_constants::multiline);
	assert(re2.flags() == op::regex_constants::icase);
#endif

	// Test matching behavior after swap
	assert(op::regex_match(subject1.begin(), subject1.end(), re2)); // re2 now has icase flag

	TEST_CASE_END("TestSwapWithFlags")
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
	std::cout << "Non-Member Swap Tests\n";
	std::cout << "========================================\n";

	// Run tests
	TestNonMemberSwap();
	TestSwapWithStdSwap();
	TestSwapWithWideCharacters();
	TestSwapWithFlags();

	std::cout << "\n========================================\n";
	std::cout << "All tests PASSED!\n";
	std::cout << "========================================\n";

	return 0;
}
