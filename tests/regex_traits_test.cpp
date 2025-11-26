// regex_traits_test.cpp --- Tests for onigpp::regex_traits
// Author: katahiromz
// License: BSD-2-Clause
#include "tests.h"

// Helper macros for test cases
#define TEST_CASE(name) \
	std::cout << "\n--- " << (name) << " ---\n"; \
	try {

#define TEST_CASE_END(name) \
	std::cout << "✅ " << (name) << " PASSED.\n"; \
	} catch (const std::exception& e) { \
		std::cout << "❌ " << (name) << " FAILED with exception: " << e.what() << "\n"; \
		assert(false); \
	} catch (...) { \
		std::cout << "❌ " << (name) << " FAILED with unknown exception.\n"; \
		assert(false); \
	}

int main() {
	TESTS_OUTPUT_INIT();

	// Oniguruma initialization (no-op for std::regex)
	ONIGPP_TEST_INIT;

	std::cout << "=== regex_traits Tests ===\n";

	// Test 1: Typedefs and basic construction
	TEST_CASE("Typedefs and Constructors")
		myns::regex_traits<char> traits1;
		myns::regex_traits<wchar_t> wtraits1;
#ifndef USE_STD_FOR_TESTS
		myns::regex_traits<char> traits2{std::locale()};
		myns::regex_traits<wchar_t> wtraits2{std::locale()};
#endif

		// Check that locale_type and char_class_type exist
		myns::regex_traits<char>::locale_type loc;
		myns::regex_traits<char>::char_class_type cct = 0;
		(void)loc;
		(void)cct;
		
		std::cout << "Typedefs and constructors work correctly\n";
	TEST_CASE_END("Typedefs and Constructors")

	// Test 2: getloc and imbue
	TEST_CASE("getloc and imbue")
		myns::regex_traits<char> traits;
		std::locale default_loc = traits.getloc();
		
		std::locale new_loc = std::locale::classic();
		std::locale old_loc = traits.imbue(new_loc);
		
		std::locale current_loc = traits.getloc();
		std::cout << "getloc and imbue work correctly\n";
	TEST_CASE_END("getloc and imbue")

	// Test 3: translate
	TEST_CASE("translate")
		myns::regex_traits<char> traits;
		
		// translate should return the character as-is
		assert(traits.translate('A') == 'A');
		assert(traits.translate('a') == 'a');
		assert(traits.translate('5') == '5');
		assert(traits.translate(' ') == ' ');
		
		std::cout << "translate works correctly\n";
	TEST_CASE_END("translate")

	// Test 4: transform
	TEST_CASE("transform")
		myns::regex_traits<char> traits;
		
		std::string s1 = "test";
		std::string result = traits.transform(s1.data(), s1.data() + s1.size());
		
		// transform should return a string (exact result is locale-dependent)
		assert(!result.empty() || s1.empty());
		std::cout << "transform works correctly\n";
	TEST_CASE_END("transform")

	// Test 5: value with different bases
	TEST_CASE("value")
		myns::regex_traits<char> traits;
		
		// Base 10 (default)
		assert(traits.value('0', 10) == 0);
		assert(traits.value('5', 10) == 5);
		assert(traits.value('9', 10) == 9);
		assert(traits.value('a', 10) == -1); // Invalid for base 10
		assert(traits.value('A', 10) == -1); // Invalid for base 10
		
		// Base 16
		assert(traits.value('0', 16) == 0);
		assert(traits.value('9', 16) == 9);
		assert(traits.value('a', 16) == 10);
		assert(traits.value('A', 16) == 10);
		assert(traits.value('f', 16) == 15);
		assert(traits.value('F', 16) == 15);
		assert(traits.value('g', 16) == -1); // Invalid for base 16
		
		// Base 8
		assert(traits.value('0', 8) == 0);
		assert(traits.value('7', 8) == 7);
		assert(traits.value('8', 8) == -1); // Invalid for base 8
		assert(traits.value('9', 8) == -1); // Invalid for base 8
		
		// Base 2
		assert(traits.value('0', 2) == 0);
		assert(traits.value('1', 2) == 1);
#ifndef USE_STD_FOR_TESTS // It seems missing
		assert(traits.value('2', 2) == -1); // Invalid for base 2
#endif

#ifndef USE_STD_FOR_TESTS // It seems missing
		// Default parameter (base 10)
		assert(traits.value('5') == 5);
		assert(traits.value('a') == -1);
#endif

		std::cout << "value works correctly for different bases\n";
	TEST_CASE_END("value")

	// Test 6: isctype
	TEST_CASE("isctype")
		myns::regex_traits<char> traits;
		
		// Test with some common character class types
		// Note: The exact behavior depends on locale and char_class_type values
		myns::regex_traits<char>::char_class_type digit_class = std::ctype_base::digit;
		myns::regex_traits<char>::char_class_type alpha_class = std::ctype_base::alpha;
		myns::regex_traits<char>::char_class_type space_class = std::ctype_base::space;
		
		// These should work with standard locale
		bool is_digit = traits.isctype('5', digit_class);
		bool is_not_digit = traits.isctype('a', digit_class);
		bool is_alpha = traits.isctype('a', alpha_class);
		bool is_space = traits.isctype(' ', space_class);
		
		std::cout << "isctype: '5' is digit = " << is_digit << "\n";
		std::cout << "isctype: 'a' is digit = " << is_not_digit << "\n";
		std::cout << "isctype: 'a' is alpha = " << is_alpha << "\n";
		std::cout << "isctype: ' ' is space = " << is_space << "\n";
		
		std::cout << "isctype works correctly\n";
	TEST_CASE_END("isctype")

	// Test 7: lookup_collatename
	TEST_CASE("lookup_collatename")
		myns::regex_traits<char> traits;
		
		std::string name = "a";
		std::string result = traits.lookup_collatename(name.data(), name.data() + name.size());
		
		// lookup_collatename returns empty string in our implementation
		// (safe, portable default)
		std::cout << "lookup_collatename works correctly\n";
	TEST_CASE_END("lookup_collatename")

	// Test 8: wchar_t traits
	TEST_CASE("wchar_t traits")
		myns::regex_traits<wchar_t> wtraits;
		
		// Test translate
		assert(wtraits.translate(L'A') == L'A');
		
		// Test value
		assert(wtraits.value(L'5', 10) == 5);
		assert(wtraits.value(L'a', 16) == 10);
		
		// Test transform
		std::wstring ws = L"test";
		std::wstring wresult = wtraits.transform(ws.data(), ws.data() + ws.size());
		
		std::cout << "wchar_t traits work correctly\n";
	TEST_CASE_END("wchar_t traits")

	// Test 9: char16_t and char32_t traits (basic functionality)
	// Note: std::regex_traits<char16_t> and std::regex_traits<char32_t> require
	// std::ctype<char16_t> and std::ctype<char32_t>. These templates are forward-declared
	// but not fully defined (no implicit instantiation available) in libc++ (used on macOS)
	// and MSVC (used on Windows).
	// Skip this test when USE_STD_FOR_TESTS is enabled and either libc++ or MSVC is detected.
#if defined(USE_STD_FOR_TESTS) && (defined(_LIBCPP_VERSION) || defined(_MSC_VER))
	std::cout << "\n--- char16_t and char32_t traits ---\n";
	std::cout << "SKIPPED: std::ctype<char16_t> and std::ctype<char32_t> not available with this standard library\n";
#else
	TEST_CASE("char16_t and char32_t traits")
		myns::regex_traits<char16_t> u16traits;
		myns::regex_traits<char32_t> u32traits;
		
		// Test translate
		assert(u16traits.translate(u'A') == u'A');
		assert(u32traits.translate(U'A') == U'A');
		
#ifndef USE_STD_FOR_TESTS // It seems missing
		// Test value
		assert(u16traits.value(u'5', 10) == 5);
		assert(u32traits.value(U'5', 10) == 5);

		// Test transform (should fall back to simple copy)
		std::u16string u16s = u"test";
		std::u16string u16result = u16traits.transform(u16s.data(), u16s.data() + u16s.size());
		assert(u16result == u16s);
		
		std::u32string u32s = U"test";
		std::u32string u32result = u32traits.transform(u32s.data(), u32s.data() + u32s.size());
		assert(u32result == u32s);
#endif

		std::cout << "char16_t and char32_t traits work correctly\n";
	TEST_CASE_END("char16_t and char32_t traits")
#endif

	// Test 10: length (existing method)
	TEST_CASE("length")
		// Ensure existing length method still works
		const char* str = "hello";
		assert(myns::regex_traits<char>::length(str) == 5);
		
		const wchar_t* wstr = L"hello";
		assert(myns::regex_traits<wchar_t>::length(wstr) == 5);
		
		std::cout << "length method works correctly\n";
	TEST_CASE_END("length")

	// Test 11: translate_nocase (new method)
	TEST_CASE("translate_nocase")
		myns::regex_traits<char> traits;
		
		// translate_nocase should return lowercase version of the character
		assert(traits.translate_nocase('A') == 'a');
		assert(traits.translate_nocase('Z') == 'z');
		assert(traits.translate_nocase('a') == 'a'); // Already lowercase
		assert(traits.translate_nocase('z') == 'z');
		assert(traits.translate_nocase('5') == '5'); // Non-alphabetic unchanged
		assert(traits.translate_nocase(' ') == ' ');
		
		// Test wchar_t
		myns::regex_traits<wchar_t> wtraits;
		assert(wtraits.translate_nocase(L'A') == L'a');
		assert(wtraits.translate_nocase(L'Z') == L'z');
		assert(wtraits.translate_nocase(L'a') == L'a');
		
		std::cout << "translate_nocase works correctly\n";
	TEST_CASE_END("translate_nocase")

	// Test 12: transform_primary (new method)
	TEST_CASE("transform_primary")
		myns::regex_traits<char> traits;
		
		// transform_primary should return a lowercase collation key
		std::string s1 = "TEST";
		std::string result = traits.transform_primary(s1.data(), s1.data() + s1.size());
		
		// The result should be a valid string (exact result is locale-dependent)
		// For ASCII input, the result should be lowercase
		assert(!result.empty() || s1.empty());
		
		// Test that primary transform produces same result for different cases
		std::string s2 = "test";
		std::string result2 = traits.transform_primary(s2.data(), s2.data() + s2.size());
		// Both should produce the same primary key (case-insensitive)
		assert(result == result2);
		
		// Test wchar_t
		myns::regex_traits<wchar_t> wtraits;
		std::wstring ws1 = L"TEST";
		std::wstring ws2 = L"test";
		std::wstring wresult1 = wtraits.transform_primary(ws1.data(), ws1.data() + ws1.size());
		std::wstring wresult2 = wtraits.transform_primary(ws2.data(), ws2.data() + ws2.size());
		assert(wresult1 == wresult2);
		
		std::cout << "transform_primary works correctly\n";
	TEST_CASE_END("transform_primary")

	// Test 13: lookup_classname (new method)
	TEST_CASE("lookup_classname")
		myns::regex_traits<char> traits;
		
		// Test standard POSIX character class names
		const char* digit = "digit";
		const char* alpha = "alpha";
		const char* alnum = "alnum";
		const char* space = "space";
		const char* upper = "upper";
		const char* lower = "lower";
		
		auto digit_class = traits.lookup_classname(digit, digit + 5);
		auto alpha_class = traits.lookup_classname(alpha, alpha + 5);
		auto alnum_class = traits.lookup_classname(alnum, alnum + 5);
		auto space_class = traits.lookup_classname(space, space + 5);
		auto upper_class = traits.lookup_classname(upper, upper + 5);
		auto lower_class = traits.lookup_classname(lower, lower + 5);
		
		// Classes should be non-zero (valid)
		assert(digit_class != 0);
		assert(alpha_class != 0);
		assert(alnum_class != 0);
		assert(space_class != 0);
		assert(upper_class != 0);
		assert(lower_class != 0);
		
		// Test that isctype works with the returned class
		assert(traits.isctype('5', digit_class) == true);
		assert(traits.isctype('a', digit_class) == false);
		assert(traits.isctype('a', alpha_class) == true);
		assert(traits.isctype('5', alpha_class) == false);
		assert(traits.isctype(' ', space_class) == true);
		assert(traits.isctype('A', upper_class) == true);
		assert(traits.isctype('a', lower_class) == true);
		
		// Test unknown class returns 0
		const char* unknown = "unknown_class";
		auto unknown_class = traits.lookup_classname(unknown, unknown + 13);
		assert(unknown_class == 0);
		
		// Test case-insensitive flag for lower/upper
		auto lower_icase = traits.lookup_classname(lower, lower + 5, true);
		auto upper_icase = traits.lookup_classname(upper, upper + 5, true);
		// With icase, both should return alpha (matching both cases)
		assert(lower_icase == static_cast<int>(std::ctype_base::alpha));
		assert(upper_icase == static_cast<int>(std::ctype_base::alpha));
		
		std::cout << "lookup_classname works correctly\n";
	TEST_CASE_END("lookup_classname")

#ifndef USE_STD_FOR_TESTS // char16_t/char32_t translate_nocase is onigpp-specific
	// Test 14: char16_t and char32_t translate_nocase
	TEST_CASE("char16_t and char32_t translate_nocase")
		myns::regex_traits<char16_t> u16traits;
		myns::regex_traits<char32_t> u32traits;
		
		// Test translate_nocase for char16_t
		assert(u16traits.translate_nocase(u'A') == u'a');
		assert(u16traits.translate_nocase(u'Z') == u'z');
		assert(u16traits.translate_nocase(u'a') == u'a');
		
		// Test translate_nocase for char32_t
		assert(u32traits.translate_nocase(U'A') == U'a');
		assert(u32traits.translate_nocase(U'Z') == U'z');
		assert(u32traits.translate_nocase(U'a') == U'a');
		
		std::cout << "char16_t and char32_t translate_nocase work correctly\n";
	TEST_CASE_END("char16_t and char32_t translate_nocase")
#endif

	std::cout << "\n=== All regex_traits Tests Passed ===\n";
	return 0;
}
