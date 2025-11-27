// match_results_format_test.cpp --- Tests for match_results::format
// Author: katahiromz
// License: BSD-2-Clause
//
// Note: This test file tests onigpp::match_results::format, which is
// an onigpp-specific implementation. When USE_STD_FOR_TESTS is defined,
// the tests are skipped as std::match_results::format has different behavior.

#include "tests.h"
#include <iterator>
#include <sstream>

#ifndef USE_STD_FOR_TESTS

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
// 1. Basic $n replacements
// -----------------------------------------------------------------

void TestBasicNumericReplacement() {
	TEST_CASE("TestBasicNumericReplacement")

	std::string text = "Hello World";
	rex::regex re("(\\w+)\\s+(\\w+)");
	rex::smatch m;

	assert(rex::regex_search(text, m, re));
	assert(m.size() == 3);  // Full match + 2 groups

	// Test $0 for full match
	std::string result0 = m.format("[$0]");
	std::cout << "  $0: " << result0 << std::endl;
	assert(result0 == "[Hello World]");

	// Test $1, $2
	std::string result1 = m.format("$2 $1");
	std::cout << "  $2 $1: " << result1 << std::endl;
	assert(result1 == "World Hello");

	// Test multi-digit group numbers (if available)
	std::string text2 = "abcdefghij";
	rex::regex re2("(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)");
	rex::smatch m2;
	assert(rex::regex_search(text2, m2, re2));
	assert(m2.size() == 11);  // Full match + 10 groups

	std::string result2 = m2.format("$10=$10, $1=$1");
	std::cout << "  $10: " << result2 << std::endl;
	assert(result2 == "j=j, a=a");

	TEST_CASE_END("TestBasicNumericReplacement")
}

// -----------------------------------------------------------------
// 2. $& replacement (full match)
// -----------------------------------------------------------------

void TestFullMatchReplacement() {
	TEST_CASE("TestFullMatchReplacement")

	std::string text = "Hello World";
	rex::regex re("\\w+\\s+\\w+");
	rex::smatch m;

	assert(rex::regex_search(text, m, re));

	std::string result = m.format("[$&]");
	std::cout << "  $&: " << result << std::endl;
	assert(result == "[Hello World]");

	TEST_CASE_END("TestFullMatchReplacement")
}

// -----------------------------------------------------------------
// 3. $` and $' replacements (prefix and suffix)
// -----------------------------------------------------------------

void TestPrefixSuffixReplacement() {
	TEST_CASE("TestPrefixSuffixReplacement")

	std::string text = "BEFORE_Match_AFTER";
	rex::regex re("Match");
	rex::smatch m;

	assert(rex::regex_search(text, m, re));

	// Test prefix
	std::string result_prefix = m.format("Before=[$`]");
	std::cout << "  Prefix: " << result_prefix << std::endl;
	assert(result_prefix == "Before=[BEFORE_]");

	// Test suffix
	std::string result_suffix = m.format("After=[$']");
	std::cout << "  Suffix: " << result_suffix << std::endl;
	assert(result_suffix == "After=[_AFTER]");

	// Test combined
	std::string result_combined = m.format("$`[$&]$'");
	std::cout << "  Combined: " << result_combined << std::endl;
	assert(result_combined == "BEFORE_[Match]_AFTER");

	TEST_CASE_END("TestPrefixSuffixReplacement")
}

// -----------------------------------------------------------------
// 4. $$ replacement (literal $)
// -----------------------------------------------------------------

void TestLiteralDollarReplacement() {
	TEST_CASE("TestLiteralDollarReplacement")

	std::string text = "Hello";
	rex::regex re("Hello");
	rex::smatch m;

	assert(rex::regex_search(text, m, re));

	std::string result = m.format("$$100.00");
	std::cout << "  $$: " << result << std::endl;
	assert(result == "$100.00");

	// Test $$ in middle
	std::string result2 = m.format("Price: $$50");
	std::cout << "  $$ in middle: " << result2 << std::endl;
	assert(result2 == "Price: $50");

	TEST_CASE_END("TestLiteralDollarReplacement")
}

// -----------------------------------------------------------------
// 5. Escape sequences (\n, \t, \r, \\)
// -----------------------------------------------------------------

void TestEscapeSequences() {
	TEST_CASE("TestEscapeSequences")

	std::string text = "Hello";
	rex::regex re("Hello");
	rex::smatch m;

	assert(rex::regex_search(text, m, re));

	// Test \n
	std::string result_n = m.format("Line1\\nLine2");
	std::cout << "  \\n: [" << result_n << "]" << std::endl;
	assert(result_n == "Line1\nLine2");

	// Test \t
	std::string result_t = m.format("Col1\\tCol2");
	std::cout << "  \\t: [" << result_t << "]" << std::endl;
	assert(result_t == "Col1\tCol2");

	// Test \r
	std::string result_r = m.format("Line1\\rLine2");
	std::cout << "  \\r length: " << result_r.size() << std::endl;
	assert(result_r == "Line1\rLine2");

	// Test \\ (literal backslash)
	std::string result_bs = m.format("Path\\\\File");
	std::cout << "  \\\\: [" << result_bs << "]" << std::endl;
	assert(result_bs == "Path\\File");

	TEST_CASE_END("TestEscapeSequences")
}

// -----------------------------------------------------------------
// 6. Unmatched submatches (replaced with empty string)
// -----------------------------------------------------------------

void TestUnmatchedSubmatches() {
	TEST_CASE("TestUnmatchedSubmatches")

	// Pattern with optional groups
	std::string text = "abc";
	rex::regex re("(a)(b)?(c)?");
	rex::smatch m;

	assert(rex::regex_search(text, m, re));
	std::cout << "  Match size: " << m.size() << std::endl;
	for (size_t i = 0; i < m.size(); ++i) {
		std::cout << "    m[" << i << "] = '" << m[i].str() << "' matched=" << m[i].matched << std::endl;
	}

	// Even if group 3 might be matched, let's test with a pattern that has unmatched groups
	std::string text2 = "a";
	rex::regex re2("(a)(b)?");
	rex::smatch m2;

	assert(rex::regex_search(text2, m2, re2));
	std::cout << "  Match2 size: " << m2.size() << std::endl;
	for (size_t i = 0; i < m2.size(); ++i) {
		std::cout << "    m2[" << i << "] = '" << m2[i].str() << "' matched=" << m2[i].matched << std::endl;
	}

	// Group 2 (b)? should be unmatched, so $2 should be empty
	std::string result = m2.format("Group1=[$1] Group2=[$2]");
	std::cout << "  Result: " << result << std::endl;
	assert(result == "Group1=[a] Group2=[]");

	// Test out-of-range group reference
	std::string result_oor = m2.format("Group99=[$99]");
	std::cout << "  Out of range: " << result_oor << std::endl;
	assert(result_oor == "Group99=[]");

	TEST_CASE_END("TestUnmatchedSubmatches")
}

// -----------------------------------------------------------------
// 7. Full match vs partial match scenarios
// -----------------------------------------------------------------

void TestFullPartialMatchScenarios() {
	TEST_CASE("TestFullPartialMatchScenarios")

	// Full match using regex_match
	std::string text = "Hello World";
	rex::regex re("(\\w+)\\s+(\\w+)");
	rex::smatch m;

	assert(rex::regex_match(text, m, re));
	std::string result_full = m.format("Full: $0, Parts: $1 and $2");
	std::cout << "  Full match: " << result_full << std::endl;
	assert(result_full == "Full: Hello World, Parts: Hello and World");

	// Partial match using regex_search (with prefix and suffix)
	// Use a more specific pattern to match "Hello World" exactly
	std::string text2 = "Start_Hello_World_End";
	rex::regex re2("Hello_World");
	rex::smatch m2;

	assert(rex::regex_search(text2, m2, re2));
	std::string result_partial = m2.format("Before=[$`] Match=[$&] After=[$']");
	std::cout << "  Partial match: " << result_partial << std::endl;
	assert(result_partial == "Before=[Start_] Match=[Hello_World] After=[_End]");

	TEST_CASE_END("TestFullPartialMatchScenarios")
}

// -----------------------------------------------------------------
// 8. OutputIterator overload test
// -----------------------------------------------------------------

void TestOutputIteratorFormat() {
	TEST_CASE("TestOutputIteratorFormat")

	std::string text = "Hello World";
	rex::regex re("(\\w+)\\s+(\\w+)");
	rex::smatch m;

	assert(rex::regex_search(text, m, re));

	// Test with back_inserter
	std::string result;
	m.format(std::back_inserter(result), std::string("$2 $1"));
	std::cout << "  back_inserter: " << result << std::endl;
	assert(result == "World Hello");

	// Test with ostream_iterator
	std::ostringstream oss;
	m.format(std::ostreambuf_iterator<char>(oss), std::string("[$0]"));
	std::cout << "  ostreambuf_iterator: " << oss.str() << std::endl;
	assert(oss.str() == "[Hello World]");

	TEST_CASE_END("TestOutputIteratorFormat")
}

// -----------------------------------------------------------------
// 9. C-string format overload test
// -----------------------------------------------------------------

void TestCStringFormat() {
	TEST_CASE("TestCStringFormat")

	std::string text = "Hello World";
	rex::regex re("(\\w+)\\s+(\\w+)");
	rex::smatch m;

	assert(rex::regex_search(text, m, re));

	// Test C-string overload
	std::string result = m.format("$2 $1");
	std::cout << "  C-string format: " << result << std::endl;
	assert(result == "World Hello");

	TEST_CASE_END("TestCStringFormat")
}

// -----------------------------------------------------------------
// 10. Wide string support test
// -----------------------------------------------------------------

void TestWideStringFormat() {
	TEST_CASE("TestWideStringFormat")

	std::wstring text = L"Hello World";
	rex::wregex re(L"(\\w+)\\s+(\\w+)");
	rex::wsmatch m;

	assert(rex::regex_search(text, m, re));

	std::wstring result = m.format(L"$2 $1");
	std::wcout << L"  Wide string format: " << result << std::endl;
	assert(result == L"World Hello");

	TEST_CASE_END("TestWideStringFormat")
}

// -----------------------------------------------------------------
// 11. ${n} safe numbered reference test
// -----------------------------------------------------------------

void TestSafeNumberedReference() {
	TEST_CASE("TestSafeNumberedReference")

	// Test ${n} syntax which allows explicit group boundaries
	// e.g., ${1}0 means "group 1 followed by literal 0" vs $10 which is "group 10"
	std::string text = "abcdefghij";
	rex::regex re("(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)");
	rex::smatch m;

	assert(rex::regex_search(text, m, re));
	assert(m.size() == 11);  // Full match + 10 groups

	// Test ${1}0 - group 1 followed by literal '0'
	std::string result1 = m.format("${1}0");
	std::cout << "  ${1}0: " << result1 << std::endl;
	assert(result1 == "a0");

	// Test ${10} - group 10 (same as $10)
	std::string result2 = m.format("${10}");
	std::cout << "  ${10}: " << result2 << std::endl;
	assert(result2 == "j");

	// Compare $10 (greedy) vs ${1}0 (explicit boundary)
	std::string result3 = m.format("$10 vs ${1}0");
	std::cout << "  $10 vs ${1}0: " << result3 << std::endl;
	assert(result3 == "j vs a0");

	// Test ${0} - full match
	std::string result4 = m.format("[${0}]");
	std::cout << "  ${0}: " << result4 << std::endl;
	assert(result4 == "[abcdefghij]");

	TEST_CASE_END("TestSafeNumberedReference")
}

#endif // !USE_STD_FOR_TESTS

// =================================================================
// Main Function
// =================================================================

int main() {
	TESTS_OUTPUT_INIT();

#ifndef USE_STD_FOR_TESTS
	// Oniguruma initialization (no-op for std::regex)
	ONIGPP_TEST_INIT;

	std::cout << "========================================================\n";
	std::cout << " match_results::format Test Suite\n";
	std::cout << "========================================================\n";

	TestBasicNumericReplacement();
	TestFullMatchReplacement();
	TestPrefixSuffixReplacement();
	TestLiteralDollarReplacement();
	TestEscapeSequences();
	TestUnmatchedSubmatches();
	TestFullPartialMatchScenarios();
	TestOutputIteratorFormat();
	TestCStringFormat();
	TestWideStringFormat();
	TestSafeNumberedReference();

	std::cout << "\n========================================================\n";
	std::cout << "✨ All match_results::format tests succeeded.\n";
	std::cout << "========================================================\n";
#else
	std::cout << "========================================================\n";
	std::cout << " match_results::format Test Suite\n";
	std::cout << "========================================================\n";
	std::cout << "\n  Skipping tests: match_results::format is onigpp-specific.\n";
	std::cout << "  When USE_STD_FOR_TESTS is defined, std::match_results::format\n";
	std::cout << "  is used, which has different behavior.\n";
	std::cout << "\n========================================================\n";
	std::cout << "✨ Test skipped (USE_STD_FOR_TESTS mode).\n";
	std::cout << "========================================================\n";
#endif

	return 0;
}
