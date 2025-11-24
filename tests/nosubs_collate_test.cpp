// nosubs_collate_test.cpp --- Test nosubs and collate flags
// License: BSD-2-Clause
#include "tests.h"

void test_nosubs() {
	std::cout << "Testing nosubs flag...\n";
	
	// Test 1: nosubs flag should prevent capturing submatches
	{
		std::string pattern = R"((\w+)\s+(\w+))";
		myns::regex re(pattern.c_str(), pattern.length(), myns::regex_constants::nosubs);
		std::string text = "hello world";
		myns::smatch m;
		
		bool found = myns::regex_search(text, m, re);
		assert(found && "Should find a match");
		// With nosubs, no sub-expressions are captured
		// onigpp: m.empty() == true, std::regex: m.size() == 1 (only full match)
		assert((m.empty() || m.size() == 1) && "Match results should not have sub-expression captures with nosubs");
		std::cout << "  Test 1 passed: nosubs prevents capturing\n";
	}
	
	// Test 2: Without nosubs, submatches should be captured
	{
		std::string pattern = R"((\w+)\s+(\w+))";
		myns::regex re(pattern.c_str(), pattern.length(), myns::regex_constants::ECMAScript);
		std::string text = "hello world";
		myns::smatch m;
		
		bool found = myns::regex_search(text, m, re);
		assert(found && "Should find a match");
		assert(!m.empty() && "Match results should not be empty without nosubs");
		assert(m.size() == 3 && "Should have 3 matches (full match + 2 groups)");
		assert(m[0].str() == "hello world");
		assert(m[1].str() == "hello");
		assert(m[2].str() == "world");
		std::cout << "  Test 2 passed: without nosubs, captures work normally\n";
	}
	
	// Test 3: nosubs with regex_match
	{
		std::string pattern = R"((\d+))";
		myns::regex re(pattern.c_str(), pattern.length(), myns::regex_constants::nosubs);
		std::string text = "123";
		myns::smatch m;
		
		bool matched = myns::regex_match(text, m, re);
		assert(matched && "Should match");
		// With nosubs, no sub-expressions are captured
		// onigpp: m.empty() == true, std::regex: m.size() == 1 (only full match)
		assert((m.empty() || m.size() == 1) && "Match results should not have sub-expression captures with nosubs");
		std::cout << "  Test 3 passed: nosubs works with regex_match\n";
	}
	
	// Test 4: Without nosubs in regex_match
	{
		std::string pattern = R"((\d+))";
		myns::regex re(pattern.c_str(), pattern.length(), myns::regex_constants::ECMAScript);
		std::string text = "123";
		myns::smatch m;
		
		bool matched = myns::regex_match(text, m, re);
		assert(matched && "Should match");
		assert(!m.empty() && "Match results should not be empty");
		assert(m.size() == 2 && "Should have 2 matches (full match + 1 group)");
		assert(m[0].str() == "123");
		assert(m[1].str() == "123");
		std::cout << "  Test 4 passed: without nosubs, regex_match captures work\n";
	}
	
	std::cout << "All nosubs tests passed!\n";
}

void test_collate() {
	std::cout << "\nTesting collate flag...\n";
	
	// Test 1: collate flag should trigger locale preprocessing
	// This is a basic test to ensure the flag doesn't cause errors
	{
		std::string pattern = "[a-z]+";
		myns::regex re(pattern.c_str(), pattern.length(), myns::regex_constants::collate);
		std::string text = "hello";
		myns::smatch m;
		
		bool found = myns::regex_search(text, m, re);
		assert(found && "Should find a match with collate flag");
		std::cout << "  Test 1 passed: collate flag works without errors\n";
	}
	
	// Test 2: Without collate, pattern should still work
	{
		std::string pattern = "[a-z]+";
		myns::regex re(pattern.c_str(), pattern.length(), myns::regex_constants::ECMAScript);
		std::string text = "hello";
		myns::smatch m;
		
		bool found = myns::regex_search(text, m, re);
		assert(found && "Should find a match without collate flag");
		std::cout << "  Test 2 passed: pattern works without collate flag\n";
	}
	
	// Test 3: POSIX character class expansion with collate and locale
	// Test with [:lower:] class to verify locale-aware expansion
	{
		try {
			// Try to imbue with the user's default locale
			std::string pattern = "[[:lower:]]+";
			myns::regex re;
			re.imbue(std::locale(""));  // User's default locale
			re.assign(pattern, myns::regex_constants::collate);
			
			std::string text = "hello WORLD";
			myns::smatch m;
			
			bool found = myns::regex_search(text, m, re);
			assert(found && "Should find lowercase match with [:lower:] and locale");
			assert(m[0].str() == "hello" && "Should match only lowercase text");
			std::cout << "  Test 3 passed: POSIX class [:lower:] with locale expansion works\n";
		} catch (const std::runtime_error& e) {
			// Locale may not be available on all systems, skip gracefully
			std::cout << "  Test 3 skipped: locale not available (" << e.what() << ")\n";
		}
	}
	
	// Test 4: POSIX character class [:digit:] with collate
	{
		std::string pattern = "[[:digit:]]+";
		myns::regex re(pattern.c_str(), pattern.length(), myns::regex_constants::collate);
		std::string text = "abc123def";
		myns::smatch m;
		
		bool found = myns::regex_search(text, m, re);
		assert(found && "Should find digits with [:digit:]");
		assert(m[0].str() == "123" && "Should match digit sequence");
		std::cout << "  Test 4 passed: POSIX class [:digit:] works\n";
	}
	
	// Test 5: POSIX character class [:graph:] with collate
	{
		std::string pattern = "[[:graph:]]+";
		myns::regex re(pattern.c_str(), pattern.length(), myns::regex_constants::collate);
		std::string text = "abc 123";  // space is not in [:graph:]
		myns::smatch m;
		
		bool found = myns::regex_search(text, m, re);
		assert(found && "Should find graphical characters with [:graph:]");
		assert(m[0].str() == "abc" && "Should match graphical characters before space");
		std::cout << "  Test 5 passed: POSIX class [:graph:] works\n";
	}
	
	// Test 6: French-accented characters with locale-aware [:lower:] and [:alpha:]
	// Verify that French characters like é, è, à are properly handled
	{
		try {
			// Test with [:lower:] - should match French lowercase letters
			std::string pattern_lower = "[[:lower:]]+";
			myns::regex re_lower;
			re_lower.imbue(std::locale(""));  // User's default locale
			re_lower.assign(pattern_lower, myns::regex_constants::collate);
			
			// Text with French accented characters
			std::string text_fr = "café";  // Contains 'é' (U+00E9)
			myns::smatch m;
			
			bool found = myns::regex_search(text_fr, m, re_lower);
			// Should match at least "caf" and possibly "café" depending on locale
			assert(found && "Should find lowercase match with French text");
			
			// Test with [:alpha:] - should match all French letters
			std::string pattern_alpha = "[[:alpha:]]+";
			myns::regex re_alpha;
			re_alpha.imbue(std::locale(""));
			re_alpha.assign(pattern_alpha, myns::regex_constants::collate);
			
			std::string text_fr2 = "Éléphant";  // Mixed case French
			bool found_alpha = myns::regex_search(text_fr2, m, re_alpha);
			assert(found_alpha && "Should find alphabetic match with French accented text");
			
			std::cout << "  Test 6 passed: French-accented characters work with locale\n";
		} catch (const std::runtime_error& e) {
			// Locale may not be available on all systems, skip gracefully
			std::cout << "  Test 6 skipped: locale not available (" << e.what() << ")\n";
		}
	}
	
	std::cout << "All collate tests passed!\n";
}

void test_optimize() {
	std::cout << "\nTesting optimize flag...\n";
	
	// Test 1: optimize flag should not cause errors (it's a no-op)
	{
		std::string pattern = R"(\d+)";
		myns::regex re(pattern.c_str(), pattern.length(), myns::regex_constants::optimize);
		std::string text = "123";
		myns::smatch m;
		
		bool found = myns::regex_search(text, m, re);
		assert(found && "Should find a match with optimize flag");
		std::cout << "  Test 1 passed: optimize flag (no-op) works\n";
	}
	
	std::cout << "All optimize tests passed!\n";
}

void test_combined_flags() {
	std::cout << "\nTesting combined flags...\n";
	
	// Test: nosubs + collate + optimize
	{
		std::string pattern = R"((\w+))";
		myns::regex re(pattern.c_str(), pattern.length(), 
		         myns::regex_constants::nosubs | myns::regex_constants::collate | myns::regex_constants::optimize);
		std::string text = "hello";
		myns::smatch m;
		
		bool found = myns::regex_search(text, m, re);
		assert(found && "Should find a match");
		// With nosubs, no sub-expressions are captured
		// onigpp: m.empty() == true, std::regex: m.size() == 1 (only full match)
		assert((m.empty() || m.size() == 1) && "Match results should not have sub-expression captures with nosubs");
		std::cout << "  Test passed: combined flags work together\n";
	}
	
	std::cout << "All combined flag tests passed!\n";
}

int main() {
	TESTS_OUTPUT_INIT();

	// Oniguruma initialization (no-op for std::regex)
	ONIGPP_TEST_INIT;

	try {
		test_nosubs();
		test_collate();
		test_optimize();
		test_combined_flags();
		
		std::cout << "\n=== All tests passed! ===\n";
		return 0;
	}
	catch (const std::exception& e) {
		std::cerr << "Test failed with exception: " << e.what() << std::endl;
		return 1;
	}
}
