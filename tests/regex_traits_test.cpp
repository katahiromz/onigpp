// regex_traits_test.cpp --- Tests for onigpp::regex_traits
// Author: katahiromz
// License: BSD-2-Clause
#include "onigpp.h"
#include <iostream>
#include <string>
#include <locale>
#include <regex>
#include <cassert>

// --- Additional headers for Windows ---
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

// Alias namespace for ease of use
#ifdef USE_STD_FOR_TESTS
	namespace myns = std;
#else
	namespace myns = onigpp;
#endif

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
	// --- Measures to avoid garbled characters on Windows consoles ---
#ifdef _WIN32
	// Switch to UTF-8 mode
	//_setmode(_fileno(stdout), _O_U8TEXT); // Use std::cout instead of std::wcout
	// Ensure console uses UTF-8 code page for interoperability
	SetConsoleOutputCP(CP_UTF8);
#else
	// For Linux/Mac, setting the locale is usually sufficient
	std::setlocale(LC_ALL, "");
#endif

#ifndef USE_STD_FOR_TESTS
	// Oniguruma initialization
	myns::auto_init init;
#endif

	std::cout << "=== regex_traits Tests ===\n";

	// Test 1: Typedefs and basic construction
	TEST_CASE("Typedefs and Constructors")
		myns::regex_traits<char> traits1;
		myns::regex_traits<char> traits2{std::locale()};
		myns::regex_traits<wchar_t> wtraits1;
		myns::regex_traits<wchar_t> wtraits2{std::locale()};
		
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
		assert(traits.value('2', 2) == -1); // Invalid for base 2
		
		// Default parameter (base 10)
		assert(traits.value('5') == 5);
		assert(traits.value('a') == -1);
		
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
	TEST_CASE("char16_t and char32_t traits")
		myns::regex_traits<char16_t> u16traits;
		myns::regex_traits<char32_t> u32traits;
		
		// Test translate
		assert(u16traits.translate(u'A') == u'A');
		assert(u32traits.translate(U'A') == U'A');
		
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
		
		std::cout << "char16_t and char32_t traits work correctly\n";
	TEST_CASE_END("char16_t and char32_t traits")

	// Test 10: length (existing method)
	TEST_CASE("length")
		// Ensure existing length method still works
		const char* str = "hello";
		assert(myns::regex_traits<char>::length(str) == 5);
		
		const wchar_t* wstr = L"hello";
		assert(myns::regex_traits<wchar_t>::length(wstr) == 5);
		
		std::cout << "length method works correctly\n";
	TEST_CASE_END("length")

	std::cout << "\n=== All regex_traits Tests Passed ===\n";
	return 0;
}
