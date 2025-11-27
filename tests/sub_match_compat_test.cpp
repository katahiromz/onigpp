// sub_match_compat_test.cpp --- Test std::sub_match compatibility features
// Author: katahiromz
// License: BSD-2-Clause
#include "tests.h"
#include <sstream>

// =================================================================
// Helper Functions
// =================================================================

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
// 1. Test templated converting constructor
// -----------------------------------------------------------------

void TestConvertingConstructor() {
	TEST_CASE("TestConvertingConstructor")

	// Test converting between compatible pointer types
	const char* str = "hello world";
	const char* start = str;
	const char* end = str + 5; // "hello"
	
	// Create a sub_match with const char* iterators
	rex::csub_match csm;
	csm.first = start;
	csm.second = end;
	csm.matched = true;
	assert(csm.str() == "hello");
	assert(csm.matched == true);
	
	// Test copy construction (same type)
	rex::csub_match csm_copy(csm);
	assert(csm_copy.matched == true);
	assert(csm_copy.str() == "hello");
	
	// Test converting unmatched sub_match
	rex::csub_match csm_unmatched;
	csm_unmatched.first = start;
	csm_unmatched.second = start;
	csm_unmatched.matched = false;
	assert(csm_unmatched.matched == false);
	assert(csm_unmatched.str() == "");
	
	rex::csub_match csm_unmatched_copy(csm_unmatched);
	assert(csm_unmatched_copy.matched == false);
	assert(csm_unmatched_copy.str() == "");

	TEST_CASE_END("TestConvertingConstructor")
}

// -----------------------------------------------------------------
// 2. Test default value for is_matched parameter
// -----------------------------------------------------------------

void TestDefaultIsMatched() {
	TEST_CASE("TestDefaultIsMatched")

	const char* str = "test";
	const char* start = str;
	const char* end = str + 4;
	
	// Test with matched flag set to true
	rex::csub_match csm;
	csm.first = start;
	csm.second = end;
	csm.matched = true;
	assert(csm.matched == true);
	assert(csm.str() == "test");
	
	// Explicitly set to false
	rex::csub_match csm_false;
	csm_false.first = start;
	csm_false.second = end;
	csm_false.matched = false;
	assert(csm_false.matched == false);
	// str() should return empty string when matched is false (std::regex compatible)
	assert(csm_false.str() == "");
	
	// Explicitly set to true
	rex::csub_match csm_true;
	csm_true.first = start;
	csm_true.second = end;
	csm_true.matched = true;
	assert(csm_true.matched == true);
	assert(csm_true.str() == "test");

	TEST_CASE_END("TestDefaultIsMatched")
}

// -----------------------------------------------------------------
// 3. Test implicit conversion to string_type
// -----------------------------------------------------------------

void TestImplicitStringConversion() {
	TEST_CASE("TestImplicitStringConversion")

	const char* str = "example";
	const char* start = str;
	const char* end = str + 7;
	
	rex::csub_match csm;
	csm.first = start;
	csm.second = end;
	csm.matched = true;
	
	// Implicit conversion to std::string
	std::string result = csm;  // Should compile due to operator string_type()
	assert(result == "example");
	
	// Test with string iterator
	std::string s = "testing";
	rex::ssub_match ssm;
	ssm.first = s.begin();
	ssm.second = s.begin() + 4;
	ssm.matched = true;
	std::string result2 = ssm;  // Should also work
	assert(result2 == "test");
	
	// Test assignment to string
	std::string assigned;
	assigned = csm;
	assert(assigned == "example");

	TEST_CASE_END("TestImplicitStringConversion")
}

// -----------------------------------------------------------------
// 4. Test length() helper method
// -----------------------------------------------------------------

void TestLengthHelper() {
	TEST_CASE("TestLengthHelper")

	const char* str = "hello world";
	const char* start = str;
	const char* end = str + 5; // "hello"
	
	rex::csub_match csm;
	csm.first = start;
	csm.second = end;
	csm.matched = true;
	assert(csm.length() == 5);
	assert(csm.str() == "hello");
	
	// Test with empty match
	rex::csub_match csm_empty;
	csm_empty.first = start;
	csm_empty.second = start;
	csm_empty.matched = true;
	assert(csm_empty.length() == 0);
	assert(csm_empty.str() == "");
	
	// Test with string iterator
	std::string s = "testing";
	rex::ssub_match ssm;
	ssm.first = s.begin();
	ssm.second = s.begin() + 7;
	ssm.matched = true;
	assert(ssm.length() == 7);
	assert(ssm.str() == "testing");
	
	// Test length matches str().length()
	assert(csm.length() == csm.str().length());
	assert(ssm.length() == ssm.str().length());

	TEST_CASE_END("TestLengthHelper")
}

// -----------------------------------------------------------------
// 5. Test integration with regex_search
// -----------------------------------------------------------------

void TestIntegrationWithRegex() {
	TEST_CASE("TestIntegrationWithRegex")

	std::string text = "User ID: u123";
	rex::regex re("ID: ([a-z0-9]+)");
	rex::smatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	assert(m.size() == 2);
	
	// Test length() on matched sub_match
	assert(m[0].length() == 8);  // "ID: u123" is 8 characters
	assert(m[1].length() == 4);  // "u123"
	
	// Test implicit conversion to string
	std::string full_match = m[0];
	std::string captured = m[1];
	assert(full_match == "ID: u123");
	assert(captured == "u123");
	
	// Test that default constructor still sets matched to false
	rex::ssub_match default_sm;
	assert(default_sm.matched == false);

	TEST_CASE_END("TestIntegrationWithRegex")
}

// -----------------------------------------------------------------
// 6. Test compare() method
// -----------------------------------------------------------------

void TestCompareMethod() {
	TEST_CASE("TestCompareMethod")

	const char* str1 = "abc";
	const char* str2 = "abd";
	const char* str3 = "abc";

	rex::csub_match sm1;
	sm1.first = str1;
	sm1.second = str1 + 3; // "abc"
	sm1.matched = true;

	rex::csub_match sm2;
	sm2.first = str2;
	sm2.second = str2 + 3; // "abd"
	sm2.matched = true;

	rex::csub_match sm3;
	sm3.first = str3;
	sm3.second = str3 + 3; // "abc"
	sm3.matched = true;

	// Test compare with another sub_match
	assert(sm1.compare(sm3) == 0);  // "abc" == "abc"
	assert(sm1.compare(sm2) < 0);   // "abc" < "abd"
	assert(sm2.compare(sm1) > 0);   // "abd" > "abc"
	
	// Test compare with string_type
	std::string s_abc = "abc";
	std::string s_abd = "abd";
	assert(sm1.compare(s_abc) == 0);
	assert(sm1.compare(s_abd) < 0);
	
	// Test compare with C-string
	assert(sm1.compare("abc") == 0);
	assert(sm1.compare("abd") < 0);
	assert(sm1.compare("abb") > 0);
	
	// Test unmatched sub_match (should compare as empty string)
	rex::csub_match sm_unmatched;
	sm_unmatched.first = str1;
	sm_unmatched.second = str1 + 3;
	sm_unmatched.matched = false;
	assert(sm_unmatched.compare("") == 0);
	assert(sm_unmatched.compare("a") < 0);

	TEST_CASE_END("TestCompareMethod")
}

// -----------------------------------------------------------------
// 7. Test comparison operators between sub_match objects
// -----------------------------------------------------------------

void TestSubMatchComparisonOperators() {
	TEST_CASE("TestSubMatchComparisonOperators")

	const char* str1 = "apple";
	const char* str2 = "banana";
	const char* str3 = "apple";

	rex::csub_match sm1;
	sm1.first = str1;
	sm1.second = str1 + 5; // "apple"
	sm1.matched = true;

	rex::csub_match sm2;
	sm2.first = str2;
	sm2.second = str2 + 6; // "banana"
	sm2.matched = true;

	rex::csub_match sm3;
	sm3.first = str3;
	sm3.second = str3 + 5; // "apple"
	sm3.matched = true;

	// Test equality operators
	assert(sm1 == sm3);
	assert(!(sm1 == sm2));
	assert(sm1 != sm2);
	assert(!(sm1 != sm3));
	
	// Test ordering operators
	assert(sm1 < sm2);      // "apple" < "banana"
	assert(!(sm2 < sm1));
	assert(!(sm1 < sm3));   // "apple" is not less than "apple"
	
	assert(sm1 <= sm2);
	assert(sm1 <= sm3);
	assert(!(sm2 <= sm1));
	
	assert(sm2 > sm1);      // "banana" > "apple"
	assert(!(sm1 > sm2));
	assert(!(sm1 > sm3));
	
	assert(sm2 >= sm1);
	assert(sm1 >= sm3);
	assert(!(sm1 >= sm2));

	TEST_CASE_END("TestSubMatchComparisonOperators")
}

// -----------------------------------------------------------------
// 8. Test comparison operators between sub_match and string_type
// -----------------------------------------------------------------

void TestSubMatchStringComparison() {
	TEST_CASE("TestSubMatchStringComparison")

	const char* str = "hello";
	rex::csub_match sm;
	sm.first = str;
	sm.second = str + 5; // "hello"
	sm.matched = true;

	std::string s_hello = "hello";
	std::string s_world = "world";
	std::string s_abc = "abc";

	// sub_match vs string
	assert(sm == s_hello);
	assert(!(sm == s_world));
	assert(sm != s_world);
	assert(!(sm != s_hello));
	assert(sm < s_world);   // "hello" < "world"
	assert(sm > s_abc);     // "hello" > "abc"
	assert(sm <= s_hello);
	assert(sm <= s_world);
	assert(sm >= s_hello);
	assert(sm >= s_abc);
	
	// string vs sub_match
	assert(s_hello == sm);
	assert(!(s_world == sm));
	assert(s_world != sm);
	assert(!(s_hello != sm));
	assert(s_abc < sm);     // "abc" < "hello"
	assert(s_world > sm);   // "world" > "hello"
	assert(s_hello <= sm);
	assert(s_abc <= sm);
	assert(s_hello >= sm);
	assert(s_world >= sm);

	TEST_CASE_END("TestSubMatchStringComparison")
}

// -----------------------------------------------------------------
// 9. Test comparison operators between sub_match and C-string
// -----------------------------------------------------------------

void TestSubMatchCStringComparison() {
	TEST_CASE("TestSubMatchCStringComparison")

	const char* str = "test";
	rex::csub_match sm;
	sm.first = str;
	sm.second = str + 4; // "test"
	sm.matched = true;

	// sub_match vs C-string
	assert(sm == "test");
	assert(!(sm == "other"));
	assert(sm != "other");
	assert(!(sm != "test"));
	assert(sm < "zebra");   // "test" < "zebra"
	assert(sm > "apple");   // "test" > "apple"
	assert(sm <= "test");
	assert(sm <= "zebra");
	assert(sm >= "test");
	assert(sm >= "apple");
	
	// C-string vs sub_match
	assert("test" == sm);
	assert(!("other" == sm));
	assert("other" != sm);
	assert(!("test" != sm));
	assert("apple" < sm);   // "apple" < "test"
	assert("zebra" > sm);   // "zebra" > "test"
	assert("test" <= sm);
	assert("apple" <= sm);
	assert("test" >= sm);
	assert("zebra" >= sm);

	TEST_CASE_END("TestSubMatchCStringComparison")
}

// -----------------------------------------------------------------
// 10. Test stream output operator
// -----------------------------------------------------------------

void TestStreamOutputOperator() {
	TEST_CASE("TestStreamOutputOperator")

	const char* str = "output test";
	rex::csub_match sm;
	sm.first = str;
	sm.second = str + 11; // "output test"
	sm.matched = true;

	std::ostringstream oss;
	oss << sm;
	assert(oss.str() == "output test");
	
	// Test with unmatched sub_match (should output empty string)
	rex::csub_match sm_unmatched;
	sm_unmatched.first = str;
	sm_unmatched.second = str + 5;
	sm_unmatched.matched = false;
	
	std::ostringstream oss2;
	oss2 << sm_unmatched;
	assert(oss2.str() == "");
	
	// Test with string iterator
	std::string s = "string iterator test";
	rex::ssub_match ssm;
	ssm.first = s.begin();
	ssm.second = s.begin() + 6; // "string"
	ssm.matched = true;
	
	std::ostringstream oss3;
	oss3 << ssm;
	assert(oss3.str() == "string");

	TEST_CASE_END("TestStreamOutputOperator")
}

// -----------------------------------------------------------------
// 11. Test with regex search results
// -----------------------------------------------------------------

void TestComparisonWithRegexResults() {
	TEST_CASE("TestComparisonWithRegexResults")

	std::string text = "The quick brown fox jumps over the lazy dog";
	rex::regex re("(\\w+) (\\w+) (\\w+)");
	rex::smatch m;
	
	bool found = rex::regex_search(text, m, re);
	assert(found);
	assert(m.size() >= 4);
	
	// Test comparison operators on captured groups
	// m[1] = "The", m[2] = "quick", m[3] = "brown"
	assert(m[1] == "The");
	assert(m[2] == "quick");
	assert(m[3] == "brown");
	
	// String comparison follows std::basic_string::compare semantics
	assert(m[1] < m[2]);    // "The" < "quick"
	assert(m[3] < m[2]);    // "brown" < "quick"
	
	// Test compare method
	assert(m[1].compare("The") == 0);
	assert(m[2].compare(std::string("quick")) == 0);
	
	// Test stream output
	std::ostringstream oss;
	oss << m[1] << " " << m[2] << " " << m[3];
	assert(oss.str() == "The quick brown");

	TEST_CASE_END("TestComparisonWithRegexResults")
}

// -----------------------------------------------------------------
// Main function
// -----------------------------------------------------------------

int main() {
	TESTS_OUTPUT_INIT();

	// Oniguruma initialization (no-op for std::regex)
	ONIGPP_TEST_INIT;

	std::cout << "=================================\n";
	std::cout << "sub_match std::sub_match Compatibility Tests\n";
	std::cout << "=================================\n";

	TestConvertingConstructor();
	TestDefaultIsMatched();
	TestImplicitStringConversion();
	TestLengthHelper();
	TestIntegrationWithRegex();
	TestCompareMethod();
	TestSubMatchComparisonOperators();
	TestSubMatchStringComparison();
	TestSubMatchCStringComparison();
	TestStreamOutputOperator();
	TestComparisonWithRegexResults();

	std::cout << "\n=================================\n";
	std::cout << "All tests passed! ✅\n";
	std::cout << "=================================\n";

	return 0;
}
