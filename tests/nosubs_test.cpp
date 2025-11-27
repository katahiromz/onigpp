// nosubs_test.cpp --- Test match-time nosubs flag behavior
// License: BSD-2-Clause
#include "tests.h"
#include <regex>

int main() {
	TESTS_OUTPUT_INIT();
	
	// Oniguruma initialization (no-op for std::regex)
	ONIGPP_TEST_INIT;
	
	try {
		std::cout << "Testing nosubs flag behavior...\n\n";
		
		// Test 1: Constructor nosubs flag with regex_search
		{
			std::cout << "Test 1: Constructor nosubs flag with regex_search\n";
			std::string pattern = "(hello)";
			std::string input = "hello";
			
			// Test with std::regex
			std::regex std_re(pattern, std::regex_constants::nosubs);
			std::smatch std_m;
			bool std_found = std::regex_search(input, std_m, std_re);
			
			// Test with onigpp
			rex::regex onigpp_re(pattern.c_str(), pattern.length(), rex::regex_constants::nosubs);
			rex::smatch onigpp_m;
			bool onigpp_found = rex::regex_search(input, onigpp_m, onigpp_re);
			
			// Verify both matched
			assert(std_found && onigpp_found && "Both should find a match");
			
			// Verify size consistency
			std::cout << "  std::regex size: " << std_m.size() << "\n";
			std::cout << "  onigpp size: " << onigpp_m.size() << "\n";
			assert(std_m.size() == onigpp_m.size() && "Match results size should match");
			
			// With nosubs, std::regex returns size 1 (full match only, no submatches)
			assert(std_m.size() == 1 && "std::regex should have size 1 with nosubs");
			assert(onigpp_m.size() == 1 && "onigpp should have size 1 with nosubs");
			
			// Verify the full match content
			assert(std_m[0].str() == "hello" && "std::regex full match should be 'hello'");
			assert(onigpp_m[0].str() == "hello" && "onigpp full match should be 'hello'");
			
			std::cout << "  ✅ PASS\n\n";
		}
		
		// Test 2: Without nosubs flag (normal behavior)
		{
			std::cout << "Test 2: Without nosubs flag (normal behavior)\n";
			std::string pattern = "(hello)";
			std::string input = "hello";
			
			// Test with std::regex
			std::regex std_re(pattern);
			std::smatch std_m;
			bool std_found = std::regex_search(input, std_m, std_re);
			
			// Test with onigpp - use explicit flag to avoid ambiguity
			rex::regex onigpp_re(pattern.c_str(), pattern.length(), rex::regex_constants::ECMAScript);
			rex::smatch onigpp_m;
			bool onigpp_found = rex::regex_search(input, onigpp_m, onigpp_re);
			
			// Verify both matched
			assert(std_found && onigpp_found && "Both should find a match");
			
			// Verify size consistency
			std::cout << "  std::regex size: " << std_m.size() << "\n";
			std::cout << "  onigpp size: " << onigpp_m.size() << "\n";
			assert(std_m.size() == onigpp_m.size() && "Match results size should match");
			
			// Without nosubs, should have full match + 1 submatch
			assert(std_m.size() == 2 && "std::regex should have size 2 without nosubs");
			assert(onigpp_m.size() == 2 && "onigpp should have size 2 without nosubs");
			
			// Verify content
			assert(std_m[0].str() == "hello" && "std::regex full match should be 'hello'");
			assert(std_m[1].str() == "hello" && "std::regex submatch[1] should be 'hello'");
			assert(onigpp_m[0].str() == "hello" && "onigpp full match should be 'hello'");
			assert(onigpp_m[1].str() == "hello" && "onigpp submatch[1] should be 'hello'");
			
			std::cout << "  ✅ PASS\n\n";
		}
		
		// Test 3: Constructor nosubs flag with regex_match
		{
			std::cout << "Test 3: Constructor nosubs flag with regex_match\n";
			std::string pattern = "(\\d+)";
			std::string input = "123";
			
			// Test with std::regex
			std::regex std_re(pattern, std::regex_constants::nosubs);
			std::smatch std_m;
			bool std_matched = std::regex_match(input, std_m, std_re);
			
			// Test with onigpp
			rex::regex onigpp_re(pattern.c_str(), pattern.length(), rex::regex_constants::nosubs);
			rex::smatch onigpp_m;
			bool onigpp_matched = rex::regex_match(input, onigpp_m, onigpp_re);
			
			// Verify both matched
			assert(std_matched && onigpp_matched && "Both should match");
			
			// Verify size consistency
			std::cout << "  std::regex size: " << std_m.size() << "\n";
			std::cout << "  onigpp size: " << onigpp_m.size() << "\n";
			assert(std_m.size() == onigpp_m.size() && "Match results size should match");
			
			// With nosubs, should have size 1 (full match only)
			assert(std_m.size() == 1 && "std::regex should have size 1 with nosubs");
			assert(onigpp_m.size() == 1 && "onigpp should have size 1 with nosubs");
			
			// Verify content
			assert(std_m[0].str() == "123" && "std::regex full match should be '123'");
			assert(onigpp_m[0].str() == "123" && "onigpp full match should be '123'");
			
			std::cout << "  ✅ PASS\n\n";
		}
		
		// Test 4: Multiple capture groups with nosubs
		{
			std::cout << "Test 4: Multiple capture groups with nosubs\n";
			std::string pattern = "(\\w+)\\s+(\\w+)";
			std::string input = "hello world";
			
			// Test with std::regex
			std::regex std_re(pattern, std::regex_constants::nosubs);
			std::smatch std_m;
			bool std_found = std::regex_search(input, std_m, std_re);
			
			// Test with onigpp
			rex::regex onigpp_re(pattern.c_str(), pattern.length(), rex::regex_constants::nosubs);
			rex::smatch onigpp_m;
			bool onigpp_found = rex::regex_search(input, onigpp_m, onigpp_re);
			
			// Verify both matched
			assert(std_found && onigpp_found && "Both should find a match");
			
			// Verify size consistency
			std::cout << "  std::regex size: " << std_m.size() << "\n";
			std::cout << "  onigpp size: " << onigpp_m.size() << "\n";
			assert(std_m.size() == onigpp_m.size() && "Match results size should match");
			
			// With nosubs, should have size 1 (full match only, no submatches)
			assert(std_m.size() == 1 && "std::regex should have size 1 with nosubs");
			assert(onigpp_m.size() == 1 && "onigpp should have size 1 with nosubs");
			
			// Verify content
			assert(std_m[0].str() == "hello world" && "std::regex full match should be 'hello world'");
			assert(onigpp_m[0].str() == "hello world" && "onigpp full match should be 'hello world'");
			
			std::cout << "  ✅ PASS\n\n";
		}
		
		// Test 5: Without nosubs, multiple capture groups
		{
			std::cout << "Test 5: Without nosubs, multiple capture groups\n";
			std::string pattern = "(\\w+)\\s+(\\w+)";
			std::string input = "hello world";
			
			// Test with std::regex
			std::regex std_re(pattern);
			std::smatch std_m;
			bool std_found = std::regex_search(input, std_m, std_re);
			
			// Test with onigpp - use explicit flag to avoid ambiguity
			rex::regex onigpp_re(pattern.c_str(), pattern.length(), rex::regex_constants::ECMAScript);
			rex::smatch onigpp_m;
			bool onigpp_found = rex::regex_search(input, onigpp_m, onigpp_re);
			
			// Verify both matched
			assert(std_found && onigpp_found && "Both should find a match");
			
			// Verify size consistency
			std::cout << "  std::regex size: " << std_m.size() << "\n";
			std::cout << "  onigpp size: " << onigpp_m.size() << "\n";
			assert(std_m.size() == onigpp_m.size() && "Match results size should match");
			
			// Without nosubs, should have full match + 2 submatches
			assert(std_m.size() == 3 && "std::regex should have size 3 without nosubs");
			assert(onigpp_m.size() == 3 && "onigpp should have size 3 without nosubs");
			
			// Verify content
			assert(std_m[0].str() == "hello world" && "std::regex full match should be 'hello world'");
			assert(std_m[1].str() == "hello" && "std::regex submatch[1] should be 'hello'");
			assert(std_m[2].str() == "world" && "std::regex submatch[2] should be 'world'");
			assert(onigpp_m[0].str() == "hello world" && "onigpp full match should be 'hello world'");
			assert(onigpp_m[1].str() == "hello" && "onigpp submatch[1] should be 'hello'");
			assert(onigpp_m[2].str() == "world" && "onigpp submatch[2] should be 'world'");
			
			std::cout << "  ✅ PASS\n\n";
		}
		
		std::cout << "=== All nosubs tests passed! ===\n";
		return 0;
	}
	catch (const std::exception& e) {
		std::cerr << "Test failed with exception: " << e.what() << std::endl;
		return 1;
	}
}
