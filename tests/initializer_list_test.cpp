// initializer_list_test.cpp --- Test for std::initializer_list overloads
// Author: katahiromz
// License: BSD-2-Clause

#include "../onigpp.h"
#include "use_std_for_tests.h"
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cassert>

// --- Additional headers for Windows ---
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

// Test helper macro
#define TEST_CASE(msg) std::cout << msg << std::endl

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

	// Oniguruma initialization (no-op for std::regex)
	ONIGPP_TEST_INIT;

	TEST_CASE("Testing regex_token_iterator with std::initializer_list overloads");
	
	// Test 1: Using initializer_list for delimiter (-1)
	{
		std::cout << "  Test 1: initializer_list with {-1} for delimiter\n";
		std::string text = "apple,banana,cherry,date";
		myns::regex delim(",");
		
		std::vector<std::string> tokens;
		// Using initializer_list overload explicitly
		myns::sregex_token_iterator token_it(text.begin(), text.end(), delim, {-1});
		myns::sregex_token_iterator token_end;
		
		for (; token_it != token_end; ++token_it) {
			tokens.push_back(token_it->str());
		}
		
		assert(tokens.size() == 4);
		assert(tokens[0] == "apple");
		assert(tokens[1] == "banana");
		assert(tokens[2] == "cherry");
		assert(tokens[3] == "date");
		std::cout << "    PASSED\n";
	}
	
	// Test 2: Using initializer_list with multiple capture groups
	{
		std::cout << "  Test 2: initializer_list with {0, 1, 2}\n";
		std::string data = "Item1:ValueA,Item2:ValueB";
		myns::regex re("(\\w+):(\\w+)");
		
		std::vector<std::string> parts;
		// Using initializer_list with multiple values
		myns::sregex_token_iterator it(data.begin(), data.end(), re, {0, 1, 2});
		myns::sregex_token_iterator end;
		
		for (; it != end; ++it) {
			parts.push_back(it->str());
		}
		
		// Should get: full match, cap1, cap2 for first match, then same for second match
		assert(parts.size() == 6);
		assert(parts[0] == "Item1:ValueA");  // full match
		assert(parts[1] == "Item1");         // cap 1
		assert(parts[2] == "ValueA");        // cap 2
		assert(parts[3] == "Item2:ValueB");  // full match
		assert(parts[4] == "Item2");         // cap 1
		assert(parts[5] == "ValueB");        // cap 2
		std::cout << "    PASSED\n";
	}
	
	// Test 3: Using single int overload
	{
		std::cout << "  Test 3: single int parameter (2nd capture group)\n";
		std::string data = "Item1:ValueA,Item2:ValueB";
		myns::regex re("(\\w+):(\\w+)");
		
		std::vector<std::string> values;
		// Using single int overload (get only 2nd capture group)
		myns::sregex_token_iterator it(data.begin(), data.end(), re, 2);
		myns::sregex_token_iterator end;
		
		for (; it != end; ++it) {
			values.push_back(it->str());
		}
		
		assert(values.size() == 2);
		assert(values[0] == "ValueA");
		assert(values[1] == "ValueB");
		std::cout << "    PASSED\n";
	}
	
	// Test 4: Using single int overload with -1
	{
		std::cout << "  Test 4: single int parameter (-1 for non-matching parts)\n";
		std::string text = "one:two:three";
		myns::regex delim(":");
		
		std::vector<std::string> tokens;
		// Using single int overload with -1
		myns::sregex_token_iterator it(text.begin(), text.end(), delim, -1);
		myns::sregex_token_iterator end;
		
		for (; it != end; ++it) {
			tokens.push_back(it->str());
		}
		
		assert(tokens.size() == 3);
		assert(tokens[0] == "one");
		assert(tokens[1] == "two");
		assert(tokens[2] == "three");
		std::cout << "    PASSED\n";
	}
	
	// Test 5: Wide string test with initializer_list
	{
		std::cout << "  Test 5: wide string with initializer_list\n";
		std::wstring text = L"alpha,beta,gamma";
		myns::wregex delim(L",");
		
		std::vector<std::wstring> tokens;
		myns::wsregex_token_iterator it(text.begin(), text.end(), delim, {-1});
		myns::wsregex_token_iterator end;
		
		for (; it != end; ++it) {
			tokens.push_back(it->str());
		}
		
		assert(tokens.size() == 3);
		assert(tokens[0] == L"alpha");
		assert(tokens[1] == L"beta");
		assert(tokens[2] == L"gamma");
		std::cout << "    PASSED\n";
	}
	
	std::cout << "\nAll tests passed!\n";
	return 0;
}
