// nosubs_collate_test.cpp --- Test nosubs and collate flags
// License: BSD-2-Clause
#include "../onigpp.h"
#include <iostream>
#include <cassert>

using namespace onigpp;

void test_nosubs() {
	std::cout << "Testing nosubs flag...\n";
	
	// Test 1: nosubs flag should prevent capturing submatches
	{
		std::string pattern = R"((\w+)\s+(\w+))";
		regex re(pattern.c_str(), pattern.length(), regex_constants::nosubs);
		std::string text = "hello world";
		smatch m;
		
		bool found = regex_search(text, m, re);
		assert(found && "Should find a match");
		assert(m.empty() && "Match results should be empty with nosubs");
		std::cout << "  Test 1 passed: nosubs prevents capturing\n";
	}
	
	// Test 2: Without nosubs, submatches should be captured
	{
		std::string pattern = R"((\w+)\s+(\w+))";
		regex re(pattern.c_str(), pattern.length(), regex_constants::normal);
		std::string text = "hello world";
		smatch m;
		
		bool found = regex_search(text, m, re);
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
		regex re(pattern.c_str(), pattern.length(), regex_constants::nosubs);
		std::string text = "123";
		smatch m;
		
		bool matched = regex_match(text, m, re);
		assert(matched && "Should match");
		assert(m.empty() && "Match results should be empty with nosubs");
		std::cout << "  Test 3 passed: nosubs works with regex_match\n";
	}
	
	// Test 4: Without nosubs in regex_match
	{
		std::string pattern = R"((\d+))";
		regex re(pattern.c_str(), pattern.length(), regex_constants::normal);
		std::string text = "123";
		smatch m;
		
		bool matched = regex_match(text, m, re);
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
		regex re(pattern.c_str(), pattern.length(), regex_constants::collate);
		std::string text = "hello";
		smatch m;
		
		bool found = regex_search(text, m, re);
		assert(found && "Should find a match with collate flag");
		std::cout << "  Test 1 passed: collate flag works without errors\n";
	}
	
	// Test 2: Without collate, pattern should still work
	{
		std::string pattern = "[a-z]+";
		regex re(pattern.c_str(), pattern.length(), regex_constants::normal);
		std::string text = "hello";
		smatch m;
		
		bool found = regex_search(text, m, re);
		assert(found && "Should find a match without collate flag");
		std::cout << "  Test 2 passed: pattern works without collate flag\n";
	}
	
	std::cout << "All collate tests passed!\n";
}

void test_optimize() {
	std::cout << "\nTesting optimize flag...\n";
	
	// Test 1: optimize flag should not cause errors (it's a no-op)
	{
		std::string pattern = R"(\d+)";
		regex re(pattern.c_str(), pattern.length(), regex_constants::optimize);
		std::string text = "123";
		smatch m;
		
		bool found = regex_search(text, m, re);
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
		regex re(pattern.c_str(), pattern.length(), 
		         regex_constants::nosubs | regex_constants::collate | regex_constants::optimize);
		std::string text = "hello";
		smatch m;
		
		bool found = regex_search(text, m, re);
		assert(found && "Should find a match");
		assert(m.empty() && "Match results should be empty with nosubs");
		std::cout << "  Test passed: combined flags work together\n";
	}
	
	std::cout << "All combined flag tests passed!\n";
}

int main() {
	auto_init init_onigpp;
	
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
