// regex_escape_test.cpp --- Tests for onigpp::regex_escape
// Author: katahiromz
// License: BSD-2-Clause

#include "../onigpp.h"
#include <iostream>
#include <cassert>

// Helper macro for test assertions
#define TEST_ASSERT(cond) do { \
	if (!(cond)) { \
		std::cerr << "FAIL: " << #cond << " at line " << __LINE__ << std::endl; \
		return 1; \
	} \
} while(0)

#define TEST_EQUAL(expected, actual) do { \
	if ((expected) != (actual)) { \
		std::cerr << "FAIL: expected \"" << (expected) << "\", got \"" << (actual) << "\" at line " << __LINE__ << std::endl; \
		return 1; \
	} \
} while(0)

int main() {
	onigpp::auto_init init;

	std::cout << "Testing onigpp::regex_escape..." << std::endl;

	// Test 1: Empty string
	{
		std::string input = "";
		std::string expected = "";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Empty string test" << std::endl;
	}

	// Test 2: String with no metacharacters
	{
		std::string input = "hello world 123";
		std::string expected = "hello world 123";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] No metacharacters test" << std::endl;
	}

	// Test 3: Dot character
	{
		std::string input = "a.b";
		std::string expected = "a\\.b";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Dot escape test" << std::endl;
	}

	// Test 4: Caret (^)
	{
		std::string input = "^start";
		std::string expected = "\\^start";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Caret escape test" << std::endl;
	}

	// Test 5: Dollar ($)
	{
		std::string input = "end$";
		std::string expected = "end\\$";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Dollar escape test" << std::endl;
	}

	// Test 6: Asterisk (*)
	{
		std::string input = "a*b";
		std::string expected = "a\\*b";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Asterisk escape test" << std::endl;
	}

	// Test 7: Plus (+)
	{
		std::string input = "a+b";
		std::string expected = "a\\+b";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Plus escape test" << std::endl;
	}

	// Test 8: Question mark (?)
	{
		std::string input = "a?b";
		std::string expected = "a\\?b";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Question mark escape test" << std::endl;
	}

	// Test 9: Parentheses ()
	{
		std::string input = "(group)";
		std::string expected = "\\(group\\)";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Parentheses escape test" << std::endl;
	}

	// Test 10: Square brackets []
	{
		std::string input = "[abc]";
		std::string expected = "\\[abc\\]";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Square brackets escape test" << std::endl;
	}

	// Test 11: Curly braces {}
	{
		std::string input = "{1,3}";
		std::string expected = "\\{1,3\\}";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Curly braces escape test" << std::endl;
	}

	// Test 12: Backslash (\)
	{
		std::string input = "a\\b";
		std::string expected = "a\\\\b";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Backslash escape test" << std::endl;
	}

	// Test 13: Pipe (|)
	{
		std::string input = "a|b";
		std::string expected = "a\\|b";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Pipe escape test" << std::endl;
	}

	// Test 14: All metacharacters in one string (from problem statement example)
	{
		std::string input = "a+b*c.d?e^f$g|h(i)j[k]{l}\\m";
		std::string expected = "a\\+b\\*c\\.d\\?e\\^f\\$g\\|h\\(i\\)j\\[k\\]\\{l\\}\\\\m";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] All metacharacters test" << std::endl;
	}

	// Test 15: Multiple consecutive metacharacters
	{
		std::string input = "***";
		std::string expected = "\\*\\*\\*";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Consecutive metacharacters test" << std::endl;
	}

	// Test 16: Metacharacters at beginning and end
	{
		std::string input = "^abc$";
		std::string expected = "\\^abc\\$";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Metacharacters at boundaries test" << std::endl;
	}

	// Test 17: C-string overload
	{
		const char* input = "a.b";
		std::string expected = "a\\.b";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] C-string overload test" << std::endl;
	}

	// Test 18: Wide string (wchar_t)
	{
		std::wstring input = L"a.b*c";
		std::wstring expected = L"a\\.b\\*c";
		std::wstring result = onigpp::regex_escape(input);
		TEST_ASSERT(expected == result);
		std::cout << "  [PASS] Wide string (wchar_t) test" << std::endl;
	}

	// Test 19: UTF-16 string (char16_t)
	{
		std::u16string input = u"a+b";
		std::u16string expected = u"a\\+b";
		std::u16string result = onigpp::regex_escape(input);
		TEST_ASSERT(expected == result);
		std::cout << "  [PASS] UTF-16 string (char16_t) test" << std::endl;
	}

	// Test 20: UTF-32 string (char32_t)
	{
		std::u32string input = U"a?b";
		std::u32string expected = U"a\\?b";
		std::u32string result = onigpp::regex_escape(input);
		TEST_ASSERT(expected == result);
		std::cout << "  [PASS] UTF-32 string (char32_t) test" << std::endl;
	}

	// Test 21: Verify escaped string works as literal in regex
	{
		std::string literal = "a+b*c";
		std::string escaped = onigpp::regex_escape(literal);
		onigpp::regex re(escaped);
		onigpp::smatch match;
		
		// Should match the exact literal string
		TEST_ASSERT(onigpp::regex_match(literal, match, re));
		
		// Should NOT match transformed strings
		std::string test1 = "ab";
		std::string test2 = "abc";
		std::string test3 = "aabbcc";
		TEST_ASSERT(!onigpp::regex_match(test1, match, re));  // '+' not acting as quantifier
		TEST_ASSERT(!onigpp::regex_match(test2, match, re));  // '*' not acting as quantifier
		TEST_ASSERT(!onigpp::regex_match(test3, match, re));  // Just to be sure
		
		std::cout << "  [PASS] Regex literal match verification test" << std::endl;
	}

	// Test 22: Special case - only backslashes
	{
		std::string input = "\\\\\\";
		std::string expected = "\\\\\\\\\\\\";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Multiple backslashes test" << std::endl;
	}

	// Test 23: Mixed content with spaces and tabs
	{
		std::string input = "hello .* world";
		std::string expected = "hello \\.\\* world";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Mixed content with whitespace test" << std::endl;
	}

	// Test 24: Numbers and letters with metacharacters
	{
		std::string input = "file[0-9]+.txt";
		std::string expected = "file\\[0-9\\]\\+\\.txt";
		std::string result = onigpp::regex_escape(input);
		TEST_EQUAL(expected, result);
		std::cout << "  [PASS] Numbers and letters with metacharacters test" << std::endl;
	}

	std::cout << "\nAll regex_escape tests passed!" << std::endl;
	return 0;
}
